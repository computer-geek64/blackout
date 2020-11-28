// structs.c
// Ashish D'Souza

#include "structs.h"


typedef struct ReferenceList {
    Reference* list;
    size_t size;
} referenceList;

typedef struct Reference {
    char *name;
    char *commit;
    git_oid oid;
} Reference;

typedef struct CommitList {
    const git_commit **list;
    size_t size;
} CommitList;

void destroyReferenceList(ReferenceList referenceList) {
    for(unsigned int i = 0; i < referenceList.size; i++) {
        free(referenceList.list[i].name);
        free(referenceList.list[i].commit);
    }
    free(referenceList.list);
}

void destroyCommitList(CommitList commitList) {
    for(unsigned int i = 0; i < commitList.size; i++) {
        git_commit_free((git_commit*) commitList.list[i]);
    }
    free(commitList.list);
}
