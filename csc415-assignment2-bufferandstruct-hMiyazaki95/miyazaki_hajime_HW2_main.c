/**************************************************************
* Class:  CSC-415-02 Spring 2023
* Name: Hajime Miyazaki
* Student ID: 922402751
* GitHub UserID: hMiyazaki95
* Project: Assignment 2 – Stucture in Memory and Buffering
*
* File: miyazaki_hajime_HW2_main.c
*
* Description: purpose of this assignment is to show how to
*              use structures, pointers, character strings,
*	       enumerated types, bitmap fields, and buffering
*	       data into blocks.
*
**************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assignment2.h"

int main(int argc, char *argv[]){
    const char* next;
    char buffer[BLOCK_SIZE];

    //creates pointer to the struct and malloc to allocate memory space for it
    personalInfo* hm = (personalInfo* ) malloc(sizeof(personalInfo));

    //check the malloc whether completed without error
    if (hm == NULL){
        printf("memory allocation faild\n");
        return 1;
    }

    // set atribute values to the struct by using command line argument and the value
    // variables student id and level.
    hm->firstName = argv[1];
    hm->lastName = argv[2];
    hm->studentID = 922402751;
    hm->level = SENIOR;

    // populate language field and specified languages
    hm->languages = KNOWLEDGE_OF_C | KNOWLEDGE_OF_CPLUSPLUS | KNOWLEDGE_OF_JAVASCRIPT;

    // Copy message from command line argument to struct
    strcpy(hm->message, argv[3]);

    // Writing personal info
    int r = writePersonalInfo(hm);
    if (r == 0){
        printf("Success\n");
    } else {
        printf("Failure\n");
    }
    int len_Str = 0;
    int index = 0;

    // to get first string
    next = getNext();

    // keep reading until there is not string. Then it receive the string length.
    // for loop to fill the block buffer with bytes. 
    //After that a character will be copied  
    //Check if buffer is filled, it commits buffer.
    // once it's empty, index for  buffer will be set to 0.
    while (next != NULL){ 
        len_Str = strlen(next); 
        for (int k=0; k<len_Str; k++){ 
            buffer[index++] = next[k]; 
            if (index == BLOCK_SIZE){ 
                commitBlock(buffer); 
                memset(buffer, 0, BLOCK_SIZE);
                index = 0;
            }
        }
        // to get next string
        next = getNext();
    }

    // If NULL received and last filled buffer was not
    // committed as it was not full filled.
    if (index != 0){
        commitBlock(buffer);
    }

    // Call the checkIt() and store its hexdump of the struct
    r = checkIt();

    // free the memory allocatee for the struct
    free(hm);

    return r;
}


