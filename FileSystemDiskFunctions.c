#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "FileSystemDisk.h"
#include "structs.h"

//Global Varibales
Boot boot;
FAT fat1;
FAT fat2;
Directory rootDirectory;
Data data;
File_Descriptor_Table fdt;
FileSpace fileSpace;

//function parameters
int fs_make(char *disk_name);
int fs_mount(char *disk_name);
int fs_umount(char *disk_name);
int fs_open(char *name);
int fs_close(int fildes);
int fs_create(char *name);
int fs_delete(char *name);
int fs_read(int fildes, void *buf, size_t nbyte);
int fs_write(int fildes, void *buf, size_t nbyte);
int fs_get_filesize(int fildes);
int fs_lseek(int fildes, off_t offset);
int fs_truncate(int fildes, off_t length);
void initialize_boot();
void initialize_fat();
void initialize_rootDir();
void initialize_file_space();
void initialize_data();
void initialize_fdt();
int getFileIndex(char *name);
int getFirstFreeFileDescriptor();
int getFirstFreeRootDirectoryIndex();
int getFirstFreeBlock();
int getSizeOfPathOfBlocks(int fildes);
int getFilePath(int fildes, short *array);
void zero_out_data_block(int index);
int findOffsetInFileSpace(int startingBlock, int offset, int *array);
int getDifferenceBetweenBlockSizes(int fileSize, int newSize, int offset, int numberOfBytesBeforeOperation, int functionFlag);

/*
make_fs: function that makes the disk, initialize the necessary elements and writes them to disk

@param char *disk_name: name of disk

@return int: 0 if successful, 1 if failure
*/
int fs_make(char *disk_name) {

    //make the disk
    if (make_disk(disk_name) != 0) {
        fprintf(stderr, "make_fs Error: Failed to create disk\n");
        return -1;
    }

    //open the disk
    if (open_disk(disk_name) != 0) {
        fprintf(stderr, "make_fs Error: Failed to open disk\n");
        return -1;
    }

    //initialize boot
    initialize_boot();

    //write boot to disk
    if (block_write(0, (char *)&boot) == -1) {
        fprintf(stderr, "make_fs Error: Failed to write boot block(s)\n");
        return -1;
    }

    //initialize fat
    initialize_fat();

    //write fats to disk
    FAT *fat1FilePtr = &fat1;
    FAT *fat2FilePtr = &fat2;
    for (int i = 0; i < boot.fat1BlockSize; ++i) {
        if (block_write(boot.fat1StartBlock + i, (char *)fat1FilePtr) == -1) {
            fprintf(stderr, "make_fs Error: Failed to write FAT block(s)");
            return -1;
        }

        if (block_write(boot.fat2StartBlock + i, (char *)fat2FilePtr) == -1) {
            fprintf(stderr, "make_fs Error: Failed to write FAT block(s)");
            return -1;
        }

        fat1FilePtr = (FAT *)((char *)fat1FilePtr + BLOCK_SIZE);
        fat2FilePtr = (FAT *)((char *)fat2FilePtr + BLOCK_SIZE);
    }

    //initialize rootDir
    initialize_rootDir();

    //write rootDir to disk
    Directory *rootDirPtr = &rootDirectory;
    for (int i = 0; i < boot.rootDirBlockSize; ++i) {
        if (block_write(boot.rootDirStartBlock + i, (char *)rootDirPtr) == -1) {
            fprintf(stderr, "make_fs Error: Failed to write rootDirectory block(s)");
            return -1;
        }

        rootDirPtr = (Directory *)((char *)rootDirPtr + BLOCK_SIZE);
    }

    //initialize fileSpace
    initialize_file_space();

    FileSpace *fileSpacePtr = &fileSpace;
    for (int i = 0; i < boot.fileSpaceBlockSize; ++i) {
        if (block_write(boot.fileSpaceStartBlock + i, (char *)fileSpacePtr) == -1) {
        fprintf(stderr, "make_fs Error: Failed to write logical space");
        return -1;
        }

        fileSpacePtr = (FileSpace *)((char *)fileSpacePtr + BLOCK_SIZE);
    }
    
    //close disk
    if (close_disk() == -1) {
        fprintf(stderr, "make_fs Error: Failed to close disk\n");
        return -1;
    }

    //return 0 if successful
    return 0;
}

