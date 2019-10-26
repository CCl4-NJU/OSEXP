#include "check.h"

char WARNING_INVALID_COMMAND[] = {"Command entered invalid!\n"};

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