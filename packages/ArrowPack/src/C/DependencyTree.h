#ifndef DEPENDENCYTREE_H
#define DEPENDENCYTREE_H

#define PATH_SEPARATOR '/'

int GetNumOfRegexMatches(const char *Text, const char *Pattern);

typedef struct FileRule
{
    char FileExtensions[4][32];
    char regexPattern[64];
    unsigned short int StartPos, EndPos; // endpos is pos from end of string
} FileRule;

char **FindAllRegexMatches(char *Text, struct FileRule rule); // function that returns an array matches as strings

typedef struct Node
{ // structure for individual nodes in the tree
    char path[256];
    struct Node *Dependents, *Dependencies;
    unsigned int DependenciesInTree, DependentsInTree;
} Node;

struct Node *CreateTree(char *Wrapped_paths, int ArrayLength); // Quite self explanatory

char **FindDependencies(char *Path); // function that returns an array of strings representing dependencies for the given file

#endif