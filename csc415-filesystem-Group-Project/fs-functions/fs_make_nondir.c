/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fs_make_nondir.c
*
* Description: Used to create the entry for a new non-directory blob
*              
*
**************************************************************/

#include "mfs.h"
#include "generalUtils.h"
#include "fsMapManager.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>


int fs_make_nondir(char * filename)
{

    char parentDirName[DIRMAX_LEN];
    char newBlobName[DIRMAX_LEN];

    if (splitBlobPath(filename, parentDirName, newBlobName) != 0)
    {
        printf("ERROR: Some problem with splitBlobPath\n");
        return 1;
    }

#if DEBUG_MKFILE
    printf("=>DEBUG_MKFILE: parentDirName '%s', newDirName '%s'\n", parentDirName, newBlobName);
#endif

    if (strcmp(newBlobName, ".") == 0 || strcmp(newBlobName, "..") == 0)
    {
        printf("Oops... can't make a file with that name :'(\n");
        return 1;
    }

    fdDir * parentDir = fs_opendir(parentDirName);

    int posInParentDir = fs_nextFreeSpot(parentDir);

#if DEBUG_MKFILE
    printf("=>DEBUG_MKFILE: position in parent directory: %d\n", posInParentDir);
#endif

    // Expand parent directory size
    if (posInParentDir == -1)
    {
        if(expandDir(parentDir) != 0)
        {
            fs_closedir(parentDir);
            return -1;
        }
    }


    //////////  Update Parent Section  //////////

    int parentDirBlocks, parentDirBytes;

    sizeofNDirEntries(parentDir->d_reclen, &parentDirBlocks, &parentDirBytes);

    DirectoryEntry * buffer = malloc(parentDirBytes);
    
    LBAread(buffer, parentDirBlocks, parentDir->directoryStartBlock);

    buffer[posInParentDir] = makeDirectoryEntry(0, 0, FALSE, newBlobName);

    LBAwrite(buffer, parentDirBlocks, parentDir->directoryStartBlock);
    free(buffer);
    
    fs_closedir(parentDir);

    return 0;
}
