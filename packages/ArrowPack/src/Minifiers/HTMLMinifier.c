#include "HTMLMinifier.h"
void MinifyHTML(char *html)
{
    // Check for NULL input
    if (html == NULL)
    {
        return;
    }
    // Get the length of the HTML string
    int htmlLen = strlen(html);
    // Allocate memory for the new HTML string
    char *newHTML = malloc(sizeof(char) * (htmlLen + 1));
    if (newHTML == NULL)
    {
        return;
    }
    // Initialize the new HTML string
    newHTML[0] = '\0';
    // Loop through the original HTML string
    for (int i = 0; i < htmlLen; i++)
    {
        // Check if the current character is whitespace or a newline
        if (html[i] == ' ' || html[i] == '\n')
        {
            // If it is, skip it and continue
            continue;
        }
        else
        {
            // If it isn't, append it to the new HTML string
            char currentChar[2] = {html[i], '\0'};
            strcat(newHTML, currentChar);
        }
    }
    // Copy the new HTML string back into the original string
    strcpy(html, newHTML);
    // Free the memory allocated for the new HTML string
    free(newHTML);
}
