/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fs_opendir.c
*
* Description: Used to  return a fdDir * from a pathname (const char *) input
*
**************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "generalUtils.h"
#include "mfs.h"

fdDir * fs_opendir(const char * pathname){

    fdDir * toReturn = malloc(sizeof(fdDir));

    if (strcmp(pathname, "/") == 0)
    {
        toReturn->d_reclen = vcb->numRootEntries;
        toReturn->dirEntryPosition = 0; // Start at entry number 0
        toReturn->directoryStartBlock = vcb->rootDirLocat;
        strcpy(toReturn->dirPathName, "/");
        return toReturn;
    }
    else
    {
        int retVal = parsePath(pathname, toReturn);
        if (retVal != 0)
            printf("Error value from parsePath\n");
        
        return toReturn;
    }
}
