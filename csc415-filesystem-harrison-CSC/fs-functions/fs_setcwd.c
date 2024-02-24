/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fs_setcwd.c
*
* Description: Used to change to current working directory to a specified value. This is a helper 
*              function for cmd_cd(), which is analogous to linux's cd
*
**************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mfs.h"
#include "generalUtils.h"


extern char g_currDir[];

/* Used to change the current working directory. The `pathname` is not guarenteed to be valid */
int fs_setcwd(char * pathname)
{
    // Return -1 in case of null pointer or too long input string or cd to "."
    if (pathname == NULL || strlen(pathname) >= MAX_CWD_LEN || strcmp(pathname, ".") == 0)
        return -1;    

    char copyPath[MAX_CWD_LEN];

    // Leading forward slash means absolute path
    if (pathname[0] == '/')
    {
        strcpy(copyPath, pathname);
        if (copyPath[strlen(copyPath)-1] != '/')
        {
            copyPath[strlen(copyPath)] = '/';
            copyPath[strlen(copyPath)+1] = '\0';
        }
    }
    // Special case for ".."
    else if (strcmp(pathname, "..") == 0)
    {
        // cd .. when at root does nothing
        if (strlen(g_currDir) == 1)
            return 1;

        strcpy(copyPath, g_currDir);
        copyPath[strlen(copyPath)-1] = '\0';

        BOOL done = FALSE;
        for (int i = strlen(copyPath)-1; (i >= 0) && (done == FALSE); i--)
        {
            if (copyPath[i] == '/')
                done = TRUE;
            else
                copyPath[i] = '\0';
        }

    }
    // Otherwise it's a relative path
    else
    {
        if ((strlen(pathname) + strlen(g_currDir)) >= MAX_CWD_LEN)
            return -1;
        
        strcpy(copyPath, g_currDir);
        strcat(copyPath, pathname);
        if (copyPath[strlen(copyPath)-1] != '/')
        {
            copyPath[strlen(copyPath)] = '/';
            copyPath[strlen(copyPath)+1] = '\0';
        }

    }

#if DEBUG_SETCWD
    printf("=>DEBUG_SETCWD: input '%s', formatted '%s'\n", pathname, copyPath);
#endif

    fdDir * dirp = malloc(sizeof(fdDir));
    // parsePath is only used here to make sure the path is valid
    int res = parsePath(copyPath, dirp);
    free(dirp);
	//checking if it is file or directory
	//if file can not change directory
    int result = fs_isFile(copyPath);
    if(result == 1){
	printf("Err: This is not a directory\n");
	return -1;
    }

#if DEBUG_SETCWD
    printf("=>DEBUG_SETCWD: parsePath returned %d\n", res);
#endif

    if (res == 0)
        strncpy(g_currDir, copyPath, MAX_CWD_LEN);
    else
        return -1;

    return 0;
}
