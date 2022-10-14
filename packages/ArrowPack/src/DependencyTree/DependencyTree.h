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

typedef struct Node Node;

typedef struct Dependency // Wraps a regular Node struct and includes the start and end positions of where the node is referenced so it doesn't need to be worked out again
{
    unsigned int StartRefPos;
    unsigned int EndRefPos;
    struct Node *Dependency;
} Dependency;

struct Node
{ // structure for individual nodes (Modules/Files) in the tree
    char *path;
    struct Node *Dependents;
    struct Dependency *Dependencies;
    unsigned int DependenciesInTree, DependentsInTree;
};

struct Node *CreateTree(char *Wrapped_paths, int ArrayLength, char *TempEntryPath); // Quite self explanatory

char **FindDependencies(char *Path); // function that returns an array of strings representing dependencies for the given file

#endif