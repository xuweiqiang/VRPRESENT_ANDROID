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

#ifndef _TCHAR_H
#define _TCHAR_H 1

#if defined(TCHAR)
#undef TCHAR
#endif

#ifndef _TCHAR_DEFINED
#define _TCHAR_DEFINED
typedef char TCHAR;
#endif

#include <stdlib.h>
#include <wchar.h>

#define _tcsdup    _strdup
#define _tcslen    strlen
#define _tcscpy    strcpy
#define _tcscpy_s(a,b,c) strcpy(a,c)
#define _tcsncpy   strncpy
#define _tcsncpy_s(a,b,c,d) strncpy(a,c,d)
#define _tcscat    strcat
#define _tcsncat   strncat
#define _tcsncicmp strnicmp
#define _tcsnicmp  strnicmp
#define _tcsicmp   stricmp
#define _tcschr    strchr
#define _tcsrchr   strrchr
#define _tcsstr    strstr
#define _stprintf  sprintf
#define _stprintf_s snprintf
#define _tprintf printf
#define wsprintf   sprintf
#define _topen     open
#define _write     write
#define _tfopen	   fopen
#define _ftprintf  fprintf
#define _fgetts fgets
#define _stscanf   sscanf
#define _vsntprintf_s(a,b,c,d,e) vsnprintf(a,b,d,e)

#define _ttoi(x) atoi(x)
#define _tstoi(x) atoi(x)
#define _itot(a, b, c) itoa(a, b, c)
#define _ltot(a, b, c) ltoa(a, b, c)
#define _tcstoul strtoul


#define _T(x) x
#define _ttol(x) atol(x)
#define wcsncpy_s(a,b,c,d) wcsncpy(a,c,d)
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#endif /* _TCHAR_H */
