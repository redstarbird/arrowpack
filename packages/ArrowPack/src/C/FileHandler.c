#include "FileHandler.h"

void CreateFileWrite(char *path, char *text)
{
    FILE *FilePTR;
    FilePTR = fopen(path, "w");
    if (!FilePTR)
    {
        printf("Error opening file %s\n", path);
        exit(1);
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