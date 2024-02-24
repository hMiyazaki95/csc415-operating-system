/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fs_isDir.c
*
* Description: Checks if a file is a directory or not
*              if it is a directory this function will return
*		1 and if it is not a directory it will return 0
*		if an error occurs it will return -1.
**************************************************************/

#include "mfs.h"
#include <unistd.h>
#include <stdio.h>
#include "generalUtils.h"
#include <string.h>
#include <stdlib.h>
//not allowed to use linux library

/* Returns 1 if the file is a directory, 0 if it's not, and -1 for some other error */
int fs_isDir(char * pathname)
{

	fdDir * dirp = malloc(sizeof(fdDir));
	if(strcmp(pathname, "/") == 0){
		free(dirp);
		return 1;
	}else{
		int result = parsePath(pathname,dirp);
		if(result == -1){
			free(dirp);
			return -1;
		}else{
			if(dirp->isDirectory == FALSE){
				free(dirp);
				return 0;
			}
		}
	}
	free(dirp);
	return 1;

}
