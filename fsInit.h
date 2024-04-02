/**************************************************************
* Class:  CSC-415-02,03  Fall 2021
* Name: Nakulan Karthikeyan, Arslan Alimov, Johnathan Huynh, Hugo Moreno
* Student ID: 920198861, 916612104, 920375700, 920467935
* GitHub Names: knakulan2001 (Nakulan), ArslanAlimov (Arslan),jhuynhw (Johnathan), hugo215(Hugo)
* TEAM-NAME: Fantastic Four
* Project: File System
*
* File: fsInit.h
*
* Description: This header file initializes the VCB structure that will be used in the fsInit.c
*              file for VCB initialization and exitting with grace by freeing the structure. 
*
**************************************************************/

#ifndef _VCB_H
#define _VCB_H
#include "mfs.h"

typedef struct VCB
{
    unsigned int long volumeBytes;
    unsigned int long blockBytes;
    unsigned int long blocks;
    unsigned int signature;
    unsigned int long freespaceLBA;
    unsigned int long rootDirLBA;
    unsigned int long fsSize;
} VCB;

int initFileSystem(uint64_t volumeSize, uint64_t blockSize);
void exitFileSystem();

extern VCB *VCBStructure;

#endif