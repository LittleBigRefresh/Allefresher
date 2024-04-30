// This file provides implementations for some common libc functions we need, since we dont want to link against libc proper

char *strcpy(char *dest, const char *src)
{
    char *ret = dest;

    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = '\0';

    return ret;
}

int strlen(const char *s)
{
    int len = 0;
    while (*s++)
        len++;
    return len;
}

int isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}