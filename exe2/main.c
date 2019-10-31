#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char b1; // 1 byte
typedef unsigned short b2; // 2 bytes
typedef unsigned char b3[3]; // 3 bytes
typedef unsigned int b4; // 4 bytes
typedef double b8; // 8 bytes
typedef unsigned char b10[10]; // 10 bytes
typedef unsigned char b11[11]; // 11 bytes

#pragma pack (1)

struct BPB{
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
};

struct DIR{
    b11 DIR_Name; // useful, 8 bytes for name, 3 bytes for extension
    b1 DIR_Attr; // useful, file attribute, 0 for file, else dir
    b10 DIR_Reserved;
    b2 DIR_WrtTime;
    b2 DIR_WrtDate;
    b2 DIR_FstClus; // useful, start clus no
    b4 DIR_FileSize; // useful
};

#pragma pack ()
//int startByte = dataBase + (currentClus - 2)*SecPerClus*BytsPerSec;
// physical sector number = 33 + FAT entry number - 2

int  BytesPerSec;
int  SecPerClus;
int  ResvdSecCnt;
int  NumFATs;
int  RootEntCnt;
int  FATSz;

int  RootEntBase;
int  DataBase;

void redprint(const char *, int);
void defprint(const char *, int);
//--------------------------------------------------------------------------------------------

char WARNING_INVALID_COMMAND[] = {"Command entered invalid!\n"};

void invalidCmdWarning(const char * cmd);

void invalidParamForLs(const char * param);

void invalidFilename(const char * filename);

int isFilenameHead(char c);

//--------------------------------------------------------------------------------------------


int isValidElement(char c);

int  getFATValue(FILE * fat12 , int num, int forcat);

void loadFAT12(FILE * FAT12, struct BPB * bpb_ptr);

//-------------------------------------------------------------------------------

// should check if the file is a dir
void handleCat(const char * filename, FILE * fat12 , struct DIR * rootEntry_ptr);

//-----------------------------------------------------------------------------------


void printRecurse(int startclus, FILE * FAT12, char * parent);

// should check if the file is a dir
void handleLs(const char * filename, FILE * fat12 , struct DIR * rootEntry_ptr);


//----------------------------------------------------------------------------------------------------

void printRecurseWithParam(int startClus, FILE * FAT12, char * parent);

//----------------------------------------------------------------------------------------------------
// should check if the file is a dir
void handleLsWithParam(const char * filename, FILE * FAT12, struct DIR * rootEntry_ptr);
//---------------------------------------------------------------------------------------------------


char DEFAULT_PATH[] = {"/"};

int main(){

    // load img
    FILE * FAT12;
    FAT12 = fopen("a.img", "rb");
    struct BPB bpb;
    struct BPB * bpb_ptr = &bpb;
    struct DIR rootEntry;
    struct DIR * rootEntry_ptr = &rootEntry;

    loadFAT12(FAT12, bpb_ptr);

    char prompt[] = {"Please enter your command:\n"};
    char input[128] = {};
    //defprint(prompt,strlen(prompt));
    printf("%s", prompt);
    fgets(input, 127, stdin);
    while(strcmp(input, "exit\n")!=0){
        //use another variable to hold input, which makes code more understandable
        char command[128];
        char filename[128] = {};
        char param[128] = {};
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
            if(strcmp(filename, DEFAULT_PATH)==0){
                printf("Please enter a file name!\n");
            }
            else{
                handleCat(filename, FAT12, rootEntry_ptr);
            }
        }
        else if(strcmp(commandType, "ls")==0){
            //check if there are filepath and param
            if(param[0] == '\0'){
                handleLs(filename, FAT12, rootEntry_ptr);
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
                    handleLsWithParam(filename, FAT12, rootEntry_ptr);
                }
            }
        }
        else{
            invalidCmdWarning(command);
        }

        printf("%s", prompt);
        fgets(input, 127, stdin);
    }

    return 0;
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

int isFilenameHead(char c){
    return (c >= 48 && c <= 57) || (c >= 65 && c <= 90) || (c >= 97 && c <= 112) 
    || (c == '.') || (c == '/');
}

//--------------------------------------------------------------------------------------------


int isValidElement(char c){
    if (!(((c >= 48)&&(c <= 57)) ||
		    ((c >= 65)&&(c <= 90)) ||
			((c >= 97)&&(c <= 122)) ||
			(c == ' '))){
        return 0;
    }
    else{
        return 1;
    }
}

int  getFATValue(FILE * fat12 , int num, int forcat) {
    if(!forcat){
        int fatBase = ResvdSecCnt * BytesPerSec;
        int fatPos = fatBase + num*3/2;
        int type = 0;
        if (num % 2 == 0) {
            type = 0;
        } else {
            type = 1;
        }
    
        b2 bytes;
        b2* bytes_ptr = &bytes;
        int check;
        check = fseek(fat12,fatPos,SEEK_SET);
        if (check == -1) 
            printf("fseek in getFATValue failed!");
    
        check = fread(bytes_ptr,1,2,fat12);
        if (check != 2)
            printf("fread in getFATValue failed!");
    
        if (type == 0) {
            return bytes<<4;
        } else {
            return bytes>>4;
        }
    }
    else{
        int fatBase = ResvdSecCnt * BytesPerSec;
        int fatPos = fatBase + num*3/2;
        int type = 0;
        if (num % 2 == 0) {
            type = 0;
        } else {
            type = 1;
        }
    
        b2 bytes;
        b2* bytes_ptr = &bytes;
        int check;
        check = fseek(fat12,fatPos,SEEK_SET);
        if (check == -1) 
            printf("fseek in getFATValue failed!");
    
        check = fread(bytes_ptr,1,2,fat12);
        if (check != 2)
            printf("fread in getFATValue failed!");
    
        if (type == 0) {
            short correct = (bytes<<4);
		    return correct>>4;
        } else {
            return bytes>>4;
        }
    }
}

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
    DataBase = BytesPerSec * ( ResvdSecCnt + FATSz*NumFATs + (RootEntCnt*32 + BytesPerSec - 1)/BytesPerSec );
}

