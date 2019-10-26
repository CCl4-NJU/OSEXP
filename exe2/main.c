#include "func.c"
#include "check.c"

//int startByte = dataBase + (currentClus - 2)*SecPerClus*BytsPerSec;
// physical sector number = 33 + FAT entry number - 2

char DEFAULT_PATH[] = {"defaultpath"};

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
    char input[128];
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
        fgets(input, 127, stdin);
    }

    return 0;
}