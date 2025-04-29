# Implementing-a-Simple-Filesystem
Note: Copy of my Temple University Repository for an assignment.

The goal of this project is to implement a simple file system on top of a virtual disk.

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
