/**************************************************************
* Class:  CSC-415-02,03  Fall 2021
* Name: Nakulan Karthikeyan, Arslan Alimov, Johnathan Huynh, Hugo Moreno
* Student ID: 920198861, 916612104, 920375700, 920467935
* GitHub Names: knakulan2001 (Nakulan), ArslanAlimov (Arslan),jhuynhw (Johnathan), hugo215(Hugo)
* TEAM-NAME: Fantastic Four
* Project: File System
*
* File: b_io.c 
*
* Description: Basic File System - Key File I/O Operations. In this
*			   file, we are basically creating a buffer that takes bytes 
*			   and reads/writes to a file, then giving it to our user. This
*              is also freeing our memory on the close of our file system 
*			   to avoid any memory leaks.
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "b_io.h"
#include "mfs.h"
#include "fsInit.h"
#include "fsLow.h"
#include <math.h>

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

// fcb struct
typedef struct b_fcb
{
    int fd;        // file descriptor
    int size;      //size
    char *buf;     //buffer
    int entry_lba; //is the first block a file
    int parentDirectoryLocation;
} b_fcb;

// creating array that will hold open files
b_fcb fcbArray[MAXFCBS];
//This is our boolean variable
static int startup = 0;

// initializing file array system
void b_init()
{
    for (int i = 0; i < MAXFCBS; i++)
    {
        fcbArray[i].fd = -1;
    }
    startup = 1;
}

// getting file descriptor that is not -1
int b_getFCB()
{
    for (int i = 0; i < MAXFCBS; i++)
    {
        if (fcbArray[i].fd == -1)
        {
            // mark fd ur getting as used
            fcbArray[i].fd = -1;
            return i;
        }
    }
    return -1;
}

/*
  opening file and storing open files info
  assigning fd to our datastruct
 */
b_io_fd b_open(char *filename, int flags)
{
    //check if our file system was not initialized and then initializing it
    if (startup == 0)
        b_init();

    /*
	Here we are getting free FCB element
    GetFCB() was give to us but it basically checks for our fd if it's in use and gives us an empty fd number if it's empty
	*/
    b_io_fd FD = b_getFCB();

    /*
     create a pointer to my datastruct
     we will be using that quiet often in here.
     rather than typing fcbArray[fd] all the times we just need to type fileArray-> much more convenient
    */
    b_fcb *fileArray = &fcbArray[FD];

    /*
    Create fdDir struct for our file we are going to store information about our file in that variable
    */
    fdDir file = {0};

    /*
    We check for filename first symbol if its not equals to "/" which is directory entry 
    if it is not then we try and find file.
    */
    if (filename[0] != '/')
        file = findFile(currentDirectory, filename);

    /*
    We check for location of current directory entry if it is equal to 0.
    which means that we are in our "main" directory , we pass nonexistant buffer 
    */
    if (file.entryLBA == 0)
        addFile(currentDirectory, filename, "NONE");

    /*
        We run couple of check's such as if our file location lba is at 0 which means first block
        our FD greater than 0 which means none of our FileDescriptors are busy and we are able to use it
        Also added if our file descriptor is not equal to -1(just as safe check its already being taken care with fd>0)
        and then we fill a struct to our fddir variable 
    */
    if (file.entryLBA == 0 && FD > 0 && FD != -1)
        file = findFile(currentDirectory, filename);

    /*
    Fill out our datastruct bfcb
    with file descriptor
    file size
    file location in the block 
    allocating size for our buffer which is 512
    and give our file parent lba.
    */
    fileArray->fd = FD;
    fileArray->size = file.length;
    fileArray->entry_lba = file.entryLBA;
    fileArray->buf = malloc(B_CHUNK_SIZE);
    fileArray->parentDirectoryLocation = file.parentDirectoryLocation;

    // output our b_io_fd or just an int.
    return FD;
}

// writing to fd
int b_write(b_io_fd fd, char *buffer, int count)
{
    //check if our file system was not initialized and then initializing it
    if (startup == 0)
        b_init();

    // check that fd is between 0 and (MAXFCBS-1)
    if (fd < 0 || fd >= MAXFCBS)
        return -1;

    /*
     create a pointer to my datastruct
     we will be using that quiet often in here.
     rather than typing fcbArray[fd] all the times we just need to type fileArray-> much more convenient
    */

    b_fcb *fileArray = &fcbArray[fd];

    /*
    create a block since we can't have 0 blocks we assign it a value of 1
    */
    int blocks = 1;
    /*
    check passing count and check for amount of block bytes in our vcbstruct 
    round up and assign blocks for us to be able to write into the buffer
    create temporary "buffer" for storage.
    */
    if (count > VCBStructure->blockBytes)
        blocks = ceil(count / VCBStructure->blockBytes) + 1;
    char *temp = malloc(VCBStructure->blockBytes);

    //copy to our temporary buffer from our main buffer passing the count (size)
    memcpy(temp, buffer, count);
    //write into our  fd into first block
    LBAwrite(temp, blocks, fileArray->entry_lba);
    return count;
}

// similar to our write except we are reading our fd
int b_read(b_io_fd fd, char *buffer, int count)
{
    /*
        First two are checks for our fd to be right value and right file descriptor 
        given by professor method needed for "safe start" of the code
        */
    if (startup == 0)
        b_init();

    if ((fd < 0) || (fd >= MAXFCBS))
        return (-1); //invalid file descriptor
    /*
     create a pointer to my datastruct
     we will be using that quiet often in here.
     rather than typing fcbArray[fd] all the times we just need to type fileArray-> much more convenient
    */
    b_fcb *fileArray = &fcbArray[fd];

    /*
    assign bytes needed to be read to our bytes that value is being passed by parameters 
    similar to our assignment 2b where our count was the size of the "data" that is being passed
    check if size of bytes passed is greater than what we have in our b_fcb then the bytes that can be read = to the leftovers.
    */

    int bytes = count;
    if (count > fileArray->size)
        bytes = fileArray->size;

    /*
    Assing 1 to a block because simply we can't have 0 blocks
    check for size passed if its more than what we have in our block bytes 
    round it up and assign value of blocks that we need 
    */
    int blocks = 1;
    if (count > VCBStructure->blockBytes)
        blocks = ceil(count / VCBStructure->blockBytes) + 1;

    /*
    create temp buffer allocate space or blocks for our data to be stored
    we read our temporary buffer, the block it is at and passing in there our  block of file
    */
    char *temp = malloc(VCBStructure->blockBytes);
    LBAread(temp, blocks, fileArray->entry_lba);
    //copy from our temporary buffer into our buffer passing a size which is given from our parameter
    memcpy(buffer, temp, count);
    return bytes;
}

// freeing memory
void b_close(int fd)
{
    b_fcb *file = &fcbArray[fd];
    //check if our buffer is not null yet and then free it
    if (file->buf != NULL)
    {
        free(file->buf);
        file->fd = -1;
        memset(file, 0, sizeof(b_fcb));
    }
    /*
    free(fcbArray[fd].buf);
    fcbArray[fd].fd = -1;
    */

    /*
      Same as 
   file->buf=NULL;
   file->entry_lba=NULL;
   file->fd=-1;
   file->parentDirectoryLocation=NULL;
   file->size=NULL;
   */
}

int b_seek(int fd, off_t offset, int whence)
{
    // Did not have to use it
    return 1;
}