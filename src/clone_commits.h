// censor_string.h
// Ashish D'Souza

#ifndef CLONE_COMMITS_H
#define CLONE_COMMITS_H

typedef struct CommitList CommitList;
int cloneCommits(git_repository *repository, int *commitCallbackFunction(int*, git_commit*));
int cloneCommitsFromHead(git_repository *repository, int *commitCallbackFunction(int*, git_commit*));
int cloneCommitsFromHeadHelper(git_commit **commit, int *commitCallbackFunction(int*, git_commit*));
int commitCallback(int *condition, git_commit *commit);
void destroyCommitList(CommitList commitList);

#endif