/*
mount_fs: function that mounts the disk. Makes changing data possible

@param char *disk_name: name of disk

@return int: 0 if successful, -1 if failure 
*/
int fs_mount(char *disk_name) {
    
    //open disk
    if (open_disk(disk_name) != 0) {
        fprintf(stderr, "mount_fs Error: Failed to open disk\n");
        return -1;
    }

    //read the boot
    if (block_read(0, (char *)&boot) == -1) {
        fprintf(stderr, "mount_fs Error: Failed to read disk\n");
        return -1;
    }

    //read the fats
    FAT *fat1FilePtr = &fat1;
    FAT *fat2FilePtr = &fat2;
    for (int i = 0; i < boot.fat1BlockSize; ++i) {
        if (block_read(boot.fat1StartBlock + i, (char *)fat1FilePtr) == -1) {
            fprintf(stderr, "mount_fs Error: Failed to write FAT block(s)");
            return -1;
        }

        if (block_read(boot.fat2StartBlock + i, (char *)fat2FilePtr) == -1) {
            fprintf(stderr, "mount_fs Error: Failed to write FAT block(s)");
            return -1;
        }

        fat1FilePtr = (FAT *)((char *)fat1FilePtr + BLOCK_SIZE);
        fat2FilePtr = (FAT *)((char *)fat2FilePtr + BLOCK_SIZE);
    }

    //read the rootDir
    Directory *rootDirPtr = &rootDirectory;
    for (int i = 0; i < boot.rootDirBlockSize; ++i) {
        if (block_read(boot.rootDirStartBlock + i, (char *)rootDirPtr) == -1) {
            fprintf(stderr, "mount_fs Error: Failed to write rootDirectory block(s)");
            return -1;
        }

        rootDirPtr = (Directory *)((char *)rootDirPtr + BLOCK_SIZE);
    }

    //read the fileSpace
    FileSpace *fileSpacePtr = &fileSpace;
    for (int i = 0; i < boot.fileSpaceBlockSize; ++i) {
        if (block_read(boot.fileSpaceStartBlock + i, (char *)fileSpacePtr) == -1) {
            fprintf(stderr, "mount_fs Error: Failed to write logical space");
            return -1;
        }

        fileSpacePtr = (FileSpace *)((char *)fileSpacePtr + BLOCK_SIZE);
    }

    //return 0 if successful
    return 0;
}

/*
umount_fs: function that unmounts the disk. Updates the information on disk

@param char *disk_name: name of disk

@return int: 0 if successful, -1 if failure 
*/
int fs_umount(char *disk_name) {
    
    //update boot
    if (block_write(0, (char *)&boot) == -1) {
        fprintf(stderr, "umount_fs Error: Failed to write boot block(s)\n");
        return -1;
    }

    //update fats
    FAT *fat1FilePtr = &fat1;
    FAT *fat2FilePtr = &fat2;

    for (int i = 0; i < boot.fat1BlockSize; ++i) {
        if (block_write(boot.fat1StartBlock + i, (char *)fat1FilePtr) == -1) {
            fprintf(stderr, "umount_fs Error: Failed to write FAT block(s)");
            return -1;
        }

        if (block_write(boot.fat2StartBlock + i, (char *)fat2FilePtr) == -1) {
            fprintf(stderr, "umount_fs Error: Failed to write FAT block(s)");
            return -1;
        }

        fat1FilePtr = (FAT *)((char *)fat1FilePtr + BLOCK_SIZE);
        fat2FilePtr = (FAT *)((char *)fat2FilePtr + BLOCK_SIZE);
    }

    //update rootDir
    Directory *rootDirPtr = &rootDirectory;
    for (int i  = 0; i < boot.rootDirBlockSize; ++i) {
        if (block_write(boot.rootDirStartBlock + i, (char *)rootDirPtr) == -1) {
            fprintf(stderr, "umount_fs Error: Failed to write rootDirectory block(s)");
            return -1;
        }

        rootDirPtr = (Directory *)((char *)rootDirPtr + BLOCK_SIZE);
    }

    //update fileSplace
    FileSpace *fileSpacePtr = &fileSpace;
    for (int i = 0; i < boot.fileSpaceBlockSize; ++i) {
        if (block_write(boot.fileSpaceStartBlock + i, (char *)fileSpacePtr) == -1) {
            fprintf(stderr, "umount_fs Error: Failed to write logical space");
            return -1;
        }

        fileSpacePtr = (FileSpace *)((char *)fileSpacePtr + BLOCK_SIZE);
    }
    
    //close disk
    if (close_disk() == -1) {
        fprintf(stderr, "umount_fs Error: Failed to close disk\n");
        return -1;
    }

    //return 0 if successful
    return 0;
}

/*
fs_open: function that opens a file

@param char *name: name of the file

@return int file descriptor: the index of the file descriptor associated with the file or -1 if failure
*/
int fs_open(char *name) {
    //check if the max number of open files are open
    if (fdt.open_file_descriptor_entries == MAX_FILE_DESCRIPTORS) {
        fprintf(stderr, "fs_open Error: Too many open files\n");
        return -1;
    }

    //get the index of the file in the directory
    int file_index = getFileIndex(name);
    
    //check if file doesn't exit
    if (file_index == -1) {
        fprintf(stderr, "fs_open Error: File could not be found\n");
        return -1;
    }
    //get the first free file descriptor
    int fdt_index = getFirstFreeFileDescriptor();
    
    //if no free file descriptors found return -1
    if (fdt_index == -1) {
        fprintf(stderr, "fs_open Error: no open file descriptor\n");
        return -1;
    }

    //set the values of the file descriptor
    fdt.fde_entries[fdt_index].file_offset = 0;
    fdt.fde_entries[fdt_index].file_descriptor = fdt_index;
    fdt.fde_entries[fdt_index].file_index = file_index;

    //update the amount open file descriptors
    fdt.open_file_descriptor_entries++;

    //return the file descriptor
    return fdt.fde_entries[fdt_index].file_descriptor;
}

