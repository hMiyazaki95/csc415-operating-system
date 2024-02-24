#include <stdio.h>
#include <string.h>
// Create an example that shous we need to save all words per thread
// and not simply the top 10.
// Each thread gets 10 words of count 2, and the word bbbbbb with count of 1
// But bbbbbb is the most commen.
char * delim = "\"\'.“”‘’?:;-,—*($%)! \t\n\x0A\r";

int main (int argc, char *argv[]) {
  int	cint;
  char	c;
  int	terminateWord = 0;
  while ((cint = fgetc(stdin)) != EOF) {
    c = cint;
    if (strchr(delim,c) == NULL) {
      printf("%c", c);
      terminateWord = 1;
    } else {
      if(terminateWord) printf("\n");
      terminateWord=0;
    }
  }
  if(terminateWord) printf("\n");
}
