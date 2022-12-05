#include <iostream>
#include <dirent.h>
#include <emscripten.h>
#include <string>
#include <vector>
#include <utility>

std::string* RecursiveWalkDir(std::string path) {

    std::string* files;
    std::string* dirs;

    uint8_t folderDepth = 1;

    std::vector<struct dirent> Entries(5);
    for (int i = 0; i < 5; i++) {
        Entries.push_back(dirent());
    }

    std::vector<DIR> Dirs(5);
    
    Dirs.push_back(*opendir(path.c_str()));
    
    
    while (folderDepth != 0)
    {
        if ((Entries[folderDepth - 1] = *readdir(Dirs[folderDepth -1]*) != NULL))
    }
    
}
