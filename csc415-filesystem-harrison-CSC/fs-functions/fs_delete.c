/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fs_delete.c
*
* Description: Deletes a non-directory blob
*
**************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mfs.h"
#include "generalUtils.h"
#include "fsMapManager.h"

/* Used to delete any file that is not a directory. It can be assumed that the input is a valid 
path to a valid non-directory file. This is the non-directory equivalent of fs_rmdir */
int fs_delete(char* filename){

    if (fs_isFile(filename) != 1)
        return 1;

	//Determine if given nothing
	if(filename == NULL){
		printf("No file name given");
		return 1;
	}

	char parentDirName[DIRMAX_LEN];
    char toDeleteName[DIRMAX_LEN];

    if (splitBlobPath(filename, parentDirName, toDeleteName) != 0)
    {
        printf("ERROR: Some problem with splitBlobPath\n");
        return 1;
    }

#if DEBUG_DELETE
    printf("=>DEBUG_DELETE: parentDirName '%s', toDeleteName '%s'\n", parentDirName, toDeleteName);
#endif

	fdDir * parentDirp = fs_opendir(parentDirName);
    fs_diriteminfo * toDelete = fs_getChildWithName(parentDirp, toDeleteName);

    if (toDelete == NULL)
    {

#if DEBUG_DELETE
        printf("=>DEBUG_DELETE: Not removing file because it couldn't be found\n");
#endif

        fs_closedir(parentDirp);
        free(toDelete);
        return -1;	
    }

    removeDirEntry(parentDirp, toDelete);


    // Label the blocks as available
    markBlocksValue(toDelete->blockStart, ceilRoundDivide(toDelete->d_reclen, vcb->blockSize), 1);

    free(toDelete);
    fs_closedir(parentDirp);

	return 0;
}
