
/**************************************************************
* Class: CSC-415-02 - Fall 2022
* Names: Gregory Arruiza, Andy Byeon, Hajime Miyazaki, Harrison Rondeau
* Student IDs: 922218151, 921120548, 922402751, 921087437
* GitHub Name: ChowMeinFan, AndyByeon, hMiyazaki95, harrison-CSC
* Group Name: 4 Threads
* Project: Basic File System
*
* File: fsMapManager.c
*
* Description: Contains the functions used to read from and write to the free space map
*
**************************************************************/

#include "fsMapManager.h"
#include "generalUtils.h"

unsigned char * g_fsBuffer;

unsigned int getNFreeBlocks(int numBlocks)
{
	for (int i = 0; i < vcb->numberOfBlocks; i++)
	{
		BOOL failed = FALSE;
		for (int j = 0; j < numBlocks && failed == FALSE && j+i < vcb->numberOfBlocks; j++)
		{
			if (getBitInUChar(g_fsBuffer[j+i / 8], ((j+i) % 8) + 1) == 0)
				failed = TRUE;
		}
		if (failed == FALSE)
			return i;
	}
	return -1;
}

void markBlocksValue(unsigned int startPos, unsigned int num, unsigned char newVal)
{
    for (unsigned int i = startPos; i < startPos + num; i++)
    {
        unsigned char binaryVal = exponent(2, i % 8);
        if (newVal == 1)
            g_fsBuffer[i / 8] |= binaryVal;
        else
            g_fsBuffer[i / 8] &= (255 - binaryVal);	
    }
}