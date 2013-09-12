//=============================================================================
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 2004 - 2005  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _IO_H
#define _IO_H

#include <windows.h>
#include <unistd.h>   // for close, read, lseek
#include <fcntl.h>    // for open

#ifndef __SYMBIAN32__ // (is already defined in Symbian's headers)
#define O_BINARY 0
#endif

/* File attribute constants for _findfirst() */

#define _A_NORMAL       0x00    /* Normal file - No read/write restrictions */
#define _A_RDONLY       0x01    /* Read only file */
#define _A_HIDDEN       0x02    /* Hidden file */
#define _A_SYSTEM       0x04    /* System file */
#define _A_SUBDIR       0x10    /* Subdirectory */
#define _A_ARCH         0x20    /* Archive file */

struct _finddata_t {
        unsigned    attrib;
        long  time_create;    /* -1 for FAT file systems */
        long  time_access;    /* -1 for FAT file systems */
        long  time_write;
        unsigned long    size;
        char        name[260];
};

int* _findfirst(const char * _Filename, struct _finddata_t * _FindData);
int _findnext(int* _FindHandle, struct _finddata_t * _FindData);
int _findclose(int* _FindHandle);


#endif
