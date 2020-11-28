// censor_string.c
// Ashish D'Souza

#include "censor_string.h"
#include "clone_commits.c"


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