//-------------------------------------------------------------------------------

// should check if the file is a dir
void handleCat(const char * filename, FILE * fat12 , struct DIR * rootEntry_ptr){
    // check if the filename contains layers. i.e. /nju/software/se1.txt
    int tempBase = RootEntBase;
    int pathIndex = 0;
    char filepath[128] = {};
    int isFirstLevel = 1;
    int fileFound = 1;
    int index;
    int targetClus = -1;
    int startClus = -1;
    int currentClus = -1;

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

        if(!fileFound){
            printf("File not found!\n");
            return;
        }

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

        if(restpath[0] == '\0'){
            strcpy(targetfile, currentdir);
            break;
        }
//----------------------------------------------------------------------------------
        // handle the seeking of dir or file
        if(isFirstLevel){
            for(index=0; index<RootEntCnt; index++){
                if(fseek(fat12,tempBase,SEEK_SET)==-1){
                    printf("fail to seek target location!\n");
                    return;
                }
                if(fread(rootEntry_ptr,1,32,fat12)!=32){
                    printf("RootEntry read failed! Size is not 32 bytes!\n");
                    return;
                }
                tempBase += 32;
                if(rootEntry_ptr->DIR_Name[0] == '\0'){
                    continue;
                }
                int nameInd = 0;
                int invalidName = 0;
                for(;nameInd < 11; nameInd++){
                    if(!isValidElement(rootEntry_ptr->DIR_Name[nameInd])){
                        invalidName = 1;
                        break;
                    }
                }
                if(invalidName){
                    continue;
                }
                int k = 0;
                if((rootEntry_ptr->DIR_Attr&0x10) == 0){
                    // it's a file, not a dir, should jump over
                    continue;
                }
                else{
                    int tempLen = -1;
                    char cmpname[128] = {};
                    for(;k < 11; k++){
                        if(rootEntry_ptr->DIR_Name[k] != ' '){
                            tempLen++;
                            cmpname[tempLen] = rootEntry_ptr->DIR_Name[k];
                        }
                        else{
                            tempLen++;
                            cmpname[tempLen] = '\0';
                            break;
                        }
                    }
                    if(strcmp(cmpname, currentdir)==0){
                        //find the first level dir
                        isFirstLevel = 0;
                        targetClus = rootEntry_ptr->DIR_FstClus;
                        break;
                    }
                }
            }
            fileFound = (targetClus == -1) ? 0 : 1;
        }else {
            startClus = targetClus;
            currentClus = startClus;
            int fatVal = 0;
            int tempDataBase = DataBase;
            while(fatVal < 0xFF8){
                if(currentClus < 0){
                    printf("File not found!\n");
                    return;
                }
                fatVal = getFATValue(fat12, currentClus, 0);
                if(fatVal == 0xFF7){
                    printf("Reading encountered bad clus!\n");
                    return;
                }

                char * tempStr = (char *)malloc(SecPerClus*BytesPerSec);
                char * tempContent = tempStr;

                int startByte = tempDataBase + (currentClus - 2) * SecPerClus*BytesPerSec;
                if(fseek(fat12, startByte, SEEK_SET)==-1){
                    printf("Fail to seek in fat12!\n");
                    return;
                }
                if(fread(tempContent, 1, SecPerClus*BytesPerSec, fat12) != SecPerClus*BytesPerSec){
                    printf("Bytes read in fat12 not enough!\n");
                    return;
                }
                int count = SecPerClus * BytesPerSec;
                int offset = 0;
                while(offset < count){
                    int secondi;
                    char cmpname2[128] = {};
                    if(tempContent[offset]=='\0'){
                        offset += 32; // next entry
                        continue;
                    }
                    int secondj;
                    int secondInvalidName = 0;
                    for(secondj = offset; secondj < offset+11; secondj++){
                        if(!isValidElement(tempContent[secondj])){
                            secondInvalidName = 1;
                            break;
                        }
                    }
                    if(secondInvalidName){
                        offset += 32;
                        continue;
                    }
                    int k=0;
                    int tempLen = -1;
                    for(; k < 11; k++){
                        if(tempContent[offset+k]!=' '){
                            tempLen++;
                            cmpname2[tempLen] = tempContent[offset+k];
                        }else{
                            tempLen++;
                            cmpname2[tempLen] = '\0';
                            break;
                        }
                    }
                    if(strcmp(currentdir, cmpname2)==0){
                        char calclus[2] = {};
                        for(int a=0; a < 2; a++){
                            calclus[a] = tempContent[offset+26+a];
                        }
                        targetClus = 256 * (int)calclus[1] + (int)calclus[0];
                        break;
                    }
                    offset+=32;
                }
                free(tempStr);

                if(startClus != targetClus){
                    break;
                }
                currentClus = fatVal;
            }
            fileFound = (targetClus == startClus) ? 0 : 1;
        }
//-----------------------------------------------------------------------------------
        strcpy(currentpath, restpath);
    }
    // having gotten the target file name
    int hasExt = 0;
    for(int test=0;targetfile[test]!='\0';test++){
        if(targetfile[test]=='.'){
            hasExt = 1;
        }
    }
    if(!hasExt){
        printf("Please indicate a common file, not directory!\n");
        return;
    }   

    int readableFileFound = 0;

    //start the last round seeking - find out the file's startClus
    if(isFirstLevel){
        // file in the root dir
        int base = (ResvdSecCnt+NumFATs*FATSz)*BytesPerSec;
        char fname[128]={};
        int i;
        for (i=0;i<RootEntCnt;i++) {
 
		fseek(fat12,base,SEEK_SET);
        fread(rootEntry_ptr,1,32,fat12);

		base += 32;
 
		if (rootEntry_ptr->DIR_Name[0] == '\0'){
            continue;
        }
 
		int j;
		int boolean = 0;
		for (j=0;j<11;j++) {
			if (!isValidElement(rootEntry_ptr->DIR_Name[j])) {
				boolean = 1;
				break;
			}
		}
		if (boolean == 1){
            continue;
        }
 
		int k;
		if ((rootEntry_ptr->DIR_Attr&0x10) == 0 ) {
			int tempLong = -1;
			for (k=0;k<11;k++) {
				if (rootEntry_ptr->DIR_Name[k] != ' ') {
					tempLong++;
					fname[tempLong] = rootEntry_ptr->DIR_Name[k];
				} else {
					tempLong++;
					fname[tempLong] = '.';
					while (rootEntry_ptr->DIR_Name[k] == ' ') k++;
					k--;
				}
			}
			tempLong++;
			fname[tempLong] = '\0';

            if(strcmp(targetfile, fname)==0){
                readableFileFound = 1;
                targetClus = rootEntry_ptr->DIR_FstClus;
                startClus = targetClus;
                currentClus = startClus;
            }

		} 
        else {
			continue;
		}
	    }
    }
    else{
        startClus = targetClus;
        currentClus = startClus;
    }
    int value = 0;
    while(value < 0xFF8 && !isFirstLevel){
        if(currentClus < 0){
            printf("File not found!\n");
            return;
        }
        value = getFATValue(fat12, currentClus, 0);
        if(value == 0xFF7){
            printf("Encountered a bad clus!\n");
            return;
        }
        char * str = (char*)malloc(SecPerClus*BytesPerSec);
        char * content = str;
        int tempDataBase = DataBase;
        int startByte = tempDataBase + (currentClus - 2)*SecPerClus*BytesPerSec;
        if(fseek(fat12,startByte,SEEK_SET)==-1){
            printf("Seeking problem!\n");
            return;
        }
        if(fread(content, 1, SecPerClus*BytesPerSec, fat12)!=SecPerClus*BytesPerSec){
            printf("Reading problem!\n");
            return;
        }
        int count = SecPerClus*BytesPerSec;
        int loop = 0;
        while(loop < count){
            int i;
            char tempfilename[128] = {};
			if (content[loop] == '\0') {
				loop += 32;
				continue;
			}
			int j;
			int boolean = 0;
			for (j=loop;j<loop+11;j++) {
				if (!isValidElement(content[j])) {
					boolean = 1;
					break;
				}	
			}
			if (boolean == 1) {
				loop += 32;
				continue;
			}
			int k;
			int tempLong = -1;
			for (k=0;k<11;k++) {
				if (content[loop+k] != ' ') {
					tempLong++;
					tempfilename[tempLong] = content[loop+k];
				} else {
					tempLong++;
					tempfilename[tempLong] = '.';
					while (content[loop+k] == ' ') k++;
					k--;
				}
			}
			tempLong++;
			tempfilename[tempLong] = '\0';

            if(strcmp(targetfile, tempfilename)==0){
                fileFound = 1;
                char calclus[2] = {};
                for(int a=0; a < 2; a++){
                    calclus[a] = content[loop+26+a];
                }
                targetClus = 256 * (int)calclus[1] + (int)calclus[0];
                char attr = content[loop+11];
                if((attr&0x10)==0){
                    readableFileFound = 1;
                }
                break;
            }

			loop += 32;
        }
        currentClus = value;
    }
    // now having gotten the target file's startClus
    currentClus = targetClus;

    if(!readableFileFound){
        printf("File not found!\n");
        return;
    }

    while (1) {
		value = getFATValue(fat12,currentClus,1);
		if (value == 0xFF7) {
			break;
		}

		char* str = (char* )malloc(SecPerClus*BytesPerSec);	//暂存从簇中读出的数据
		char* content = str;
		
		int startByte = DataBase + (currentClus - 2)*SecPerClus*BytesPerSec;
		fseek(fat12,startByte,SEEK_SET);
		fread(content,1,SecPerClus*BytesPerSec,fat12);
				
        char txt[513] = {};
        int id = 0;
        for(; id < 512 && content[id] != '\0'; id++){
            txt[id] = content[id];
        }
        txt[id] = '\0';
        printf("%s", txt);

		free(str);
		currentClus = value;

		if(value>= 0xFF8 || currentClus < 0){
			break;
		}
    }


