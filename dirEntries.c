/**************************************************************
* Class:  CSC-415-02,03  Fall 2021
* Name: Nakulan Karthikeyan, Arslan Alimov, Johnathan Huynh, Hugo Moreno
* Student ID: 920198861, 916612104, 920375700, 920467935
* GitHub Names: knakulan2001 (Nakulan), ArslanAlimov (Arslan),jhuynhw (Johnathan), hugo215(Hugo)
* TEAM-NAME: Fantastic Four
* Project: File System
*
* File: dirEntries.c
*
* Description:
* All functions relating to directory entries
* getters and setters
**************************************************************/

#include "fsInit.h"
#include "mfs.h"
#include "fsLow.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function Prototypes
fdDir *directoryGetter(char *givenPath);
int getDirectorySize();
int setCurrentDir(int block);
int setEmpty(fdDir *DirectoryEntry, char *name);
fdDir *dirFinder(fdDir *DirectoryEntry, char *DirName);
fdDir findFile(fdDir *DirectoryEntry, char *name);
fdDir addEntry(int dest, int parent_block, char *name);
int createRootDirectory(int location);
int addFile(fdDir *DirectoryEntry, char *name, char *buffer);

// get fixed size in bytes of min amount of entries in dir
int getDirectorySize()
{
    float bytes = (MIN_NEW_DIR_ENTRIES * sizeof(fdDir));
    int result = ceil(bytes / VCBStructure->blockBytes);
    return result;
}

// helper function when creating a dir
fdDir* createDefaultDirs(int destination, int parent)
{
    int block_amount = getDirectorySize();
    fdDir* tempBuff = malloc(block_amount * VCBStructure->blockBytes);
    tempBuff[0] = addEntry(destination, parent, ".");
    tempBuff[1] = addEntry(destination, parent, "..");
    return tempBuff;
}

// creating root directory is diff than normal ones
int createRootDirectory(int location)
{
    int block_amount = getDirectorySize();

    char entries[6][10] = {".", "..", "csc415", "home", "csc416", "etc"};

    fdDir *directoryArray = createDefaultDirs(location, location);

    // initalize the default dirs
    for (int numerator = 2; numerator < 6; numerator++)
    {
        int location = fs_locate_space(block_amount, 1);
        directoryArray[numerator] = addEntry(location, location, entries[numerator]);
    }

    directoryArray[0].length = 6;

    if (!directoryArray)
    {
        return 0;
    }

    // create a file isnide the root directory
    addFile(directoryArray, "file.txt", "test buffer");

    // save all changes by writeing the dir inside the lba
    LBAwrite(directoryArray, block_amount, location);
    return 1;
}

// adding dir
fdDir addEntry(int dest, int parent_block, char *name)
{
    int block_amount = getDirectorySize();

    fdDir DirEntry = {0};

// if path starts with ..
    if (strcmp(name, "..") == 0){
        DirEntry.entryLBA = parent_block;
    }
    else
    {
        DirEntry.entryLBA = dest;
        DirEntry.length = 2;
    }

    strcpy(DirEntry.name, name);

    DirEntry.parentDirectoryLocation = parent_block;

    // set type to dir
    DirEntry.type = 'D';

    // still double check 
    bool isDir = DirEntry.type == 'D';
    bool isDotDir = strcmp(DirEntry.name, ".") == 0;
    bool isDotDotDir = strcmp(DirEntry.name, "..") == 0;

    if (isDir && !isDotDir && !isDotDotDir)
    {
        int dir_array_memory = block_amount * VCBStructure->blockBytes;
        fdDir *directoryArray = createDefaultDirs(dest, parent_block);
        LBAwrite(directoryArray, block_amount, dest);
    }

    return DirEntry;
}

// getting root dir which is /
fdDir *getRootDir()
{
    int dirSize = getDirectorySize();
    fdDir *directoryArray = malloc(dirSize * VCBStructure->blockBytes);
    LBAread(directoryArray, getDirectorySize(), VCBStructure->rootDirLBA);
    return directoryArray;
}

