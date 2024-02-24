/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fs_isFile.c
*
* Description: Used to determine wheterh a file, based upon its filename is a non-directory blob
*  	       
*
**************************************************************/


#include "mfs.h"
#include <unistd.h>
#include <stdio.h>
#include "generalUtils.h"
#include <string.h>
#include <stdlib.h>


/* This will return 1 if the file in question exists and is not a directory, else return 0. */
int fs_isFile(char * filename)
{

	fdDir * dirp = malloc(sizeof(fdDir));
        if(strcmp(filename, "/") == 0){
                free(dirp);
                return 0;
        }else{
                int result = parsePath(filename,dirp);
                if(result == -1){
                        free(dirp);
                        return -1;
                }else{
                        if(dirp->isDirectory == FALSE){
                                free(dirp);
                                return 1;
                        }
                }
        }
        free(dirp);
        return 0;

}
