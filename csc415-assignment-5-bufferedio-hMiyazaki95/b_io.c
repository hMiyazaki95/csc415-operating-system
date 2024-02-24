/**************************************************************
* Class:  CSC-415-0# Spring 2023
* Name: Hajime Miyazaki
* Student ID: 922402751
* GitHub UserID: hMiyazaki95
* Project: Assignment 5 – Buffered I/O
*
* File: b_io.c
*
* Description: To handle buffered IO where you do the buffering
*
**************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "b_io.h"
#include "fsLowSmall.h"

#define MAXFCBS 20	//The maximum number of files open at one time


// This structure is all the information needed to maintain an open file
// It contains a pointer to a fileInfo structure and any other information
// that you need to maintain your open file.
typedef struct b_fcb
	{
	fileInfo * fi;	//holds the low level systems file info

	// Add any other needed variables here to track the individual open file
	char	chunkBuf[B_CHUNK_SIZE];
	char	*bufLoc;
	uint64_t	lbaPosition;
	int		numCharRemain;
#define DEBUG
#ifdef DEBUG
  int		requestCount;
#endif
	} b_fcb;
	
//static array of file control blocks
b_fcb fcbArray[MAXFCBS];

// Indicates that the file control block array has not been initialized
int startup = 0;	

// Method to initialize our file system / file control blocks
// Anything else that needs one time initialization can go in this routine
void b_init ()
	{
	if (startup)
		return;			//already initialized

	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].fi = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free File Control Block FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].fi == NULL)
			{
			fcbArray[i].fi = (fileInfo *)-2; // used but not assigned
			return i;		//Not thread safe but okay for this project
			}
		}

	return (-1);  //all in use
	}

// b_open is called by the "user application" to open a file.  This routine is 
// similar to the Linux open function.  	
// You will create your own file descriptor which is just an integer index into an
// array of file control blocks (fcbArray) that you maintain for each open file.  
// For this assignment the flags will be read only and can be ignored.
#ifdef DEBUG
#define MAX_FILES 4
FILE *debug[MAX_FILES]; // debug
FILE *blockFile[MAX_FILES];
FILE *returnBuffer[MAX_FILES];
FILE *requestOut[MAX_FILES];
#endif

b_io_fd b_open (char * filename, int flags)
	{
	if (startup == 0) b_init();  //Initialize our system

	//*** TODO ***//  Write open function to return your file descriptor
	//				  You may want to allocate the buffer here as well
	//				  But make sure every file has its own buffer

#ifdef DEBUG
	if (0) {
		debug[0] = stdout;
		debug[1] = stdout;
	} else {
		debug[0] = fopen("debug-0.txt", "w");
		debug[1] = fopen("debug-1.txt", "w");
	}
	returnBuffer[0] = fopen("returnBuffer-0.txt", "w");
	returnBuffer[1] = fopen("returnBuffer-1.txt", "w");
	blockFile[0] = fopen("blockFile-0.txt", "w");
	blockFile[1] = fopen("blockFile-1.txt", "w");
	requestOut[0] = fopen("requestOut-0.txt", "w");
	requestOut[1] = fopen("requestOut-1.txt", "w");
#endif

	// This is where you are going to want to call GetFileInfo and b_getFCB
	fileInfo *fInfo= GetFileInfo (filename);
	if (fInfo == NULL)
		{
		// Could not open file
		return (-1);
		}

	b_io_fd fcbIndex = b_getFCB();
	if (fcbIndex == -1)
		{
		// All 20 FCBs are in use.
		return (-1);
		}
	// Save pointer to file info
	fcbArray[fcbIndex].fi = fInfo;
#ifdef DEBUG
	fprintf(debug[fcbIndex], "f_open filename %s location %d index: %d XYZ\n",
		filename, fInfo->location, fcbIndex);
#endif
	fcbArray[fcbIndex].lbaPosition = fInfo->location;
	// Initialize bufLoc pointer to the end of the buffer to indicate I
	// have not read any buffer yet
	fcbArray[fcbIndex].bufLoc = &(fcbArray[fcbIndex].chunkBuf[B_CHUNK_SIZE]);

  // Maximum number of chars to return is based on the file size
	fcbArray[fcbIndex].numCharRemain = fInfo->fileSize;

