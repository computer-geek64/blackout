// main.c
// Ashish D'Souza

#define TRUE (1)
#define FALSE (0)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <git2.h>
#include "structs.c"
#include "main.h"
#include "censor_string.c"


int main(int argc, char **argv) {
    // Check if argv has string
    if(argc < 2) {
        printf("No repository path specified\n");
        return 1;
    }

    const char *repositoryPathArgument = *(argv + 1);

    // Initialize libgit2
    git_libgit2_init();

    // Find git repository root path
    char *repositoryRoot = NULL;
    int errorCode = findRepositoryRoot(&repositoryRoot, repositoryPathArgument);
    if(errorCode != 0) {
        git_libgit2_shutdown();
        return errorCode;
    }

    // Open Git repository
    git_repository *repository;
    errorCode = git_repository_open(&repository, repositoryRoot);
    if(errorCode < 0) {
        handleGitError(errorCode);

        // Cleanup
        git_repository_free(repository);
        free(repositoryRoot);
        git_libgit2_shutdown();
        return errorCode;
    }

    //errorCode = censorString("password", "********", repository);
    ReferenceList referenceList;
    errorCode = getDirectReferences(&referenceList, repository);
    if(errorCode != 0) {
        git_repository_free(repository);
        free(repositoryRoot);
        git_libgit2_shutdown();
        return errorCode;
    }

    printf("Cloning commits now...\n");

    const char* targetString = "sudouser512@gmail.com";
    errorCode = cloneCommitsFromHead(repository, &isStringInCommitBlobs, &commitCallback, (void*) targetString);
    if(errorCode) {
        // Cleanup
        git_repository_free(repository);
        free(repositoryRoot);
        git_libgit2_shutdown();
        return errorCode;
    }

    // Cleanup
    git_repository_free(repository);
    free(repositoryRoot);
    git_libgit2_shutdown();
    return 0;
}

int findRepositoryRoot(char **repositoryRoot, const char *repositoryPathArgument) {
    // Allocate memory for repositoryRootBuffer
    git_buf *repositoryRootBuffer = malloc(sizeof(repositoryRootBuffer));
    if(repositoryRootBuffer == NULL) {
        handleMemoryAllocationError();

        return 1;
    }

    // Find repository root path
    int errorCode = git_repository_discover(repositoryRootBuffer, repositoryPathArgument, TRUE, NULL);
    if(errorCode < 0) {
        printf("fatal: not a git repository (or any of the parent directories): .git\n");

        // Cleanup
        git_buf_dispose(repositoryRootBuffer);
        free(repositoryRootBuffer);
        return errorCode;
    }

    // Copy repository root to string out pointer
    *repositoryRoot = malloc((strlen(repositoryRootBuffer->ptr) + 1) * sizeof(char));
    if(*repositoryRoot == NULL) {
        handleMemoryAllocationError();

        // Cleanup
        git_buf_dispose(repositoryRootBuffer);
        free(repositoryRootBuffer);
        return 1;
    }

    strcpy(*repositoryRoot, repositoryRootBuffer->ptr);
    git_buf_dispose(repositoryRootBuffer);
    free(repositoryRootBuffer);
    return 0;
}

void handleGitError(int errorCode) {
    const git_error *e = git_error_last();
    printf("Error Code %d: %s\n", errorCode, e->message);
}

void handleMemoryAllocationError() {
    printf("Memory allocation failure\n");
}

int replaceStringInCommitBlobs(git_commit **commit, CommitList commitList, git_repository *repository, void *payload) {
    // Rewrites commit message
    git_oid newCommitOid;
    git_tree *oldCommitTree;
    int errorCode = git_commit_tree(&oldCommitTree, *commit);
    if(errorCode < 0) {
        handleGitError(errorCode);

        // Cleanup
        git_tree_free(oldCommitTree);
        return errorCode;
    }

    const char* message = "This commit message has been overwritten.";
    git_commit_create(&newCommitOid, repository, NULL, git_commit_author(*commit), git_commit_committer(*commit), git_commit_message_encoding(*commit), message, oldCommitTree, commitList.size, commitList.size > 0 ? commitList.list : NULL);
    git_commit_lookup(commit, repository, &newCommitOid);

    return 0;

    /*// Replaces all instances of a string in commit blobs
    // Get target string from payload
    const char* targetString = (const char*) payload;

    */
}

int isStringInCommitBlobs(int *commitUpdateRequired, git_commit *commit, git_repository *repository, void *payload) {
    // Get target string from payload
    const char* targetString = (const char*) payload;

    // Walk through the commit tree
    git_tree *tree;
    int errorCode = git_commit_tree(&tree, commit);
    if(errorCode < 0) {
        handleGitError(errorCode);

        // Cleanup
        git_tree_free(tree);
        return errorCode;
    }

    // Create payload for tree walk
    typedef struct TreeWalkPayload {
        const char *targetString;
        git_repository *repository;
        int commitUpdateRequired;
    } TreeWalkPayload;
    TreeWalkPayload treeWalkPayload = {targetString, repository, FALSE};

    errorCode = git_tree_walk(tree, GIT_TREEWALK_PRE, treeWalkCallback, (void*) &treeWalkPayload);
    if(errorCode < 0) {
        handleGitError(errorCode);

        // Cleanup
        git_tree_free(tree);
        return errorCode;
    }

    *commitUpdateRequired = treeWalkPayload.commitUpdateRequired;

    // Cleanup
    git_tree_free(tree);
    return 0;
}

int treeWalkCallback(const char *root, const git_tree_entry *entry, void *payload) {
    // Get values from payload
    typedef struct TreeWalkPayload {
        const char *targetString;
        struct git_repository *repository;
        int commitUpdateRequired;
    } TreeWalkPayload;
    TreeWalkPayload *treeWalkPayload = (TreeWalkPayload*) payload;

    // Skip node if it is not a blob (must be directory)
    if(git_tree_entry_type(entry) != GIT_OBJECT_BLOB) {
        return 0;
    }

    // Check if file is binary data
    git_blob *blob;
    git_blob_lookup(&blob, treeWalkPayload->repository, git_tree_entry_id(entry));
    if(git_blob_is_binary(blob)) {
        // File is binary
    }
    else {
        // File is non-binary (text)
        const char* content = (const char*) git_blob_rawcontent(blob);
        if(strstr(content, treeWalkPayload->targetString) != NULL) {
            treeWalkPayload->commitUpdateRequired = TRUE;
        }
    }

    return 0;
}