/*
fs_close: function to close a file

@param int fildes: the index of the file descriptor

@return int: 0 if successful and -1 if failure
*/
int fs_close(int fildes) {
    //check if the file descriptor is valid and that it is open
    if (fildes < 0 || fildes >= MAX_FILE_DESCRIPTORS || fdt.fde_entries[fildes].file_descriptor == -2) {
        fprintf(stderr, "fs_close Error: File descriptor is invalid or not open\n");
        return -1;
    }
    //reset the file descriptors values
    fdt.fde_entries[fildes].file_descriptor = -2;
    fdt.fde_entries[fildes].file_offset = -2;
    fdt.fde_entries[fildes].file_index = -2;

    //decrement the amount of open files
    fdt.open_file_descriptor_entries--;

    //return 0 if successful
    return 0;
}

/*
fs_create: function that creates a file

@param char *name: name of file

@return int: 0 if successful or -1 if failure
*/
int fs_create(char *name) {
    //check to see if the file already exits
    int fileIndex = getFileIndex(name);

    //if it does return -1
    if (fileIndex != -1) {
        fprintf(stderr, "fs_create Error: File already exists\n");
        return -1;
    }

    //check to see if the length is a valid size
    if (strlen(name) + 1 > MAX_FILENAME_LENGTH || strlen(name) < 1) {
        fprintf(stderr, "fs_create Error: File name is not in valid range\n");
        return -1;
    }

    //check to see that there is space in the directory
    if (rootDirectory.numOfEntries >= MAX_FILES_IN_DIR) {
        fprintf(stderr, "fs_create Error: Maximum file capacity reached\n");
        return -1;
    }

    //check to make sure that there is at least 1 block available
    if (data.num_free_blocks <= 0) {
        fprintf(stderr, "fs_create Error: not enough free blocks\n");
        return -1;
    }

    //get the first index in directory
    int firstFreeRootDirIndex = getFirstFreeRootDirectoryIndex();
    
    //check if the search failed
    if (firstFreeRootDirIndex == -1) {
        fprintf(stderr, "fs_create Error: could not fine free index");
        return -1;
    }

    //update the array that shows which entries are free
    rootDirectory.free_entries[firstFreeRootDirIndex] = 1;

    //get first free block
    int firstFreeBlock = getFirstFreeBlock();

    //check if search failed
    if (firstFreeBlock == -1) {
        fprintf(stderr, "fs_create Error: could not find free blocks");
        return -1;
    }

    //update the time variables
    time_t currentTime;
    time(&currentTime);
    struct tm *localTime = localtime(&currentTime);
    int time = (int) currentTime;
    int currentDate = (localTime->tm_year + 1900) * 10000 + (localTime->tm_mon + 1) * 100 + localTime->tm_mday;
    strcpy(rootDirectory.dir_entries[firstFreeRootDirIndex].filename, name);
    rootDirectory.dir_entries[firstFreeBlock].createTime = time;
    rootDirectory.dir_entries[firstFreeBlock].createDate = currentDate;
    rootDirectory.dir_entries[firstFreeBlock].startBlock = firstFreeBlock;

    //set the file size to 0
    rootDirectory.dir_entries[firstFreeBlock].fileSize = 0;

    //increment the number of entries in root directory
    rootDirectory.numOfEntries++;

    //update the array that shows which blocks are free
    data.free_blocks[firstFreeBlock] = 1;

    //decrement number of free blocks
    data.num_free_blocks--;

    //set the fat chain to -1
    fat1.fat_entries[firstFreeBlock] = -1;

    //update copy of fat
    fat2.fat_entries[firstFreeBlock] = fat1.fat_entries[firstFreeBlock];

    //return 0 if successful
    return 0;
}

