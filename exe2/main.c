#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef unsigned char b1; // 1 byte
typedef unsigned short b2; // 2 bytes
typedef unsigned char b3[3]; // 3 bytes
typedef unsigned int b4; // 4 bytes
typedef double b8; // 8 bytes
typedef unsigned char b10[10]; // 10 bytes
typedef unsigned char b11[11]; // 11 bytes

#pragma pack (1)

struct {
    b3 BS_jmpBOOT;
    b8 BS_OEMName;
    b2 BPB_BytesPerSec;
    b1 BPB_SecPerClus;
    b2 BPB_ResvdSecCnt;
    b1 BPB_NumFATs;
    b2 BPB_RootEntCnt;
    b2 BPB_TotSec16;
    b1 BPB_Media;
    b2 BPB_FATSz16;
    b2 BPB_NumHeads;
    b4 BPB_HiddSec;
    b4 BPB_TotSec32;
    b1 BS_DrvNum;
    b1 BS_Reserved1;
    b1 BS_BootSig;
    b1 BS_VollD;
    b11 BS_VolLab;
    b8 BS_FileSysType;
} BSB;

struct {
    b11 DIR_Name;
    b1 DIR_Attr;
    b10 DIR_Reserved;
    b2 DIR_WrtTime;
    b2 DIR_WrtDate;
    b2 DIR_FstClus;
    b4 DIR_FileSize;
} DIR;

#pragma pack ()
// physical sector number = 33 + FAT entry number - 2

char WARNING_INVALID_COMMAND[] = {"Command entered invalid!\n"};
char DEFAULT_PATH[] = {"defaultpath"};

// should check if the file is a dir
void handleCat(const char * filename){
    printf("cat function\n");
    printf("filename: %s\n", filename);
}

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

void invalidCmdWarning(const char * cmd){
    printf("%s",WARNING_INVALID_COMMAND);
    printf("You entered: %s", cmd);
}

void invalidParamForLs(const char * param){
    printf("Your param for ls command is invalid!\n");
    printf("You entered %s\n", param);
}

void invalidFilename(const char * filename){
    printf("Your filename input is invalid!\n");
    printf("You entered %s\n", filename);
}

bool isFilenameHead(char c){
    return (c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 112) 
    || (c == '.') || (c == '/');
}

int main(){
    char prompt[] = {"Please enter your command:\n"};
    char input[1024];
    printf("%s", prompt);
    fgets(input, 1023, stdin);
    while(strcmp(input, "exit\n")!=0){
        //use another variable to hold input, which makes code more understandable
        char command[1024];
        char filename[1024] = {};
        char param[1024] = {};
        strcpy(command, input);

        //pre-process the input, and detect undefined command
        //also filter out the filename, either explicit or default
        char commandType[4] = {'\0','\0','\0','\0'};
        int i = 0;
        for(; command[i] != ' ' && command[i] != '\n' && i < 4; i++){
            commandType[i] = command[i];
        }

        // in case user inputs multiple space
        while(command[i] == ' '){
            i++;
        }
        for(int j = 0; command[i] != ' ' 
            && command[i] != '\n' && command[i] != '-'; i++, j++){
                if(j == 0 && isFilenameHead(command[i])){
                    filename[j] = command[i];
                }
                else{
                    filename[j] = command[i];
                }
            }
        if(filename[0] == '\0'){
            strcpy(filename, DEFAULT_PATH);
        }

        //finally check if there is param -l...
        while(command[i] == ' '){
            i++;
        }
        for(int j = 0; command[i] != '\n'; i++, j++){
            param[j] = command[i];
        }

        if(commandType[3] != '\0'){
            //since max length of main command is 3 -> cat
            invalidCmdWarning(command);
            printf("%s", prompt);
            fgets(input, 1023, stdin);
            continue;
        }

        if(strcmp(commandType, "cat")==0){
            handleCat(filename);
        }
        else if(strcmp(commandType, "ls")==0){
            //check if there are filepath and param
            if(param[0] == '\0'){
                handleLs(filename);
            }
            else{
                int validParam = 1;
                //ensure that the heading part is correct
                if(param[0] == '-' && param[1] == 'l'){
                    for(int k = 2; param[k] != '\0'; k++){
                        char c = param[k];
                        if(c!='-' && c!='l' && c!=' '){
                            validParam = 0;
                            break;
                        }
                    }
                }
                else{
                    validParam = 0;
                }

                if(!validParam){
                    invalidParamForLs(param);
                }
                else{
                    handleLsWithParam(filename);
                }
            }
        }
        else{
            invalidCmdWarning(command);
        }

        printf("%s", prompt);
        fgets(input, 1023, stdin);
    }

    return 0;
}