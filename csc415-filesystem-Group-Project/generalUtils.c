/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: generalUtils.c
*
* Description: Stores a few helper functions to be used elsewhere in the program
*
**************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "generalUtils.h"
#include "mfs.h"
#include "fsMapManager.h"

unsigned long exponent(unsigned long base, unsigned long power)
{
    unsigned long toReturn = 1;
    for (int i = 0; i < power; i++)
        toReturn *= base;

    return toReturn;
}

unsigned int ceilRoundDivide(unsigned int numerator, unsigned int denominator)
{
	return (numerator + denominator - 1) / denominator;
}

unsigned char getBitInUChar(unsigned char inputByte, int position)
{
    unsigned char binaryVal = exponent(2, position - 1);
    return (inputByte & binaryVal) / binaryVal;
}

void changeBitInBuffer(unsigned char * buffer, int position, unsigned char newValue)
{
    unsigned char binaryVal = exponent(2, (position % 8));
    if (newValue == 1)
        buffer[position / 8] |= binaryVal;
    else
        buffer[position / 8] &= (255 - binaryVal);
}

DirectoryEntry makeDirectoryEntry(unsigned int size, unsigned int startPos,
	BOOL isDirectory, const char * fileName)
{
	// creates data structure inside the procedure
	DirectoryEntry tempDirectoryEntry;
	//fills out the various fields within the data structure
	tempDirectoryEntry.size = size;
	tempDirectoryEntry.startPos = startPos;
	tempDirectoryEntry.created = time(NULL);
	tempDirectoryEntry.isDirectory = isDirectory;
	strncpy(tempDirectoryEntry.fileName, fileName, 256);
	// return the storage location of the data structure inside the procedure
	return tempDirectoryEntry;
}

void examineDirectoryEntry(DirectoryEntry de)
{
	printf("\n");
	printf("  name: %s\n", de.fileName);
	printf("  start block: %d\n", de.startPos);
	printf("  size: %d\n", de.size);
	printf("\n");
}

int parsePath(const char * userInput, fdDir * dirp){
    
	char inputCopy[256];
	
	strncpy(inputCopy, userInput, 256);

	char * token = strtok(inputCopy, "/");
    
#if DEBUG_PARSEPATH
	if (token == NULL)
        printf("=>DEBUG_PARSEPATH: first token is NULL\n");
#endif

	// Always start with root directory
	dirp->d_reclen = vcb->numRootEntries;
	dirp->dirEntryPosition = 0; // Start at entry number 0
	dirp->directoryStartBlock = vcb->rootDirLocat;
	dirp->isDirectory = TRUE;
	strcpy(dirp->dirPathName, "/");
	
    while (token != NULL)
	{

#if DEBUG_PARSEPATH
			printf("=>DEBUG_PARSEPATH: Processing token '%s'\n", token);
#endif

		fs_diriteminfo * childDir = fs_getChildWithName(dirp, token);


		if (childDir == NULL)
		{

#if DEBUG_PARSEPATH
			printf("=>DEBUG_PARSEPATH: Child '%s' not found\n", token);
#endif

			free(childDir);
			return -1;
		}
		else
		{

#if DEBUG_PARSEPATH
			printf("=>DEBUG_PARSEPATH: Moving to block number: %d\n", childDir->blockStart);
#endif

			// Update the current directory entry
			dirp->directoryStartBlock = childDir->blockStart;
			
			strcat(dirp->dirPathName, childDir->d_name);
			strcat(dirp->dirPathName, "/");

			dirp->d_reclen = childDir->d_reclen / sizeof(DirectoryEntry);
			dirp->isDirectory = childDir->isDir;

			dirp->created = childDir->created;

#if DEBUG_PARSEPATH
			printf("=>DEBUG_PARSEPATH: calcualting %d entries based on size of %d\n",
				dirp->d_reclen, childDir->d_reclen);
			printf("=>DEBUG_PARSEPATH: sizeof %d\n", sizeof(DirectoryEntry));
#endif

			free(childDir);

			dirp->dirEntryPosition = 0;
		}
        
        token = strtok(NULL, "/");
    }
	
    return 0;
}

void sizeofNDirEntries(int n, int * blocks, int * bytes)
{
	if (blocks != NULL)
		*blocks = ceilRoundDivide(n * sizeof(DirectoryEntry), vcb->blockSize);

	if (bytes != NULL)
	*bytes = (*blocks) * vcb->blockSize;
}

