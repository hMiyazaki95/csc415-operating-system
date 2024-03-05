/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"

#include "generalUtils.h"
#include "fsMapManager.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
{
	char * buff;	     // holds the open file buffer
	int index;		     // holds the current position in the buffer
	int startingBlock;   // What block number the file starts at
	int size;  		     // The size of the file in bytes
	int bytesRead;
	int bytesWritten;
	int blocksDone;      // The number of blocks read/written so far from this file
	BOOL bufferValid;    // Says whether the data currently in the buffer is valid for reading
	unsigned int flags;  // The flags given for b_open
	char parentDirName[MAX_CWD_LEN]; // Name of the parent directory
	char thisFileName[MAX_CWD_LEN];  // Name of this file

} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = FALSE;	//Indicates that this has not been initialized

/* This function is used to see if a flag is set. This function specifically helps with the flag
O_RDONLY. Because that macro expands to 0, it can only be tested for indirectly */
BOOL isFlagSet(int allFlags, int flag)
{
	// O_RDONLY case
	if (flag == O_RDONLY)
	{
		if ((allFlags & O_RDWR) == O_RDWR)
			return FALSE;
		else if ((allFlags & O_WRONLY) == O_WRONLY)
			return FALSE;
		else
			return TRUE;
	}
	else
	{
		if ((allFlags & flag) == flag)
			return TRUE;
		else
			return FALSE;
	}
}

// Method to initialize our file system
void b_init()
{
	if (startup == TRUE)
		return;

	// init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
	{
		fcbArray[i].buff = NULL; //indicates a free fcbArray
	}
		
	startup = TRUE;
}

// Method to get a free FCB element
b_io_fd b_getFCB()
{
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].buff == NULL)
		{
			return i;		// Not thread safe (But do not worry about it for this assignment)
		}
	}
	return -1;  //all in use
}



// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open(char * filename, int flags)
{
    if (startup == FALSE)
		b_init();  //Initialize our system

    b_io_fd fcbIndex = b_getFCB();
    if (fcbIndex == -1)
	{
        // All FCBs are in use.
        return (-1);
	}

    char parentDirName[DIRMAX_LEN];
    char newBlobName[DIRMAX_LEN];
	
	//checks if the filename argument is not valid path
	//checks if the path is too long to be split into parenDirName and newBlobName arrays
	if(splitBlobPath(filename, parentDirName, newBlobName) != 0)
	{
		printf("ERROR: Some problem with splitBlobPath\n");
		return 1;
	}
	
	// checks if the directory specified by the parentDirName exist.
	if (fs_isDir(parentDirName) != 1)
	{
		printf("ERROR: this command cannot create a new directory\n");
		return 1;
	}

	// The file does not currently exist
	if (fs_isFile(filename) == -1 || isFlagSet(flags, O_TRUNC) == TRUE)
	{

		if (isFlagSet(flags, O_CREAT) == FALSE)
			return -1;

#if DEBUG_OPEN
		printf("=>DEBUG_OPEN: Creating new file\n");
#endif

		fs_make_nondir(filename);
	}
	// Does currently exist / use existing
	else
	{
#if DEBUG_OPEN
		printf("=>DEBUG_OPEN: File does exist currently\n");
#endif

		printf("Err: A file with that name already exists!!!\n");
		return -1;

	}
	
	fdDir * dirp = malloc(sizeof(fdDir));

	if (parsePath(parentDirName, dirp) != 0)
		printf("Error with parse path\n");

	fs_diriteminfo * child = fs_getChildWithName(dirp, newBlobName);

	free(dirp);

#if DEBUG_OPEN
	printf("=>DEBUG_OPEN: Starting block: %d\n", child->blockStart);
	printf("=>DEBUG_OPEN: # bytes: %d\n", child->d_reclen);
#endif


	fcbArray[fcbIndex].index = 0;
	fcbArray[fcbIndex].startingBlock = child->blockStart;
	fcbArray[fcbIndex].bytesRead = 0;
	fcbArray[fcbIndex].bytesWritten = 0;
	fcbArray[fcbIndex].blocksDone = 0;
	fcbArray[fcbIndex].size = child->d_reclen;
	fcbArray[fcbIndex].buff = malloc(B_CHUNK_SIZE);
	fcbArray[fcbIndex].bufferValid = FALSE;
	fcbArray[fcbIndex].flags = flags;
	strcpy(fcbArray[fcbIndex].parentDirName, parentDirName);
	strcpy(fcbArray[fcbIndex].thisFileName, newBlobName);

	free(child);

	return fcbIndex;
}

// This function was not implemented
// int b_seek (b_io_fd fd, off_t offset, int whence)
// 	{
// 	if (startup == 0) b_init();  //Initialize our system

// 	// check that fd is between 0 and (MAXFCBS-1)
// 	if ((fd < 0) || (fd >= MAXFCBS))
// 		{
// 		return (-1); 					//invalid file descriptor
// 		}
		
		
// 	return (0); //Change this
// 	}



