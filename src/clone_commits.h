// censor_string.h
// Ashish D'Souza

#ifndef CLONE_COMMITS_H
#define CLONE_COMMITS_H

int cloneCommits(git_repository *repository, int (*isCommitUpdateRequiredFunction)(int*, git_commit*), int (*commitCallbackFunction)(git_commit**, CommitList, git_repository*));
int cloneCommitsFromHead(git_repository *repository, int (*isCommitUpdateRequiredFunction)(int*, git_commit*), int (*commitCallbackFunction)(git_commit**, CommitList, git_repository*));
int cloneCommitsFromHeadHelper(git_commit **commit, ReferenceList referenceList, git_repository *repository, int (*isCommitUpdateRequiredFunction)(int*, git_commit*), int (*commitCallbackFunction)(git_commit**, CommitList, git_repository*));
int updateCommitRefs(git_commit *oldCommit, git_commit *newCommit, ReferenceList referenceList, git_repository *repository);
int getDirectReferences(ReferenceList *referenceList, git_repository *repository);

#endif