/*
    char * str = (char *)malloc(SecPerClus*BytesPerSec);
    char * content = str;
    int physicalSecNo = 33 + targetClus - 2;
    int coffset = physicalSecNo * SecPerClus * BytesPerSec;
    fseek(fat12,coffset,SEEK_SET);
    fread(content, 1, SecPerClus*BytesPerSec, fat12);
    char txt[513] = {};
    int id = 0;
    for(; id < 512 && content[id] != '\0'; id++){
        txt[id] = content[id];
    }
    txt[id] = '\0';
    printf("%s", txt);
    */
}

//-----------------------------------------------------------------------------------


void printRecurse(int startclus, FILE * FAT12, char * parent){
    printf("%s:\n",parent);
    //red here!!!!!!!!!!!!!!!!!!!!!!!
    printf("\033[31m.  ..  \033[0;0m");
    //defprint("\033[31m.  ..  ",13);

    //as in handleLs, create arr to store dirs and clus
    int dictind = 0;
    char * subdirs[224] = {};
    int subclus[224] = {};

    int dataBase = DataBase;
    int currentClus = startclus;
    int val = 0;
    while(val < 0xFF8){
        val = getFATValue(FAT12, currentClus, 0);
        if(val == 0xFF7){
            printf("Encountered bad clus!\n");
            return;
        }
        char * str = (char*)malloc(SecPerClus*BytesPerSec);
        char* content = str;

        int startByte = dataBase + (currentClus - 2)*SecPerClus*BytesPerSec;
        fseek(FAT12, startByte, SEEK_SET);
        fread(content,1,SecPerClus*BytesPerSec,FAT12);

        int count = SecPerClus*BytesPerSec;
        int loop=0;
        while(loop<count){
            char tempName[12] = {};
            if(content[loop]=='\0'){
                loop+=32;
                continue;
            }
            int boolean=0;
            for(int j=loop; j<loop+11; j++){
                if(!isValidElement(content[j])){
                    boolean=1;
                    break;
                }
            }
            if(boolean){
                loop+=32;
                continue;
            }
            int k;
            int tempLong = -1;

            char attr = content[loop+11];
            if((attr&0x10)==0){
                // it's a file, should only print
                for(k=0; k<11; k++){
                    if(content[loop+k]!=' '){
                        tempLong++;
                        tempName[tempLong] = content[loop+k];
                    }else{
                        tempLong++;
                        tempName[tempLong] = '.';
                        while(content[loop+k]==' '){
                            k++;
                        }
                        k--;
                    }
                }
                printf("%s  ", tempName);
            } else{
                // it's a dir, save it and print it
                for(k=0; k<11; k++){
                    if(content[loop+k]!=' '){
                        tempLong++;
                        tempName[tempLong] = content[loop+k];
                    }else{
                        break;
                    }
                }
                char * tosave = (char*)malloc(12);
                strcpy(tosave, tempName);
                char calclus[2] = {};
                for(int a=0; a < 2; a++){
                    calclus[a] = content[loop+26+a];
                }
                subclus[dictind] = 256 * (int)calclus[1] + (int)calclus[0];
                subdirs[dictind] = tosave;
                dictind++;
                //redprint(tempName, strlen(tempName));
                //defprint("  ", 2);
                //red print here!!!!!!!!!!!!!!!!
                printf("\033[31m");
                printf("%s  ", tempName);
                printf("\033[0;0m");
            }
            loop+=32;
        }
        free(str);
        currentClus = val;
    }
    printf("\n");
    for(int i=0; i<dictind; i++){
        char firstline[128] = {};
        strcpy(firstline, parent);
        int parentlen = strlen(firstline);
        char * tempsub = subdirs[i];
        int n=0;
        for(; tempsub[n] != '\0'; n++){
            firstline[n+parentlen] = tempsub[n];
        }
        firstline[n+parentlen] = '/';
        printRecurse(subclus[i], FAT12, firstline);
    }

    for(int i=0; i<dictind; i++){
        char * f = subdirs[i];
        free(f);
    }

    return;
}

