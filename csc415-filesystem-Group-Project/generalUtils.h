/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: generalUtils.h
*
* Description: This file Has the prototypes for the generalUtils helper functions, as well as a 
*			   number of #define directives
*
**************************************************************/

#pragma once

#include <time.h>
#include <sys/types.h>

#include "fsLow.h"
#include "mfs.h"

#define TRUE 1
#define FALSE 0
#define BOOL int

#define DIRMAX_LEN 4096

// This is the number of entries that a new directory will be given
#define NEWDIR_NUM_ENTRIES 20

typedef struct
{
	char magic[8];
	unsigned long numberOfBlocks;
	unsigned long blockSize;
	unsigned int rootDirLocat;

	// Number of entries in the root directory
	unsigned int numRootEntries;
	unsigned int fsMapStart;
} VolumeControlBlock;

extern VolumeControlBlock * vcb;

typedef struct
{
	unsigned int size; // Stores the size of the file (bytes)
	unsigned int startPos; // Where in the file system the data starts
	time_t created; // Time when the file was first created
	time_t modified; // Time when the file was most recently modified
	BOOL isDirectory; // TRUE means the file is a directory; FALSE means file is not directory
	char fileName[256]; // Name of the file. Limited length
} DirectoryEntry;


/* Return the value of `base` put to the power of `power` */
unsigned long exponent(unsigned long base, unsigned long power);

/* Divide `numerator` by `denominator`, returning the ceiling-rounded result */
unsigned int ceilRoundDivide(unsigned int numerator, unsigned int denominator);

/* Get the value (1 or 0) of the bit in `inputByte` for bit number `position` (1 to 8) */
unsigned char getBitInUChar(unsigned char inputByte, int position);

/* Changes the bit in `buffer` to `newValue` (0 or 1) */
void changeBitInBuffer(unsigned char * buffer, int position, unsigned char newValue);

/* Make a DirectoryEntry object with all fields filled out. Requires `size`, `startPos`,
`isDirectory`, and `fileName`  as parameters; Time variables handled automatically */
DirectoryEntry makeDirectoryEntry(unsigned int size, unsigned int startPos, 
	BOOL isDirectory, const char * fileName);

/* Prints out the contents of a directory entry */
void examineDirectoryEntry(DirectoryEntry de);

/* Gives back the index of the directory entry. 0 return value means success */
int parsePath(const char * userInput, fdDir * dirp);

/* Will modify the value pointed to by `blocks` to be the number of blocks required
to fit `n` number of directory entries. Likewise for `bytes`. Either can take NULL
as a parameter if that value is not needed */
void sizeofNDirEntries(int n, int * blocks, int * bytes);

/* This function is used to expand a directory that has reached its max entry limit.
It will return 0 on success, else non-0 return value. `parentDir` is the directory
that you want to increase in size (max entries) */
int expandDir(fdDir * parentDir);

/* This function takes in a file path, such as "/example/dir/blob" and splits it into
the strings for the path to the parent folder "/example/dir/" and the string for
the blob itself "blob". Return 0 means success */
int splitBlobPath(const char * inPath, char * dirPath, char * blobPath);

/* Will try to edit the child of `dirp` whose name is `name`. This can change the size (bytes) 
* and/or the starting position (block). Making either `newSize` or `newStart` -1 will make it
* ignored non-0 return value means error */
int changeEntryWithName(fdDir * dirp, const char * name, int newSize, int newStart);

/* Helper function used by both fs_delete and fs_remove */
void removeDirEntry(fdDir * parentDirp, fs_diriteminfo * toRemove);



// Debugging options. 0 = no extra info, 1 = extra info
#define DEBUG_MKDIR 	  0
#define DEBUG_RM 		  0
#define DEBUG_READDIR 	  0
#define DEBUG_SETCWD      0
#define DEBUG_LS		  0
#define DEBUG_DELETE      0
#define DEBUG_CP		  0
#define DEBUG_CAT		  0

#define DEBUG_PARSEPATH   0
#define DEBUG_EXPANDDIR   0
#define DEBUG_MKFILE      0

#define DEBUG_OPEN  	  0
#define DEBUG_READ     	  0
#define DEBUG_WRITE		  0
#define DEBUG_WRITECHECK  0
