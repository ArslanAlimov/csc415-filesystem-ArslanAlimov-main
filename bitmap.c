/**************************************************************
* Class:  CSC-415-02,03  Fall 2021
* Name: Nakulan Karthikeyan, Arslan Alimov, Johnathan Huynh, Hugo Moreno
* Student ID: 920198861, 916612104, 920375700, 920467935
* GitHub Names: knakulan2001 (Nakulan), ArslanAlimov (Arslan),jhuynhw (Johnathan), hugo215(Hugo)
* TEAM-NAME: Fantastic Four
* Project: File System
*
* File: bitmap.c
*
* Description: In this file, we are initializing a free-space bitmap 
*              that allows us to find the amount of free space from the 
*              Volume. This was accomplished by creating a bool* array 
*              called bitmap (where 1 would mean the relevant block is in 
*              use and 0 otherwise). There are four important features that 
*              our bitmap will manage: the freespace initialization, free space 
*              locating, indicating the block as used or otherwise, and freeing 
*              memory.
*
**************************************************************/

#include "mfs.h"
#include "fsInit.h"
#include "fsLow.h"

int fs_initialize(int block)
{ 
    /* This function goes along with the VCB init, in which the freespace map location 
     * needs to be stored inside the VCB structure.
    */
    bool *bitmap = (bool *)calloc(VCBStructure->volumeBytes, 1);
    // Initializes the bitmap through a boolean array with all zeros then account for block 0,
    // which is the VCB that is also marked with 1.
    // Furthermore, we also include the size of the bitmap, which would be from the first block
    // to the final one (no need for specification).

    if (bitmap == NULL) // Validation to ensure the index of the bitmap is in scope.
    {
        // If the block that needs to be marked as used is out of scope, then it would obviously
        // be impossible to access said index, meaning it would return an error value.
        return -1;
    }
    else
    {   /* After such validation is completed, we index to the starting block which needs
         * to be marked as used and proceed to mark all further blocks that are used after
         * gathering which ones are already in use.
        */
        for (int i = 0; i < VCBStructure->fsSize; i++)
        {
            bitmap[i] = 1;
        }
        // We mark the blocks as used by changing the 0 to a 1, creating our boolean array
        // which is saved inside the Volume using LBAwrite.
        LBAwrite(bitmap, VCBStructure->fsSize, block);
        return VCBStructure->fsSize + block;
    }
}

int fs_locate_space(int block_req, bool fill_space)
{
    /* In the fs_locate_space() function, we locate free space by accessing our bitmap 
     * via a LBAread function call to read from where the bitmap is located. Afterwards, 
     * using simple for-loops, to locate a consecutive amount of zeros (which represent 
     * free space as mentioned earlier).
    */
    bool *bitmap = (bool *)calloc(VCBStructure->volumeBytes, 1);

    if (bitmap == NULL) // As in previous function, validation to ensure the index of the bitmap is in scope.
    {
        return -1;
    }

    LBAread(bitmap, VCBStructure->blocks, VCBStructure->freespaceLBA);

    int blockCount = 0;
    int index = 0;
    // for-loop to locate a consecutive amount of zeros (which represent free space as mentioned earlier).
    for (int x = 0; x < VCBStructure->blocks; x++)

    {
        if (bitmap[x] == false)
        {
            blockCount = blockCount + 1;
            if (blockCount == block_req)
            {
                index = x - block_req - 1;
                break;
            }
            continue;
        }
        blockCount = 0;
    }
    // After such validation is completed, we index to the starting block which needs 
    // to be marked as used and proceed to mark all further blocks that are used after 
    // gathering which ones are already in use.
    if (fill_space == true)
    {
        int position_end = index + block_req;

        for (int x = index; x < position_end; x++)
        {
            bitmap[x] = 1;
        }
        // We mark the blocks as used by changing the 0 to a 1, creating our boolean array
        // which is saved inside the Volume using LBAwrite.
        LBAwrite(bitmap, VCBStructure->blocks, VCBStructure->freespaceLBA);
    }

    return index;
}

int deallocate(int index, int blockCount)
{   /* Frees up memory and indicates a block as free! Unfortunately, we were unable to 
     * create a defragmentation function, but it was not required and our program works 
     * perfectly without it! 
    */ 
    int position_end = index + blockCount;

    if (index > VCBStructure->blocks)
    {
        return -1;
    }

    bool *bitmap = (bool *)calloc(VCBStructure->volumeBytes, 1);

    if (bitmap == NULL)
    {
        return -1;
    }

    LBAread(bitmap, VCBStructure->blocks, VCBStructure->freespaceLBA);

    for (int i = index; i < position_end; i++)
    {
        bitmap[i] = 0;
    }

    LBAwrite(bitmap, VCBStructure->blocks, VCBStructure->freespaceLBA);
    return 1;
}
