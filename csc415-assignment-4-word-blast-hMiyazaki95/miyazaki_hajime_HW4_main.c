/**************************************************************
* Class:  CSC-415-0# Spring 2023
* Name: Hajime Miyazaki
* Student ID: 922402751
* GitHub ID: hmiyazaki95
* Project: Assignment 4 – Word Blast
*
* File: miyazaki_hajime_HW4_main.c 
*
* Description: to read WarandPeace and count and tally each of the word that are 6 or more characters long by only using linux file functions.
*
**************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include <error.h>
#include <errno.h>
#include <string.h>

// You may find this Useful
char * delim = "\"\'.“”‘’?:;-,—*($%)! \t\n\x0A\r";

void *safeMalloc (size_t size)
    {
    void *ret = malloc(size);
    if (ret == NULL)
	{
	error(8, 0, "Ran out of memory\n");
	}
    return ret;
    }

#if TIMER
// Our own timers for profiling
struct timespec *timeStart ()
    {
    struct timespec *beginTime = safeMalloc(sizeof(struct timespec));
    clock_gettime(CLOCK_REALTIME, beginTime);
    return beginTime;
    }

int	timeEnd (char *label, struct timespec *startTime)
    {
    struct timespec endTime;
    clock_gettime(CLOCK_REALTIME, &endTime);

    time_t sec = endTime.tv_sec - startTime->tv_sec;
    long n_sec = endTime.tv_nsec - startTime->tv_nsec;
    if (endTime.tv_nsec < startTime->tv_nsec)
        {
        --sec;
        n_sec = n_sec + 1000000000L;
        }

    printf("TIME: %s total time was %ld.%09ld seconds\n", label, sec, n_sec);
    free(startTime);
    }
#endif

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// lseek needs mutex lock to be thread safe
off_t safeLseek(int fd, off_t offset, int whence)
    {
    pthread_mutex_lock(&mutex);
    off_t ret = lseek(fd, offset, whence);
    pthread_mutex_unlock(&mutex);
    return ret;
    }

// Single thread-safe string hash table
typedef struct {
    char	*word;
    int		count;
} WORD_COUNT;

// One shared hash table
WORD_COUNT	*hashTable;
int		hashSize;
int		hashMaxCount = 0;

// Find the has bucket for a word
int hashIndex (char *word)
    {
    int		len = strlen(word);
    long	sum = 0;
    for (int i=0; i < len; i++)
	sum += word[i];
    return sum % hashSize;
    }

#if DEBUG
int wordCount = 0;
#endif

// Thread-safe hash insert using linear probing as described in
// https://benhoyt.com/writings/hash-table-in-c/
int hashInsert (int threadNum, char *word)
    {
#if DEBUG
    printf("Thread %d saving %d '%s'\n", threadNum, ++wordCount, word);
#endif
    int hashBucket = hashIndex(word);
#if DEBUG
    printf("Thread %d start hashBucket %d\n", threadNum, hashBucket);
#endif
    // Two thread cannot change a global data structure at the same time.
    pthread_mutex_lock(&mutex);
    // Find an empty hash bucket, or a matching bucket, as in linear probing
    while ((hashTable[hashBucket].word != (char *) NULL)
	&& (strcmp(hashTable[hashBucket].word, word) != 0))
	{
	    hashBucket++;
	    if (hashBucket > hashSize) hashBucket = 0;
	}
#if DEBUG
    printf("Thread %d found hashBucket %d\n", threadNum, hashBucket);
#endif
    if (hashTable[hashBucket].word == (char *) NULL)
	{
	// Empty hash bucket, so fill it in.
	hashTable[hashBucket].word = word;
	hashTable[hashBucket].count = 1;
#if DEBUG
	printf("Thread %d add '%s' to empty bucket %d\n", threadNum, word, hashBucket);
#endif
	}
    else
	{
	// Word is already there
	// Add count.
	hashTable[hashBucket].count++;
	// Don't need a second copy of the same word.
	free(word);
#if DEBUG
	printf("Thread %d add count to duplicate '%s' bucket %d\n", threadNum, word, hashBucket);
	printf("Compare: '%s' '%s' %d\n", hashTable[hashBucket].word, word,
	    strcmp(hashTable[hashBucket].word, word));
#endif
	}
    // Keep track of max count;
    if (hashTable[hashBucket].count > hashMaxCount)
	hashMaxCount = hashTable[hashBucket].count;
    // Done modifying global data structure, so done with mutex.
    pthread_mutex_unlock(&mutex);
    }

// debug function to print the entire hash table
int hashPrintAll ()
    {
    for (int i=0; i < hashSize; i++)
	{
	if(hashTable[i].word != NULL)
	    printf("%d '%s' %d\n", i, hashTable[i].word, hashTable[i].count);
	}
    }

// Create an empty hash table
int hashCreate (int buckets)
    {
#if DEBUG
    printf("Creating hash table size: %d\n", buckets);
#endif
    hashTable = safeMalloc(sizeof(WORD_COUNT) * buckets);
    hashSize = buckets;
    for (int i=0; i < hashSize; i++)
	hashTable[i].word = NULL;
    }

int hashFree ()
    {
    for (int i=0; i < hashSize; i++)
	{
	if(hashTable[i].word != NULL)
	    // Free the dupricate words which makes more space efficient
	    free(hashTable[i].word);
	}
        free (hashTable);
	// For safety
	hashTable = NULL;
	hashSize = 0;
    }

// Sort words by count
// Use a link list so we can insert a word/count anywhere in the list easily
// There will only be 10 items in the linked list
// I will get rid of a word/count when it no longer is in the top 10
// and reuse that structure for the new item.
// Build linked list in count order, from largest to smallest
typedef struct wordsBySize {
    char		*word;
    int			count;
    // Doubly linked list
    struct wordsBySize	*prev;
    struct wordsBySize	*next;
} WORDS_BY_SIZE;

#ifdef DEBUG
int printWordBySizeTable (char *label, WORDS_BY_SIZE	*head)
    {
    int	number = 1;
    printf("\n");
    while (head)
	{
	printf("Tabel %s number %d word %s count %d\n",
	    label, number++, head->word, head->count);
	head = head->next;
	}
    }

int printWordBySizeTableWhenSmall (char *label, WORDS_BY_SIZE	*head)
    {
    int	number = 1;
    WORDS_BY_SIZE	*saveHead = head;
    while (head)
	{
	number++;
	if (head->next == head)
	{
	    printf("linked-list points are wrong\n");
	    break;
	}
	head = head->next;
	}
    if (number < 10)
	{
	printWordBySizeTable (label, saveHead);
	}
    }
#endif

int hashPrintByCount ()
    {
    // Do an insert sort of only the top 10 items
    WORDS_BY_SIZE	*head = (WORDS_BY_SIZE *) NULL;
    WORDS_BY_SIZE	*last = (WORDS_BY_SIZE *) NULL;
    int itemsInLinkedList = 0;
    for (int i=0; i < hashSize; i++)
	{
	if (hashTable[i].word != NULL)
	    {
	    // do I  need to insert this word/count into the top 10
	    if (itemsInLinkedList < 10)
		{
		itemsInLinkedList++;
		WORDS_BY_SIZE	*new = safeMalloc(sizeof(WORDS_BY_SIZE));
		new->word = hashTable[i].word;
		new->count = hashTable[i].count;
		new->prev = (WORDS_BY_SIZE *) NULL;
		new->next = (WORDS_BY_SIZE *) NULL;
		if (head)
		    {
		    // Find where to insert
		    WORDS_BY_SIZE	*prev = (WORDS_BY_SIZE *) NULL;
		    WORDS_BY_SIZE	*this = head;
		    while (this && (hashTable[i].count < this->count))
			{
			prev = this;
			this = this->next;
			}
		    if (prev)
			{
			// Insert new item between prev and this
			prev->next = new;
			new->prev = prev;
			new->next = this;
			if (this)
			    {
			    this->prev = new;
			    }
			}
		    else
			{
			// Insert before head
			new->next = head;
			head->prev = new;
			head = new;
			}
		    if (this == NULL)
			{
			// no next item, so this is the last item
			last = new;
			}
		    }
		else
		    {
		    // new is the only item in the linked list.
		    head = new;
		    last = new;
		    }
#ifdef DEBUG
		printWordBySizeTableWhenSmall ("ok-done", head);
		printWordBySizeTable ("<10", head);
#endif
		}
	    // Table has 10 items and we need to insert item into table
	    // and flush the last item
	    else if (hashTable[i].count > last->count)
		{
		// Find where to insert
		WORDS_BY_SIZE	*prev = (WORDS_BY_SIZE *) NULL;
		WORDS_BY_SIZE	*this = head;
		while (this && (hashTable[i].count < this->count))
		    {
		    prev = this;
		    this = this->next;
		    }
		// this can never be NULL

		// Need to keep a pointer before I unlink the last item
		WORDS_BY_SIZE	*secondLast = last->prev;
		// Unlink last from linked lists.
		last->prev->next = NULL;

		// Reuse last data structure, and insert into the new location.
		// between prev and this
		WORDS_BY_SIZE	*new = last;
		last = secondLast;

		new->word = hashTable[i].word;
		new->count = hashTable[i].count;
		if (prev)
		    {
		    // Insert new item between prev and this
		    prev->next = new;
		    new->prev = prev;
		    if (this == new)
			{
			// I am replacing the last item, there is no next
			new->next = NULL;
			last = new;
			}
		    else
			{
			new->next = this;
			this->prev = new;
			}
		    }
		else
		    {
		    // Insert before head
		    new->next = head;
		    new->prev = NULL;
		    head->prev = new;
		    head = new;
		    }

#ifdef DEBUG
		printWordBySizeTableWhenSmall ("gone-wrong", head);
		printWordBySizeTable ("insert", head);
#endif
		}
	    }
	}

    WORDS_BY_SIZE	*this = head;
    WORDS_BY_SIZE	*next;
    int			number = 1;
    while (this)
	{
	printf("Number %d is %s with a count of %d\n",
	    number++, this->word, this->count);
    	// Need to save pointer since it won't be vaild after free();
    	next = this->next;
	// Free while printing
	free(this);
    	this = next;
	}
    }

// If in the middle of a word, move to the end of the word
off_t moveToEndOfWord(int fd, off_t offset)
    {
    off_t	curLoc;
    char	buf[1];
    // Keep moving while I am inside of a word (not a delimiter)
    while ((pread(fd, buf, 1, offset) == 1)
	&& (buf[0] != '\0')
	&& strchr(delim, buf[0]) == NULL)
	{
	offset++;
	}
    // Now I am either at a delimiter or at the end of the file, so back by one.
    return offset;
    }

// Info I want to pass into each thread
typedef struct {
    int		threadNum;
    off_t	fileBegin;
    off_t	fileEnd;
    int		fd;
    int		iret;
} THREAD_ARG;

// Save thread buffer to a file for testing
// Note: the location in the file is modified.
int debugPrintBuffer(THREAD_ARG *threadArg, off_t fileBegin, off_t fileEnd)
    {
    char	fname[16];
    sprintf(fname, "debug-%d.log", threadArg->threadNum);

    FILE	*file = fopen(fname, "w");

    printf("Thread %d buffer is now fileBegin %ld fileEnd %ld\n",
	threadArg->threadNum, fileBegin, fileEnd);
    fprintf(file, "Thread buffer is now fileBegin %ld fileEnd %ld\n", fileBegin, fileEnd);
    off_t curLoc = fileBegin;
    char buf[1];
    while (curLoc <= fileEnd)
	{
	pread(threadArg->fd, buf, 1, curLoc);
	fprintf(file, "%c", buf[0]);
	curLoc++;
	}
    fprintf(file, "\nThread %d buffer is ready\n", threadArg->threadNum);
    fclose(file);
    printf("Thread %d buffer is ready\n", threadArg->threadNum);
    }

// Thread that counts the 6 or greater letter words in a subset of the file
void *threadFunc (void *ptr)
    {
    THREAD_ARG	*threadArg = (THREAD_ARG *)ptr;
#if DEBUG
    printf("Thread %d got: fileBegin %ld fileEnd %ld\n",
	threadArg->threadNum,
	threadArg->fileBegin,
    	threadArg->fileEnd);
#endif
    // characters that thread looks at should be whole words.
    // check if end of the buffer is at a whole word
    // check if beginning of the buffer is at a whole word
    off_t	fileBegin = threadArg->fileBegin;
    off_t	fileEnd = threadArg->fileEnd;
    // ----------------------------------------------------------------------
    // Move fileEnd to the end of the word in case we are in the middle
    // of an existing word.
    // fileEnd
    fileEnd = moveToEndOfWord(threadArg->fd, fileEnd);

    // ----------------------------------------------------------------------
    // fileBegin
    // Only move beginning part of buffer if it is in the middle of the file.
    // meaning not at the beginning. I need to check the beginning of the file.
    if (fileBegin != 0)
	fileBegin = moveToEndOfWord(threadArg->fd, fileBegin);

    if (fileEnd < fileBegin)
	{
	fprintf(stderr, "ERROR: Negative buffer length: begin: %ld end: %ld\n",
	    fileBegin, fileEnd);
	error(9, 0, "Thread %d buffer size should never be negative", threadArg->threadNum);
	}

    // ----------------------------------------------------------------------
    // DEBUG
#if DEBUG
    debugPrintBuffer(threadArg, fileBegin, fileEnd);
#endif

#if TIMER
    struct timespec *ts = timeStart ();
#endif
    // ----------------------------------------------------------------------
    // I need to remember ALL words because a small word count in a large
    // number of threads could result in a word that is used most often.
    // Therefore, I need to remember all word counts, and select top 10 outside
    // of the threads
    // Return linked list of ALL words and counts.
    off_t	curLoc = fileBegin;
    off_t	beginWord;
    char	buf[1];
    while (curLoc < fileEnd)
	{
	// pread(threadArg->fd, buf, 1, curLoc);
	// Get a word
	beginWord = curLoc;
	while ((pread(threadArg->fd, buf, 1, curLoc) == 1)
	    && (buf[0] != '\0')
	    && strchr(delim, buf[0]) == NULL)
	    {
	    curLoc++;
	    }

        // if word length >= 6, save the word
        off_t	wordLength = curLoc - beginWord;
#define WORD_LENGTH 6
        if (wordLength >= WORD_LENGTH)
	    {
	    char	*word = safeMalloc(sizeof(char) * (wordLength + 1));
	    ssize_t	gotChars = pread(threadArg->fd, word, wordLength, beginWord);
	    // Must terminate the string
	    word[wordLength] = '\0';
	    if (gotChars != wordLength)
		{
		fprintf(stderr, "Should have read %ld characters but only got %ld characters '%s'\n",
		    wordLength, gotChars, word);
		exit(11);
		}
	    // Use a hash table since they are efficient in saving strings
	    // O(n) time on average, assuming a well distributed hashing function.
	    // Save the word and increment a counter.
	    // Hash table will only keep one copy of the word storage, and
	    // will free other copies of that word, so I am memory efficient.
	    hashInsert (threadArg->threadNum, word);
	    }

	// Skip past all consecutive delimiters
	while ((pread(threadArg->fd, buf, 1, curLoc) == 1)
	    && (buf[0] != '\0')
	    && strchr(delim, buf[0]) != NULL)
	    {
	    curLoc++;
	    }
	}
#if TIMER
    timeEnd ("parse words", ts);
#endif
    }

int main (int argc, char *argv[])
    {
    char	*fileName;	// name of the file to read – WarAndPeace.txt
    int		threadCount;	// number of threads you should spawn to evenly divide the work
    int		fd;

    //***TO DO***  Look at arguments, open file, divide by threads
    //             Allocate and Initialize and storage structures
    if (argc != 3)
	{
	error(1, 0, "2 arguments are required: filename and threadcount");
	}
    else
	{
	fileName = argv[1];
	fd = open(fileName, O_RDONLY);
	if (fd == -1)
	    {
	    fprintf(stderr, "Could not open fileName '%s' %s\n", fileName, strerror(errno));
	    exit(2);
	    }

	if (sscanf(argv[2], "%d", &threadCount) != 1)
	    {
	    error(3, 0, "threadCount argument requires an integer");
	    }
	if (threadCount < 1)
	    {
	    error(4, 0, "threadCount must be greater than 0\n");
	    }
	}
#if DEBUG
    printf("DEBUG VERSION\n");
    printf("fileName = %s\n", fileName);
    printf("threadCount = %d\n", threadCount);
#endif
    off_t fileLength = lseek(fd, 0, SEEK_END);
    if (fileLength == -1)
	{
	fprintf(stderr, "Problem fseek into file '%s' %s\n", fileName, strerror(errno));
	// exit closes all open files and frees allocated memory
	exit(5);
	}
    if (fileLength == 0)
	{
	printf("No words in input file '%s'\n", fileName);
	// exit closes all open files and frees allocated memory
	exit(0);
	}
#if (DEBUG||PROFILE)
    printf("Input file '%s' has '%ld' characters.\n", fileName, fileLength);
#endif
    long	bufferLength = fileLength/threadCount;
#if DEBUG
    printf("Buffer length is %ld characters\n", bufferLength);
#endif

#if TIMER
    struct timespec *ts = timeStart ();
#endif
    // Create a big enough hash table assuming file is entirely 6 letter
    // words with 1 delimiter and about 50% overhead (4/7).
    hashCreate (fileLength/4);
#if TIMER
    timeEnd ("hashCreate", ts);
#endif

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Time stamp start
    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************
    // *** TO DO ***  start your thread processing
    pthread_t	*allThreads;
    allThreads = calloc(sizeof(pthread_t), threadCount);
    if (allThreads == NULL)
	{
	error(6, 0, "Ran out of memory for thread array\n");
	}

    int		threadNum;
    off_t	bufferOffset;
    pthread_t	*thisThread;
    THREAD_ARG	*threadArg = safeMalloc(sizeof(THREAD_ARG) * threadCount);
    for (threadNum=0, bufferOffset=0;
	    threadNum < threadCount;
	    threadNum++, bufferOffset+=bufferLength)
	{
	threadArg[threadNum].threadNum = threadNum;
        threadArg[threadNum].fileBegin = bufferOffset;
        threadArg[threadNum].fileEnd = bufferOffset + bufferLength;
	threadArg[threadNum].fd = fd;
	//
	threadArg[threadNum].iret = pthread_create(&allThreads[threadNum], NULL, threadFunc,
	    (void*) &threadArg[threadNum]);
	if (threadArg[threadNum].iret != 0)
	    {
	    fprintf(stderr, "ERROR pthread_create() failed with code: %d\n",
		threadArg[threadNum].iret);
	    exit(7);
	    }
	}

    // wait for the threads to finish
    for (threadNum=0; threadNum < threadCount; threadNum++)
	{
	pthread_join(allThreads[threadNum], NULL);
	}

    // Got to wait until all threads are done before I can print the counts.
#if DEBUG||PROFILE
    hashPrintAll();
#endif

    // ***TO DO *** Process TOP 10 and display
    hashPrintByCount();

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Clock output
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
        {
        --sec;
        n_sec = n_sec + 1000000000L;
        }

    printf("Total Time was %ld.%09ld seconds\n", sec, n_sec);
    //**************************************************************

    // ***TO DO *** cleanup
    hashFree();
    free(threadArg);
    free(allThreads);
    close(fd);
    }
