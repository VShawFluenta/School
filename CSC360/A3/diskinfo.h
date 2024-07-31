#ifndef DISKINFO_H
#define DISKINFO_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern int inTestingMode;

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

void readBootSector(FILE *fp, struct BootSector *bootSector);
void readFAT(FILE *fp, struct BootSector *bootSector, uint8_t **fat);
void readRootDirectory(FILE *fp, struct BootSector *bootSector, struct DirectoryEntry *rootDir);
void PrintFileName(uint8_t nameArray[]);
void PrintArray(uint8_t array[], int n);
int countFiles(struct DirectoryEntry *dir, uint32_t dirSize, FILE *fp, struct BootSector *bootSector, uint8_t *fat);
uint32_t countFreeClusters(uint8_t *fat, uint32_t totalClusters);
void findAndPrintDiskLabel(struct BootSector* bootSector, struct DirectoryEntry *rootDir);

#endif // DISKINFO_H
