#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "diskFunctionsLibrary.h" 

#define ROOT_DIR_OFFSET(bootSector) ((bootSector->BPB_RsvdSecCnt + bootSector->BPB_NumFATs * bootSector->BPB_FATSz16) * bootSector->BPB_BytsPerSec)
#define CLUSTER_OFFSET(bootSector, cluster) (((cluster - 2) * bootSector->BPB_SecPerClus + bootSector->BPB_RsvdSecCnt + bootSector->BPB_NumFATs * bootSector->BPB_FATSz16) * bootSector->BPB_BytsPerSec)

void copyFileFromImage(struct BootSector *bootSector, struct DirectoryEntry *rootDir, uint8_t *diskData, const char *fullfilename) {
    struct DirectoryEntry *fileEntry = NULL;

    // Find the file in the root directory
    for (uint32_t i = 0; i < bootSector->BPB_RootEntCnt; i++) {
        if (rootDir[i].DIR_Name[0] == 0x00) {
            break;  // No more entries
        }
        if (rootDir[i].DIR_Name[0] == 0xE5) {
            continue;  // Deleted entry
        }
        if ((rootDir[i].DIR_Attr & 0x0F) == 0x0F) {
            continue;  // Long file name entry
        }

        char name[9];
        char extension[4];
        int n =0; 
        for(n =0; n < 8; n++){
            if(rootDir[i].DIR_Name[n] != 0x20){
                name[n]= (char)rootDir[i].DIR_Name[n];
            }
        }
        name[n]='\n';
        int e =0; 
        for(e =0; e < 3; e++){
            if(rootDir[i].DIR_Name[e+8] != 0x20){
                extension[e]= (char)rootDir[i].DIR_Name[e+8];
            }
        }
        extension[e]='\n';

        char filename[9]; 
        char fileExtension[3]; 

        for(n =0; n < 8; n++){
            if(fullfilename[n] != 0x2E){
                filename[n]= (char)fullfilename[n];
            }
        }
        filename[n]='\n';

        for(e =0; e < 3; e++){
            if(rootDir[i].DIR_Name[e+8] != 0x20){
                fileextension[e]= (char)fullfilename[e+8];
            }
        }
        fileextension[e]='\n';



        

        // strncpy(name, (char *)rootDir[i].DIR_Name, 11);
        // name[11] = '\0';

        // Check if this is the file we're looking for
        if (strncmp(name, filename, strlen(filename)) == 0 && strncmp(name, filename, strlen(filename)) == 0) {
            fileEntry = &rootDir[i];
            break;
        }
    }

    if (fileEntry == NULL) {
        printf("File not found.\n");
        return;
    }

    // Calculate the file's starting cluster and size
    uint16_t firstCluster = fileEntry->DIR_FstClusLO;
    uint32_t fileSize = fileEntry->DIR_FileSize;

    // Open the destination file
    FILE *outputFile = fopen(filename, "w");
    if (outputFile == NULL) {
        perror("fopen");
        return;
    }

    // Read and write the file's content
    uint32_t bytesPerCluster = bootSector->BPB_SecPerClus * bootSector->BPB_BytsPerSec;
    uint8_t *clusterData = malloc(bytesPerCluster);

    while (fileSize > 0) {
        uint32_t clusterOffset = CLUSTER_OFFSET(bootSector, firstCluster);
        memcpy(clusterData, diskData + clusterOffset, bytesPerCluster);

        uint32_t bytesToWrite = (fileSize > bytesPerCluster) ? bytesPerCluster : fileSize;
        fwrite(clusterData, 1, bytesToWrite, outputFile);

        fileSize -= bytesToWrite;

        // Read the next cluster from the FAT
        uint16_t fatEntry;
        if (firstCluster % 2 == 0) {
            fatEntry = *(uint16_t *)(diskData + bootSector->BPB_RsvdSecCnt * bootSector->BPB_BytsPerSec +  firstCluster +firstCluster/2) & 0xFFF;
        } else {
            fatEntry = *(uint16_t *)(diskData + bootSector->BPB_RsvdSecCnt * bootSector->BPB_BytsPerSec + firstCluster +firstCluster/2) >> 4;
        }

        firstCluster = fatEntry;
        if (firstCluster >= 0xFF8) {
            break;  // End of file
        }
    }

    free(clusterData);
    fclose(outputFile);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <disk_image> <filename>\n", argv[0]);
        return 1;
    }

    const char *diskImage = argv[1];
    const char *filename = argv[2];

    int fd = open(diskImage, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    uint8_t *diskData = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (diskData == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    struct BootSector *bootSector = (struct BootSector *)diskData;
    struct DirectoryEntry *rootDir = (struct DirectoryEntry *)(diskData + ROOT_DIR_OFFSET(bootSector));

    copyFileFromImage(bootSector, rootDir, diskData, filename);

    munmap(diskData, sb.st_size);
    close(fd);
    return 0;
}