#ifdef DEBUG
	fcbArray[fcbIndex].requestCount = 0;
#endif
  return(fcbIndex);
	}

#ifdef DEBUG
// buffer may not be '\0' terminated,
// so make a copy that is for printing and strlen
char debug_buffer[10000];
char *safeBuffer (char *buffer, int count)
{
  strncpy(debug_buffer, buffer, count);
	// For printing and strlen
	debug_buffer[count] = '\0';
	return(debug_buffer);
}
#endif

// b_read functions just like its Linux counterpart read.  The user passes in
// the file descriptor (index into fcbArray), a buffer where thay want you to 
// place the data, and a count of how many bytes they want from the file.
// The return value is the number of bytes you have copied into their buffer.
// The return value can never be greater then the requested count, but it can
// be less only when you have run out of bytes to read.  i.e. End of File	
int b_read (b_io_fd fd, char * buffer, int count)
	{
	//*** TODO ***//  
	// Write buffered read function to return the data and # bytes read
	// You must use LBAread and you must buffer the data in B_CHUNK_SIZE byte chunks.

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

	// and check that the specified FCB is actually in use	
	if (fcbArray[fd].fi == NULL)		//File not open for this descriptor
		{
		return -1;
		}	

	// Your Read code here - the only function you call to get data is LBAread.
	// Track which byte in the buffer you are at, and which block in the file	

	// Save the number of chars the caller is asking for
	int enterCount = count;
  // Count the number of bytes actually copied into buffer
	int retCount = 0;
	// Keep track of where in the buffer I need to copy blocks to
	char *outBuffer = buffer;

#ifdef DEBUG
	fprintf(debug[fd], "ENTER fileName %s count %d location %d charRemain: %d\n",
		fcbArray[fd].fi->fileName, count, fcbArray[fd].fi->location,
			fcbArray[fd].numCharRemain);
#endif

  // Don't even do a strcpy if there is nothing left to copy
	if (fcbArray[fd].numCharRemain <= 0) {
#ifdef DEBUG
		fprintf(debug[fd], "RETURN 0: filename: %s\n",
			fcbArray[fd].fi->fileName);
		fprintf(requestOut[fd], "A filename: %s count: %d ret: 0 actual: %d\n",
			fcbArray[fd].fi->fileName, enterCount, fcbArray[fd].requestCount);
#endif
		return 0;
	}

#ifdef DEBUG
	if(count > fcbArray[fd].numCharRemain) {
		fprintf(debug[fd], "Help\n");
	}
#endif

// number of chars remaining in chunkBuf by subtracting the beginning
// of chunkBuf from the remaining chunkBuf pointer, bufLoc. 
// At the beginning, bufLoc == chunkBuf, so the difference is 0,
// and 512 - 0 means that there are 512 chars in chunkBuf.
// When chunkBuf has been entirely copied to buffer,
// bufLoc == chunkBuf[B_CHUNK_SIZE], so the difference is 512, and
// 512 - 512 = 0 meaning there are no chars left in chunkBuf to copy
// into buffer.
#define NUM_OF_CHARS_REMAIN_IN_CHUNKBUF \
		(B_CHUNK_SIZE - (fcbArray[fd].bufLoc - fcbArray[fd].chunkBuf))

  // If count is asking for more than I have in chunkBuf,
	// I need to get more blocks.
  while (count > NUM_OF_CHARS_REMAIN_IN_CHUNKBUF)
		{
			// strcpy is safe since fcbArray[fd].chunkBuf has an extra '\0' at the end
			// Copy out what I have
			int len = NUM_OF_CHARS_REMAIN_IN_CHUNKBUF;
			// Copy no more than the number of chars remaining
			if (len > fcbArray[fd].numCharRemain)
				len == fcbArray[fd].numCharRemain;

			// NOTE: memcpy is ok if len == 0
			memcpy(outBuffer, fcbArray[fd].bufLoc, len);

		  // Next copy from chunkBuf to buffer starts at the end of what I
			// just copied
			outBuffer += len;
			count -= len;
			retCount += len;
			// Keep track of number of chars left in the file.
			fcbArray[fd].numCharRemain -= len;
#ifdef DEBUG
			if (count < 0) {
			  fprintf(debug[fd], "DEBUG: count should never be negative.");
			}
#endif

			if (fcbArray[fd].numCharRemain <= 0) {
#ifdef DEBUG
				fcbArray[fd].requestCount += retCount;
				fprintf(debug[fd],
					"RETURN 1: filename: %s remain: %d<=0 count: %d >>>%s<<< last: '%c' bad: '%c'\n",
					fcbArray[fd].fi->fileName, fcbArray[fd].numCharRemain,
					retCount, safeBuffer(buffer, retCount),
					buffer[retCount-1], buffer[retCount]);
				fprintf(requestOut[fd], "B filename: %s count: %d ret: %d actual: %d\n",
					fcbArray[fd].fi->fileName, enterCount, retCount, fcbArray[fd].requestCount);
#endif
				return retCount;
			}

			uint64_t blocksRead =
				LBAread(fcbArray[fd].chunkBuf, 1, fcbArray[fd].lbaPosition);
			fcbArray[fd].lbaPosition += blocksRead;

			// reset chunkBuf pointer to the beginning of the chunkBuf
			fcbArray[fd].bufLoc = fcbArray[fd].chunkBuf;

#ifdef DEBUG
			fprintf(debug[fd],
				"blocksRead %ld filename %s string >>>%s<<< lbaPosition %ld\n",
				blocksRead, fcbArray[fd].fi->fileName,
				safeBuffer(fcbArray[fd].chunkBuf,  B_CHUNK_SIZE),
				fcbArray[fd].lbaPosition);
			fprintf(blockFile[fd], "%s",
				safeBuffer(fcbArray[fd].chunkBuf,  B_CHUNK_SIZE));
#endif

			if (blocksRead == 0) {
				// Nothing more to read
#ifdef DEBUG
				fcbArray[fd].requestCount += retCount;
				fprintf(debug[fd],
					"RETURN 2: filename: %s count: %d >>>%s<<< last: '%c' bad: '%c'\n",
					fcbArray[fd].fi->fileName, retCount, safeBuffer(buffer, retCount),
					buffer[retCount-1], buffer[retCount]);
				fprintf(returnBuffer[fd], "%s", buffer);
				fprintf(debug[fd], "CHECK1: charRemain: %d bufChar: %ld\n",
						fcbArray[fd].numCharRemain, strlen(safeBuffer(buffer, retCount)));
				fprintf(requestOut[fd], "C filename: %s count: %d ret: %d actual: %d\n",
					fcbArray[fd].fi->fileName, enterCount, retCount, fcbArray[fd].requestCount);
#endif
				return retCount;
			}
		}

		// count < the number of chars in the chunkBuf
		// so, give the caller the number of chars they asked for

		// Limit number of chars to copy, based on the file size
		if (count > fcbArray[fd].numCharRemain)
			count = fcbArray[fd].numCharRemain;

		// count limits copy into outBuffer, which might mean not writing a '\0'
		// as appropriate
		// buffer argument and chunkBuf memory locations do not overlaps,
		// as memcpy requires
		memcpy(outBuffer, fcbArray[fd].bufLoc, count);
		fcbArray[fd].bufLoc += count;
		retCount += count;
		fcbArray[fd].numCharRemain -= count;

		// Return what I actually copied
#ifdef DEBUG
		fcbArray[fd].requestCount += retCount;
		fprintf(debug[fd],
			"RETURN 3: filename: %s count: %d >>>%s<<< last: '%c' bad: '%c'\n",
			fcbArray[fd].fi->fileName, retCount, safeBuffer(buffer, retCount),
			buffer[retCount-1], buffer[retCount]);
		fprintf(returnBuffer[fd], "%s", safeBuffer(buffer, retCount));
		fprintf(debug[fd], "CHECK2: charRemain: %d bufChar: %ld buffer: >>%s<< chunk: ###%s###\n",
			fcbArray[fd].numCharRemain, strlen(buffer), safeBuffer(buffer, retCount),
			fcbArray[fd].bufLoc);
		fprintf(requestOut[fd], "D filename: %s count: %d ret: %d actual: %d\n",
			fcbArray[fd].fi->fileName, enterCount, retCount, fcbArray[fd].requestCount);
#endif
		return (retCount);
		}
	


// b_close frees an allocated memory and places the file control block back 
// into the unused pool of file control blocks.
int b_close (b_io_fd fd)
	{
	//*** TODO ***//  Release any resources
	}
	