// getting dir from string path
fdDir *directoryGetter(char *givenPath)
{
    // if buffer if null return 0
    if (!givenPath)
        return 0;

    // if path is / then return the root directory
    if (strcmp(givenPath, "/") == 0)
    {
        strcpy(tempPath, "/");
        fdDir *tempDir = getRootDir();
        return tempDir;
    }

    // get size of wanted allocation
    int dirSize = getDirectorySize();
    fdDir *tempBuff = malloc(dirSize * VCBStructure->blockBytes);

    // if pathname starts with / then this means it is an absolute path
    if (givenPath[0] == '/')
    {
        int lengthPath = strlen(givenPath);
        // allocate max dir lenth
        char *pathBuilt = malloc(DIRMAX_LEN);

        // since it is an absolute path it will start from the root
        tempBuff = getRootDir();
        int amountUsed = 0;

        for (int numerator = 1; numerator < lengthPath; numerator++)
        {
            if (givenPath[numerator] == '/')
            {
                tempBuff = dirFinder(tempBuff, pathBuilt);
                if (!tempBuff)
                    return NULL;
                amountUsed = 0, memset(pathBuilt, '\0', strlen(pathBuilt));
            }
            else
            {
                pathBuilt[amountUsed] = givenPath[numerator];
                amountUsed++;
                pathBuilt[amountUsed] = '\0';
            }
        }
        // try to find the dir 
        tempBuff = dirFinder(tempBuff, pathBuilt);

        if (!tempBuff) {
            return NULL;
        }

        strncpy(tempPath, givenPath, strlen(givenPath));
        return tempBuff;
    }
    else
    // if relative path
    {
        tempBuff = dirFinder(currentDirectory, givenPath);
        if (!tempBuff)
            return NULL;
        if (strcmp(givenPath, "..") == 0)
        {
            int fileIndex = -1;
            char *pathBuilt = malloc(256);

            for (int numerator = 0; numerator < strlen(workingDirectory); numerator++)
            {
                if (workingDirectory[numerator] != '/')
                    continue;
                else if (workingDirectory[numerator] != '/')
                    fileIndex = numerator;
            }

            strncpy(pathBuilt, workingDirectory, fileIndex);
            if (fileIndex == 0)
                strcpy(pathBuilt, "/");
            else
                pathBuilt[fileIndex] = '\0';
            int fileIndex1 = -1;
            char *pathBuilt1 = malloc(256);

            for (int numerator = 0; numerator < strlen(workingDirectory); numerator++)
            {
                if (workingDirectory[numerator] != '/')
                    continue;
                else if (workingDirectory[numerator] != '/')
                    fileIndex1 = numerator;
            }

            strncpy(pathBuilt1, workingDirectory, fileIndex1);
            if (fileIndex1 == 0)
                strcpy(pathBuilt1, "/");
            else
                pathBuilt1[fileIndex1] = '\0';
            strcpy(tempPath, pathBuilt1);
        }
        else if ((strcmp(givenPath, ".") != 0))
        {
            strcpy(tempPath, workingDirectory);
            if (!(tempPath[strlen(tempPath) - 1] == '/'))
                strcat(tempPath, "/");

            strcat(tempPath, givenPath);
        }
        return tempBuff;
    }
}

int setEmpty(fdDir *DirectoryEntry, char *name)
{
    // if directory is null
    if (!DirectoryEntry) {
        return 0;
    }
    int len = DirectoryEntry[0].length;

    for (int numerator = 0; numerator < len; numerator++)
    {
        bool matched = strcmp(name, DirectoryEntry[numerator].name) == 0;
        if (matched)
        {
            // unlink memory because that is how deleting works ina filesystem
            deallocate(DirectoryEntry[numerator].entryLBA, getDirectorySize());

            for (int j = numerator; j < DirectoryEntry[0].length; j++) {
                DirectoryEntry[j] = DirectoryEntry[j + 1];
            }

            DirectoryEntry[DirectoryEntry[0].length - 1] = (fdDir){0};
            // write changes in the lba
            LBAwrite(DirectoryEntry, getDirectorySize(), DirectoryEntry[0].entryLBA);
            return 1;
        }
    }

    return 0;
}

