#ifndef FileSystemDiskFunctions_h
#define FileSystemDiskFunctions_h

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
int getIndexesOfPath(short *array, int *indexOfPath, int pathSize);
void zero_out_data_block(int index);

#endif