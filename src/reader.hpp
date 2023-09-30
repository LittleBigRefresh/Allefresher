extern "C"
{
#include <systemctrl.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
}

#define MAX_LINE_SIZE 256

int readFileFirstLine(char *path, char *target);