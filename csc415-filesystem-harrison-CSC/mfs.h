/**************************************************************
* Class:  CSC-415
* Name: Professor Bierman
* Student ID: N/A
* Project: Basic File System
*
* File: mfs.h
*
* Description: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/
#ifndef _MFS_H
#define _MFS_H
#include <sys/types.h>
#include <unistd.h>
#include <time.h>


#include <dirent.h>
#define FT_REGFILE	DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK	DT_LNK

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif

#ifndef BOOL
#define TRUE 1
#define FALSE 0
#define BOOL int
#endif


// This is the max length that the current working directory can be
#define MAX_CWD_LEN 1024


extern char g_currDir[];


// This structure is returned by fs_readdir to provide the caller with information
// about each file as it iterates through a directory
typedef struct
{
    unsigned short d_reclen;      // Size of this item (in bytes)
    int isDir			  ;       // Is this a directory?
    char d_name[256]; 		      // filename max filename is 255 characters
	char d_fullName[MAX_CWD_LEN]; // name of the file's full pathname
	unsigned int blockStart; 	  // What number block this entry starts at
	int posInParent;		      // What is the index number of this in its parent?

	time_t created;
} fs_diriteminfo;

// This is a private structure used only by fs_opendir, fs_readdir, and fs_closedir
// Think of this like a file descriptor but for a directory - one can only read
// from a directory.  This structure helps you (the file system) keep track of
// which directory entry you are currently processing so that everytime the caller
// calls the function readdir, you give the next entry in the directory
typedef struct
{
	unsigned short  d_reclen;			// Number of entries in this directory
	unsigned short	dirEntryPosition;	// Current entry number in directory
	uint64_t	directoryStartBlock;	// Starting block number of directory
	char dirPathName[MAX_CWD_LEN];		// Name of the path to the directory
	int isDirectory;					// Says whether this is a directory
	time_t created;
} fdDir;

// Key directory functions
int fs_mkdir(const char * pathname, mode_t mode);
int fs_make_nondir(char * filename);
int fs_rmdir(const char * pathname);

// Directory iteration functions
fdDir * fs_opendir(const char * pathname);

// This function is somewhat like fs_readdir, but it will return the first free entry position
// num in the directory, else return -1
int fs_nextFreeSpot(fdDir * dirp);

fs_diriteminfo * fs_getChildWithName(fdDir * dirp, const char * name);

fs_diriteminfo * fs_readdir(fdDir * dirp);
int fs_itemNumChildren(fs_diriteminfo * item);

int fs_closedir(fdDir * dirp);

// Misc directory functions
char * fs_getcwd(char * pathname, size_t size);
int fs_setcwd(char * pathname);   // linux cd
int fs_isFile(char * filename);	// return 1 if file, 0 otherwise
int fs_isDir(char * pathname);		// return 1 if directory, 0 otherwise
int fs_delete(char * filename);	// removes a file


// This is the strucutre that is filled in from a call to fs_stat
struct fs_stat
{
	off_t     st_size;    		/* total size, in bytes */
	blkcnt_t  st_blocks;  		/* number of 512B blocks allocated */
	time_t    st_createtime;   	/* time of last status change */
	
	/* add additional attributes here for your file system */
};

int fs_stat(const char * path, struct fs_stat * buf);

#endif

