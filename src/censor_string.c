// censor_string.c
// Ashish D'Souza

#include "censor_string.h"
#include <git2.h>


typedef struct ReferenceList {
    Reference* list;
    size_t size;
} referenceList;

typedef struct Reference {
    char *name;
    char *commit;
    git_oid oid;
} Reference;

int censorString(const char *replace, const char *replacement, git_repository *repository) {
    // Walk through revisions
    git_revwalk *walker = NULL;
    int errorCode = git_revwalk_new(&walker, repository);
    if(errorCode < 0) {
        handleGitError(errorCode);

        // Cleanup
        git_revwalk_free(walker);
        return errorCode;
    }

    errorCode = git_revwalk_push_head(walker);
    if(errorCode < 0) {
        handleGitError(errorCode);

        // Cleanup
        git_revwalk_free(walker);
        return errorCode;
    }

    git_oid oid;
    while(git_revwalk_next(&oid, walker) == 0) {
        git_commit *commit = NULL;
        errorCode = git_commit_lookup(&commit, repository, &oid);
        if(errorCode < 0) {
            handleGitError(errorCode);

            // Cleanup
            git_commit_free(commit);
            git_revwalk_free(walker);
            return errorCode;
        }

        printf("%s: %s\n", git_oid_tostr_s(&oid), git_commit_summary(commit));

        /*if(strcmp(git_oid_tostr_s(&oid), "ec2a45bc6bc6f65bd657034afd69cadb2c94ba8d") == 0) {
            git_oid commitOid;
            git_commit_amend(&commitOid, commit, "refs/heads/master", NULL, NULL, NULL, "I updated readme.md", NULL);
            git_commit *newCommit = NULL;
            git_commit_lookup(&newCommit, repository, &commitOid);
            printf("New: %s\n", git_oid_tostr_s(&oid));
            git_commit_free(newCommit);
        }*/

        // Walk through the commit tree
        /*git_tree *tree = NULL;
        errorCode = git_commit_tree(&tree, commit);
        if(errorCode < 0) {
            handleGitError(errorCode);

            // Cleanup
            git_tree_free(tree);
            git_commit_free(commit);
            git_revwalk_free(walker);
            return errorCode;
        }
        void *payload = NULL;
        errorCode = git_tree_walk(tree, GIT_TREEWALK_PRE, tree_walk_cb, payload);

        // Cleanup
        git_tree_free(tree);*/
        git_commit_free(commit);
    }

    // Print refs
    ReferenceList referenceList;
    getDirectReferences(&referenceList, repository);
    printf("%s: %s\n", referenceList.list->name, referenceList.list->commit);
    printf("%s\n", git_oid_tostr_s(&referenceList.list->oid));
    destroyReferenceList(referenceList);

    git_revwalk_free(walker);
}

/*int cloneCommits(int (*commitCallbackFunction)(), git_repository *repository) {
    // Walk through revisions
    git_revwalk *walker = NULL;
    int errorCode = git_revwalk_new(&walker, repository);
    if(errorCode < 0) {
        handleGitError(errorCode);

        // Cleanup
        git_revwalk_free(walker);
        return errorCode;
    }

    errorCode = git_revwalk_push_head(walker);
    if(errorCode < 0) {
        handleGitError(errorCode);

        // Cleanup
        git_revwalk_free(walker);
        return errorCode;
    }

    git_oid commitOid;
    while(git_revwalk_next(&commitOid, walker) == 0) {
        git_commit *commit = NULL;
        errorCode = git_commit_lookup(&commit, repository, &commitOid);
        if(errorCode < 0) {
            handleGitError(errorCode);

            // Cleanup
            git_commit_free(commit);
            git_revwalk_free(walker);
            return errorCode;
        }

        (*commitCallbackFunction)(*commit);
    }
}*/

/*int commitCallback(git_commit commit) {
    (void)(commit);
    return 0;
}*/

int getDirectReferences(ReferenceList *referenceList, git_repository *repository) {
    git_strarray referenceNameList;
    int errorCode = git_reference_list(&referenceNameList, repository);
    if(errorCode < 0) {
        handleGitError(errorCode);

        // Cleanup
        git_strarray_free(&referenceNameList);
        return errorCode;
    }

    referenceList->list = (Reference*) malloc(referenceNameList.count * sizeof(Reference));
    if(referenceList->list == NULL) {
        handleMemoryAllocationError();

        // Cleanup
        git_strarray_free(&referenceNameList);
        return 1;
    }

    referenceList->size = 0;
    for(unsigned int i = 0; i < referenceNameList.count; i++) {
        git_reference *commitReference;
        errorCode = git_reference_lookup(&commitReference, repository, referenceNameList.strings[i]);
        if(errorCode < 0) {
            handleGitError(errorCode);

            // Cleanup
            git_reference_free(commitReference);
            destroyReferenceList(*referenceList);
            git_strarray_free(&referenceNameList);
            return errorCode;
        }

        if(git_reference_type(commitReference) == GIT_REFERENCE_DIRECT) {
            // Direct reference
            const git_oid *commitOid = git_reference_target(commitReference);

            referenceList->list[referenceList->size].name = malloc((strlen(referenceNameList.strings[i]) + 1) * sizeof(char));
            if(referenceList->list[referenceList->size].name == NULL) {
                handleMemoryAllocationError();

                // Cleanup
                git_reference_free(commitReference);
                destroyReferenceList(*referenceList);
                git_strarray_free(&referenceNameList);
                return 1;
            }
            strcpy(referenceList->list[referenceList->size].name, referenceNameList.strings[i]);

            const char *commitHash = git_oid_tostr_s(commitOid);
            referenceList->list[referenceList->size].commit = malloc((strlen(commitHash) + 1) * sizeof(char));
            if(referenceList->list[referenceList->size].commit == NULL) {
                handleMemoryAllocationError();

                // Cleanup
                git_reference_free(commitReference);
                destroyReferenceList(*referenceList);
                git_strarray_free(&referenceNameList);
                return 1;
            }
            strcpy(referenceList->list[referenceList->size].commit, commitHash);

            referenceList->list[referenceList->size].oid = *commitOid;

            printf("%s: %s\n", referenceList->list[referenceList->size].name, referenceList->list[referenceList->size].commit);
            referenceList->size++;
        }

        git_reference_free(commitReference);
    }

    Reference* referenceListRealloc = (Reference*) realloc(referenceList->list, referenceList->size * sizeof(Reference));
    if(referenceListRealloc == NULL) {
        handleMemoryAllocationError();

        // Cleanup
        destroyReferenceList(*referenceList);
        git_strarray_free(&referenceNameList);
        return 1;
    }
    referenceList->list = referenceListRealloc;

    git_strarray_free(&referenceNameList);
}

void destroyReferenceList(ReferenceList referenceList) {
    for(unsigned int i = 0; i < referenceList.size; i++) {
        free(referenceList.list[i].name);
        free(referenceList.list[i].commit);
    }
    free(referenceList.list);
}