// should check if the file is a dir
void handleLs(const char * filename, FILE * fat12 , struct DIR * rootEntry_ptr){

    int noFilenameInput = 0;
    if(strcmp(filename, "/")==0){
        noFilenameInput = 1;
    }
    char firstline[128] = {};

    if(noFilenameInput || (filename[0]=='.'&&filename[1]=='\0')){
        firstline[0] = '/';
        noFilenameInput = 1; // if input .
        printf("/:\n");
    }
    else{
        //three kinds -> ./nju/blabla, nju/blabla, /nju/blabla
        if(filename[0] == '.'){
            int i=0;
            for(int j=1; filename[j] != '\0'; j++, i++){
                firstline[i] = filename[j];
            }
            firstline[i] = '/';
        }
        else if(filename[0] != '/'){
            firstline[0] = '/';
            int i=1;
            for(int j=0; filename[j]!='\0'; j++){
                firstline[i] = filename[j];
                i++;
            }
            firstline[i] = '/';
        }
        else{
            int i=0;
            for(int j=0; filename[j]!='\0'; j++,i++){
                firstline[i] = filename[j];
            }
            firstline[i] = '/';
        }
    }

    
    int t = 0; //this is used for cutting the input path
    char * targetdirs[128] = {};
    int level = 0;
    for(int i=1; firstline[i] != '\0'; i++){
        char * temptarget = (char*)malloc(12);
        int j=0;
        while(firstline[i] != '/'){
            temptarget[j] = firstline[i];
            i++; j++;
        }
        temptarget[j] = '\0';
        targetdirs[level] = temptarget;
        level++;
    }

    //the dirs variable store many with malloc var
    //remember to free all of them after the printing
    int dictIndex = 0;
    char * dirs[224] = {};
    int clus[224] = {};
    int currentlevel = 0;

    //start from root entry
    int rootbase = RootEntBase;
    char fname [12] = {};

    //first seek out entries in the root level
    for(int i = 0; i<RootEntCnt; i++, rootbase+=32){
        fseek(fat12, rootbase, SEEK_SET);
        fread(rootEntry_ptr, 1, 32, fat12);

        if(rootEntry_ptr->DIR_Name[0]=='\0'){
            continue;
        }
        int invalidName = 0;
        for(int j=0; j<11; j++){
            if(!isValidElement(rootEntry_ptr->DIR_Name[j])){
                invalidName = 1;
                break;
            }
        }
        if(invalidName){
            continue;
        }
        char * tempname = (char *)malloc(12);
        int j = 0; int k=0;
        for(;j<11 && rootEntry_ptr->DIR_Name[j]!=' '; j++, k++){
            tempname[k] = rootEntry_ptr->DIR_Name[j];
        }
        if((rootEntry_ptr->DIR_Attr&0x10)!=0){
            // is a dir, needs to be in the dict
            tempname[k] = '\0';
            dirs[dictIndex] = tempname;
            clus[dictIndex] = rootEntry_ptr->DIR_FstClus;
            dictIndex++;
            if(noFilenameInput){
                //redprint(tempname, strlen(tempname));
                //defprint("  ",2);
                //print red here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                printf("\033[31m");
                printf("%s  ", tempname);
                printf("\033[0;0m");
            }
        }
        else{
            if(noFilenameInput){
                while(rootEntry_ptr->DIR_Name[j] == ' '){
                    j++;
                }
                tempname[k] = '.'; k++;
                for(;j < 11 && rootEntry_ptr->DIR_Name[j] != ' '; j++, k++){
                    tempname[k] = rootEntry_ptr->DIR_Name[j];
                }
                printf("%s  ", tempname);
            }
            free(tempname);
        }
    }
    if(noFilenameInput){
        printf("\n");
    }

    if(noFilenameInput){
        for(int n=0; n<dictIndex; n++){
            char currentdir[12] = {};
            strcpy(currentdir, dirs[n]);
            char parampath[128] = {};
            strcpy(parampath, firstline);
            int pathlen = strlen(parampath);
            int i=0;
            for(; currentdir[i] != '\0'; i++){
                parampath[i+pathlen] = currentdir[i];
            }
            parampath[i+pathlen] = '/';
            printRecurse(clus[n], fat12, parampath);
        }
    }
    else{
        // start finding level by level
        // dir stored in targetdirs
        //dirs stores first level dir
        //clus stores first level clus
        //dictInd represent the size
        int targetclus = -1;
        for(int i=0; i<level; i++){
            int pathexist = 0;
            char currenttarget[12] = {};
            strcpy(currenttarget, targetdirs[i]);
            if(i == 0){
                for(int n=0; n<dictIndex; n++){
                    if(strcmp(currenttarget, dirs[n])==0){
                        pathexist = 1;
                        targetclus = clus[n];
                        break;
                    }
                }
            }else{
                int database = DataBase;
                int currentClus = targetclus;
                int fatval = 0;
                while(fatval < 0xFF8){
                    fatval = getFATValue(fat12, currentClus, 0);
                    if(fatval == 0xFF7){
                        printf("Encountered bad clus!\n");
                        return;
                    }
                    char * str = (char*)malloc(SecPerClus*BytesPerSec);
                    char * content = str;
                    int startByte = database + (currentClus-2)*SecPerClus*BytesPerSec;
                    fseek(fat12,startByte,SEEK_SET);
                    fread(content, 1, SecPerClus*BytesPerSec, fat12);
                    int count = SecPerClus*BytesPerSec;
                    int loop=0;
                    while(loop<count){
                        int i;
                        char tempName[12]={};
                        if(content[loop]=='\0'){
                            loop+=32;
                            continue;
                        }
                        int invalidName = 0;
                        for(int j=loop; j<loop+11; j++){
                            char test = content[j];
                            if(!isValidElement(content[j])){
                                invalidName = 1;
                                break;
                            }
                        }
                        if(invalidName){
                            loop+=32;
                            continue;
                        }
                        //DIR_Attr offset is 12
                        char attr = content[loop+11];
                        if((attr&0x10)==0){
                            // it's a file, should jump over
                            loop+=32;
                            continue;
                        }
                        else{
                            int k;
                            int tempLong = -1;
                            for(k=0; k<11; k++){
                                if(content[loop+k]!=' '){
                                    tempLong++;
                                    tempName[tempLong] = content[loop+k];
                                } else{
                                    tempLong++;
                                    tempName[tempLong] = '\0';
                                    break;
                                }
                            }
                            if(strcmp(currenttarget, tempName)==0){
                                pathexist = 1;
                                char calclus[2] = {};
                                for(int a=0; a < 2; a++){
                                    calclus[a] = content[loop+26+a];
                                }
                                targetclus = 256 * (int)calclus[1] + (int)calclus[0];
                                break;
                            } else{
                                loop+=32;
                            }
                        }
                    }
                    if(pathexist){
                        break;
                    } else{
                        free(str);
                        currentClus = fatval;
                    }
                }
            }
            if(!pathexist){
                printf("Your input path %s doesn't exist!\n", firstline);
                return;
            }
        }
        //targetclus found
        printRecurse(targetclus, fat12, firstline);
    }

    //remember to free the dictionary
    /*
    for(int m=0; m<level; m++){
        char * f = targetdirs[m];
        free(f);
    }
    for(int m=0; m<dictIndex; m++){
        char * f = dirs[m];
        free(f);
    }
    */
}


