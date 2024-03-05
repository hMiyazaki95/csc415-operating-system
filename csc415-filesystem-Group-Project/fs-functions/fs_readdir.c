/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fs_readdir.c
*
* Description: This file contains the function fs_readdir, which is used to get a single
*              fs_diriteminfo * from a fdDir *
*
**************************************************************/

#include "stdlib.h"
#include <string.h>

#include "mfs.h"
#include "generalUtils.h"

fs_diriteminfo * fs_readdir(fdDir * dirp)
{
    // Indicates we are done
    if (dirp->dirEntryPosition >= dirp->d_reclen)
    {

#if DEBUG_READDIR
    printf("=>DEBUG_READDIR: Returning NULL due to reaching the last permitted entry\n");
#endif

        return NULL;
    }
    
    fs_diriteminfo * tmpInfo = malloc(sizeof(fs_diriteminfo));

    DirectoryEntry * tmpBuff = malloc(ceilRoundDivide(sizeof(DirectoryEntry) * dirp->d_reclen,
        vcb->blockSize) * vcb->blockSize);

    LBAread(tmpBuff, ceilRoundDivide(sizeof(DirectoryEntry) * dirp->d_reclen, vcb->blockSize),
        dirp->directoryStartBlock);

    strcpy(tmpInfo->d_name, tmpBuff[dirp->dirEntryPosition].fileName);
    
    strcpy(tmpInfo->d_fullName, dirp->dirPathName);
    strcat(tmpInfo->d_fullName, tmpInfo->d_name);

    if (strlen(tmpInfo->d_name) == 0)
    {

#if DEBUG_READDIR
    printf("=>DEBUG_READDIR: Returning NULL due to reaching an empty string name\n");
#endif

        free(tmpInfo);
        free(tmpBuff);
        return NULL;
    }


    tmpInfo->blockStart = tmpBuff[dirp->dirEntryPosition].startPos;

    tmpInfo->posInParent = dirp->dirEntryPosition;
    tmpInfo->d_reclen = tmpBuff[dirp->dirEntryPosition].size;
    tmpInfo->isDir = tmpBuff[dirp->dirEntryPosition].isDirectory;
    tmpInfo->created = tmpBuff[dirp->dirEntryPosition].created;

    free(tmpBuff);

    dirp->dirEntryPosition++;

    return tmpInfo;
}

int fs_nextFreeSpot(fdDir * dirp)
{
    int i = 0;
	fs_diriteminfo * di = fs_readdir(dirp);
	while (di != NULL) 
	{
		i++;
        free(di);
		di = fs_readdir(dirp);
    }
    free(di);

    if (i >= dirp->d_reclen)
        return -1;
    else
        return i;
}

/* Gets the child entry in `dirp` with the name of `name`. NULL means that
the no entry exists with that name */
fs_diriteminfo * fs_getChildWithName(fdDir * dirp, const char * name)
{   
	fs_diriteminfo * di = fs_readdir(dirp);
	while (di != NULL) 
	{
		if (strncmp(di->d_name, name, strlen(name)) == 0)
            return di;

        free(di);
		di = fs_readdir(dirp);
    }
    free(di);
    return NULL;
}

int fs_itemNumChildren(fs_diriteminfo * item)
{
    DirectoryEntry * buffer = malloc(
        ceilRoundDivide(item->d_reclen, vcb->blockSize) * vcb->blockSize);

    LBAread(buffer, ceilRoundDivide(item->d_reclen, vcb->blockSize), item->blockStart);

    for (int i = 0; i < item->d_reclen / sizeof(DirectoryEntry); i++)
    {
        if (strcmp(buffer[i].fileName, "") == 0)
        {
            free(buffer);
            return i;
        }
    }

    free(buffer);
    return item->d_reclen / sizeof(DirectoryEntry);
}