#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>


int inTestingMode =0; 
#define timeOffset 14 //offset of creation time in directory entry
#define dateOffset 16 //offset of creation date in directory entry

#pragma pack(push, 1)
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


    uint32_t BPB_HiddSec; //bytes 28-32, to be ignored. 
    uint32_t BPB_TotSec32; //Bytes 32-36, to be ignored.
    uint16_t ignore;  
    uint8_t  Boot_Signature; //this should be 0x29 (41) if the Volume Id, Volume label, and File System type. 
    uint32_t VolumeID;//this is only present if Boot signature is correct
    uint8_t  VolumeLable[11];
    uint8_t  FileSystemType[8];

    int FileCount ; 



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
    fseek(fp, 0, SEEK_SET); //ADD ERROR MESSAGES/CHECKING TODO
    fread(bootSector, sizeof(struct BootSector), 1, fp);
    bootSector -> FileCount =0; 
}
void readFAT(FILE *fp, struct BootSector *bootSector, uint8_t **fat) {
    uint32_t fatSize = bootSector->BPB_FATSz16 * bootSector->BPB_BytsPerSec;
    *fat = malloc(fatSize);
    fseek(fp, bootSector->BPB_RsvdSecCnt * bootSector->BPB_BytsPerSec, SEEK_SET);
    fread(*fat, fatSize, 1, fp);
}

void readRootDirectory(FILE *fp, struct BootSector *bootSector, struct DirectoryEntry *rootDir) {
    int rootDirStart = (bootSector->BPB_RsvdSecCnt + bootSector->BPB_NumFATs * bootSector->BPB_FATSz16) * bootSector->BPB_BytsPerSec;
    fseek(fp, rootDirStart, SEEK_SET); //TODO ADD ERROR CHECKING/MESSAGES
    fread(rootDir, sizeof(struct DirectoryEntry), bootSector->BPB_RootEntCnt, fp);
}
void PrintFileName(uint8_t nameArray[]){
       // Null-terminate the name field for safety, though not strictly necessary
    char name[12];
    for (int i = 0; i < 11; i++) {
        name[i] = nameArray[i];
    }
    name[11] = '\0';

    // Print the filename and extension
    printf("[%.8s.%.3s]\n", name, name + 8);
}
void PrintArray(uint8_t array[], int n){
    char string[n+1];
    for (int i = 0; i < n; i++) {
        string[i] = array[i];
        if(inTestingMode){ 
        printf("%02X ", array[i]);
        }
    }
    string[n] = '\0';
    printf("[%s]\n",string); 
}

