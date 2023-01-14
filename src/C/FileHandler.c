#include "FileHandler.h"
#include <emscripten.h>

void CopyFile(char *FileToCopy, char *FileToCopyTo)
{
    printf("Copying file: %s to location: %s\n", FileToCopy, FileToCopyTo);
    FILE *input = fopen(FileToCopy, "r");
    FILE *output = fopen(FileToCopyTo, "w");
    if (!input || !output)
    {
        ThrowFatalError("Error opening file %s or %s\n", FileToCopy, FileToCopyTo);
    }

    // Create a buffer to hold the data from the input file
    char buffer[1024];

    // Read the data from the input file and write it to the output file
    // in chunks of BUFFER_SIZE bytes
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, 1024, input)) > 0)
    {
        fwrite(buffer, 1, bytesRead, output);
    }

    // Close the input and output files
    fclose(input);
    fclose(output);
}

void CreateFileWrite(char *path, char *text)
{
    EnsureDirectory(GetTrueBasePath(path));
    printf("Creating file %s, text: %s\n", path, text);
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
    int currentChar = 0;
    do
    {
        if (feof(filePTR))
        {
            break;
        }
        buffer[currentChar] = fgetc(filePTR);
        currentChar++;
    } while (1);
    fclose(filePTR);
    buffer[currentChar - 1] = '\0';
    return buffer;
    /*
        FILE *filePtr;
        filePtr = fopen(path, "r");
        printf("Reading from file %s\n", path);
        char *buffer;
        if (filePtr == NULL)
        {
            printf("%s has returned NULL\n", path);
            return NULL;
        }
        printf("confused\n");
        fseek(filePtr, 0, SEEK_END);         // seek to end of file
        long int length = ftell(filePtr);    // get length of file
        fseek(filePtr, 0, SEEK_SET);         // go back to start of file
        buffer = (char *)malloc(length + 1); // allocate memory for buffer to store file contents
        printf(":(\n");
        if (buffer)
        {
            printf("buffer\n");
            if (fread(buffer, length, 1, filePtr) == -1)
            {
                printf("error reading data from file :(\n");
                exit(1);
            } // read file into buffer
        }
        printf(":D\n");
        printf("Buffer: %s\n", buffer);
        printf("sdasdas\n");
        if (fclose(filePtr) != 0)
        {
            printf("Error closing file: %s\n", path);
            exit(1);
        } // close the file
        printf("what?!?!?!?!?");
        return buffer;*/
}
bool FileExists(char *FilePath)
{
    return access(FilePath, F_OK) == 0;
}

bool DirectoryExists(const char *path)
{
    struct stat sb;
    return stat(path, &sb) == 0 && S_ISDIR(sb.st_mode);
}

void EnsureDirectory(const char *DirectoryPath)
{
    struct stat st;
    if (stat(DirectoryPath, &st) != 0)
    {
        // directory does not exist, create it
        if (mkdir(DirectoryPath, 0700) != 0)
        {
            perror("Error creating directory");
        }
    }
    else
    {
        // directory exists, check if it is actually a directory
        if (!S_ISDIR(st.st_mode))
        {
            fprintf(stderr, "%s is not a directory\n", DirectoryPath);
        }
    }
}
