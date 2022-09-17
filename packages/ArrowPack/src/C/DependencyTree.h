#ifndef DEPENDENCYTREE_H
#define DEPENDENCYTREE_H

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#endif // DEBUG
#ifndef _WIN32
#define PATH_SEPARATOR '/'
#endif // !_W

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

void *CreateTree(char *Wrapped_paths, unsigned int ArrayLength, unsigned int AbsoluteArrayLength, char *entry, struct Node *Tree);

#endif