// Interface to write function	
int b_write(b_io_fd fd, char * buffer, int count)
{
	if (startup == FALSE)
		b_init();  // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		return -1; 					// invalid file descriptor
		
	if (fcbArray[fd].buff == NULL)
		return -1;

	// Make sure that the file is not read only
	if (isFlagSet(fcbArray[fd].flags, O_RDONLY) == TRUE)
	{

#if DEBUG_WRITE
		printf("=>DEBUG_WRITE: returning due to flags\n");
#endif

		return -1;
	}

	// Used to keep track of how many bytes have been written in this specific b_write call
	int bytesWrittenLocal = 0;
	

	// See how many bytes from the parameter buffer can be put into the fcb buffer. The answer will
	// Be the count or the number of bytes left in the fcb buffer, whichever is lower
	int writeFromCurrBuff;
	if (count >  B_CHUNK_SIZE - fcbArray[fd].index)
		writeFromCurrBuff = B_CHUNK_SIZE - fcbArray[fd].index;
	else
		writeFromCurrBuff = count;

#if DEBUG_WRITE
		printf("=>DEBUG_WRITE: Step 1: %d bytes\n", writeFromCurrBuff);
#endif

	// If we are able to write to the existing fcb buffer
	if (writeFromCurrBuff > 0)
	{
		memcpy(fcbArray[fd].buff + fcbArray[fd].index, buffer, writeFromCurrBuff);
		fcbArray[fd].index += writeFromCurrBuff;
		bytesWrittenLocal += writeFromCurrBuff;
	}

	// If the fcb buffer is full, write its contents
	if (fcbArray[fd].index == B_CHUNK_SIZE)
	{

#if DEBUG_WRITE
		printf("=>DEBUG_WRITE: Step 1: Writing buffer\n");
#endif

		if (writeSizeCheck(fd) != 0)
		{
			return -1;
		}
		LBAwrite(fcbArray[fd].buff, 1, fcbArray[fd].startingBlock + fcbArray[fd].blocksDone);
		fcbArray[fd].blocksDone++; // Mark off that 1 more block is done
		fcbArray[fd].index = 0; // Reset index back to 0
	}

	int numDirectCopy = (count - bytesWrittenLocal) / B_CHUNK_SIZE;

#if DEBUG_WRITE
	printf("=>DEBUG_WRITE: Step 2: Directly copying %d blocks\n", numDirectCopy);
#endif

	if (numDirectCopy > 0)
	{
		if (writeSizeCheck(fd) != 0)
		{
			return -1;
		}
		LBAwrite(buffer + bytesWrittenLocal,
			numDirectCopy,			
			fcbArray[fd].startingBlock + fcbArray[fd].blocksDone
		);
		fcbArray[fd].blocksDone += numDirectCopy;
		fcbArray[fd].index = 0;
		bytesWrittenLocal += B_CHUNK_SIZE * numDirectCopy;
	}

	// This will now be set the the number of bytes not yet accounted for
	writeFromCurrBuff = count - bytesWrittenLocal;

#if DEBUG_WRITE
		printf("=>DEBUG_WRITE: Step 3: %d bytes\n", writeFromCurrBuff);
#endif

	if (writeFromCurrBuff > 0)
	{
		memcpy(fcbArray[fd].buff + fcbArray[fd].index,
			buffer + bytesWrittenLocal, writeFromCurrBuff);
		fcbArray[fd].index += writeFromCurrBuff;
		bytesWrittenLocal += writeFromCurrBuff;
	}

	fcbArray[fd].bytesWritten += bytesWrittenLocal;

#if DEBUG_WRITE
		printf("=>DEBUG_WRITE: Total bytes written for this call: %d\n", bytesWrittenLocal);
#endif

	return count;
}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read(b_io_fd fd, char * buffer, int count)
{
	// boolean flag to that checks whether the sustem has been initialized
	if (startup == FALSE)
		b_init();
	// if the fd doesn't have valid index into the FCB array, return -1
	if ((fd < 0) || (fd >= MAXFCBS))
		return -1;
	// checks if the buffer pointer of the FCB corresponding to the given fd is null.
	// if it's null file is not open.
	if (fcbArray[fd].buff == NULL)
		return -1;

	// Make sure that the file is not write only
	if (isFlagSet(fcbArray[fd].flags, O_WRONLY) == TRUE)
	{
		return -1;
	}

	int bytesReadLocal = 0;
	
	int validCount;
	// checks if reading byte count will pass the capacity of the file size
	if (fcbArray[fd].bytesRead + count > fcbArray[fd].size)
		validCount = fcbArray[fd].size - fcbArray[fd].bytesRead;
	else
		validCount = count;

	// checks how many bytes can be read from the current position
	int readFromCurrBuffer;
	if (B_CHUNK_SIZE - fcbArray[fd].index < validCount)
		readFromCurrBuffer = B_CHUNK_SIZE - fcbArray[fd].index;
	else
		readFromCurrBuffer = validCount;


	if (readFromCurrBuffer > 0 && fcbArray[fd].bufferValid == TRUE)
	{
		memcpy(buffer, fcbArray[fd].buff + fcbArray[fd].index, readFromCurrBuffer);
		fcbArray[fd].index += readFromCurrBuffer;
		bytesReadLocal += readFromCurrBuffer;
	}

	int numDirectCopy = validCount / B_CHUNK_SIZE;
	if (numDirectCopy > 0)
	{
		LBAread(buffer + bytesReadLocal,
			numDirectCopy,
			fcbArray[fd].startingBlock + fcbArray[fd].blocksDone
		);
		fcbArray[fd].blocksDone += numDirectCopy;
		fcbArray[fd].bufferValid = FALSE;
		fcbArray[fd].index = 0;
		bytesReadLocal += B_CHUNK_SIZE * numDirectCopy;
	}

	readFromCurrBuffer = validCount - bytesReadLocal;
	if (readFromCurrBuffer > 0)
	{
		LBAread(fcbArray[fd].buff, 1, fcbArray[fd].startingBlock + fcbArray[fd].blocksDone);
		memcpy(buffer + bytesReadLocal, fcbArray[fd].buff, readFromCurrBuffer);
		fcbArray[fd].blocksDone++;
		fcbArray[fd].index = readFromCurrBuffer;
		fcbArray[fd].bufferValid = TRUE; 
		bytesReadLocal += readFromCurrBuffer;
	}

	fcbArray[fd].bytesRead += bytesReadLocal;

	return validCount;
}
	
