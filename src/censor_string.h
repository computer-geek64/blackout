// censor_string.h
// Ashish D'Souza

#ifndef CENSOR_STRING_H
#define CENSOR_STRING_H

int censorString(const char *str, const char *replacement, git_repository *repository);
int isStringInCommitBlobs(int *commitUpdateRequired, git_commit *commit, git_repository *repository, void *payload);
int replaceStringInCommitBlobs(git_commit **commit, CommitList commitList, git_repository *repository, void *payload);
int isStringInCommitBlobsCallback(const char* root, const git_tree_entry *entry, void *payload);
int replaceStringInCommitBlobsCallback(const char* root, const git_tree_entry *entry, void *payload);

#endif
