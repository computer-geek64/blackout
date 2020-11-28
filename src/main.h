// main.h
// Ashish D'Souza

#ifndef MAIN_H
#define MAIN_H

int main(int argc, char **argv);
int findRepositoryRoot(char **repositoryRoot, const char *repositoryPathArgument);
void handleGitError(int errorCode);
void handleMemoryAllocationError();
int isCommitUpdateRequired(int *condition, git_commit *commit);
int commitCallback(git_commit **commit, CommitList commitList, git_repository*);

#endif
