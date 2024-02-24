
/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fs_closedir.c
*
* Description: Frees the memory that was allocated by fs_opendir
*
**************************************************************/

#include <stdlib.h>

#include "mfs.h"

int fs_closedir(fdDir * dirp)
{
    free(dirp);
}   
