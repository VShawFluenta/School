/* called with : 
./disklist disk.IMA

Instructions for part 2.
Starting from the root directory, the directory listing should be formatted as follows:
• Directory Name, followed by a line break, followed by “==================”, followed by
a line break.
• List of files or subdirectories:
– The first column will contain:
∗ F for regular files, or
∗ D for directories
then, followed by a single space
– then 10 characters to show the file size in bytes, followed by a single space
– then 20 characters for the file name, followed by a single space
– then the file creation date and creation time.
– then a line break.
Note: For a directory entry, if the field of “First Logical Cluster” is 0 or 1, then this
directory entry should be skipped and not listed.*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "diskFunctionsLibrary.h"
#include <time.h>



int main(int argc, char *argv[]) {
    inTestingMode =1; 
    if (argc != 2) {
        printf("Usage: %s <disk image file>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("Error opening disk image file");
        return 1;
    }

    struct BootSector bootSector;
    readBootSector(fp, &bootSector);

    uint8_t *fat;
    readFAT(fp, &bootSector, &fat);

    struct DirectoryEntry *rootDir = malloc(bootSector.BPB_RootEntCnt * sizeof(struct DirectoryEntry));
    readRootDirectory(fp, &bootSector, rootDir);

    listDirectory(fp, &bootSector, rootDir, bootSector.BPB_RootEntCnt * sizeof(struct DirectoryEntry), 1);

    free(rootDir);
    free(fat);
    fclose(fp);

    return 0;
}
