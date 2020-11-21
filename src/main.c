// main.c
// Ashish D'Souza

#define TRUE (1)
#define FALSE (0)

#include <stdio.h>
#include <string.h>
#include <git2.h>
#include "main.h"
#include "censor_string.c"
#include "undo_last_commit.c"


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
    git_repository *repository = NULL;
    errorCode = git_repository_open(&repository, repositoryRoot);
    if(errorCode < 0) {
        handleError(errorCode);

        // Cleanup
        git_repository_free(repository);
        free(repositoryRoot);
        git_libgit2_shutdown();
        return errorCode;
    }

    errorCode = censorString("password", "********", repository);
    if(errorCode != 0) {
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
        printf("Memory allocation failure\n");
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
        printf("Memory allocation failure\n");

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


int tree_walk_cb(const char *root, const git_tree_entry *entry, void *payload) {
    printf("\tFilename: %s\n", git_tree_entry_name(entry));
    printf("\tType: %s\n", git_object_type2string(git_tree_entry_type(entry)));
    return 0;
}


void handleError(int errorCode) {
    const git_error *e = git_error_last();
    printf("Error Code %d: %s\n", errorCode, e->message);
}