//----------------------------------------------------------------------------------------------------

void printRecurseWithParam(int startClus, FILE * FAT12, char * parent){
    int dictind = 0;
    char * subdirs[224] = {};
    int subclus[224] = {};
    int subdir_subdirno[224] = {};
    int subdir_subfileno[224] = {};

    int fdictind = 0;
    char * subfiles[224] = {};
    long subfilesize[224] = {};
    //for recurse usage, and for the current level print

    int dataBase = DataBase;
    int currentClus = startClus;
    int val = 0;
    while(val < 0xFF8){
        val = getFATValue(FAT12, currentClus, 0);
        if(val == 0xFF7){
            printf("Encountered bad clus!\n");
            return;
        }
        char * str = (char*)malloc(SecPerClus*BytesPerSec);
        char * content = str;

        int startByte = dataBase + (currentClus - 2)*SecPerClus*BytesPerSec;
        fseek(FAT12, startByte, SEEK_SET);
        fread(content,1,SecPerClus*BytesPerSec,FAT12);

        int count = SecPerClus*BytesPerSec;
        int loop=0;
        while(loop<count){
            char tempName[12] = {};
            if(content[loop]=='\0'){
                loop+=32;
                continue;
            }
            int boolean=0;
            for(int j=loop; j<loop+11; j++){
                if(!isValidElement(content[j])){
                    boolean=1;
                    break;
                }
            }
            if(boolean){
                loop+=32;
                continue;
            }
            int k;
            int tempLong = -1;

            char attr = content[loop+11];
            if((attr&0x10)==0){
                // it's a file, should store in subfiles and size arr
                for(k=0; k<11; k++){
                    if(content[loop+k]!=' '){
                        tempLong++;
                        tempName[tempLong] = content[loop+k];
                    }else{
                        tempLong++;
                        tempName[tempLong] = '.';
                        while(content[loop+k]==' '){
                            k++;
                        }
                        k--;
                    }
                }
                char * tosave = (char*)malloc(12);
                strcpy(tosave, tempName);
                char calclus[4] = {};
                for(int a=0; a < 4; a++){
                    //offset to the file size area
                    calclus[a] = content[loop+28+a];
                }
                subfiles[fdictind] = tosave;
                subfilesize[fdictind] = (long)(16777216*(int)calclus[3] 
                    + 65536*(int)calclus[2] + 256*(int)calclus[1] + (int)calclus[0]);
                fdictind++;
            } else{
                // it's a dir, save its name, clus, subdirno and subfileno
                for(k=0; k<11; k++){
                    if(content[loop+k]!=' '){
                        tempLong++;
                        tempName[tempLong] = content[loop+k];
                    }else{
                        break;
                    }
                }
                char * tosave = (char*)malloc(12);
                strcpy(tosave, tempName);
                char calclus[2] = {};
                for(int a=0; a < 2; a++){
                    calclus[a] = content[loop+26+a];
                }
                subclus[dictind] = 256 * (int)calclus[1] + (int)calclus[0];
                subdirs[dictind] = tosave;
//----------------------------------------------------------------------------------------------
                //start counting
                int db = DataBase;
                int cc = subclus[dictind];
                int fatval = 0;
                while(fatval<0xFF8){
                    fatval = getFATValue(FAT12, cc, 0);
                    if(fatval == 0xFF7){
                        printf("Encountered bad clus!\n");
                        return;
                    }

                    char * sstr = (char*)malloc(SecPerClus*BytesPerSec);//sub str
                    char * scontent = sstr;
                    int sb = db+(cc-2)*SecPerClus*BytesPerSec;
                    fseek(FAT12, sb, SEEK_SET);
                    fread(scontent, 1, SecPerClus*BytesPerSec,FAT12);
                    int scount = SecPerClus*BytesPerSec;
                    int sloop=0;
                    while(sloop < scount){
                        if(scontent[sloop]=='\0'){
                            sloop+=32;
                            continue;
                        }
                        int sbool = 0;
                        for(int sj=sloop; sj<sloop+11; sj++){
                            if(!isValidElement(scontent[sj])){
                                sbool = 1;
                                break;
                            }
                        }
                        if(sbool){
                            sloop+=32;
                            continue;
                        }
                        char sattr = scontent[sloop+11];
                        if((sattr&0x10)==0){
                            //file
                            subdir_subfileno[dictind] = subdir_subfileno[dictind] + 1;
                        }
                        else{
                            subdir_subdirno[dictind] = subdir_subdirno[dictind] + 1;
                        }
                        sloop+=32;
                    }
                    free(sstr);
                    cc = fatval;
                }
//-----------------------------------------------------------------------------------------------
                dictind++;
            }
            loop+=32;
        }
        free(str);
        currentClus = val;
    }

    //dictind
    //subdirs
    //subclus
    //subdir_subdirno
    //subdir_subfileno

    //fdictind
    //subfiles
    //subfilesize
    //printf("\033[31m");
    printf("%s",parent);
    //printf("\033[0;0m");

    printf(" %d %d:\n", dictind, fdictind);

    printf("\033[31m");
    printf(".\n..\n"); //red!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    printf("\033[0;0m");
    

    for(int i=0; i<fdictind; i++){
        printf("%s  %d\n",subfiles[i],subfilesize[i]);
    }
    for(int i=0; i<dictind; i++){

        printf("\033[31m");
        printf("%s",subdirs[i]);
        printf("\033[0;0m");

        printf("  %d %d\n", subdir_subdirno[i], subdir_subfileno[i]);
    }
    printf("\n");
    for(int i=0; i<dictind; i++){
        char firstline[128] = {};
        strcpy(firstline, parent);
        int parentlen = strlen(firstline);
        char * tempsub = subdirs[i];
        int n=0;
        for(; tempsub[n] != '\0'; n++){
            firstline[n+parentlen] = tempsub[n];
        }
        firstline[n+parentlen] = '/';
        printRecurseWithParam(subclus[i], FAT12, firstline);
    }

    return;
}

