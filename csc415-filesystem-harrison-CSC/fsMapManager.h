
/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fsMapManager.h
*
* Description: Contains the function prototypes for fsMapManager.c, as well as the declaration
*              of the global fsBuffer variable
*
**************************************************************/

extern unsigned char * g_fsBuffer;

/* Return position of first block where the next (`numBlocks`-1) are free. -1 means error */
unsigned int getNFreeBlocks(int numBlocks);

/* Mark `num` blocks starting at `startPos` as `newVal` (0 or 1) */
void markBlocksValue(unsigned int startPos, unsigned int num, unsigned char newVal);