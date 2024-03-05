/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fs_rmdir.c
*
* Description: This function takes in a directory name given by the user and deletes it and it's 
*              files. It will throw throw an error if not given a directory or given a file.
*
**************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "mfs.h"
#include "generalUtils.h"
#include "fsMapManager.h"

/* Tries to remove a directory. It is assumed that the input pathname is a valid path to a valid
directory. The directory being empty is not assumed, and that possibility is handled by this
function. Returning a 0 means success. */
int fs_rmdir(const char *pathname){

    char cpyPath[DIRMAX_LEN];
    strcpy(cpyPath, pathname);
    if (fs_isDir(cpyPath) != 1)
        return -1;

    char parentDirName[DIRMAX_LEN];
    char toRemoveName[DIRMAX_LEN];

    if (splitBlobPath(pathname, parentDirName, toRemoveName) != 0)
    {
        printf("ERROR: Some problem with splitBlobPath\n");
        return 1;
    }


#if DEBUG_RM
    printf("=>DEBUG_RM: parentDirName '%s', toRemoveName '%s'\n", parentDirName, toRemoveName);
#endif

    if (strcmp(toRemoveName, ".") == 0 || strcmp(toRemoveName, "..") == 0)
    {
        printf("'.' and '..' not able to be deleted\n");
        return 1;
    }

    fdDir * parentDirp = fs_opendir(parentDirName);
    fs_diriteminfo * toRemove = fs_getChildWithName(parentDirp, toRemoveName);


    int numChildren = fs_itemNumChildren(toRemove);

#if DEBUG_RM
        printf("=>DEBUG_RM: Directory has %d children\n", numChildren);
#endif

    if (numChildren > 2)
    {
        printf("This cannot be removed due to having children\n");
        return 1;
    }

    removeDirEntry(parentDirp, toRemove);


    // Label the blocks as available
    markBlocksValue(toRemove->blockStart, ceilRoundDivide(toRemove->d_reclen, vcb->blockSize), 1);

    free(toRemove);
    fs_closedir(parentDirp);
    return 0;
}

