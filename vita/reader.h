#include <string.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/clib.h>

#define MAX_LINE_SIZE 256

int readFileFirstLine(char *path, char *target);