//Basic algo: if the "file" in the directory is not a directory, then determine whether it is a file. If it is a subdirectory, call count files on that subdirectory. 
//Need  a way to not flag the folder above itself or itself as subdirectories so we don't call them infinetely
int countFiles(struct DirectoryEntry *dir, uint32_t dirSize, FILE *fp, struct BootSector *bootSector, uint8_t *fat) {
// bootSector.FileCount =0; 
        if(inTestingMode){
            printf("Running count files\n");
        }

int FileCount =0; 
    for (uint32_t i = 0; i < dirSize / sizeof(struct DirectoryEntry); ++i) {
        if(inTestingMode){
        printf("This is iteration %d of the count files function\n", i);//Note this is currently infinite. 
        printf("The Directory Entry name under inspection is ");
        PrintFileName( dir[i].DIR_Name); 
        }
        if (dir[i].DIR_Name[0] == 0x00) {
            if(inTestingMode){
                printf("No more entries in this directory\n");
            }
            break;  // No more entries in this directory. COULD THIS BE REPLACED BY RETURN FOR CLARITY??

        }
        if (dir[i].DIR_Name[0] == 0xE5) {
            if(inTestingMode){
                printf("Deleted entry\n");
            }
            continue;  // Deleted entry
        }
        if ((dir[i].DIR_Attr & 0x0F) == 0x0F) {
            if(inTestingMode){
                printf("Long file name entry\n");
            }
            continue;  // Long file name entry
        }
        if ((dir[i].DIR_Attr & 0x10 )== 0x10 ) {
            if(dir[i].DIR_Name[0] ==0x2E){
                if(inTestingMode){
                printf("Found self directory or parent directory. Don't count this\n");
                }
                continue; 
            }
            // Subdirectory, recursively count files
            uint16_t firstCluster = dir[i].DIR_FstClusLO;
            if (firstCluster == 0) {
                continue;
            }
            if(inTestingMode){
                printf("Found a subdirectory\n");//Note this is currently infinite. Haha! Not it is not and it works!! (ish) 
            }

            //Note to self, the first cluster is the location of the first part of this file. I think
            // Read subdirectory entries. We need to change the directory we are considering to be the newly discovered subdirectory. 
            //The location of the start of the new subdir is (num sectors leading up to the subdir)*num bytes in each sector. 
            //num sectors is: all the sectos of the FAT Table, The location of the first cluster of the subdir (this is the important part) Note to self this is two less because reasnons I can't remember. 
            //+ 
            uint32_t subDirOffset = ((firstCluster - 2) * bootSector->BPB_SecPerClus + 33) * bootSector->BPB_BytsPerSec;
            struct DirectoryEntry *subDir = malloc(bootSector->BPB_SecPerClus * bootSector->BPB_BytsPerSec);
            fseek(fp, subDirOffset, SEEK_SET);
            fread(subDir, bootSector->BPB_SecPerClus * bootSector->BPB_BytsPerSec, 1, fp);
            FileCount += countFiles(subDir, bootSector->BPB_SecPerClus * bootSector->BPB_BytsPerSec, fp, bootSector, fat);
            if(inTestingMode){
                printf("Just called Count files, the current count of files is %d\n", FileCount); 
            }
            free(subDir);
        } else {
            FileCount++;
            if(inTestingMode){ 
            printf("Found the file ");
            PrintFileName(dir[i].DIR_Name);
            }
        }
        // bootSector->FileCount++;
    }
    return FileCount;
}



//Takes the location of the FAT and determines how many free clusters there are 
uint32_t countFreeClusters(uint8_t *fat, uint32_t totalClusters) {
    uint32_t freeClusters = 0;
    for (uint32_t i = 2; i < totalClusters + 2; i++) { // FAT12 cluster numbering starts at 2, so we need to go to a higher value, 
    //(because we start at i =2, there are the same amout of opperations)
        uint16_t entry;
        if (i % 2 == 0) {//if i is even, then we need to effectivly add (using or |, since there should be no overlap) 
        /*there are 1.5 the amount of indicies compared to actual 8 bit bytes so for even numbers, we take 1.5* i (= 1+i/2 to get an integer) for 
        the first part, which is the whole byte we need, then we take the next half byte. 
        if we are looking for cluster 4, then we consider */
            entry = (fat[i + i / 2] | ((fat[i + i / 2 + 1] & 0x0F/*only consider the second half (the most significant bits)*/) << 8)); //shifting to 
            //turn 8 biy bytes into a 12 bit thing but putting it at the beginning, which makes it into the least significant byte in little Endian. 
            //This algo seems wrong, like it would put things in the wrong order, but it seems to work. 
        } else {
            entry = ((fat[i + i / 2] >> 4) /*shift this right by 4, removing the right 4 bits*/| (fat[i + i / 2 + 1] << 4)/*take the whole next byte, and move it left to make space for the half byte*/);
        }
        if (entry == 0x000) {
            ++freeClusters;
        }
    }
    return freeClusters;
}

void findAndPrintDiskLabel(struct BootSector* bootSector, struct DirectoryEntry *rootDir ){
    char volumeLabel[12]; // 11 bytes for label + 1 for null terminator
    int found = 0;

    for (uint32_t i = 0; i < bootSector->BPB_RootEntCnt; i++) {
        if (rootDir[i].DIR_Attr == 0x08) { // Attribute for volume label
            memcpy(volumeLabel, rootDir[i].DIR_Name, 11);
            volumeLabel[11] = '\0'; // Null-terminate the string
            found = 1;
            break;
        }
    }
    if (found) {
        printf(" [%s]\n", volumeLabel);
    } else {
        printf("Volume Label not found in root directory\n");
    }

}

