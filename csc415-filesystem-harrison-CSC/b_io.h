/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: b_io.h
*
* Description: Interface of basic I/O functions
*
**************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include <fcntl.h>
#include "generalUtils.h"

typedef int b_io_fd;

BOOL isFlagSet(int allFlags, int flag);

b_io_fd b_open(char * filename, int flags);
int b_read(b_io_fd fd, char * buffer, int count);
int b_write(b_io_fd fd, char * buffer, int count);
// int b_seek(b_io_fd fd, off_t offset, int whence);
int b_close(b_io_fd fd);
int writeSizeCheck(b_io_fd fd);

#endif

