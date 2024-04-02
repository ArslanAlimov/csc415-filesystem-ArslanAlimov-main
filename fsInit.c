/**************************************************************
* Class:  CSC-415-02,03  Fall 2021
* Name: Nakulan Karthikeyan, Arslan Alimov, Johnathan Huynh, Hugo Moreno
* Student ID: 920198861, 916612104, 920375700, 920467935
* GitHub Names: knakulan2001 (Nakulan), ArslanAlimov (Arslan),jhuynhw (Johnathan), hugo215(Hugo)
* TEAM-NAME: Fantastic Four
* Project: File System
*
* File: fsInit.c
*
* Description: In our fsInit.c file, we form the foundation of our file 
*              system by getting the VCB initialized when there is no 
*              sample volume file.  If there is already a volume, 
*              then the user will automatically skip the format process 
*              and enter the shell, after checking to make sure the directory 
*              has been initialized successfully.
*
**************************************************************/

#include "fsInit.h"
#include "fsLow.h"
#include "mfs.h"

#define SIGNATURE 80085

int initFileSystem(uint64_t volumeSize, uint64_t blockSize)
{
    VCBStructure = malloc(blockSize);
    LBAread(VCBStructure, 1, 0);

    if (VCBStructure->signature == SIGNATURE)
    {
        printf("Volume Formatted\n");
    }
    else
    {
        VCBStructure->volumeBytes = volumeSize;

        VCBStructure->blockBytes = blockSize;

        VCBStructure->blocks = volumeSize / blockSize;

        VCBStructure->signature = SIGNATURE;

        VCBStructure->freespaceLBA = 1;

        VCBStructure->fsSize = VCBStructure->blocks / VCBStructure->blockBytes;

        fs_initialize(VCBStructure->freespaceLBA);

        int root_location = fs_locate_space(getDirectorySize(), 1);
        createRootDirectory(root_location);
        VCBStructure->rootDirLBA = root_location;

        LBAwrite(VCBStructure, 1, 0);

        printf("VCB successfully initialized.\n");
    }

    return 0;
}

void exitFileSystem()
{
    free(currentDirectory);
    free(VCBStructure);
    currentDirectory = NULL;
    VCBStructure = NULL;
    printf("Exiting System\n");
}