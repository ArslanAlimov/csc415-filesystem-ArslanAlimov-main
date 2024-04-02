/**************************************************************
* Class:  CSC-415-02,03  Fall 2021
* Name: Nakulan Karthikeyan, Arslan Alimov, Johnathan Huynh, Hugo Moreno
* Student ID: 920198861, 916612104, 920375700, 920467935
* GitHub Names: knakulan2001 (Nakulan), ArslanAlimov (Arslan),jhuynhw (Johnathan), hugo215(Hugo)
* TEAM-NAME: Fantastic Four
* Project: File System
*
* File: mfs.c
*
* Description: This file contains function implementations that allow us 
*              to successfully the professor's functions/commands in the 
*              other files. It implements the functions in mfs.h header file
*              and in turn helps implement the file system interface needed by 
*              the driver to interact with your filesystem.
*
**************************************************************/

#include "mfs.h"
#include "fsInit.h"
#include "fsLow.h"
#define DIRSIZE 4096

// make directory given the pathname
// pathname could be relative or absolute 
int fs_mkdir(char *pathname, mode_t mode)
{
    // if buffer is null
    if (!pathname)
        return 0;

    // check if exists already
    if (directoryGetter(pathname))
    {
        printf("Directory already Exists\n");
        return 0;
    }

    int block_amount = getDirectorySize();
    int location = fs_locate_space(getDirectorySize(), 1);

    // if pathname is absolute 
    if (pathname[0] == '/')
    {
        int index1 = -1;
        // 
        char *builder1 = malloc(256);

        for (int i = 0; i < strlen(pathname); i++)
        {
            if (pathname[i] != '/')
                continue;
            else if (pathname[i] != '/')
            {
                index1 = i;
            }
        }

        strncpy(builder1, pathname, index1);

        if (index1 == 0)
        {
            strcpy(builder1, "/");
        }
        else
        {
            builder1[index1] = '\0';
        }

        // get dir
        fdDir *out = directoryGetter(builder1);
        int index = -1;

        // allocate enough to build new path
        char *builder = malloc(1026);

        for (int i = 0; i < strlen(pathname); i++)
        {

            if (pathname[i] == '/')
            {
                index = i;
            }
        }

        strncpy(builder, (pathname + (index + 1)), strlen(pathname) - index);

        if ((strlen(pathname) - index) == 1)
        {
            strcpy(builder, "/");
        }

        out[out[0].length] = addEntry(location, out[0].entryLBA, builder);
        out[0].length++;
        // save changes
        LBAwrite(out, block_amount, out[0].entryLBA);
    }
    else
    {
        // if pathname is relative 
        int len = currentDirectory[0].length;
        currentDirectory[len] = addEntry(location, currentDirectory[0].entryLBA, pathname);
        currentDirectory[0].length++;
        LBAwrite(currentDirectory, block_amount, currentDirectory[0].entryLBA);
    }

    // write directory inside the lba
    int len2 = getDirectorySize();
    LBAread(currentDirectory, len2, currentDirectory[0].entryLBA);

    return 1;
}

int fs_rmdir(char *pathname)
{
    fdDir *temp = directoryGetter(pathname);
    // if fdDir is null return 0
    if (!temp) {
        return 0;
    }

    fdDir *parentDE = dirFinder(currentDirectory, "..");

    // check if dir is null 
    if (!parentDE) {
        return 0;
    }

    setEmpty(parentDE, pathname);

    // save changes
    LBAread(currentDirectory, getDirectorySize(), currentDirectory[0].entryLBA);

    return 1;
}

fdDir *opened;
int count;

fdDir *fs_opendir(char *name)
{
    if (!name)
        return 0;
    return directoryGetter(name);
}

fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    // if no fdDir then  
    if (!opened)
    {
        opened = malloc(getDirectorySize(MIN_NEW_DIR_ENTRIES) * VCBStructure->blockBytes);
        opened = dirp;
        count = 0;
    }

    // count is greater than length
    if (count >= opened[0].length) {
        return NULL;
    }

    // allocating mem for fs_diriteminfo
    fs_diriteminfo *temp = malloc(sizeof(fs_diriteminfo));

    temp->d_reclen = opened[count].length;
    temp->fileType = opened[count].type;

    strcpy(temp->d_name, opened[count].name);

    // incrementing index
    count++;
    return temp;
}

int fs_closedir(fdDir *dirp)
{
    opened = NULL;
    count = 0;
    return 1;
}

char *fs_getcwd(char *buf, size_t size)
{
    // check buff len 
    if (strlen(buf) > size) {
        return NULL;
    }

    // check len of current path
    if (strlen(workingDirectory) > size) {
        return NULL;
    }

    memcpy(buf, workingDirectory, size);
    return buf;
}

int fs_setcwd(char *buf)
{
    // if buffer is null
    if (!buf) {
        return 1;
    }

    fdDir *temp = directoryGetter(buf);
    // check dir array returned
    if (!temp)
    {
        return 1;
    }

    if (setCurrentDir(temp[0].entryLBA))
    {
        // if temp path is not null 
        if (tempPath)
        {
            strcpy(workingDirectory, tempPath);
        }

        return 0;
    }

    return 1;
}

int fs_isFile(char *path)
{
    // if buffer is null
    if (!path)
    {
        return 0;
    }

    fdDir temp = findFile(currentDirectory, path);
    // if file is not found
    if (temp.entryLBA == 0)
    {
        return 0;
    }

    bool isFile = (temp.type == 'F');
    // if is file return 1 else 0
    if (isFile)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int fs_isDir(char *path)
{
    // if path is null return 0
    if (!path)
    {
        return 0;
    }

    fdDir *temp = directoryGetter(path);
    
    // if temp is null
    if (!temp)
    {
        return 0;
    }

    bool isDir = temp[0].type == 'D';
    // if is dir 1 else 0 
    if (isDir)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int fs_delete(char *filename)
{
    // check if buffer is null
    if (!filename)
    {
        return 0;
    }

    // check if dir is null
    if (!currentDirectory)
    {
        return 0;
    }

    // delete and unlink
    for (int i = 0; i < currentDirectory[0].length; i++)
    {
        bool isDir = strcmp(filename, currentDirectory[i].name) == 0;

        if (isDir)
        {
            int len = currentDirectory[0].length;

            deallocate(currentDirectory[i].entryLBA, currentDirectory[i].d_reclen);

            for (int j = i; j < len; j++)
            {
                currentDirectory[j] = currentDirectory[j + 1];
            }

            currentDirectory[len] = (fdDir){0};

            LBAwrite(currentDirectory, getDirectorySize(), currentDirectory[0].entryLBA);
            return 1;
        }
    }

    return 0;
}

int fs_stat(char *path, struct fs_stat *buf)
{
    // open dir
    fdDir *test = directoryGetter(path);

    // check if null 
    if (!test)
    {
        return 1;
    }

    // get info
    buf->st_size = test[0].length * VCBStructure->blockBytes;

    buf->st_blksize = 20;

    buf->st_blocks = test[0].length;

    return 0;
}
