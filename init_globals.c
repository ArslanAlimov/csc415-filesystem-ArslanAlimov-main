/**************************************************************
* Class:  CSC-415-02,03  Fall 2021
* Name: Nakulan Karthikeyan, Arslan Alimov, Johnathan Huynh, Hugo Moreno
* Student ID: 920198861, 916612104, 920375700, 920467935
* GitHub Names: knakulan2001 (Nakulan), ArslanAlimov (Arslan),jhuynhw (Johnathan), hugo215(Hugo)
* TEAM-NAME: Fantastic Four
* Project: File System
*
* File: init_globals.c
*
* Description: This file allows these variables to be used by any
*              file accross the project (global variables).
*
**************************************************************/

#include "fsInit.h"
#include "mfs.h"

VCB *VCBStructure = {0};
fdDir *currentDirectory = {0};
char *workingDirectory = {0};
char *tempPath = {0};

