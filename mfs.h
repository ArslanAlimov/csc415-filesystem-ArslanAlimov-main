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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "b_io.h"
#include <stdbool.h>

#include <dirent.h>
#define FT_REGFILE DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK DT_LNK

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif

#define MIN_NEW_DIR_ENTRIES 20

// This structure is returned by fs_readdir to provide the caller with information
// about each file as it iterates through a directory
typedef struct fs_diriteminfo
{
	unsigned short d_reclen; /* length of this record */
	unsigned char fileType;
	char d_name[256]; /* filename max filename is 255 characters */
} fs_diriteminfo;

// This is a private structure used only by fs_opendir, fs_readdir, and fs_closedir
// Think of this like a file descriptor but for a directory - one can only read
// from a directory.  This structure helps you (the file system) keep track of
// which directory entry you are currently processing so that everytime the caller
// calls the function readdir, you give the next entry in the directory
typedef struct
{
	char name[256];		  // bin, etc, var, home etc
	char type;			  // 'F' or 'D'
	short int d_reclen;	  // allocated length
	short int length;	  // actually used length
	short int parentDirectoryLocation;  // parent of directory entry
	uint64_t entryLBA; // location of current directory entry
} fdDir;

// Key directory functions
int fs_mkdir(char *pathname, mode_t mode);
int fs_rmdir(char *pathname);

// Directory iteration functions
fdDir *fs_opendir(char *name);
struct fs_diriteminfo *fs_readdir(fdDir *dirp);
int fs_closedir(fdDir *dirp);

// Misc directory functions
char *fs_getcwd(char *buf, size_t size);
int fs_setcwd(char *buf);	   //linux chdir
int fs_isFile(char *path);	   //return 1 if file, 0 otherwise
int fs_isDir(char *path);	   //return 1 if directory, 0 otherwise
int fs_delete(char *filename); //removes a file

// This is the strucutre that is filled in from a call to fs_stat
struct fs_stat
{
	off_t st_size;		  /* total size, in bytes */
	blksize_t st_blksize; /* blocksize for file system I/O */
	blkcnt_t st_blocks;	  /* number of 512B blocks allocated */
	time_t st_accesstime; /* time of last access */
	time_t st_modtime;	  /* time of last modification */
	time_t st_createtime; /* time of last status change */
						  /* add additional attributes here for your file system */
};

extern fdDir *currentDirectory;
extern char *workingDirectory;
extern char *tempPath;

int fs_stat(char *path, struct fs_stat *buf);

// Dir Functions wpp
int getDirectorySize();
int intializeRoot();
fdDir *dirFinder(fdDir *DirectoryEntry, char *name);
fdDir addEntry(int dest, int parent_block, char *name);
int createRootDirectory(int location);
fdDir *directoryGetter(char *givenPath);
int setEmpty(fdDir *DirectoryEntry, char *name);
int setCurrentDir(int block);
fdDir findFile(fdDir *DirectoryEntry, char *name);
int addFile(fdDir *DirectoryEntry, char *name, char *buffer);
int fs_initialize(int block);
int deallocate(int index, int count);
int fs_locate_space(int block_amount, bool fill_space);

// max chars in a pathname
#define DIRMAX_LEN 256
#endif
