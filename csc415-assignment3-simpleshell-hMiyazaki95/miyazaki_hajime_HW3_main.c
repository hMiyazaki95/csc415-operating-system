/**************************************************************
* Class:  CSC-415-02 Spring 2023
* Name: Hajime Miyazaki
* Student ID: 922402751
* GitHub ID: hmiyazaki95
* Project: Assignment 3 – Simple Shell
*
* File: miyazaki_hajime_HW2_main.c
*
* Description: To to implement my own shell that runs on top
*	       of the redular command-line interpreter for
*              Linux
*
**************************************************************/
// header files
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


// read user input from cmd
void readFromCmd(char cmd[], char * lines[]) {

	// fgets() from stdio.h reads values from stdin and puts them into cmd.
	// 1024 is the max number of characters.
        // code below to read the user input
	if (fgets(cmd, 1024, stdin)) {
		//to get size of the the read/input string
		size_t len = strlen(cmd);
		if (cmd[len - 1] == '\n') {
			// if end of line then null terminate
			cmd[len - 1] = '\0';
		}
		// to instantiate string token.
		char *string_token;
		// strtok breaks the string into separate lines based on the given delimiter.
		string_token = strtok(cmd, " ");
		int i = 0;
		// iterate through tokenized command-line
		while (string_token != NULL) {
			// append each line to lines array.
			lines[i] = string_token;
			string_token = strtok(NULL, " ");
			i++;
		}
		lines[i] = NULL;
	} 
	else {
		exit(0);
	}
}

// global variable to save error number.
extern int errno;


int main() {

	// command line array with specified buffer size to store the user input.
	char commandLine[1024];

	// args pointer array for passing to prompt.
	char* args[100];

	// The path variable execvp()
	char* path = "/bin/";
	char fullPath[20];

	while (1) {

		printf("Prompt$ ");

		char temp_user_input_str[50];

		// read input from user
		readFromCmd(commandLine, args);

		// if user types exit return
		if (strcmp(commandLine, "exit") == 0) {
			break;
		}

		// initialize process id of type pid_t.
		pid_t childProcessID;

		// create child process
		int pid = fork();


		// exit status.
		int status; 
                // check if parent process
		if (pid == 0) {
			// copy "/bin/" to full path
			strcpy(fullPath, path);
			// concat path with args
			strcat(fullPath, args[0]);
			if (execvp(fullPath, args) < 0) { 
				// if execvp() fails, then print the error code and number and exit with code 2.
				printf("Invalid input.\nExit code: %d\n", errno);
				exit(2);
			}

		} 
		//if parent process wait for child to exit
		else {
			childProcessID = wait(&status);
			// if the process is exited normally, print the process id and exit status.
			if (WIFEXITED(status)) {
				printf("exited %d with Child %d\n", childProcessID, WEXITSTATUS(status));
			}
		}
	}

	return 0;

}
