#include "JSMinifier.h"

void RemoveSingleLineComments(char *Text)
{
    struct RegexMatch *SingleLineCommentLocations = GetAllRegexMatches(Text, "//[^\n]*\n", 0, 0);
    int SingleLineCommentLocatsCount = RegexMatchArrayLength(SingleLineCommentLocations);
    for (int i = SingleLineCommentLocatsCount - 1; i >= 0; i--)
    {
        RemoveSectionOfString(Text, SingleLineCommentLocations[i].StartIndex, SingleLineCommentLocations[i].EndIndex);
    }
}