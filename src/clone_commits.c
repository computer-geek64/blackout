// clone_commits.c
// Ashish D'Souza

#include "clone_commits.h"
#include <git2.h>


int cloneCommits(git_repository *repository, int (*isCommitUpdateRequiredFunction)(int*, git_commit*, git_repository*, void*), int (*commitCallbackFunction)(git_commit**, CommitList, git_repository*, void*), void *payload) {
    (void)(repository);
    (void)(isCommitUpdateRequiredFunction);
    (void)(commitCallbackFunction);
    (void)(payload);
    return 0;
}

int cloneCommitsFromHead(git_repository *repository, int (*isCommitUpdateRequiredFunction)(int*, git_commit*, git_repository*, void*), int (*commitCallbackFunction)(git_commit**, CommitList, git_repository*, void*), void *payload) {
    // Get HEAD commit
    git_oid oid;
    int errorCode = git_reference_name_to_id(&oid, repository, "HEAD");
    if(errorCode < 0) {
        handleGitError(errorCode);

        // Cleanup
        return errorCode;
    }
    git_commit *head;
    errorCode = git_commit_lookup(&head, repository, &oid);
    if(errorCode < 0) {
        handleGitError(errorCode);

        // Cleanup
        git_commit_free(head);
        return errorCode;
    }

    // Get references
    ReferenceList referenceList;
    errorCode = getDirectReferences(&referenceList, repository);
    if(errorCode != 0) {
        // Cleanup
        destroyReferenceList(referenceList);
        git_commit_free(head);
        return errorCode;
    }

    // Clone commits from HEAD
    errorCode = cloneCommitsFromHeadHelper(&head, referenceList, repository, isCommitUpdateRequiredFunction, commitCallbackFunction, payload);
    if(errorCode != 0) {
        // Cleanup
        destroyReferenceList(referenceList);
        git_commit_free(head);
        return errorCode;
    }

    git_commit_free(head);

    // Update working tree and index to current head commit
    git_checkout_options checkoutOptions;
    git_checkout_options_init(&checkoutOptions, GIT_CHECKOUT_OPTIONS_VERSION);
    checkoutOptions.checkout_strategy = GIT_CHECKOUT_FORCE;
    git_checkout_head(repository, &checkoutOptions);
    return 0;
}

// Recursive commit Directed Acyclic Graph (DAG) parser and cloner
int cloneCommitsFromHeadHelper(git_commit **commit, ReferenceList referenceList, git_repository *repository, int (*isCommitUpdateRequiredFunction)(int*, git_commit*, git_repository*, void*), int (*commitCallbackFunction)(git_commit**, CommitList, git_repository*, void*), void *payload) {
    /* Get parents of current commit
     * Call cloneCommitsFromHeadHelper recursively on each parent
     * Clone current commit if current commit meets expectations or if parents were cloned (using parent commits)
     */

    // Get parents of current commit
    const size_t parentCount = git_commit_parentcount(*commit);

    CommitList commitList = {malloc(parentCount * sizeof(git_commit*)), 0};
    if(commitList.list == NULL && parentCount > 0) {
        handleMemoryAllocationError();

        // Cleanup
        return 1;
    }

    int commitUpdateRequired;
    int errorCode = (*isCommitUpdateRequiredFunction)(&commitUpdateRequired, *commit, repository, payload);
    if(errorCode != 0) {
        // Cleanup
        destroyCommitList(commitList);
        return errorCode;
    }
    int parentCommitUpdated = FALSE;

    for(; commitList.size < parentCount; commitList.size++) {
        git_commit *parentCommit;
        errorCode = git_commit_parent(&parentCommit, *commit, commitList.size);
        if(errorCode < 0) {
            handleGitError(errorCode);

            // Cleanup
            git_commit_free(parentCommit);
            destroyCommitList(commitList);
            return errorCode;
        }

        // Recursively get new parents
        git_commit *oldParentCommit = parentCommit;
        errorCode = cloneCommitsFromHeadHelper(&parentCommit, referenceList, repository, isCommitUpdateRequiredFunction, commitCallbackFunction, payload);
        if(errorCode != 0) {
            // Cleanup
            git_commit_free(parentCommit);
            git_commit_free(oldParentCommit);
            destroyCommitList(commitList);
            return errorCode;
        }

        if(oldParentCommit != parentCommit) {
            parentCommitUpdated = TRUE;
            git_commit_free(oldParentCommit);
        }

        commitList.list[commitList.size] = parentCommit;
    }

    // Check if commit needs to be cloned
    if(commitUpdateRequired) {
        // Update current commit with new data
        git_commit *oldCommit = *commit;
        errorCode = (*commitCallbackFunction)(commit, commitList, repository, payload);
        if(errorCode != 0) {
            // Cleanup
            if(oldCommit != *commit) {
                git_commit_free(*commit);
                *commit = oldCommit;
            }
            destroyCommitList(commitList);
            return errorCode;
        }

        // Update any refs
        updateCommitRefs(oldCommit, *commit, referenceList, repository);
        git_commit_free(oldCommit);
    }
    else if(parentCommitUpdated) {
        // Copy current commit, only changing parent list
        git_commit *oldCommit = *commit;
        git_oid newCommitOid;
        git_tree *oldCommitTree;
        errorCode = git_commit_tree(&oldCommitTree, *commit);
        if(errorCode < 0) {
            handleGitError(errorCode);

            // Cleanup
            git_tree_free(oldCommitTree);
            destroyCommitList(commitList);
            return errorCode;
        }
        git_commit_create(&newCommitOid, repository, NULL, git_commit_author(*commit), git_commit_committer(*commit), git_commit_message_encoding(*commit), git_commit_message(*commit), oldCommitTree, parentCount, commitList.list);
        git_commit_lookup(commit, repository, &newCommitOid);

        // Update any refs
        updateCommitRefs(oldCommit, *commit, referenceList, repository);
        git_commit_free(oldCommit);
    }

    return 0;
}

int updateCommitRefs(git_commit *oldCommit, git_commit *newCommit, ReferenceList referenceList, git_repository *repository) {
    for(unsigned int i = 0; i < referenceList.size; i++) {
        if(strcmp(referenceList.list[i].commit, git_oid_tostr_s(git_commit_id(oldCommit))) == 0) {
            git_reference *newReference;
            git_reference_create(&newReference, repository, referenceList.list[i].name, git_commit_id(newCommit), TRUE, NULL);
            git_reference_free(newReference);
        }
    }

    return 0;
}

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
