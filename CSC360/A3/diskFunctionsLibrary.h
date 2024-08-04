#ifndef DISKFUNCTIONSLIBRARY_H
#define DISKFUNCTIONSLIBRARY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

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
    uint16_t ignore;
    uint8_t  Boot_Signature;
    uint32_t VolumeID;
    uint8_t  VolumeLable[11];
    uint8_t  FileSystemType[8];
    int FileCount;
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

extern int inTestingMode;

int checkAvailableSpace(uint8_t *fat, uint32_t fatSize, uint32_t fileSize, uint32_t clusterSize);
void setDirectoryEntry(struct DirectoryEntry *entry, const char *filename, uint16_t firstCluster, uint32_t fileSize, struct stat *st) ;
void writeToFAT(uint8_t *fat, uint16_t cluster, uint16_t value) ;
uint16_t findFreeCluster(uint8_t *fat, uint32_t fatSize) ;
int findFreeDirectoryEntry(struct DirectoryEntry *rootDir, uint32_t rootDirSize);
void getShortFileName(char *fullName, char *shortName) ;

int isBinaryFile(const char *filename);
void toUpperCase(const char *str,  char*output);
void readBootSector(FILE *fp, struct BootSector *bootSector);
void readFAT(FILE *fp, struct BootSector *bootSector, uint8_t **fat);
void readRootDirectory(FILE *fp, struct BootSector *bootSector, struct DirectoryEntry *rootDir);
void writeFormattedOutput(uint8_t *clusterData, size_t bytesToWrite, FILE *outputFile, int isBinary);
void PrintFileName(uint8_t nameArray[]);
void PrintArray(uint8_t array[], int n);
int countFiles(struct DirectoryEntry *dir, uint32_t dirSize, FILE *fp, struct BootSector *bootSector, uint8_t *fat);
uint32_t countFreeClusters(uint8_t *fat, uint32_t totalClusters);
void findAndPrintDiskLabel(struct BootSector* bootSector, struct DirectoryEntry *rootDir);
void listDirectory(FILE *fp, struct BootSector *bootSector, struct DirectoryEntry *dir, uint32_t dirSize, uint8_t subdirName[], int isRoot, int depth);

#endif // DISKFUNCTIONSLIBRARY_H
