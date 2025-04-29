#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "FileSystemDisk.h"
#include "structs.h"
#include "FileSystemDiskFunctions.h"

extern Boot boot;
extern FAT fat1;
extern FAT fat2;
extern Directory rootDirectory;
extern Data data;
extern File_Descriptor_Table fdt;
extern FileSpace fileSpace;

int main(int argc, char *argv[]) {
    initialize_fdt();
    initialize_data();

    char *name;

    if (argc > 2) {
        puts("Invalid Input");
        return -1;
    }
    else if (argc == 2) name = argv[1];

    else name = "virtual disk";

    if (fs_make(name) == -1) exit(1);

    if (fs_mount(name) == -1) exit(1);

    char *fileName = "file1.txt";
    if (fs_create(fileName) == -1) exit(1);

    int fildes = fs_open(fileName);

    if(fildes == -1) exit(1);

    char array[26] = {0};

    for (int i = 0; i < 26; ++i) array[i] = i+65;

    int bytesWritten = fs_write(fildes, array, 26);

    if (bytesWritten == -1) exit(1);

    if (fs_close(fildes) == -1) exit(1);

    int pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    else if (pid == 0) {
        printf("%s", "What was written: ");
        for (int i = 0; i < bytesWritten; ++i) printf("%c ", array[i]);
        
        int fildes = fs_open(fileName);

        if(fildes == -1) exit(1);

        char array2[26] = {0};

        printf("\n%s", "Array before read: ");

        for (int i = 0; i < 26; ++i) printf("%c ", array2[i]);

        printf("\n%s", "After Read operation: ");

        int bytesRead = fs_read(fildes, array2, 26);

        if (bytesRead == -1) exit(1);

        for (int i = 0; i < bytesRead; ++i) printf("%c ", array2[i]);

        if (fs_close(fildes) == -1) exit(1);

        exit(0);
    }

    else if (pid > 0) wait(NULL);

    puts("");
    char *fileName2 = "file2.txt";
    if (fs_create(fileName2) == -1) exit(1);

    int fildes11 = fs_open(fileName);

    if (fildes11 == -1) exit(1);

    int fildes2 = fs_open(fileName2);

    if(fildes2 == -1) exit(1);

    char array3[26] = {0};

    int bytesRead = fs_read(fildes11, array3, 26);

    if (bytesRead == -1) exit(1);

    int bytesWritten2 = fs_write(fildes2, array3, 26);

    if (bytesWritten2 == -1) exit(1);

    char file1Array[26] = {0};
    char file2Array[26] = {0};

    int file1BytesRead = fs_read(fildes11, file1Array, 26);

    if (file1BytesRead == -1) exit(1);

    int file2BytesRead = fs_read(fildes2, file2Array, 26);

    if (file2BytesRead == -1) exit(1);

    printf("%s", "file1: ");

    for (int i = 0; i < file1BytesRead; ++i) printf("%c ", file1Array[i]);

    printf("\n%s", "file2: ");

    for (int i = 0; i < file2BytesRead; ++i) printf("%c ", file2Array[i]);

    if (fs_close(fildes11) == -1) exit(1);

    if (fs_close(fildes2) == -1) exit(1);

    puts("");
    printf("%s\n", "directory before deletetion (-2 means empty slot)");
    for (int i = 0; i < MAX_FILES_IN_DIR; ++i) {
        printf("%d ", rootDirectory.free_entries[i]);
    }

    if (fs_delete(fileName) == -1) exit(1);

    puts("");
    printf("%s\n", "directory after deletetion (-2 means empty slot)");
    for (int i = 0; i < MAX_FILES_IN_DIR; ++i) {
        printf("%d ", rootDirectory.free_entries[i]);
    }

    if (fs_umount(name) == -1) exit(1);
}