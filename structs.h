#ifndef structs_h
#define structs_h

#define MAX_FILENAME_LENGTH 15
#define MAX_FILES_IN_DIR 64
#define MAX_FILE_DESCRIPTORS 32
#define TOTAL_BLOCKS 8192
#define DATA_BLOCKS 4096

typedef struct Boot {
    int bootByteSize;
    int bootBlockSize;
    int fat1StartBlock;
    int fat1ByteSize;
    int fat1BlockSize;
    int fat2StartBlock;
    int rootDirStartBlock;
    int rootDirByteSize;
    int rootDirBlockSize;
    int fileSpaceStartBlock;
    int fileSpaceByteSize;
    int fileSpaceBlockSize;
} Boot;

typedef struct FAT {
    short fat_entries[DATA_BLOCKS];
} FAT;

typedef struct File {
    char filename[MAX_FILENAME_LENGTH];                                
    int createTime;                
    int createDate;                                       
    int startBlock;
    int fileSize; 
} File;

typedef struct Directory {
    File dir_entries[MAX_FILES_IN_DIR];
    int free_entries[MAX_FILES_IN_DIR];
    int numOfEntries;
} Directory;

typedef struct File_Descriptor_Entry {
    int file_offset;
    int file_descriptor;
    int file_index;
} File_Descriptor_Entry;

typedef struct File_Descriptor_Table {
    File_Descriptor_Entry fde_entries[MAX_FILE_DESCRIPTORS];
    int open_file_descriptor_entries;
} File_Descriptor_Table;

typedef struct Data {
    int free_blocks[DATA_BLOCKS];
    int num_free_blocks;
} Data;

typedef struct FileSpace {
    char array[DATA_BLOCKS][BLOCK_SIZE];
} FileSpace;

#endif