int expandDir(fdDir * parentDir)
{
	int parentOldSizeBlocks, parentOldSizeBytes;
	sizeofNDirEntries(parentDir->d_reclen, &parentOldSizeBlocks, &parentOldSizeBytes);

	DirectoryEntry * parentBuff = malloc(parentOldSizeBytes);

	int parentOldStartBlock = parentDir->directoryStartBlock;

	LBAread(parentBuff, parentOldSizeBlocks, parentDir->directoryStartBlock);


	markBlocksValue(parentDir->directoryStartBlock, parentOldSizeBlocks, 1);

	int newParentEntryNum = parentDir->d_reclen + 20;

	int newParentStart = getNFreeBlocks(newParentEntryNum);

	if (newParentStart == -1)
	{
		printf("ERROR: Volume out of space\n");
		return 2;
	}

	int parentNewSizeBlocks, parentNewSizeBytes;
	sizeofNDirEntries(newParentEntryNum, &parentNewSizeBlocks, &parentNewSizeBytes);


	// Change "." entry
	parentBuff[0].size = sizeof(DirectoryEntry) * newParentEntryNum;

	// Check if parent is root dir
	if (parentBuff[0].startPos == parentBuff[1].startPos)
	{
		parentBuff[0].startPos = newParentStart;

#if DEBUG_EXPANDDIR
		printf("=>DEBUG_EXPANDDIR: parent is the root directory\n");
#endif

		parentBuff[1].size = sizeof(DirectoryEntry) * newParentEntryNum;
		parentBuff[1].startPos = newParentStart;

		vcb->numRootEntries = newParentEntryNum;
		vcb->rootDirLocat = newParentStart;
	}
	// Parent is not root
	else
	{
		parentBuff[0].startPos = newParentStart;

#if DEBUG_EXPANDDIR
		printf("=>DEBUG_EXPANDDIR: parent is not the root directory\n");
#endif   

		int entriesInGrandparent = parentBuff[1].size / sizeof(DirectoryEntry);

		int grandparentStartBlock = parentBuff[1].startPos;

		int grandparentOldSizeBlocks, grandparentOldSizeBytes;
		sizeofNDirEntries(entriesInGrandparent, &grandparentOldSizeBlocks,
			&grandparentOldSizeBytes);

		DirectoryEntry * grandparentBuff = malloc(grandparentOldSizeBytes);

		LBAread(grandparentBuff, grandparentOldSizeBlocks, grandparentStartBlock);

		for (int i = 0; i < entriesInGrandparent; i++)
		{
			if (strcmp(grandparentBuff[i].fileName, "") != 0 &&
				grandparentBuff[i].startPos == parentOldStartBlock)
			{
				grandparentBuff[i].size = sizeof(DirectoryEntry) * newParentEntryNum;
				grandparentBuff[i].startPos = newParentStart;
			}
		}
		LBAwrite(grandparentBuff, grandparentOldSizeBlocks, grandparentStartBlock);
	}

	// Write changes to disk
	LBAwrite(parentBuff, parentNewSizeBlocks, newParentStart);
	free(parentBuff);

	// Mark blocks as occupied
	markBlocksValue(newParentStart, parentNewSizeBlocks, 0);

	// Change (fdDir *)
	parentDir->d_reclen = newParentEntryNum;
	parentDir->directoryStartBlock = newParentStart;
}

int splitBlobPath(const char * inPath, char * dirPath, char * blobPath)
{
    char pathnameCpy[DIRMAX_LEN];


    strcpy(pathnameCpy, inPath);
    

    // Remove a trailing slash if it exits
    if (pathnameCpy[strlen(pathnameCpy)-1] == '/')
        pathnameCpy[strlen(pathnameCpy)-1] = '\0';


	// Relative path option
    if (pathnameCpy[0] == '/')
    {
        int finalSlashIndex = -1;
        for (int i = strlen(pathnameCpy)-1; (i >= 0) && (finalSlashIndex == -1); i--)
        {
            if (pathnameCpy[i] == '/')
                finalSlashIndex = i;
        }

        if (finalSlashIndex == -1)
            return 1;
        
        strncpy(dirPath, pathnameCpy, finalSlashIndex + 1);
        dirPath[finalSlashIndex + 1] = '\0';

        strcpy(blobPath, pathnameCpy+finalSlashIndex+1);
    }
	// Relative path option
    else
    {
        strcpy(dirPath, g_currDir);

        int finalSlashIndex = -1;
        for (int i = strlen(pathnameCpy)-1; (i >= 0) && (finalSlashIndex == -1); i--)
        {
            if (pathnameCpy[i] == '/')
                finalSlashIndex = i;
        }

        strncat(dirPath, pathnameCpy, finalSlashIndex+1);

        strcpy(blobPath, pathnameCpy+finalSlashIndex+1);
    }

	return 0;
}

int changeEntryWithName(fdDir * dirp, const char * name, int newSize, int newStart)
{   
    int numParentBlocks, numParentBytes;
    sizeofNDirEntries(dirp->d_reclen, &numParentBlocks, &numParentBytes);

    DirectoryEntry * parentDir = malloc(numParentBytes);

    LBAread(parentDir, numParentBlocks, dirp->directoryStartBlock);

    BOOL done = FALSE;
    for (int i = 0; (i < dirp->d_reclen) && (done == FALSE); i++)
    {
        if (strcmp(parentDir[i].fileName, name) == 0)
        {
			if (newSize != -1)
            	parentDir[i].size = newSize;
			if (newStart != -1)
            	parentDir[i].startPos = newStart;
				
            done = TRUE;
        }
    }

    LBAwrite(parentDir, numParentBlocks, dirp->directoryStartBlock);

    free(parentDir);

    if (done == FALSE)
        return 1;
    else
        return 0;
}

void removeDirEntry(fdDir * parentDirp, fs_diriteminfo * toRemove)
{
	int parentSizeBlocks, parentSizeBuffBytes;
    sizeofNDirEntries(parentDirp->d_reclen, &parentSizeBlocks, &parentSizeBuffBytes);

    DirectoryEntry * buffer = malloc(parentSizeBuffBytes);

    LBAread(buffer, parentSizeBlocks, parentDirp->directoryStartBlock);


    BOOL exit = FALSE;
    for (int i = toRemove->posInParent; exit == FALSE; i++)
    {
        if (strcmp(buffer[i].fileName, "") != 0)
        {
            buffer[i] = buffer[i+1];
        }
        else
        {
            strcpy(buffer[i].fileName, "");
            exit = TRUE;
        }
    }


    LBAwrite(buffer, parentSizeBlocks, parentDirp->directoryStartBlock);

    free(buffer);
}