/*
fs_delete: function that delete a file

@param char *name: the name of file

@return int: 0 if successful or -1 if failure
*/
int fs_delete(char *name) {
    //find the index of the file
    int file_index = getFileIndex(name);

    //if it doesn't exit return -1
    if (file_index == -1) {
        fprintf(stderr, "fs_delete Error: File doesn't exists\n");
        return -1;
    }

    //check if the file is open
    for (int i = 0; i < MAX_FILE_DESCRIPTORS; ++i) {
        if (file_index == fdt.fde_entries[i].file_index && fdt.fde_entries[i].file_descriptor != -2) {
            fprintf(stderr, "fs_delete Error: Cannot delete open file\n");
            return -1;
        }
    }

    //get first block in file
    short currentBlock = rootDirectory.dir_entries[file_index].startBlock;

    //zero out the blocks
    do {
        zero_out_data_block(currentBlock);
        short nextBlock = fat1.fat_entries[currentBlock];
        data.free_blocks[currentBlock] = -2;
        fat1.fat_entries[currentBlock] = -2;
        currentBlock = nextBlock;
    } while (currentBlock != -1);

    //update the free_entries array
    rootDirectory.free_entries[file_index] = -2;

    //decrement the number of entries
    rootDirectory.numOfEntries--;

    //reset values
    memset(rootDirectory.dir_entries[file_index].filename, '\0', MAX_FILENAME_LENGTH);
    rootDirectory.dir_entries[file_index].createDate = -2;
    rootDirectory.dir_entries[file_index].createTime = -2;
    rootDirectory.dir_entries[file_index].startBlock = -2;
    rootDirectory.dir_entries[file_index].fileSize = -2;

    //return 0 if successful
    return 0;
}

/*
fs_read: function to read a file

@param int fildes: the file descriptor

@param void *buf: the location to store data that is read

@param size_t nbytes: the number of bytes to read

@return int BytesRead: the number of bytes read or -1 if failure
*/
int fs_read(int fildes, void *buf, size_t nbyte) {

    //check if a valid file descriptor was passed in
    if (fildes < 0 || fildes >= MAX_FILE_DESCRIPTORS || fdt.fde_entries[fildes].file_descriptor == -2) {
        fprintf(stderr, "fs_read Error: invalid file descriptor\n");
        return -1;
    }

    //get first block
    short currentBlock = rootDirectory.dir_entries[fdt.fde_entries[fildes].file_index].startBlock;

    //get file size
    int fileSize = fs_get_filesize(fildes);

    //check if fileSize was obtained
    if (fileSize == -1) {
        fprintf(stderr, "fs_read Error: File size unobtainable\n");
        return -1;
    }

    //check if file is empty
    if (fileSize == 0) {
        fprintf(stderr, "fs_read Error: File size is empty\n");
        return -1;
    }

    //if the number of bytes to read is > than fileSize set nbyte to fileSize
    if (nbyte > fileSize) nbyte = fileSize;

    //store the offset
    int offset = fdt.fde_entries[fildes].file_offset;

    //check if offest is out of bounds
    if (offset < 0 || offset >= fileSize) {
        fprintf(stderr, "fs_read Error: Offset out of bounds\n");
        return -1;
    }

    //update nbyte is the offset + nbytes is > than fileSize
    if (offset + nbyte > fileSize) nbyte = fileSize - offset;

    //offset array
    int offsetValues[2] = {0};

    //get the values in the 2d array that match the offset
    int success = findOffsetInFileSpace(currentBlock, offset, offsetValues);

    //check if operation was successful
    if (success == -1) {
        fprintf(stderr, "fs_read Error: Offset could not be matched");
        return -1;
    }

    //typecast the destination
    char *dest = (char *) buf;

    //index in the dest buffer
    int currentIndexInBuf = 0;

    //the amount of bytes read
    int bytesRead = 0;

    //checker to ensure the loop goes one extra time after finding -1
    int additionalIteration = 0;

    //get the amount of blocks the file has
    int pathSize = getSizeOfPathOfBlocks(fildes);

    //check if block size was obtained
    if (pathSize == -1) {
        fprintf(stderr, "fs_read Error: block size of file was not obtained");
        return -1;
    }

    //loop and store values in destination buffer
    do {
        for (int i = offsetValues[1]; i < BLOCK_SIZE && nbyte > 0; ++i, ++currentIndexInBuf) {
            dest[currentIndexInBuf] = fileSpace.array[offsetValues[0]][i];
            nbyte--;
            bytesRead++;
        }
        short nextBlock = fat1.fat_entries[offsetValues[0]];
        offsetValues[0] = nextBlock;
        offsetValues[1] = 0;
        if (offsetValues[0] == -1 && additionalIteration == 0 && pathSize > 1) additionalIteration = 1;
    } while ((offsetValues[0] != -1 || additionalIteration) && nbyte > 0);

    //return bytesRead
    return bytesRead;
}

