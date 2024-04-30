#include "reader.h"

// Reads the file at `path` into `target`
int readFileFirstLine(char *path, char *target)
{
    SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0777);
    // If opening the file failed, just die out
    if (fd < 0)
    {
        sceClibPrintf("Unable to open \"%s\" code %x!\n", path, fd);
        return 0;
    }

    char data[MAX_LINE_SIZE];
    // Read the data in
    int read = sceIoRead(fd, data, sizeof(data));

    // Close the file now that we are done with it
    sceIoClose(fd);

    // If we read too much data to fit inside our buffer, something *probably* went wrong
    if (read >= MAX_LINE_SIZE)
    {
        sceClibPrintf("Way too much data in %s!\n", path);
        return 0;
    }

    // The amount of valid bytes
    int valid = 0;
    // Iterate over all the data we read
    while (valid < read)
    {
        // If the character is whitespace,
        if (isspace(data[valid]))
        {
            // Break out, as we are at the end of the domain
            break;
        }

        // Mark one more character as valid
        valid++;
    }

    // If theres no valid characters, something went wrong (maybe extra padding at the start of the file?)
    if (valid == 0)
    {
        sceClibPrintf("No string found in %s! Make sure theres no padding at the start of the file!\n", path);
        return 0;
    }

    // Null terminate the data
    data[valid] = 0;

    // Copy the read data into the domain file
    strcpy(target, data);

    return 1;
}