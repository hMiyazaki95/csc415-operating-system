/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "generalUtils.h"
#include "fsMapManager.h"

VolumeControlBlock * vcb;

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n",
		numberOfBlocks, blockSize);

	vcb = malloc(blockSize);

	LBAread(vcb, 1, 0);

	// Check if the magic number is present (see if the volume is formatted)
	if (strncmp(vcb->magic, "4THREADS", 8) == 0)
	{
		printf("Volume is formatted\n");

		// Checks if our volume is the correct size
		if (vcb->numberOfBlocks == numberOfBlocks)
			printf("Expected number of blocks found\n");
		else
		{
			printf("ERROR: unexpected number of blocks found. Please delete the existing volume if"
				"you want to change the block count\n");
			free(vcb);
			return 1;
		}


		if (vcb->blockSize == blockSize)
			printf("Expected block size found\n");
		else
		{
			printf("ERROR: unexpected block size found. Please delete the existing volume if you"
				" want to change the block size\n");
			free(vcb);
			return 1;
		}


		//////////  Get FS-Map Section  //////////

		int bytesForFreeSpace = ceilRoundDivide(numberOfBlocks, 8);
		int blocksForFreeSpace = ceilRoundDivide(bytesForFreeSpace, blockSize);

		g_fsBuffer = malloc(blocksForFreeSpace * blockSize);		

		LBAread(g_fsBuffer, blocksForFreeSpace, 1);

	}
	else
	{
		printf("Formatting volume...\n");



		//////////  Volume Control Block Section  //////////

		strncpy(vcb->magic, "4THREADS", 8);
		vcb->numberOfBlocks = numberOfBlocks;
		vcb->blockSize = blockSize;

		int bytesForFreeSpace = ceilRoundDivide(numberOfBlocks, 8);
		int blocksForFreeSpace = ceilRoundDivide(bytesForFreeSpace, blockSize);

		vcb->rootDirLocat = blocksForFreeSpace + 1;
		vcb->numRootEntries = NEWDIR_NUM_ENTRIES;

		vcb->fsMapStart = 1; // This will always be 1

		// Write the block now
		LBAwrite(vcb, 1, 0);



		//////////  Free Space Map Section  //////////

		g_fsBuffer = malloc(bytesForFreeSpace);

		// Initialize all bits in the free space map to 1 (block is free)
		markBlocksValue(0, bytesForFreeSpace, 1);

		// bytes for the "." and ".." directories + 18 other placeholders
		int rootDirSize = NEWDIR_NUM_ENTRIES * sizeof(DirectoryEntry);
		
		// Mark the VCB block, free space map, and root directory as occupied
		markBlocksValue(0, blocksForFreeSpace + ceilRoundDivide(rootDirSize, blockSize), 0);
		


		//////////  Root Directory Section  //////////

		DirectoryEntry * buffer = malloc(rootDirSize);

		buffer[0] = makeDirectoryEntry(rootDirSize, blocksForFreeSpace + 1, TRUE, ".");

		buffer[1] = makeDirectoryEntry(rootDirSize, blocksForFreeSpace + 1, TRUE, "..");

		// Write on the empty placeholder directories
		for (int i = 2; i < NEWDIR_NUM_ENTRIES; i++)
		{
			buffer[i] = makeDirectoryEntry(rootDirSize, blocksForFreeSpace + 1, TRUE, "");
		}

		LBAwrite(buffer, ceilRoundDivide(rootDirSize, blockSize), blocksForFreeSpace + 1);
		free(buffer);


		printf("Volume formatted\n");
	}

	return 0;
}
	
	
void exitFileSystem()
{
	int bytesForFreeSpace = ceilRoundDivide(vcb->numberOfBlocks, 8);

	// Write the free space map to disk. Will always start at block num 1
	LBAwrite(g_fsBuffer, ceilRoundDivide(bytesForFreeSpace, vcb->blockSize), 1);

	LBAwrite(vcb, 1, 0);

	free(vcb);
	free(g_fsBuffer);

	printf("System exiting\n");
}
