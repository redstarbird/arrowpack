/* This file is used for creating progress bars */
#include "ProgressBar.h"
#define PROGRESS_BAR_LENGTH 50

void print_progress_bar(int current, int total)
{
    const int bar_length = 50; // The number of '=' characters to use for the bar
    int percentage = (int)current / total * 100;

    printf("\r[");

    int i;
    printf("\033[s"); // Save the current cursor position
    printf("[");
    for (i = 0; i < PROGRESS_BAR_LENGTH; i++)
    {
        if (i < percentage / 2)
        {
            printf("=");
        }
        else
        {
            printf(" ");
        }
    }
    printf("] %3d%%\033[u", percentage); // Restore the cursor position and clear the line
    fflush(stdout);
}