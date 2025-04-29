# Implementing-a-Simple-Filesystem
Note: Copy of my Temple University Lab Repository.

The goal of this project is to implement a simple file system on top of a virtual disk. To this end, you will implement a library of functions that offer a set of basic file system calls (such as open, read, write, ...) to applications. The file data and file system meta-information will be stored on a virtual disk. This virtual disk is a single file that is stored on the "real" file system provided by the Linux operating system. That is, you are basically implementing your file system on top of the Linux file system.

To create and access the virtual disk, we have provided a few definitions and helper functions that you can find in this header file and this source file. Note that, in your library, you are not allowed to create any "real" files on the Linux file system itself. Instead, you must use the provided helper functions and store all the data that you need on the virtual disk. As you can see by looking at the provided header and source files, the virtual disk has 8,192 blocks, and each block holds 4KB. Using the helper files you can create an empty disk, open and close a disk, and read and write entire disk blocks (by providing a block number in the range between 0 and 8,191 inclusive).

To make things easier, your file system does not have to support a directory hierarchy. Instead, all files are stored in a single root directory on the virtual disk. In addition, your file system does not have to store more than 64 files (of course, you can create and delete files, and deleted files do not count against this 64 file limit). Finally, out of the 8,192 blocks available on disk, only 4,096 must be reserved as data blocks. That is, you have ample space to store your file meta-information. However, you have to free data blocks (make them available again) when the corresponding file is deleted. The maximum file size is 16 megabytes (all 4,096 data blocks, each block with 4KB).

# Project-3-S24
## Project 3: Implementing a Simple File System

### Deliverable 1
- structs.h
     - boot, fat, directory, data, file, file descriptor, and file descriptor table structs
     - make_fs
     - mount_fs  
     - umount_fs

### Deliverable 2
- all other required function designs
- some helper functions
- updates to certain structs in structs.h

### Deliverable 3
- added struct that is a 2d array that mimics the disk
- added a few function that make life easier in truncate and write
- updated read, write, and truncate to make function calls so that they aren't nearly as big
- fixed offset errors caused by lseek
- updated make, mount, and umount to ensure proper block placement
- Main.c
     - the test program
- Overall Error fixes in make, mount, umount, read, write, truncate, & lseek
- removed unnecessary file data and file descriptor data 

### Compilation
- make
- ./main [name of disk] -> name of disk optional

### Testing
Not all my tests are shown here but everything required in the slides is. I testing extensively using specific examples. I tested appending a file, lseek, and update paths after write and read and all my example worked as intended. 

### Know Issues
Sometimes when running on the server garbage values appear at the end of desired outputs. I tried to figure out why but they only show up sometimes. What I mean by sometimes is that running the same code over and over again sometimes causes the garbage values to appear. I believe this is a server issue as I changed nothing in my code yet they show up sometimes.

### IMPORTANT NOTE
For deliverable 2, I was told by ta's in discord server that we were allowed to use block read and block write in fs_read and fs_write. This obviously goes against the whole point of the project but at the time I didn't realize that fact. I have fixed this and followed the correct implementation. However, the code in my deliverable 2 partially follows the wrong information I was told. I talked to the professor about this and she told me to explain the situation here in the readme.
