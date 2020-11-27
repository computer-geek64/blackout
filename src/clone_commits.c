// clone_commits.c
// Ashish D'Souza

#include "clone_commits.h"
#include <git2.h>


typedef struct CommitList {
    git_commit **list;
    size_t size;
} CommitList;

int cloneCommits(git_repository *repository, int *commitCallbackFunction(int*, git_commit*)) {
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

        int bool = TRUE;
        *commitCallbackFunction(&bool, commit);
    }
}

int cloneCommitsFromHead(git_repository *repository, int *commitCallbackFunction(int*, git_commit*)) {
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

    // Clone commits from HEAD
    errorCode = cloneCommitsFromHeadHelper(&head, commitCallbackFunction);
    if(errorCode != 0) {
        // Cleanup
        git_commit_free(head);
        return errorCode;
    }

    // Update HEAD ref to point to head commit
    return 0;
}

int cloneCommitsFromHeadHelper(git_commit **commit, int *commitCallbackFunction(int*, git_commit*)) {
    // get parents of current commit
    // call cloneCommitsFromHeadHelper recursively on each parent
    // check if parents needed cloning
    // clone current commit if current commit meets expectations or if parents were cloned (using parent commits)

    // Get parents of current commit
    CommitList commitList;
    const size_t parentCount = git_commit_parentcount(*commit);
    commitList.list = (git_commit**) malloc(parentCount * sizeof(git_commit*));
    commitList.size = 0;

    int commitNeedsUpdate = FALSE; // Callback function goes here
    int parentCommitUpdated = FALSE;

    for(; commitList.size < parentCount; commitList.size++) {
        git_commit *parentCommit;
        int errorCode = git_commit_parent(&parentCommit, *commit, commitList.size);
        if(errorCode < 0) {
            handleGitError(errorCode);

            // Cleanup
            git_commit_free(parentCommit);
            destroyCommitList(commitList);
            return errorCode;
        }

        git_commit *oldParentCommit = parentCommit;
        errorCode = cloneCommitsFromHeadHelper(&parentCommit, commitCallbackFunction);
        if(errorCode != 0) {
            // Cleanup
            git_commit_free(parentCommit);
            destroyCommitList(commitList);
            return errorCode;
        }

        if(oldParentCommit != parentCommit) {
            parentCommitUpdated = TRUE;
            git_commit_free(oldParentCommit);
        }

        commitList.list[commitList.size] = parentCommit;
    }

    if(commitNeedsUpdate) {
        // Update current commit with new data
        // Update any refs as well
    }
    else if(parentCommitUpdated) {
        // Copy current commit, just create a duplicate with ParentList
        // Update any refs as well
    }

    return 0;
}

int commitCallback(int *condition, git_commit *commit) {
    *condition = TRUE;
    (void)(commit);
    return 0;
}

void destroyCommitList(CommitList commitList) {
    for(unsigned int i = 0; i < commitList.size; i++) {
        git_commit_free(commitList.list[i]);
    }
    free(commitList.list);
}