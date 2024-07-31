//V Shaw
//V01020304
//This is my first attempt 
/*

Explanation and requiremnts: 
In part I, you will write a program that displays information about the file system. In order to complete
part I, you will need to understand the file system structure of MS-DOS, including FAT Partition Boot
Sector, FAT File Allocation Table, FAT Root Folder, FAT Folder Structure, and so on. For example, your
program for part I will be invoked as follows:
./diskinfo disk.IMA
The output should include the following information:
OS Name:
Label of the disk:
Total size of the disk:
Free size of the disk:
==============
The number of files in the disk
(including all files in the root directory and files in all subdirectories):
=============
Number of FAT copies:
Sectors per FAT:


Things I need to find, and functions I need to make: 
find the number of files in the disk: 
            The number of files in the disk:
            starting from root directory, traverse the directory tree.
            # of files = # of directory entries in each directory
            However, skip a directory entry if:
            the value of it’s attribute field is 0x0F or
            the first logical cluster field is 0 or 1 or
            the Volume Label bit of its attribute field is set as 1 or
            the directory entry is free (see fat-12 doc for details)
Figure out how the heck to access a file

*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#pragma pack(push, 1)
struct BootSector {
    uint8_t  BS_jmpBoot[3];
    uint8_t  BS_OEMName[8];
    uint16_t BPB_BytsPerSec;
    uint8_t  BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t  BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t  BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DirectoryEntry {
    uint8_t  DIR_Name[11];
    uint8_t  DIR_Attr;
    uint8_t  DIR_NTRes;
    uint8_t  DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
};
#pragma pack(pop)

void readBootSector(FILE *fp, struct BootSector *bootSector) {
    fseek(fp, 0, SEEK_SET);
    fread(bootSector, sizeof(struct BootSector), 1, fp);
}

void readRootDirectory(FILE *fp, struct BootSector *bootSector, struct DirectoryEntry *rootDir) {
    int rootDirStart = (bootSector->BPB_RsvdSecCnt + bootSector->BPB_NumFATs * bootSector->BPB_FATSz16) * bootSector->BPB_BytsPerSec;
    fseek(fp, rootDirStart, SEEK_SET);
    fread(rootDir, sizeof(struct DirectoryEntry), bootSector->BPB_RootEntCnt, fp);
}

int main() {
    FILE *fp = fopen("disk_image.ima", "rb");
    if (!fp) {
        perror("Failed to open disk image");
        return 1;
    }

    struct BootSector bootSector;
    readBootSector(fp, &bootSector);

    struct DirectoryEntry *rootDir = malloc(bootSector.BPB_RootEntCnt * sizeof(struct DirectoryEntry));
    readRootDirectory(fp, &bootSector, rootDir);

    for (int i = 0; i < bootSector.BPB_RootEntCnt; i++) {
        if (rootDir[i].DIR_Name[0] == 0x00) {
            break; // No more entries
        }
        if (rootDir[i].DIR_Name[0] == 0xE5) {
            continue; // Deleted entry
        }
        printf("%.11s\n", rootDir[i].DIR_Name);
    }

    free(rootDir);
    fclose(fp);

    return 0;
}


