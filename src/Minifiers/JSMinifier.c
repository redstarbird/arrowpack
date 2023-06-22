#include "JSMinifier.h"

void RemoveSingleLineComments(char *Text)
{
    struct RegexMatch *SingleLineCommentLocations = GetAllRegexMatches(Text, "//[^\n]*\n", 0, 0); // Finds locations of single-line comments
    int SingleLineCommentLocatsCount = RegexMatchArrayLength(SingleLineCommentLocations);         // Finds the number of single-line comments
    for (int i = SingleLineCommentLocatsCount - 1; i >= 0; i--)                                   // Removes the single-line comments in reverse order
    {
        RemoveSectionOfString(Text, SingleLineCommentLocations[i].StartIndex, SingleLineCommentLocations[i].EndIndex);
    }
}