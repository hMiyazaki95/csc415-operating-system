/**************************************************************
* Class:  CSC-415-0# Spring 2023
* Name: Hajime Miyazaki
* Student ID: 922402751
* GitHub Name: hMiyazaki95
* Project: Assignment 1 – Command Line Arguments
*
* File: miyazaki_hajime_HW1_main.c
*
* Description: To get and display the command line arguments
*
***************************************************************/

#include <stdio.h>

int main(int argc, char* argv[]) {
	printf("\nThere are %d arguments in the command line.\n", argc);
	// iterate through the argument vector to display the string
	// associated with each command line element
	for (int i = 0; i < argc; i++) {
		//%02d is i in Forloop. 02 is left padded with zeros up to 2 digit 
		//\t is space between
		//%s takes strings
		// \n is new  line
		printf("Argument %02d:\t%s\n", i, argv[i]);
	}
	return 0;
}