void print_date_time(struct DirectoryEntry *entry) {
    int time, date;
    int hours, minutes, day, month, year;

    time = entry->DIR_CrtTime;
    date = entry->DIR_CrtDate;

    // The year is stored as a value since 1980
    // The year is stored in the high seven bits
    year = ((date & 0xFE00) >> 9) + 1980;
    // The month is stored in the middle four bits
    month = (date & 0x1E0) >> 5;
    // The day is stored in the low five bits
    day = (date & 0x1F);

    printf("%d-%02d-%02d ", year, month, day);
    // The hours are stored in the high five bits
    hours = (time & 0xF800) >> 11;
    // The minutes are stored in the middle 6 bits
    minutes = (time & 0x7E0) >> 5;

    printf("%02d:%02d\n", hours, minutes);
}

void listDirectory(FILE *fp, struct BootSector *bootSector, struct DirectoryEntry *dir, uint32_t dirSize, uint8_t subdirName[], int isRoot, int depth) {
    for(int t =0; t < depth; t ++){
        printf("\t"); 
    }if (isRoot) {
        printf("Root Directory\n");
    }else {
        printf("Subdirectory ");
        PrintFileName(subdirName); 
    }
    for(int t =0; t < depth; t ++){
        printf("\t"); 
    }
    printf("==================\n");
    /*Broad algorithm
    loop over every entry in the root directory, finding and printing the names of each file there. If the are 
    a file, then just list the file size (also in the directory entry), it the entry is a directory, indicated 
    by the attribute bit number 10 being on, then we need to call list directories on that one. However,
    we shold be careful to not print out directories that are the root or self, ie if the name of the 
    "sub directory" is . or 0x2E, then we should skip it. 
    
    */
// (dirsize ==0 ? (dir[i].DIR_Name[0] != 0x00), (i < dirSize / sizeof(struct DirectoryEntry)))
    for (uint32_t i = 0; dir[i].DIR_Name[0] != 0x00 ; ++i) {
        if(inTestingMode){
            printf("        This is iteration %d of listDirectories for-loop\n", i); 
        }
        if (dir[i].DIR_Name[0] == 0x00) {
            break;  // No more entries in this directory
        }
        if (dir[i].DIR_Name[0] == 0xE5 || (dir[i].DIR_Attr & 0x0F) == 0x0F) {
            continue;  // Deleted entry or long file name entry
        }
        if (dir[i].DIR_FstClusLO == 0 || dir[i].DIR_FstClusLO == 1) {
                continue;  // Skip this directory entry
            }
        if (dir[i].DIR_Attr & 0x10) { // Directory
            
            if(dir[i].DIR_Name[0] ==0x2E){
                if(inTestingMode){
                printf("Found self directory or parent directory. Don't count this\n");
                }
                continue; 
            }
            for(int t =0; t < depth; t ++){
                printf("\t"); 
            }
            char name[12];
            memcpy(name, dir[i].DIR_Name, 11);
            name[11] = '\0';
            printf("D %10s %20s", "-", name);
            print_date_time(&dir[i]); 
            //Must find a way to call this after all the files have been found. 



            uint16_t firstCluster = dir[i].DIR_FstClusLO;
            if (firstCluster == 0) {
                continue;
            }
            uint32_t subDirOffset = ((firstCluster - 2) * bootSector->BPB_SecPerClus + 33) * bootSector->BPB_BytsPerSec;
            
            // if(inTestingMode){
            //     printf("Offset is %ld\n",( bootSector->BPB_RsvdSecCnt + bootSector->BPB_NumFATs * bootSector->BPB_FATSz16 /*Sectors per fat*num fats*/ + bootSector->BPB_RootEntCnt /*entries in root dir*/ * sizeof(struct DirectoryEntry) / bootSector->BPB_BytsPerSec) /** bootSector->BPB_BytsPerSec*/);
            // }
            struct DirectoryEntry *subDir = malloc(bootSector->BPB_SecPerClus * bootSector->BPB_BytsPerSec);
            fseek(fp, subDirOffset, SEEK_SET);
            fread(subDir, bootSector->BPB_SecPerClus * bootSector->BPB_BytsPerSec, 1, fp);
            //TODO MUST FIND A WAY TO CONTINUE READING IF THE DIRECTORY IS LONGER THAN ONE SECTOR AND WE NEED TO START READING FROM MORE.  Use the fat table to find the next data index
            if(inTestingMode){
                printf("        Calling listDir with directory with size of %d, which is %ld entries\n", dir[i].DIR_FileSize,dir[i].DIR_FileSize/sizeof(struct DirectoryEntry)); 
            }
            listDirectory(fp, bootSector, subDir, dir[i].DIR_FileSize, dir[i].DIR_Name, 0, depth +1);





        } else { // Regular file
            if(inTestingMode){
                if(dir[i].DIR_FileSize==0){ 
                printf("The size is 0, something is wrong");
                    if(isRoot){
                        printf(" in the root directory\n");
                    }else{
                        printf(" in a subdirectory"); 
                    }
                if( dir[i].DIR_Attr & 0x02){
                    printf("This file is hidden\n");
                }
                }
            }

            /*Notes on time formatting:
            Date (2 bytes)

Bits 0-4: Day of the month (1-31)
Bits 5-8: Month (1 = January, 2 = February, ..., 12 = December)
Bits 9-15: Year (offset from 1980, i.e., 0 = 1980, 1 = 1981, ..., 127 = 2107)
Time (2 bytes)

Bits 0-4: 2-second count (0-29, representing seconds 0, 2, 4, ..., 58)
Bits 5-10: Minutes (0-59)
Bits 11-15: Hours (0-23)
*/
            char name[12];
            memcpy(name, dir[i].DIR_Name, 11);
            name[11] = '\0';
            struct tm t;
            uint16_t date = dir[i].DIR_CrtDate;
           
            uint16_t time = dir[i].DIR_CrtTime;
             if(inTestingMode){
                printf("Date is %0X in hex and %d in decimal\n", date, date); 
                printf("time is %0X in hex and %d in decimal\n", time, time); 

            }
            t.tm_sec = (dir[i].DIR_CrtTime & 0x1F) * 2;
            t.tm_min = (dir[i].DIR_CrtTime >> 5) & 0x3F;
            t.tm_hour = (dir[i].DIR_CrtTime >> 11) & 0x1F;


            t.tm_mday = dir[i].DIR_CrtDate & 0x1F;
            t.tm_mon = (date & 0x1E0) >> 5;
            // t.tm_year = ((dir[i].DIR_CrtDate >> 9) & 0x7F) + 1980; // Bits 9-15
            t.tm_year = ((date & 0xFE00) >> 9) + 1980;
	//the month is stored in the middle four bits
	// t.month = (date & 0x1E0) >> 5;
	//the day is stored in the low five bits
	// day = (date & 0x1F);
            t.tm_isdst = -1;
            char dateBuf[20];
            if(inTestingMode){
                printf("The year currently is %d\n", t.tm_year);
            }
            strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d %H:%M:%S", &t);
            for(int t =0; t < depth; t ++){
                printf("\t"); 
            }
            // printf("F %10u %20s %s\n", dir[i].DIR_FileSize, name, dateBuf);
            printf("F %10u %20s ", dir[i].DIR_FileSize, name);
                        print_date_time(&dir[i]); 

        }
        
    }
    for(int t =0; t < depth; t ++){
                printf("\t"); 
            }
    if (isRoot) {
        printf("End of Root Directory");
    }else {
        printf("End of ");
        PrintFileName(subdirName); 
    }
    for(int t =0; t < depth; t ++){
        printf("\t"); 
    }
    printf("~~~~~~~~~~~~~~~~~~~~~~~~\n");
 
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DISKGET specific functions


