#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datastructure.h"

//load img into FAT12 variable
void loadFAT12(FILE * FAT12, struct BPB * bpb_ptr);

// should check if the file is a dir
void handleCat(const char * filename, FILE * fat12 , struct DIR * rootEntry_ptr);

// should check if the file is a dir
void handleLs(const char * filename, FILE * fat12 , struct DIR * rootEntry_ptr);

// should check if the file is a dir
void handleLsWithParam(const char * filename);