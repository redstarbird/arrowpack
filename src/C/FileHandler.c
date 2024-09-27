/* This file is responsible for all of the file handling functions */
#include "FileHandler.h"
#include <emscripten.h>

// Copy a file from one place to another
void CopyFile(char *FileToCopy, char *FileToCopyTo)
{
    ColorGreen();
    printf("Copying file: %s to location: %s\n", FileToCopy, FileToCopyTo);
    ColorNormal();
    FILE *input = fopen(FileToCopy, "r");
    EnsureDirectory(GetTrueBasePath(FileToCopyTo));
    FILE *output = fopen(FileToCopyTo, "w");
    if (!input || !output)
    {
        ThrowFatalError("Error opening file %s or %s\n", FileToCopy, FileToCopyTo);
    }
    size_t BufferSize = 1024;
    // Create a buffer to hold the data from the input file
    char *buffer = malloc(BufferSize);

    // Read the data from the input file and write it to the output file
    // in chunks of BUFFER_SIZE bytes
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BufferSize, input)) > 0)
    {
        fwrite(buffer, 1, bytesRead, output);
        BufferSize *= 2;
        buffer = realloc(buffer, BufferSize);
    }
    // Close the input and output files
    fclose(input);
    fclose(output);
}

// Creates a new file and writes data to it
void CreateFileWrite(char *path, char *text)
{
    EnsureDirectory(GetTrueBasePath(path));
    FILE *FilePTR;
    FilePTR = fopen(path, "w");
    if (!FilePTR)
    {
        ThrowFatalError("Error opening file %s\n", path);
    }
    if (strlen(text) > 0)
    {
        for (int i = 0; i < strlen(text); i++)
        {
            fputc(text[i], FilePTR);
        }
    }

    fclose(FilePTR);
}
void CreateFile(char *path)
{
}

// Reads data from a file and returns it as a string
char EMSCRIPTEN_KEEPALIVE *ReadDataFromFile(char *path)
{ // returns contents of file
    FILE *filePTR = fopen(path, "r");

    if (filePTR == NULL)
    {
        printf("Error opening file %s\n", path);
        return NULL;
    }
    fseek(filePTR, 0, SEEK_END);      // seek to end of file
    long int length = ftell(filePTR); // get length of file
    fseek(filePTR, 0, SEEK_SET);      // go back to start of file

    char *buffer = malloc(length + 1);
    if (buffer == NULL)
    {
        printf("Error creating buffer\n");
        exit(1);
    }

    // int currentChar = 0;
    fread(buffer, 1, length, filePTR);
    fclose(filePTR);
    buffer[length] = '\0';

    return buffer;
}

// Checks whether a file exists
bool FileExists(char *FilePath)
{
    return access(FilePath, F_OK) == 0;
}

// Checks whether a directory exists
bool DirectoryExists(const char *path)
{
    struct stat sb;
    return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
}

// Makes sure that a directory exists, if it doesn't exist then it will be created
void EnsureDirectory(const char *DirectoryPath)
{
    char *Temp = strdup(DirectoryPath);
    struct stat st;
    char *ParentDir = strdup(DirectoryPath);
    int i;
    for (i = strlen(ParentDir) - 1; i >= 0; i--)
    {
        if (ParentDir[i] == '/')
        {
            ParentDir[i] = '\0';
            break;
        }
    }
    if (i < 0)
    {
        ParentDir = ".";
    }
    if (stat(ParentDir, &st) != 0)
    {
        // Parent directory does not exist, create it recursively
        EnsureDirectory(ParentDir);
    }
    if (stat(DirectoryPath, &st) != 0)
    {
        // Directory does not exist, create it
        if (mkdir(DirectoryPath, 0700) != 0)
        {
            perror("Error creating directory");
        }
    }
    else
    {
        // Directory exists, check if it is actually a directory
        if (!S_ISDIR(st.st_mode))
        {
            fprintf(stderr, "%s is not a directory\n", DirectoryPath);
        }
    }
    free(Temp);
    free(ParentDir);
}
