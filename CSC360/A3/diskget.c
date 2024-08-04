#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include "diskFunctionsLibrary.h" 
#define ROOT_DIR_OFFSET(bootSector) ((bootSector->BPB_RsvdSecCnt + bootSector->BPB_NumFATs * bootSector->BPB_FATSz16) * bootSector->BPB_BytsPerSec)
#define CLUSTER_OFFSET(bootSector, cluster) (((cluster - 2) * bootSector->BPB_SecPerClus + 33) * bootSector->BPB_BytsPerSec)

void copyFileFromImage(struct BootSector *bootSector, struct DirectoryEntry *rootDir, uint8_t *diskData, const char *fullfilename) {
    struct DirectoryEntry *fileEntry = NULL;
         if(inTestingMode){
                printf("Given File is {%s} with lenth of %ld\n", fullfilename, strlen(fullfilename));
        }


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

        
        char filename[9]; 
        char fileExtension[3]; 
        int fn =0; 
        int fe =0; 
        for(fn =0; fn < 8; fn++){
            if(fullfilename[fn] != 0x2E){
                filename[fn]= toupper((char)fullfilename[fn]);
            }else {
                if(inTestingMode){
                    printf("Found [%c] at iteration %d\n", fullfilename[fn], fn); 
                }
                break; 
            }
        }
        filename[fn]='\0';
            if(inTestingMode){
                printf("Given Extension is is [");
            }
            fn ++; 
        for(fe =0; fe < 3 && (fe+fn)< strlen(fullfilename) ; fe++){
            
            if(fullfilename[fe+fn] != 0x20){
                fileExtension[fe]= toupper((char)fullfilename[fe+fn]);
            }else{
                break; 
            }
            if(inTestingMode){
                printf("(%c, %d)", fullfilename[fe+fn], fe+fn);
                
            }
        }
        if(inTestingMode){
                printf("]\n");
            }
        fileExtension[fe]='\0';


        char name[9];
        char extension[4]; 
        extension[0]='\0'; 
        int n =0; 
        for(n =0; n < 8; n++){
            if(rootDir[i].DIR_Name[n] != 0x20){
                name[n]= (char)rootDir[i].DIR_Name[n];
            }else {
                break;
            }
        }
        name[n]='\0';
        int e =0; 
         if(inTestingMode){
                printf("Directory Extension is [");
            }
        int count =0; 
        for(e =0; e < 3; e++){
            if(inTestingMode){
                    printf("%c",(char)rootDir[i].DIR_Name[e+8] );
                }
            if(rootDir[i].DIR_Name[e+8] != ' '){
                extension[e]= (char)rootDir[i].DIR_Name[e+8];
                count ++; 
                
            }else{
                break; 
            }
        }
        if(inTestingMode){
                printf("]\n");
            }
        extension[count]='\0';



        

        // strncpy(name, (char *)rootDir[i].DIR_Name, 11);
        // name[11] = '\0';

        // Check if this is the file we're looking for
        if(inTestingMode){
                printf("Comparing given filename [%s] with name in Dir [%s]\n", filename, name);
                printf("Comparing given fileExtension [%s] with extension in Dir [%s]\n", fileExtension, extension);

            }
        if (strncmp(name, filename, strlen(filename)) == 0 && strncmp(extension, fileExtension, strlen(extension)) == 0) {
            if(inTestingMode){
                printf("FOUND THE FILE\n"); 
            }
            fileEntry = &rootDir[i];
            break;
        }
        if(inTestingMode){
            printf("after comparison\n");
                printf("Comparing given filename [%s] with name in Dir [%s]\n", filename, name);
                printf("Comparing given fileExtension [%s] with extension in Dir [%s]\n\n\n", fileExtension, extension);

            }
    }

    if (fileEntry == NULL) {
        printf("File not found.\n");
        return;
    }

    // Calculate the file's starting cluster and size
    uint16_t firstCluster = fileEntry->DIR_FstClusLO;
    uint32_t fileSize = fileEntry->DIR_FileSize;
    if(inTestingMode){
        printf("The size of the file to be copied is %d\n", fileSize); 
    }
    int isBinary = isBinaryFile(fullfilename);

    char upperFilename[strlen(fullfilename)]; 
    // combineStringsWithPeriod(filename, fileExtension, upperFilename);
    // Open the destination file
    toUpperCase(fullfilename, upperFilename); 
    FILE *outputFile ;
    if(isBinary){ 
        outputFile = fopen(upperFilename, "wb");
        if (outputFile == NULL) {
        perror("fopen");
        return;
        }
    } else{
        outputFile = fopen(upperFilename, "w");
        if (outputFile == NULL) {
            perror("fopen");
            return;
        }
    }
    int s =0;
    // Read and write the file's content
    uint32_t bytesPerCluster = 512 /*bootSector->BPB_SecPerClus * bootSector->BPB_BytsPerSec*/;
    uint8_t *clusterData = malloc(bytesPerCluster);

    while (fileSize > 0) {
        if(inTestingMode){
            printf("This is the %dth iteration of the printing while loop. There are %d bytes left to print\n", s, fileSize);
        }
        uint32_t bytesToWrite = (fileSize > bytesPerCluster) ? bytesPerCluster : fileSize;

        uint32_t clusterOffset = CLUSTER_OFFSET(bootSector, firstCluster);
        memcpy(clusterData, diskData + clusterOffset, bytesToWrite);

        // fwrite(clusterData, 1, bytesToWrite, outputFile);
          writeFormattedOutput(clusterData, bytesToWrite, outputFile, isBinary);

        fileSize -= bytesToWrite;

        // Read the next cluster from the FAT
        uint16_t fatEntry;
        if (firstCluster % 2 == 0) {
            fatEntry = *(uint16_t *)(diskData + bootSector->BPB_RsvdSecCnt * bootSector->BPB_BytsPerSec +  firstCluster +firstCluster/2) & 0xFFF;
        } else {
            fatEntry = *(uint16_t *)(diskData + bootSector->BPB_RsvdSecCnt * bootSector->BPB_BytsPerSec + firstCluster +firstCluster/2) >> 4;
        }
        if(inTestingMode){
            printf("About to look for more data at sector %d\n", fatEntry);
        }

        firstCluster = fatEntry;
        if (firstCluster >= 0xFF8) {
            break;  // End of file
        }
        s ++; 
    }

    free(clusterData);
    fclose(outputFile);
}



int main(int argc, char *argv[]) {
    // inTestingMode =1; 
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

    uint8_t *diskData = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);//Note to self disk data is the pointer to the 
    //start of the IMAGE file, not specifically the file we want to print. 
    if (diskData == MAP_FAILED) {
        perror("mmap failed");
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