/*
fs_write: function to write to a file

@param int fildes: the file descriptor

@param void *buf: the buffer that holds the data to be written

@param size_t nbytes: the number of bytes to be written

@return bytesWritten: the number of bytes written or -1 if failure

*/
int fs_write(int fildes, void *buf, size_t nbyte) {
    //check for valid file descriptor
    if (fildes < 0 || fildes >= MAX_FILE_DESCRIPTORS || fdt.fde_entries[fildes].file_descriptor == -2) {
        fprintf(stderr, "fs_write Error: invalid file descriptor\n");
        return -1;
    }

    //check if nbyte is > than the max file Size
    if (nbyte > BLOCK_SIZE * DATA_BLOCKS || nbyte <= 0) {
        fprintf(stderr, "fs_write Error: invalid value for nbyte\n");
        return -1;
    }

    //get fileSize
    int fileSize = fs_get_filesize(fildes);

    //check if fileSize was unobtainable
    if (fileSize == -1) {
        fprintf(stderr, "fs_write Error: File size unobtainable\n");
        return -1;
    }

    //store offset 
    int offset = fdt.fde_entries[fildes].file_offset;

    //check if offset is out of bounds
    if ((offset < 0 || offset > fileSize)) {
        fprintf(stderr, "fs_write Error: Offset out of bounds\n");
        return -1;
    }

    //calculate newSize
    int newSize = offset + nbyte;

    //check to see if new size is > than the max file size
    if(newSize >= DATA_BLOCKS * BLOCK_SIZE) {
        fprintf(stderr,"fs_write Error: max file size passed\n");
        return -1;
    }

    //the amount of blocks needed for new size
    int newBlocksNeeded = 0;

    //only need to calculate if the newSize is > than old size
    if (fileSize < newSize) {
        newBlocksNeeded = getDifferenceBetweenBlockSizes(fileSize, newSize, offset, nbyte, 1);
        rootDirectory.dir_entries[fdt.fde_entries[fildes].file_index].fileSize = newSize;
    }

    //check if there are enough blocks
    if (data.num_free_blocks < newBlocksNeeded) {
        fprintf(stderr, "fs_write Error: not enough blocks available for appending file");
        return -1;
    }

    //get first block
    short currentBlock = rootDirectory.dir_entries[fdt.fde_entries[fildes].file_index].startBlock;

    //if more blocks are needed update the fat path and other relevent variables
    if (newBlocksNeeded) {
        currentBlock = rootDirectory.dir_entries[fdt.fde_entries[fildes].file_index].startBlock; 
        while (newBlocksNeeded > 0) {
            short nextBlock = fat1.fat_entries[currentBlock];
            if (nextBlock != -1) {
                currentBlock = nextBlock;
                continue;
            }

            int freeBlock = getFirstFreeBlock();

            if (freeBlock == -1) {
                fat1.fat_entries[currentBlock] = -1;
                fat2.fat_entries[currentBlock] = fat1.fat_entries[currentBlock];
                break;
            }

            if (newBlocksNeeded == 0) {
                fat1.fat_entries[currentBlock] = -1;
                fat2.fat_entries[currentBlock] = fat1.fat_entries[currentBlock];
            }

            else {
                fat1.fat_entries[currentBlock] = freeBlock;
                fat2.fat_entries[currentBlock] = fat1.fat_entries[currentBlock];
                fat1.fat_entries[freeBlock] = -1;
                fat2.fat_entries[freeBlock] = fat1.fat_entries[freeBlock];
                data.num_free_blocks--;
                data.free_blocks[freeBlock] = 1;
            }
            newBlocksNeeded--;
        }
    }

    //offset array
    int offsetValues[2] = {0};

    //get offset in the form of 2d array
    int success = findOffsetInFileSpace(currentBlock, offset, offsetValues);

    //check if process was successful
    if (success == -1) {
        fprintf(stderr, "fs_read Error: Offset could not be matched");
        return -1;
    }

    //typecast source buf
    char *source = (char *) buf;

    //current index in source
    int currentIndexInBuf = 0;

    //the number of bytes actually written
    int bytesWritten = 0;

    //used to ensure proper number of iterations
    int additionalIteration = 0;

    //get pathSize
    int pathSize = getSizeOfPathOfBlocks(fildes);

    if (pathSize == -1) {
        fprintf(stderr, "fs_write Error: block size unobtainable");
        return -1;
    }

    //loop and write data to file
    do {
        for (int i = offsetValues[1]; i < BLOCK_SIZE && nbyte > 0; ++i, ++currentIndexInBuf) {
            fileSpace.array[offsetValues[0]][i] = source[currentIndexInBuf];
            nbyte--;
            bytesWritten++;
        }

        short nextBlock = fat1.fat_entries[offsetValues[0]];
        offsetValues[0] = nextBlock;
        offsetValues[1] = 0;
        if (offsetValues[0] == -1 && additionalIteration == 0 && pathSize > 1) additionalIteration = 1;
    } while ((offsetValues[0] != -1 || additionalIteration) && nbyte > 0);

    //return bytesWritten
    return bytesWritten;
}

/*
fs_lseek: function that updates offset

@param int fildes: the file descriptor

@param off_t offset: the new location of offset

@return int: 0 if successful -1 if failure
*/
int fs_lseek(int fildes, off_t offset) {
    //check for valid file descriptor
    if (fildes < 0 || fildes >= MAX_FILE_DESCRIPTORS || fdt.fde_entries[fildes].file_descriptor == -2) {
        fprintf(stderr, "fs_lseek Error: invalid file descriptor");
        return -1;
    }

    //check for valid offset
    if (offset < 0 || offset > rootDirectory.dir_entries[fdt.fde_entries[fildes].file_index].fileSize) {
        fprintf(stderr, "fs_lseek Error: offset error");
        return -1;
    }

    //update the offset
    fdt.fde_entries[fildes].file_offset = (int) offset;

    //return 0 if successful
    return 0;
}

