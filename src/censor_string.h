// censor_string.h
// Ashish D'Souza

#ifndef CENSOR_STRING_H
#define CENSOR_STRING_H

typedef struct ReferenceList ReferenceList;
typedef struct Reference Reference;
int censorString(const char *str, const char *replacement, git_repository *repository);
//int cloneCommits(int (*commitCallbackFunction)(), git_repository *repository);
//int commitCallback(git_commit commit);
int getDirectReferences(ReferenceList *referenceList, git_repository *repository);
void destroyReferenceList(ReferenceList referenceList);

#endif
