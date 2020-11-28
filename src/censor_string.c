// censor_string.c
// Ashish D'Souza

#include "censor_string.h"
#include "clone_commits.c"


int censorString(const char *substring, const char *replacement, git_repository *repository) {
    typedef struct ReplaceStringPayload {
        const char* targetString;
        const char* replacementString;
    } ReplaceStringPayload;
    ReplaceStringPayload replaceStringPayload = {substring, replacement};

    int errorCode = cloneCommitsFromHead(repository, &isStringInCommitBlobs, &replaceStringInCommitBlobs, (void*) &replaceStringPayload);
    if(errorCode != 0) {
        // Cleanup
        return errorCode;
    }

    return 0;
}

int isStringInCommitBlobs(int *commitUpdateRequired, git_commit *commit, git_repository *repository, void *payload) {
    // Get values from payload
    typedef struct ReplaceStringPayload {
        const char* targetString;
        const char* replacementString;
    } ReplaceStringPayload;
    ReplaceStringPayload *replaceStringPayload = (ReplaceStringPayload*) payload;

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
    TreeWalkPayload treeWalkPayload = {replaceStringPayload->targetString, repository, FALSE};

    errorCode = git_tree_walk(tree, GIT_TREEWALK_PRE, isStringInCommitBlobsCallback, (void*) &treeWalkPayload);
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

int replaceStringInCommitBlobs(git_commit **commit, CommitList commitList, git_repository *repository, void *payload) {
    // Replaces all instances of a string in commit blobs
    // Get values from payload
    typedef struct ReplaceStringPayload {
        const char* targetString;
        const char* replacementString;
    } ReplaceStringPayload;
    ReplaceStringPayload *replaceStringPayload = (ReplaceStringPayload*) payload;

    // Walk through the commit tree
    git_tree *tree;
    int errorCode = git_commit_tree(&tree, *commit);
    if(errorCode < 0) {
        handleGitError(errorCode);

        // Cleanup
        git_tree_free(tree);
        return errorCode;
    }

    // Create payload for tree walk
    typedef struct TreeWalkPayload {
        const char* targetString;
        const char* replacementString;
        git_repository *repository;
        git_tree_update *updates;
        size_t nupdates;
    } TreeWalkPayload;
    TreeWalkPayload treeWalkPayload = {replaceStringPayload->targetString, replaceStringPayload->replacementString, repository, NULL, 0};

    errorCode = git_tree_walk(tree, GIT_TREEWALK_PRE, replaceStringInCommitBlobsCallback, (void*) &treeWalkPayload);
    if(errorCode < 0) {
        handleGitError(errorCode);

        // Cleanup
        git_tree_free(tree);
        return errorCode;
    }

    // Create updated tree
    git_oid newTreeOid;
    git_tree_create_updated(&newTreeOid, repository, tree, treeWalkPayload.nupdates, treeWalkPayload.updates);
    git_tree *newTree;
    git_tree_lookup(&newTree, repository, &newTreeOid);

    // Create new commit
    git_oid newCommitOid;
    git_commit_create(&newCommitOid, repository, NULL, git_commit_author(*commit), git_commit_committer(*commit), git_commit_message_encoding(*commit), git_commit_message(*commit), newTree, commitList.size, commitList.size > 0 ? commitList.list : NULL);
    git_commit_lookup(commit, repository, &newCommitOid);

    // Cleanup
    git_tree_free(newTree);
    git_tree_free(tree);
    return 0;
}

int isStringInCommitBlobsCallback(const char *root, const git_tree_entry *entry, void *payload) {
    // Get values from payload
    typedef struct TreeWalkPayload {
        const char *targetString;
        git_repository *repository;
        int commitUpdateRequired;
    } TreeWalkPayload;
    TreeWalkPayload *treeWalkPayload = (TreeWalkPayload*) payload;

    // Skip node if it is not a blob (must be subtree/directory)
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

    // Cleanup
    git_blob_free(blob);
    return 0;
}

int replaceStringInCommitBlobsCallback(const char *root, const git_tree_entry *entry, void *payload) {
    // Get values from payload
    typedef struct TreeWalkPayload {
        const char *targetString;
        const char *replacementString;
        git_repository *repository;
        git_tree_update *updates;
        size_t nupdates;
    } TreeWalkPayload;
    TreeWalkPayload *treeWalkPayload = (TreeWalkPayload*) payload;

    // Skip node if it is not a blob (must be subtree/directory)
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
        // Get file contents of blob and replace substring
        const char* content = (const char*) git_blob_rawcontent(blob);
        const void *buffer;
        replaceString((char**) &buffer, content, treeWalkPayload->targetString, treeWalkPayload->replacementString);

        // Create new blob with new string
        git_oid newBlobOid;
        int errorCode = git_blob_create_from_buffer(&newBlobOid, treeWalkPayload->repository, buffer, (strlen((char*) buffer) + 1) * sizeof(char));
        if(errorCode < 0) {
            handleGitError(errorCode);

            // Cleanup
            free((char*) buffer);
            git_blob_free(blob);
            return errorCode;
        }

        // Create git_tree_update
        const char *filename = (char*) git_tree_entry_name(entry);
        char *path = malloc((strlen(root) + strlen(filename) + 1) * sizeof(char));
        sprintf(path, "%s%s", root, filename);
        git_tree_update update = {GIT_TREE_UPDATE_UPSERT, newBlobOid, git_tree_entry_filemode(entry), path};

        // Add git_tree_update to updates list
        git_tree_update *updatesRealloc = (git_tree_update*) realloc(treeWalkPayload->updates, (treeWalkPayload->nupdates + 1) * sizeof(git_tree_update));
        if(updatesRealloc == NULL) {
            handleMemoryAllocationError();

            // Cleanup
            free((char*) buffer);
            git_blob_free(blob);
            return errorCode;
        }
        treeWalkPayload->updates = updatesRealloc;

        treeWalkPayload->updates[treeWalkPayload->nupdates] = update;
        treeWalkPayload->nupdates++;

        free((char*) buffer);
    }

    // Cleanup
    git_blob_free(blob);
    return 0;
}
