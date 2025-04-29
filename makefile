main: FileSystemDisk.c FileSystemDiskFunctions.c Main.c 
	gcc FileSystemDisk.c FileSystemDiskFunctions.c Main.c -o main -Wall -Werror


clean:
	rm -rf *.o main