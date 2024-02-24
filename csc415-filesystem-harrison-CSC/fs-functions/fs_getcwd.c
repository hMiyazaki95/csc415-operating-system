/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fs_getcwd.c
*
* Description: Returns a copy of the current working directory global variable to the passed in
               char array. Max length is DIRMAX_LEN, which expands to 4096 (plus null terminator)
*
**************************************************************/

#include <string.h>

#include "mfs.h"

char * fs_getcwd(char * pathname, size_t size)
{
    strncpy(pathname, g_currDir, size);
    return pathname;
}