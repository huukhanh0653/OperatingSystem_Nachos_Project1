//! Utility for User program

char buffer[33];

int len(char *str)
{
    int len = 0;
    while (*str != '\0')
    {
        str++;
        len++;
    }
    return len;
}

unsigned abs(int x) { return (x > 0 ? x : -x); }

char *str(int x)
{
    int temp = x;
    int i = 0;
    while (temp > 0)
    {
        buffer[i] = temp / 10 + 48;
        temp /= 10;
    }
    return buffer;
}

char*_concat(char s1[], char s2[])
{
    int i, j;
    i = len(s1);
    for (j = 0; s2[j] != '\0'; i++, j++)
    {
        s1[i] = s2[j];
    }
    s1[i] = '\0';
    return s1;
}

void ClrStr(char* buffer,int len)
{
    int i;
    for (i = 0; i < len; i++)
        buffer[i] = 0;
}

void concatWithSpace(char s1[], char s2[])
{
    _concat(s1, " ");
    _concat(s1, s2);
}