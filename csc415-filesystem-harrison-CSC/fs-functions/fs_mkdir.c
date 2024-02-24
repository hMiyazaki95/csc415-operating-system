/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fs_mkdir.c
*
* Description: Creates a new empty directory. Empty here means that the directory only has "." and 
*              ".." as children
*              
*
**************************************************************/

#include "mfs.h"
#include "generalUtils.h"
#include "fsMapManager.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


int fs_mkdir(const char * pathname, mode_t mode)
{

    char parentDirName[DIRMAX_LEN];
    char newDirName[DIRMAX_LEN];

    if (splitBlobPath(pathname, parentDirName, newDirName) != 0)
    {
        printf("ERROR: Some problem with splitBlobPath\n");
        return 1;
    }


#if DEBUG_MKDIR
    printf("=>DEBUG_MKDIR: parentDirName '%s', newDirName '%s'\n", parentDirName, newDirName);
#endif

    if (strcmp(newDirName, ".") == 0 || strcmp(newDirName, "..") == 0)
    {
        printf("Oops... can't make a directory with that name :'(\n");
        return 1;
    }

    fdDir * temp = malloc(sizeof(fdDir));
       int val =  parsePath(parentDirName, temp);
        //The default number of entries is 20
	if(fs_getChildWithName(temp, newDirName) != NULL){
		printf("Err: directory already exists\n");
		return 7;
	}
	//handling wrong path
	if(val == -1){
		printf("Err: wrong path\n");
		return 8;
	}
	free(temp);


    fdDir * parentDir = fs_opendir(parentDirName);

    int posInParentDir = fs_nextFreeSpot(parentDir);

#if DEBUG_MKDIR
    printf("=>DEBUG_MKDIR: position in parent directory: %d\n", posInParentDir);
#endif


    // Expand parent directory size
    if (posInParentDir == -1)
    {
        if(expandDir(parentDir) != 0)
            return -1;
    }


    //////////  Make New Directory Section  //////////

    int newDirSizeBlocks, newDirSizeBytes;

    sizeofNDirEntries(NEWDIR_NUM_ENTRIES, &newDirSizeBlocks, &newDirSizeBytes);

    int startPos = getNFreeBlocks(newDirSizeBlocks);


#if DEBUG_MKDIR
    printf("=>DEBUG_MKDIR: new start block: %d\n", startPos);
#endif


    if (startPos == -1) // This means we have run out of room on the volume
    {
        fs_closedir(parentDir);
        return 2;
    }


    DirectoryEntry * buffer = malloc(newDirSizeBytes);

    buffer[0] = makeDirectoryEntry(NEWDIR_NUM_ENTRIES * sizeof(DirectoryEntry),
        startPos, TRUE, ".");
    buffer[1] = makeDirectoryEntry(NEWDIR_NUM_ENTRIES * sizeof(DirectoryEntry),
        parentDir->directoryStartBlock, TRUE, "..");

    for (int i = 2; i < NEWDIR_NUM_ENTRIES; i++)
    {
        // Empty string name indicates not valid entry
        buffer[i] = makeDirectoryEntry(NEWDIR_NUM_ENTRIES * sizeof(DirectoryEntry),
            startPos, TRUE, "");
    }

    LBAwrite(buffer, newDirSizeBlocks, startPos);
    free(buffer);





    //////////  Update Parent Section  //////////

    int parentDirBlocks, parentDirBytes;

    sizeofNDirEntries(parentDir->d_reclen, &parentDirBlocks, &parentDirBytes);

    buffer = malloc(parentDirBytes);
    
    LBAread(buffer, parentDirBlocks, parentDir->directoryStartBlock);

    buffer[posInParentDir] = makeDirectoryEntry(NEWDIR_NUM_ENTRIES * sizeof(DirectoryEntry),
        startPos, TRUE, newDirName);

    LBAwrite(buffer, parentDirBlocks, parentDir->directoryStartBlock);
    free(buffer);




    // Indicate that the blocks are now used by the new directory
    markBlocksValue(startPos, newDirSizeBlocks, 0);


    fs_closedir(parentDir);

    return 0;
}