/*
fs_truncate: function that truncates a file

@param int fildes: the file descriptor

@param off_t length: the desired length of the file after truncation

@return int: 0 if successful and -1 if failure
*/
int fs_truncate(int fildes, off_t length) {
    //check for valid file descriptor
    if (fildes < 0 || fildes >= MAX_FILE_DESCRIPTORS || fdt.fde_entries[fildes].file_descriptor == -2) {
        fprintf(stderr, "fs_truncate Error: invalid file descriptor");
        return -1;
    }

    //get fileSize
    int fileSize = fs_get_filesize(fildes);

    //check if process was successful
    if (fileSize == -1) {
        fprintf(stderr, "fs_truncate Error: File size unobtainable");
        return -1;
    }

    int numberOfBlocksBeforeTruncate = 0;

    //update length if it's greater than fileSize
    if (length > fileSize) length = fileSize;

    //gets difference in block size between length and fileSize
    int difference = getDifferenceBetweenBlockSizes(fileSize, length, 0, numberOfBlocksBeforeTruncate, 2);

    //store current fileSize that will be updated later
    int sizeAfterDeletedBlocks = fileSize;

    //if less blocks are needed after truncation
    if (difference > 0) {

        //get pathSize
        int pathSize = getSizeOfPathOfBlocks(fildes);

        //check if process was successful
        if (pathSize == -1) {
            fprintf(stderr, "fs_truncate Error: path size could not be determined");
            return -1;
        }

        //array of indexes of blocks
        short *array = (short *)malloc(pathSize * sizeof(short));

        //store values in array
        int success = getFilePath(fildes, array);

        //check if process is successful
        if (success == -1 || array == NULL) {
            fprintf(stderr, "fs_truncate Error: file path not found");
            return -1;
        }

        //last index
        int index = pathSize - 1;
        const int difference2 = difference;

        //update fat path and zero out any blocks no longer needed
        while (difference != 0) {
            fat1.fat_entries[array[index]] = -2;
            fat2.fat_entries[array[index]] = fat1.fat_entries[array[index]]; 
            fat1.fat_entries[array[index-1]] = -1;
            fat2.fat_entries[array[index-1]] = fat1.fat_entries[array[index-1]];
            zero_out_data_block(array[index]);
            data.free_blocks[array[index]] = -2;
            data.num_free_blocks++;

            difference--;
            index--;
        }

        //check if more truncatation is required 
        sizeAfterDeletedBlocks -= (difference2 * BLOCK_SIZE);
        free(array);

        //finish the truncation if necessary
        if (length != sizeAfterDeletedBlocks) {
            short currentBlock = rootDirectory.dir_entries[fdt.fde_entries[fildes].file_index].startBlock;
            short nextBlock = 0;
            do {
                nextBlock = fat1.fat_entries[currentBlock];
                if (nextBlock != -1) {
                    currentBlock = nextBlock;
                }
            } while (nextBlock != -1);

            while (length != sizeAfterDeletedBlocks) {
                for (int i = BLOCK_SIZE-1; i >= 0; --i) {
                    fileSpace.array[currentBlock][i] = 0;
                    sizeAfterDeletedBlocks--;
                    if (sizeAfterDeletedBlocks == length) break;
                }
            }
        }
    }
    
    //if the block size is the same do the truncation from the
    else if (difference == -1) {
        if (length != sizeAfterDeletedBlocks) {
            short currentBlock = rootDirectory.dir_entries[fdt.fde_entries[fildes].file_index].startBlock;
            short nextBlock = 0;
            do {
                nextBlock = fat1.fat_entries[currentBlock];
                if (nextBlock != -1) {
                    currentBlock = nextBlock;
                }
            } while (nextBlock != -1);

            int startTruncation = fileSize - 1;
            for (int i = startTruncation; i >= 0; --i) {
                fileSpace.array[currentBlock][i] = 0;
                sizeAfterDeletedBlocks--;
                if (sizeAfterDeletedBlocks == length) break;
            }
        }
    }

    //update file size
    rootDirectory.dir_entries[fdt.fde_entries[fildes].file_index].fileSize = length;

    //return 0 if successful
    return 0;
}

/*
fs_get_filesize: function that returns size of the file

@param int fildes: the file descriptor

@return int: the file size or -1 if the file descriptor (param) is invalid
*/
int fs_get_filesize(int fildes) {
    //check for valid fild descriptor
    if (fildes < 0 || fildes >= MAX_FILE_DESCRIPTORS || fdt.fde_entries[fildes].file_descriptor == -1) {
        fprintf(stderr, "fs_read Error: invalid file descriptor");
        return -1;
    }
    //returns fileSize
    return rootDirectory.dir_entries[fdt.fde_entries[fildes].file_index].fileSize;
}

