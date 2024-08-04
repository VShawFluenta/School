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
#include <string.h>
#include "diskFunctionsLibrary.h"




//for testing only? Maaaaaaybe combine into one main that will control other things?? No idea how to use the make file to compile into 4. 
int main(int argc, char *argv[]) {

                                                            // inTestingMode =1; 



    if(inTestingMode){
        printf("In testing mode\n\n");
    }
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <IMA file>\n", argv[0]);
        return 1;
    }

    FILE *pointerToFile = fopen(argv[1], "rb");
    if (!pointerToFile) {
        perror("Failed to open disk image");
        return 1;
    } else{
         if(inTestingMode){
        printf("Opened File correctly\n\n");
    }
    }

    struct BootSector bootSector;
    readBootSector(pointerToFile, &bootSector);

    struct DirectoryEntry *rootDir = malloc(bootSector.BPB_RootEntCnt * sizeof(struct DirectoryEntry));
    readRootDirectory(pointerToFile, &bootSector, rootDir);


    for (int i = 0; inTestingMode&&i < bootSector.BPB_RootEntCnt; i++) {
        if (rootDir[i].DIR_Name[0] == 0x00) {
            break; // No more entries
        }
        if (rootDir[i].DIR_Name[0] == 0xE5) {
            continue; // Deleted entry
        }
        PrintFileName(rootDir[i].DIR_Name);
        // printf("%.11s\n", rootDir[i].DIR_Name);
    }
    //PRINTING DISKINFO
    /* for reference
    struct BootSector {
    uint8_t  BS_jmpBoot[3];//These two are ignored. 
    uint8_t  BS_OEMName[8];//
    uint16_t BPB_BytsPerSec;//Bytes per sector
    uint8_t  BPB_SecPerClus; //sectors per cluster. 
    uint16_t BPB_RsvdSecCnt;//number of reserved sectors
    uint8_t  BPB_NumFATs; //number of fats, should be 2 in this case
    uint16_t BPB_RootEntCnt; //Maximum number of root directory entries (two bytes)
    uint16_t BPB_TotSec16; //total sector count. this shouls be 16 bits (two bytes)
    uint8_t  BPB_Media; //don't use this, but keep so that the alignment isn't thrown off
    uint16_t BPB_FATSz16;//Sectors per FAT, (two bytes)
    uint16_t BPB_SecPerTrk; //Sectors per track, not sure hwat this is? 
    uint16_t BPB_NumHeads;//Number of heads
    // uint8_t  ToIgnore; //bytes 28-36



    uint32_t BPB_HiddSec; //bytes 28-32, to be ignored. 
    uint32_t BPB_TotSec32; //Bytes 32-36, to be ignored. 
    uint8_t  Boot_Signature; //this should be 0x29 (41) if the Volume Id, Volume label, and File System type. 
    uint32_t VolumeID;//this is only present if Boot signature is correct
    uint8_t  VolumeLable[11];
    uint8_t  FileSystemType[8];*/

    //NOTE TO SELF the 16 bit values might need conversion from big to little Endain, and Hex to binary. 


    uint8_t *fat;
    readFAT(pointerToFile, &bootSector, &fat);
    uint32_t totalSectors = (bootSector.BPB_TotSec16 != 0) ? bootSector.BPB_TotSec16 : bootSector.BPB_TotSec32;
    uint32_t fatSize = bootSector.BPB_FATSz16 * bootSector.BPB_BytsPerSec;
    uint32_t rootDirSectors = ((bootSector.BPB_RootEntCnt * 32) + (bootSector.BPB_BytsPerSec - 1)) / bootSector.BPB_BytsPerSec;
uint32_t dataSectors = totalSectors - (bootSector.BPB_RsvdSecCnt + (bootSector.BPB_NumFATs * bootSector.BPB_FATSz16) + rootDirSectors);
uint32_t totalClusters = dataSectors / bootSector.BPB_SecPerClus;
uint32_t freeClusters = countFreeClusters(fat, totalClusters);
    uint32_t freeSpace = freeClusters * bootSector.BPB_SecPerClus * bootSector.BPB_BytsPerSec;

    if(inTestingMode){
        printf("Calculates free space and it is %d\n", freeSpace);
    }
   
    // struct DirectoryEntry *rootDir;
    // readRootDir(pointerToFile, &bootSector, &rootDir);
    bootSector.FileCount = countFiles(rootDir, bootSector.BPB_RootEntCnt * sizeof(struct DirectoryEntry), pointerToFile, &bootSector, fat);
    if(inTestingMode){
        printf("Calculates number of files and there are %d\n", bootSector.FileCount);
    }
   
    printf("This OS is: "); PrintArray(bootSector.BS_OEMName,8);
    if(inTestingMode){ 
    printf("The boot signature is 0x%02X \n", bootSector.Boot_Signature);
        printf("The bytes per sector is  %d \n", bootSector.BPB_BytsPerSec);
    }


    printf("Lable of this disk: "); findAndPrintDiskLabel(&bootSector, rootDir);
    // PrintArray(bootSector.VolumeLable,11);//FIND IN ROOT DIR TODO NOT CURRENTLY CORRECT!!
    if(inTestingMode){ 
        printf("Volume Label from boot sector is: [%.11s]\n", bootSector.VolumeLable);
    }
    uint32_t totalsectors = bootSector.BPB_TotSec16 != 0 ? bootSector.BPB_TotSec16 : bootSector.BPB_TotSec32;

    printf("Total size of the disk: %d bytes = %d*%d\n",bootSector.BPB_BytsPerSec*totalsectors,bootSector.BPB_BytsPerSec,totalsectors);
    printf("Free size of the disk: %d\n", freeSpace);
    printf("==============\n");
    printf("The number of files in the disk: %d\n", bootSector.FileCount);
// (including all files in the root directory and files in all subdirectories):
    printf("==============\n");
    printf("Number of FAT copies: %d\n", bootSector.BPB_NumFATs);
    printf("Sectors per FAT: %d\n", bootSector.BPB_FATSz16);

    //clean up
    free(rootDir);
    fclose(pointerToFile);

    return 0;
}


