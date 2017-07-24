#include <string.h>

void substring(char *string, int start, int end)
{
    char* i = string;
    char* j = string;
    while(*j != 0)
    {
        *i = *j++;
        if(*i != ' ')
            i++;
    }
    *i = 0;
}

char* trim(char *str) {
    char *s = str;
    int len = strlen(s);
    if(s[0] == ' '){
        s = s+1;
    }
    substring(s,0,len);

    return s;
}