/*
initialize_boot: function that initializes the boot;
*/
void initialize_boot() {
    boot.bootByteSize = sizeof(Boot);
    boot.bootBlockSize = (sizeof(Boot) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    boot.fat1StartBlock = boot.bootBlockSize;
    boot.fat1ByteSize = sizeof(FAT);
    boot.fat1BlockSize = (sizeof(FAT) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    boot.fat2StartBlock = boot.fat1StartBlock + boot.fat1BlockSize;
    boot.rootDirStartBlock = boot.fat2StartBlock + boot.fat1BlockSize;
    boot.rootDirByteSize = sizeof(Directory);
    boot.rootDirBlockSize = (sizeof(Directory) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    boot.fileSpaceStartBlock = 4096;
    boot.fileSpaceByteSize = sizeof(FileSpace);
    boot.fileSpaceBlockSize = (sizeof(FileSpace) + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

/*
initialize_fat: function that initializes both fat1 and fat2
*/
void initialize_fat() {
    for (short i = 0; i < DATA_BLOCKS; i++) {
       fat1.fat_entries[i] = -2;
       fat2.fat_entries[i] = fat1.fat_entries[i];
    }
}

/*
initialize_rootDir: function that initializes the root directory
*/
void initialize_rootDir() {
    rootDirectory.numOfEntries = 0;
    for (int i = 0; i < MAX_FILES_IN_DIR; ++i) {
        rootDirectory.free_entries[i] = -2;
        memset(rootDirectory.dir_entries[i].filename, '\0', MAX_FILENAME_LENGTH);
        rootDirectory.dir_entries[i].createDate = -2;
        rootDirectory.dir_entries[i].createTime = -2;
        rootDirectory.dir_entries[i].startBlock = -2;
        rootDirectory.dir_entries[i].fileSize = -2;
    }
}

/*
initialize_file_space: function that initializes the file space
*/
void initialize_file_space() {
    for (int i = 0; i < DATA_BLOCKS; ++i) {
        for (int j = 0; j < BLOCK_SIZE; ++j) {
            fileSpace.array[i][j] = 0;
        }
    }
}

/*
initialize_data: function that initializes the data
*/
void initialize_data() {
    data.num_free_blocks = DATA_BLOCKS;
    for (int i = 0; i < data.num_free_blocks; ++i) {
        data.free_blocks[i] = -2;
    }
}

/*
initialize_fdt: function that initializes the file descriptor table
*/
void initialize_fdt() {
    for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++) {
        fdt.fde_entries[i].file_descriptor = -2;
        fdt.fde_entries[i].file_offset = -2;
        fdt.fde_entries[i].file_index = -2;
    }
    fdt.open_file_descriptor_entries = 0;
}

/*
getFileIndex: Function to check if the file exists

@param char *name: the name of the file

@return int file_index: the index of the file or -1 if it doesn't exist   
*/
int getFileIndex(char *name) {
    int file_index = -1;
    for (int i = 0; i < MAX_FILES_IN_DIR; ++i) {
        if (strcmp(rootDirectory.dir_entries[i].filename, name) == 0) {
            file_index = i;
            break;
        }
    }
    return file_index;
}

/*
getFirstFreeFileDescriptor: Function to get index of first free file descriptor

@return int fdt_index: the index of the first free file descriptor or -1 if no free descriptors
*/
int getFirstFreeFileDescriptor() {
    int fdt_index = -1;
    for (int i = 0; i < MAX_FILE_DESCRIPTORS; ++i) {
        if (fdt.fde_entries[i].file_descriptor == -2) {
            fdt_index = i;
            break;
        }
    }
    return fdt_index;
}

/*
getFirstFreeRootDirectoryIndex: function that gets first free index in root directory

@return int firstFreeRootDirIndex: the index of the first free spot if successful or -1 if failure
*/
int getFirstFreeRootDirectoryIndex() {
    int firstFreeRootDirIndex = -1;
    for (int i = 0; i < MAX_FILES_IN_DIR; ++i) {
        if (rootDirectory.free_entries[i] == -2) {
            firstFreeRootDirIndex = i;
            break;
        }
    }
    return firstFreeRootDirIndex;
}

/*
getFirstFreeBlock: function that gets first free blocks

@return int firstFreeBlock: the index of the first free block or -1 if no free blocks
*/
int getFirstFreeBlock() {
    int firstFreeBlock = -1;
    for (int i = 0; i < DATA_BLOCKS; ++i) {
        if (data.free_blocks[i] == -2) {
            firstFreeBlock = i;
            break;
        }
    }
    return firstFreeBlock;
}

/*
getSizeOfPathOfBlocks: function that determines the amount of blocks the file uses

@param int fildes: the file descriptor associated with file

@return int pathSize: the size of the path or -1
*/
int getSizeOfPathOfBlocks(int fildes) {
    if (fildes < 0 || fildes >= MAX_FILE_DESCRIPTORS || fdt.fde_entries[fildes].file_descriptor == -2) {
        fprintf(stderr, "fs_write Error: invalid file descriptor");
        return -1;
    }

    short currentBlock = rootDirectory.dir_entries[fdt.fde_entries[fildes].file_index].startBlock;
    int pathSize = 0;
    int iterations = 0;
    while (currentBlock != -1 || iterations == DATA_BLOCKS) {
        short nextBlock = fat1.fat_entries[currentBlock];
        currentBlock = nextBlock;
        iterations++;
        pathSize++;
    }
    if (pathSize <=0 || pathSize >= DATA_BLOCKS || iterations == DATA_BLOCKS) pathSize = -1;
    return pathSize;
}

/*
getFilePath: function that stores the indexs of path of block chain in an array

@param int fildes: the file descriptor
@oaram short *array: the array where the values get stored

@return 0 if successful or -1 if failure
*/
int getFilePath(int fildes, short *array) {
    if (fildes < 0 || fildes >= MAX_FILE_DESCRIPTORS || fdt.fde_entries[fildes].file_descriptor == -2) {
        fprintf(stderr, "fs_write Error: invalid file descriptor");
        return -1;
    }
    short currentBlock = rootDirectory.dir_entries[fdt.fde_entries[fildes].file_index].startBlock;
    int index = 0;
    while (currentBlock != -1) {
        array[index] = currentBlock;
        index++;
        short nextBlock = fat1.fat_entries[currentBlock];
        currentBlock = nextBlock;
    }
    return 0;
}

/*
zero_out_data_block: function that zeros out a data block

@param: the index of the block
*/
void zero_out_data_block(int index) {
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        fileSpace.array[index][i] = 0;
    }
}

/*
findOffsetInFileSpace: functiont that stores the two index values in the 2d array that match the file offset

@param int currentBlock: the starting block of the file

@param int offset: the desired offset

@param int *array: a pointer to the array where the values will be stored

@return int: 0 if success or -1 if failure
*/
int findOffsetInFileSpace(int currentBlock, int offset, int *array) {
    if (fat1.fat_entries[currentBlock] == -2) {
        return -1;
    }

    int startBlockOffset = -2;
    int startIndexOffset = -2;
    int currentIndex = 0;

    do {
        for (int i = 0; i < BLOCK_SIZE; ++i) {
            if (currentIndex == offset) {
                startBlockOffset = currentBlock;
                startIndexOffset = i;
                break;
            }
            currentIndex++;
        }

        if (currentIndex == offset || currentBlock == -1) {
            break;
        }
        short nextBlock = fat1.fat_entries[currentBlock];
        currentBlock = nextBlock;
    } while(currentBlock != -1 && offset != currentIndex);

    if (startBlockOffset == -2 || startIndexOffset == -2) {
        return -1;
    }

    array[0] = startBlockOffset;
    array[1] = startIndexOffset;
    return 0;
}

/*
getDifferenceBetweenBlockSizes: function that returns the differences in block size after the write and before

@param int fileSize: the fileSize prior to any opperation

@param int newSize: the desired necessary fileSize after the operation

@param int offset: the current offset in the file that is used for if statements

@param int numberOfBytesBeforeOperation: the number of bytes desired to write

@return int newBlocksNeeded: the amount of blocks needed for the new fileSize
*/
int getDifferenceBetweenBlockSizes(int fileSize, int newSize, int offset, int numberOfBytesBeforeOperation, int functionFlag) {
    int blockDifference = 0;

    int numberOfBlocksBeforeOperation = 0;
    if (fileSize % BLOCK_SIZE == 0) numberOfBlocksBeforeOperation = fileSize / BLOCK_SIZE;
    else numberOfBlocksBeforeOperation = (fileSize / BLOCK_SIZE) + 1;
    if (numberOfBlocksBeforeOperation == 0) numberOfBlocksBeforeOperation = 1;

    if (functionFlag == 1) {
        int newBlocksNeeded = 0;
        if (offset + numberOfBytesBeforeOperation > fileSize) {
            int numberOfBlocks = 0;
            if (newSize % BLOCK_SIZE == 0) numberOfBlocks = newSize / BLOCK_SIZE;
            else numberOfBlocks = (newSize / BLOCK_SIZE) + 1;
            if (numberOfBlocks == 0) numberOfBlocks = 1;

            if (numberOfBlocks - numberOfBlocksBeforeOperation > 0 && functionFlag == 1) newBlocksNeeded = numberOfBlocks - numberOfBlocksBeforeOperation;
            else newBlocksNeeded = 0;
        }
        blockDifference = newBlocksNeeded;
    }

    if (functionFlag == 2) {
        int numberOfBlocks = 0;
        if (newSize % BLOCK_SIZE == 0) numberOfBlocks = newSize / BLOCK_SIZE;
        else numberOfBlocks = (newSize / BLOCK_SIZE) + 1;
        if (numberOfBlocks == 0) numberOfBlocks = 1;

        blockDifference = numberOfBlocksBeforeOperation - numberOfBlocks;
    }
    return blockDifference;
}