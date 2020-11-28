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
#include "util.c"
#include "censor_string.c"


int main(int argc, char **argv) {
    // Initialize libgit2
    git_libgit2_init();

    // Find git repository root path
    char *repositoryRoot;
    int errorCode = findRepositoryRoot(&repositoryRoot, ".");
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

    if(argc < 3) {
        printf("No search/replacement text specified\n");

        // Cleanup
        git_repository_free(repository);
        free(repositoryRoot);
        git_libgit2_shutdown();
        return 1;
    }

    // Censor string
    errorCode = censorString(*(argv + 1), *(argv + 2), repository);
    if(errorCode != 0) {
        // Cleanup
        git_object_free((git_object*) repository); // Solves occasional "malloc_consolidate(): invalid chunk size" errors by not using git_repository_free
        free(repositoryRoot);
        git_libgit2_shutdown();
        return errorCode;
    }

    // Cleanup
    git_object_free((git_object*) repository); // Solves occasional "malloc_consolidate(): invalid chunk size" errors by not using git_repository_free
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
