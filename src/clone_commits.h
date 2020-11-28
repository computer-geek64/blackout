// censor_string.h
// Ashish D'Souza

#ifndef CLONE_COMMITS_H
#define CLONE_COMMITS_H

int cloneCommits(git_repository *repository, int (*isCommitUpdateRequiredFunction)(int*, git_commit*, git_repository*, void*), int (*commitCallbackFunction)(git_commit**, CommitList, git_repository*, void*), void *payload);
int cloneCommitsFromHead(git_repository *repository, int (*isCommitUpdateRequiredFunction)(int*, git_commit*, git_repository*, void*), int (*commitCallbackFunction)(git_commit**, CommitList, git_repository*, void*), void *payload);
int cloneCommitsFromHeadHelper(git_commit **commit, ReferenceList referenceList, CommitMap *commitMap, git_repository *repository, int (*isCommitUpdateRequiredFunction)(int*, git_commit*, git_repository*, void*), int (*commitCallbackFunction)(git_commit**, CommitList, git_repository*, void*), void *payload);
int updateCommitRefs(git_commit *oldCommit, git_commit *newCommit, ReferenceList referenceList, git_repository *repository);
int getDirectReferences(ReferenceList *referenceList, git_repository *repository);

#endif