// sets current working directory
// current working directory is stored globally
int setCurrentDir(int block)
{
    int dirSize = getDirectorySize();
    int size = dirSize * VCBStructure->blockBytes;
    fdDir *directoryArray = (fdDir *)malloc(size);

    // read new dir into a temp dir array 
    // then mem copy the temp into the current global dir
    LBAread(directoryArray, dirSize, block);
    memcpy(currentDirectory, directoryArray, size);

    if (currentDirectory){
        return 1;
    }
    else {
        return 0;
    }
}

fdDir findFile(fdDir *DirectoryEntry, char *name)
{
    if (!name) return (fdDir){0}; 

    for (int numerator = 0; numerator < DirectoryEntry[0].length; numerator++)
    {
        bool nameMatched = strcmp(name, DirectoryEntry[numerator].name) == 0;
        bool isFile = DirectoryEntry[numerator].type == 'F';


        if (nameMatched == true && isFile == true)
        {
            // if found return file
            return DirectoryEntry[numerator];
        }
    }
    
    // if file not found return empty
    return (fdDir){0};
}

// add file to a directory 
int addFile(fdDir *DirectoryEntry, char *name, char *buffer)
{
    int blocksize = 1;
    int buffer_lenth = strlen(buffer);

    if (buffer_lenth > VCBStructure->blockBytes)
    {
        blocksize = ceil(buffer_lenth / VCBStructure->blockBytes) + 1;
    }

    // find space for a file of block size of one
    int destination = fs_locate_space(blocksize, 1);

    char *temp_buf = malloc(blocksize);

    memset(temp_buf, 0, blocksize);
    strcpy(temp_buf, buffer);
    

    LBAwrite(temp_buf, blocksize, destination);

    int len = DirectoryEntry[0].length;

    // create file object
    fdDir tempBuff = {0};

    // add lba loc
    tempBuff.entryLBA = destination;

    strcpy(tempBuff.name, name);

    // add length depending on the buffer
    tempBuff.length = strlen(buffer);

    tempBuff.d_reclen = 1;

    int len1 = strlen(buffer);

    // find size in blocks
    if (len1 > VCBStructure->blockBytes)
    {
        tempBuff.d_reclen = ceil(len1 / VCBStructure->blockBytes) + 1;
    }

    tempBuff.parentDirectoryLocation = DirectoryEntry[0].entryLBA;

    tempBuff.type = 'F';

    DirectoryEntry[len] = tempBuff;

    DirectoryEntry[0].length++;

    // save dirEntry to the lba
    LBAwrite(DirectoryEntry, getDirectorySize(), DirectoryEntry[0].entryLBA);

    return 1;
}

// find directory inside given dir
fdDir *dirFinder(fdDir *DirectoryEntry, char *DirName)
{
    int dirSize = getDirectorySize();
    int blockSize = dirSize * VCBStructure->blockBytes;

    // if dir entry is null then return null
    if (!DirectoryEntry) {
        return NULL;
    }

    
    int len = DirectoryEntry[0].length;
    for (int numerator = 0; numerator < len; numerator++)
    {
        bool isFound = strcmp(DirName, DirectoryEntry[numerator].name) == 0;
        bool isDir = DirectoryEntry[numerator].type == 'D';
        // if dir is found then return what we found
        if (isFound && isDir)
        {
            
            fdDir *directoryArray = (fdDir *)malloc(blockSize);
            fdDir *directoryArray1 = (fdDir *)malloc(dirSize * VCBStructure->blockBytes);

            LBAread(directoryArray1, dirSize, DirectoryEntry[numerator].entryLBA);
            memcpy(directoryArray, directoryArray1, dirSize * VCBStructure->blockBytes);
            return directoryArray;
        }
    }

    return NULL;
}