// Interface to close the file by freeing up resources and 
// writing any data remaining in the write buffer to the storage device. 0 means success
int b_close(b_io_fd fd)
{
	if (writeSizeCheck(fd) != 0)
	{
			return -1;
	}
	// See if there is any data in write buffer that still needs to be written
	if (fcbArray[fd].index > 0 && fcbArray[fd].bytesWritten > 0)
	{
		// Get existing data
		unsigned char * rawData = malloc(
			ceilRoundDivide(fcbArray[fd].bytesWritten, vcb->blockSize) * vcb->blockSize);
		LBAread(rawData, ceilRoundDivide(fcbArray[fd].size, vcb->blockSize),
			fcbArray[fd].startingBlock);

		// Add data still in buffer
		memcpy(rawData + (fcbArray[fd].blocksDone * vcb->blockSize),
			fcbArray[fd].buff, fcbArray[fd].index);

		// Write data to new region
		LBAwrite(rawData, ceilRoundDivide(fcbArray[fd].bytesWritten, vcb->blockSize),
			fcbArray[fd].startingBlock);
	}

	free(fcbArray[fd].buff);

	fcbArray[fd].buff = NULL;

	return 0;
}


// This is a helper function designed to handle making a blob bigger. It handles allocating disk 
// space, and changing the size of its entry in the parent. Returning 1 means error
int writeSizeCheck(b_io_fd fd)
{

	// In case file's number of blocks increased
	if (ceilRoundDivide(fcbArray[fd].bytesWritten, vcb->blockSize) > 
		ceilRoundDivide(fcbArray[fd].size, vcb->blockSize))
	{
#if DEBUG_WRITECHECK
		printf("=>DEBUG_WRITECHECK: adding new blocks\n");
#endif

		// Get existing data 
		unsigned char * rawData = malloc(ceilRoundDivide(fcbArray[fd].size, vcb->blockSize) 
			* vcb->blockSize);
		LBAread(rawData, ceilRoundDivide(fcbArray[fd].size, vcb->blockSize),
			fcbArray[fd].startingBlock);

		// Mark those blocks as free
		markBlocksValue(fcbArray[fd].startingBlock,
			ceilRoundDivide(fcbArray[fd].size, vcb->blockSize), 1);

		// Find new start pos (possibly the same)
		int startPos = getNFreeBlocks(ceilRoundDivide(fcbArray[fd].bytesWritten, vcb->blockSize));

		if (startPos == -1)
		{
			printf("ERROR: the filesystem has run out of disk space. No changes made\n");
			return 1;
		}
		
		// Write data to new region
		LBAwrite(rawData, ceilRoundDivide(fcbArray[fd].size, vcb->blockSize), startPos);

		// Update fcb data on where the starting block is
		fcbArray[fd].startingBlock = startPos;

		// Change parent to recognize change
		fdDir * parentDirp = malloc(sizeof(fdDir));
		if (parsePath(fcbArray[fd].parentDirName, parentDirp) != 0)
			printf("Error with parse path\n");
		changeEntryWithName(parentDirp, fcbArray[fd].thisFileName, -1, startPos);
		free(parentDirp);
	}


	// In case file's number of bytes increased, change the directory entry
	if (fcbArray[fd].bytesWritten > fcbArray[fd].size)
	{

#if DEBUG_WRITECHECK
		printf("=>DEBUG_WRITECHECK: increasing file size\n");
#endif

		// Open up the parent
		fdDir * parentDirp = malloc(sizeof(fdDir));
		if (parsePath(fcbArray[fd].parentDirName, parentDirp) != 0)
			printf("Error with parse path\n");

		// Write the new size
		changeEntryWithName(parentDirp, fcbArray[fd].thisFileName, fcbArray[fd].bytesWritten, -1);

		// Close the parent
		free(parentDirp);

		// Update the fcb
		fcbArray[fd].size = fcbArray[fd].bytesWritten;
	}

	return 0;

}
