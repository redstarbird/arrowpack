#ifndef DEPENDENCYTREE_H
#define DEPENDENCYTREE_H

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#endif // DEBUG
#ifndef _WIN32
#define PATH_SEPARATOR '/'
#endif // !_W

char ** FindAllRegexMatches(char * Text, struct FileRule rule); // function that returns an array matches as strings

typedef struct Node { // structure for individual nodes in the tree
    char path[256];
    struct Node *Dependents, *Dependencies;
    unsigned int DependenciesInTree, DependentsInTree;
};

typedef struct FileRule {
    char FileExtensions[4][32];
    char regexPattern[64];
    unsigned short int StartPos, EndPos; // endpos is pos from end if string
};

struct Node * CreateTree(char **paths, unsigned short int ArrayLength, char * entryPath);

#endif