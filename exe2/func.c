#include "func.h"

int  BytesPerSec;
int  SecPerClus;
int  ResvdSecCnt;
int  NumFATs;
int  RootEntCnt;
int  FATSz;

int  RootEntBase;
int  DataBase;

void loadFAT12(FILE * FAT12, struct BPB * bpb_ptr){
    if(fseek(FAT12, 0, SEEK_SET)==-1){
        printf("Failure occured while seeking img!\n");
    }
    // 57 bytes in total
    if(fread(bpb_ptr, 1, 57, FAT12)==-1){
        printf("Failure occured while reading FAT12!\n");
    }

    // initialize global vals so that using them is easy
	BytesPerSec = bpb_ptr->BPB_BytesPerSec;
	SecPerClus = bpb_ptr->BPB_SecPerClus;
	ResvdSecCnt = bpb_ptr->BPB_ResvdSecCnt;
	NumFATs = bpb_ptr->BPB_NumFATs;
	RootEntCnt = bpb_ptr->BPB_RootEntCnt;
	FATSz = bpb_ptr->BPB_FATSz16;

    RootEntBase  = (ResvdSecCnt + NumFATs * FATSz) * BytesPerSec;
    DataBase = BytesPerSec * ( ResvdSecCnt + FATSz*NumFATs + (RootEntCnt*32 + BytesPerSec - 1)/BytsPerSec );
}

//-------------------------------------------------------------------------------

// should check if the file is a dir
void handleCat(const char * filename, FILE * fat12 , struct DIR * rootEntry_ptr){
    // check if the filename contains layers. i.e. /nju/software/se1.txt
    int pathIndex = 0;
    char filepath[128] = {};
    if(filename[0] == '.' && filename[1] == '.'){
        printf("Unsupported way of finding path!\n");
        return;
    }
    else if(filename[0] == '.'){
        // locate the index on the '/' position
        pathIndex++;
    }
    else{
        pathIndex = 0;
    }

    for(int i = 0; filename[pathIndex] != '\0'; i++, pathIndex++){
        filepath[i] = filename[pathIndex];
    }

    char currentpath[128] = {};
    char restpath[128] = {};
    char targetfile[128] = {};
    strcpy(currentpath, filepath);
    while(1){
        char currentdir [128] = {};
        int i = 0;
        if(currentpath[0] == '/'){
            i = 1;
        }
        int j = 0;
        while(currentpath[i] != '/' && currentpath[i] != '\0'){
            currentdir[j] = currentpath[i];
            i++; j++;
        }
        currentdir[j] = '\0';

        j = 0;
        for(; currentpath[i] != '\0'; i++, j++){
            restpath[j] = currentpath[i];
        }
        restpath[j] = '\0';

        // handle the seeking of dir of file


        strcpy(currentpath, restpath);
        if(currentpath[0] == '\0'){
            strcpy(targetfile, currentdir);
            break;
        }
    }
    // having gotten the target file name
    printf("%s", targetfile);
}

//-----------------------------------------------------------------------------------

// should check if the file is a dir
void handleLs(const char * filename){
    printf("ls function without -l\n");
    printf("filename: %s\n", filename);
}

// should check if the file is a dir
void handleLsWithParam(const char * filename){
    printf("ls function with -l\n");
    printf("filename: %s\n", filename);
}