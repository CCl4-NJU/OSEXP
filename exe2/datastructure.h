
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