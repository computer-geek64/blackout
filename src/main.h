// main.h
// Ashish D'Souza

#ifndef MAIN_H
#define MAIN_H

int main(int argc, char **argv);
int findRepositoryRoot(char **repositoryRoot, const char *repositoryPathArgument);
int tree_walk_cb(const char *root, const git_tree_entry *entry, void *payload);
void handleGitError(int errorCode);
void handleMemoryAllocationError();

#endif
