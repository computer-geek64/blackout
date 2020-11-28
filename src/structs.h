// structs.h
// Ashish D'Souza

#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct ReferenceList ReferenceList;
typedef struct Reference Reference;
typedef struct CommitList CommitList;
typedef struct CommitMap CommitMap;
typedef struct CommitMapping CommitMapping;

void destroyReferenceList(ReferenceList referenceList);
void destroyCommitList(CommitList commitList);

#endif
