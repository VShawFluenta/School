#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "diskFunctionsLibrary.h"

#define ROOT_DIR_OFFSET(bootSector) ((bootSector->BPB_RsvdSecCnt + bootSector->BPB_NumFATs * bootSector->BPB_FATSz16) * bootSector->BPB_BytsPerSec)
#define CLUSTER_OFFSET(bootSector, cluster) (((cluster - 2) + 33) * 512)


void copyFileToImage(struct BootSector *bootSector, struct DirectoryEntry *rootDir, uint8_t *diskData, const char *filepath, const char *targetDirPath) {
    struct stat st;
    if (stat(filepath, &st) == -1) {
        perror("stat");
        return;
    }

    uint32_t fileSize = st.st_size;

    // Find free directory entry
    int freeEntryIndex = findFreeDirectoryEntry(rootDir, bootSector->BPB_RootEntCnt);
    if (freeEntryIndex == -1) {
        printf("No free directory entry found.\n");
        return;
    }

    // Check available space
    uint32_t fatSize = bootSector->BPB_FATSz16 * bootSector->BPB_BytsPerSec;
    uint32_t clusterSize = 512 /*bootSector->BPB_SecPerClus * bootSector->BPB_BytsPerSec*/;
    if (!checkAvailableSpace(diskData + bootSector->BPB_RsvdSecCnt * bootSector->BPB_BytsPerSec, fatSize, fileSize, clusterSize)) {
        printf("No enough free space in the disk image.\n");
        return;
    }

    FILE *inputFile = fopen(filepath, "rb");
    if (!inputFile) {
        perror("fopen");
        return;
    }

    struct DirectoryEntry *entry = &rootDir[freeEntryIndex];
    uint16_t firstCluster = findFreeCluster(diskData + bootSector->BPB_RsvdSecCnt * bootSector->BPB_BytsPerSec, fatSize);

    if (firstCluster == 0xFFFF) {
        printf("No free cluster found.\n");
        fclose(inputFile);
        return;
    }

    setDirectoryEntry(entry, filepath, firstCluster, fileSize, &st);

    uint8_t buffer[512];
    uint16_t currentCluster = firstCluster;
    uint16_t nextCluster;

    while (fread(buffer, 1, 512, inputFile) > 0) {
        uint32_t clusterOffset = CLUSTER_OFFSET(bootSector, currentCluster);
        memcpy(diskData + clusterOffset, buffer, CLUSTER_SIZE);
        nextCluster = findFreeCluster(diskData + bootSector->BPB_RsvdSecCnt * bootSector->BPB_BytsPerSec, fatSize);
        if (nextCluster == 0xFFFF) {
            printf("Error: disk is full.\n");
            fclose(inputFile);
            return;
        }
        writeToFAT(diskData + bootSector->BPB_RsvdSecCnt * bootSector->BPB_BytsPerSec, currentCluster, nextCluster);
        currentCluster = nextCluster;
    }
    writeToFAT(diskData + bootSector->BPB_RsvdSecCnt * bootSector->BPB_BytsPerSec, currentCluster, 0xFFF);
    fclose(inputFile);
}
void printUsage() {
    printf("Usage: ./diskput disk.IMA /subdir1/subdir2/foo.txt\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printUsage();
        return 1;
    }

    const char *diskImagePath = argv[1];
    const char *filePath = argv[2];

    FILE *diskImage = fopen(diskImagePath, "r+b");
    if (!diskImage) {
        perror("fopen");
        return 1;
    }

    struct BootSector bootSector;
    if (fread(&bootSector, sizeof(struct BootSector), 1, diskImage) != 1) {
        perror("fread");
        fclose(diskImage);
        return 1;
    }

    uint32_t rootDirOffset = ROOT_DIR_OFFSET(&bootSector);
    uint32_t rootDirSize = bootSector.BPB_RootEntCnt * sizeof(struct DirectoryEntry);
    struct DirectoryEntry *rootDir = malloc(rootDirSize);
    if (!rootDir) {
        perror("malloc");
        fclose(diskImage);
        return 1;
    }

    if (fseek(diskImage, rootDirOffset, SEEK_SET) != 0) {
        perror("fseek");
        free(rootDir);
        fclose(diskImage);
        return 1;
    }

    if (fread(rootDir, rootDirSize, 1, diskImage) != 1) {
        perror("fread");
        free(rootDir);
        fclose(diskImage);
        return 1;
    }

    fseek(diskImage, 0, SEEK_END);
    long diskSize = ftell(diskImage);
    fseek(diskImage, 0, SEEK_SET);

    uint8_t *diskData = malloc(diskSize);
    if (!diskData) {
        perror("malloc");
        free(rootDir);
        fclose(diskImage);
        return 1;
    }

    if (fread(diskData, diskSize, 1, diskImage) != 1) {
        perror("fread");
        free(diskData);
        free(rootDir);
        fclose(diskImage);
        return 1;
    }

    copyFileToImage(&bootSector, rootDir, diskData, filePath, "");

    fseek(diskImage, 0, SEEK_SET);
    if (fwrite(&bootSector, sizeof(struct BootSector), 1, diskImage) != 1) {
        perror("fwrite");
        free(diskData);
        free(rootDir);
        fclose(diskImage);
        return 1;
    }

    if (fseek(diskImage, rootDirOffset, SEEK_SET) != 0) {
        perror("fseek");
        free(diskData);
        free(rootDir);
        fclose(diskImage);
        return 1;
    }

    if (fwrite(rootDir, rootDirSize, 1, diskImage) != 1) {
        perror("fwrite");
        free(diskData);
        free(rootDir);
        fclose(diskImage);
        return 1;
    }

    if (fseek(diskImage, 0, SEEK_SET) != 0) {
        perror("fseek");
        free(diskData);
        free(rootDir);
        fclose(diskImage);
        return 1;
    }

    if (fwrite(diskData, diskSize, 1, diskImage) != 1) {
        perror("fwrite");
        free(diskData);
        free(rootDir);
        fclose(diskImage);
        return 1;
    }

    free(diskData);
    free(rootDir);
    fclose(diskImage);

    return 0;
}
