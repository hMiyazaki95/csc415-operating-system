/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fs_stat.c
*
* Description: Stat outputs information about the various properties in a specified file. We give
*		the function a pathfile and struct that contains our information
*
**************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "generalUtils.h"
#include "mfs.h"

int fs_stat(const char *path, struct fs_stat *buf){
	
        char parentDirName[DIRMAX_LEN];
        char blobName[DIRMAX_LEN];

        if (splitBlobPath(path, parentDirName, blobName) != 0)
        {
                printf("ERROR: Some problem with splitBlobPath\n");
                return 1;
        }

        //Temporary buffer to hold our values returned by parsePath
	fdDir * parentInfo = malloc(sizeof(fdDir));

	// If the file doesn't exist return error
        int index = parsePath(parentDirName, parentInfo);
        if(index != 0){
                free(parentInfo);
                return 1;
        }

        fs_diriteminfo * childInfo = fs_getChildWithName(parentInfo, blobName);

        if (childInfo == NULL)
        {
                free(parentInfo);
                return 1;
        }

	//Fills buffer with our values
	buf->st_size = childInfo->d_reclen;
	buf->st_blocks = ceilRoundDivide(buf->st_size, vcb->blockSize);
	buf->st_createtime = childInfo->created;

        free(parentInfo);
        free(childInfo);

	return 0;
}
