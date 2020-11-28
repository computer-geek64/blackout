// main.h
// Ashish D'Souza

#ifndef MAIN_H
#define MAIN_H

int main(int argc, char **argv);
int findRepositoryRoot(char **repositoryRoot, const char *repositoryPathArgument);
void handleGitError(int errorCode);
void handleMemoryAllocationError();
int replaceStringInCommitBlobs(git_commit **commit, CommitList commitList, git_repository *repository, void *payload);
int isStringInCommitBlobs(int *commitUpdateRequired, git_commit *commit, git_repository *repository, void *payload);
int treeWalkCallback(const char* root, const git_tree_entry *entry, void *payload);

#endif
