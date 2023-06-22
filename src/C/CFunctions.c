/* This file is for extra C specific functions and is not currently used */
#include "CFunctions.h"

// Runs a command line command and returns the output of the command
char *GetCommandOutput(const char *command)
{
    if (command == NULL)
    {
        // Invalid input
        return NULL;
    }
    char *output;
    // Open a pipe to run the command
    FILE *fp = popen(command, "r");
    if (fp == NULL)
    {
        // Failed to run the command
        return NULL;
    }

    // Allocate a buffer to read the output of the command
    const size_t BUF_SIZE = 1024;
    char *buf = malloc(BUF_SIZE);
    if (buf == NULL)
    {
        // Failed to allocate memory
        pclose(fp);
        return NULL;
    }

    // Read the output of the command
    size_t output_len = 0;
    while (fgets(buf, BUF_SIZE, fp) != NULL)
    {
        output_len += strlen(buf);

        // Allocate more memory for the output string
        char *new_output = realloc(output, output_len + 1);
        if (new_output == NULL)
        {
            // Failed to allocate memory
            free(output);
            pclose(fp);
            return NULL;
        }

        output = new_output;
        strcat(output, buf);
    }

    // Close the pipe
    pclose(fp);

    // Null-terminate the output string
    output[output_len] = '\0';

    return output;
}