//----------------------------------------------------------------------------------------------------
// should check if the file is a dir
void handleLsWithParam(const char * filename, FILE * FAT12, struct DIR * rootEntry_ptr){

    int noFilenameInput = 0;
    if(strcmp(filename, "/")==0){
        noFilenameInput = 1;
    }
    char firstline[128] = {};

    if(noFilenameInput || (filename[0]=='.'&&filename[1]=='\0')){
        firstline[0] = '/';
        noFilenameInput = 1; // if input .
    }
    else{
        //three kinds -> ./nju/blabla, nju/blabla, /nju/blabla
        if(filename[0] == '.'){
            int i=0;
            for(int j=1; filename[j] != '\0'; j++, i++){
                firstline[i] = filename[j];
            }
            firstline[i] = '/';
        }
        else if(filename[0] != '/'){
            firstline[0] = '/';
            int i=1;
            for(int j=0; filename[j]!='\0'; j++){
                firstline[i] = filename[j];
                i++;
            }
            firstline[i] = '/';
        }
        else{
            int i=0;
            for(int j=0; filename[j]!='\0'; j++,i++){
                firstline[i] = filename[j];
            }
            firstline[i] = '/';
        }
    }

    
    int t = 0; //this is used for cutting the input path
    char * targetdirs[128] = {};
    int level = 0;
    for(int i=1; firstline[i] != '\0'; i++){
        char * temptarget = (char*)malloc(12);
        int j=0;
        while(firstline[i] != '/'){
            temptarget[j] = firstline[i];
            i++; j++;
        }
        temptarget[j] = '\0';
        targetdirs[level] = temptarget;
        level++;
    }

    //the dirs variable store many with malloc var
    //remember to free all of them after the printing
    int dictIndex = 0;
    char * dirs[224] = {};
    int clus[224] = {};
    int currentlevel = 0;

    //for the firstlevel, if no input or input means the root
    //we need to count the info of sub dirs
    int dirno [224] = {};
    int fileno [224] = {};
    //dir no of subdir

    //another set of arr to store subfile info
    int fdictIndex = 0;
    char * files[224] = {};
    long sizes[224] = {};

    //start from root entry
    int rootbase = RootEntBase;
    char fname [12] = {};

    //first seek out entries in the root level
    for(int i = 0; i<RootEntCnt; i++, rootbase+=32){
        fseek(FAT12, rootbase, SEEK_SET);
        fread(rootEntry_ptr, 1, 32, FAT12);

        if(rootEntry_ptr->DIR_Name[0]=='\0'){
            continue;
        }
        int invalidName = 0;
        for(int j=0; j<11; j++){
            if(!isValidElement(rootEntry_ptr->DIR_Name[j])){
                invalidName = 1;
                break;
            }
        }
        if(invalidName){
            continue;
        }
        char * tempname = (char *)malloc(12);
        int j = 0; int k=0;
        for(;j<11 && rootEntry_ptr->DIR_Name[j]!=' '; j++, k++){
            tempname[k] = rootEntry_ptr->DIR_Name[j];
        }
        if((rootEntry_ptr->DIR_Attr&0x10)!=0){
            // is a dir, needs to be in the dict
            //next is to find its subdir no and file no
            tempname[k] = '\0';
            dirs[dictIndex] = tempname;
            clus[dictIndex] = rootEntry_ptr->DIR_FstClus;
//-----------------------------------------------------------------------------------------------
            int database = DataBase;
            int cc = rootEntry_ptr->DIR_FstClus; //represent for current clus
            //for dir, increase val in dirno[dictInd]
            //file -> fileno[dictInd]
            int fatval = 0;
            while(fatval < 0xFF8){
                fatval = getFATValue(FAT12, cc, 0);
                if(fatval == 0xFF7){
                    printf("Encountered bad clus!\n");
                    return;
                }
                char * str = (char*)malloc(SecPerClus*BytesPerSec);
                char * content = str;
                int startByte = database + (cc - 2) * SecPerClus * BytesPerSec;
                fseek(FAT12, startByte, SEEK_SET);
                fread(content, 1, SecPerClus*BytesPerSec, FAT12);
                int count = SecPerClus * BytesPerSec;
                int loop = 0;
                while(loop < count){
                    if(content[loop]=='\0'){
                        loop+=32;
                        continue;
                    }
                    int boolean = 0;
                    for(int j=loop; j<loop+11; j++){
                        if(!isValidElement(content[j])){
                            boolean = 1;
                            break;
                        }
                    }
                    if(boolean){
                        loop += 32;
                        continue;
                    }
                    char attr = content[loop+11];
                    if((attr&0x10)==0){
                        fileno[dictIndex] = fileno[dictIndex] + 1;
                    }
                    else{
                        dirno[dictIndex] = dirno[dictIndex] + 1;
                    }
                    loop+=32;
                }
                free(str);
                cc = fatval;
            }
//-----------------------------------------------------------------------------------------------
            dictIndex++;
        }
        else{
            // is a file, needs to store size and name
            while(rootEntry_ptr->DIR_Name[j] == ' '){
                j++;
            }
            tempname[k] = '.'; k++;
            for(;j < 11 && rootEntry_ptr->DIR_Name[j] != ' '; j++, k++){
                tempname[k] = rootEntry_ptr->DIR_Name[j];
            }
            files[fdictIndex] = tempname;
            sizes[fdictIndex] = (long)rootEntry_ptr->DIR_FileSize;
            fdictIndex++;
        }
    }

    if(noFilenameInput){
        printf("/ %d %d:\n", dictIndex, fdictIndex);
        //red print!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        for(int i=0; i<dictIndex; i++){
            //printf("\033[31m");
            printf("%s",dirs[i]);
            //printf("\033[0;0m");
            printf("  %d %d\n", dirno[i], fileno[i]);
        }
        for(int i=0; i<fdictIndex; i++){
            printf("%s  %d\n",files[i],sizes[i]);
        }
        printf("\n");
        for(int n=0; n<dictIndex; n++){
            char currentdir[12] = {};
            strcpy(currentdir, dirs[n]);
            char parampath[128] = {};
            strcpy(parampath, firstline);
            int pathlen = strlen(parampath);
            int i=0;
            for(; currentdir[i] != '\0'; i++){
                parampath[i+pathlen] = currentdir[i];
            }
            parampath[i+pathlen] = '/';
            printRecurseWithParam(clus[n], FAT12, parampath);
        }
    }
    else{
        // start finding level by level
        // dir stored in targetdirs
        //dirs stores first level dir
        //clus stores first level clus
        //dictInd represent the size
        int targetclus = -1;
        for(int i=0; i<level; i++){
            int pathexist = 0;
            char currenttarget[12] = {};
            strcpy(currenttarget, targetdirs[i]);
            if(i == 0){
                for(int n=0; n<dictIndex; n++){
                    if(strcmp(currenttarget, dirs[n])==0){
                        pathexist = 1;
                        targetclus = clus[n];
                        break;
                    }
                }
            }else{
                int database = DataBase;
                int currentClus = targetclus;
                int fatval = 0;
                while(fatval < 0xFF8){
                    fatval = getFATValue(FAT12, currentClus, 0);
                    if(fatval == 0xFF7){
                        printf("Encountered bad clus!\n");
                        return;
                    }
                    char * str = (char*)malloc(SecPerClus*BytesPerSec);
                    char * content = str;
                    int startByte = database + (currentClus-2)*SecPerClus*BytesPerSec;
                    fseek(FAT12,startByte,SEEK_SET);
                    fread(content, 1, SecPerClus*BytesPerSec, FAT12);
                    int count = SecPerClus*BytesPerSec;
                    int loop=0;
                    while(loop<count){
                        int i;
                        char tempName[12]={};
                        if(content[loop]=='\0'){
                            loop+=32;
                            continue;
                        }
                        int invalidName = 0;
                        for(int j=loop; j<loop+11; j++){
                            char test = content[j];
                            if(!isValidElement(content[j])){
                                invalidName = 1;
                                break;
                            }
                        }
                        if(invalidName){
                            loop+=32;
                            continue;
                        }
                        //DIR_Attr offset is 12
                        char attr = content[loop+11];
                        if((attr&0x10)==0){
                            // it's a file, should jump over
                            loop+=32;
                            continue;
                        }
                        else{
                            int k;
                            int tempLong = -1;
                            for(k=0; k<11; k++){
                                if(content[loop+k]!=' '){
                                    tempLong++;
                                    tempName[tempLong] = content[loop+k];
                                } else{
                                    tempLong++;
                                    tempName[tempLong] = '\0';
                                    break;
                                }
                            }
                            if(strcmp(currenttarget, tempName)==0){
                                pathexist = 1;
                                char calclus[2] = {};
                                for(int a=0; a < 2; a++){
                                    calclus[a] = content[loop+26+a];
                                }
                                targetclus = 256 * (int)calclus[1] + (int)calclus[0];
                                break;
                            } else{
                                loop+=32;
                            }
                        }
                    }
                    if(pathexist){
                        break;
                    } else{
                        free(str);
                        currentClus = fatval;
                    }
                }
            }
            if(!pathexist){
                printf("Your input path %s doesn't exist!\n", firstline);
                return;
            }
        }
        //targetclus found
        printRecurseWithParam(targetclus, FAT12, firstline);
    }
}
//---------------------------------------------------------------------------------------------------
