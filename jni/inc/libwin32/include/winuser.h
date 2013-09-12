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
// winuser.h -- 
//     Win32 USER procedure declarations, constant definitions and macros

#ifndef _WINUSER_H_
#define _WINUSER_H_

typedef LRESULT (* WNDPROC) (HWND, UINT, WPARAM, LPARAM);

typedef struct tagWNDCLASS {
	UINT	style;
	WNDPROC	lpfnWndProc;
	int	cbClsExtra;
	int	cbWndExtra;
	HINSTANCE	hInstance;
	HICON	hIcon;
	HCURSOR	hCursor;
	HBRUSH	hbrBackground;
	LPCSTR	lpszMenuName;
	LPCSTR	lpszClassName;
} WNDCLASS;


/*
 * Message structure
 */
typedef struct tagMSG {
  HWND        hwnd;
  UINT        message;
  WPARAM      wParam;
  LPARAM      lParam;
  DWORD       time;
  POINT       pt;
} MSG, *PMSG, *LPMSG;

/*
 * Window Messages
 */

#define WM_CLOSE                        0x0010
#define WM_QUIT                         0x0012
#define WM_COMMAND                      0x0111
#define WM_TIMER                        0x0113

/*
* NOTE: All Message Numbers below 0x0400 are RESERVED.
*
* Private Window Messages Start Here:
*/
#define WM_USER                         0x0400

#define WM_APP                          0x8000
/*  wParam for WM_SIZING message  */
#define WMSZ_LEFT           1
#define WMSZ_RIGHT          2
#define WMSZ_TOP            3
#define WMSZ_TOPLEFT        4
#define WMSZ_TOPRIGHT       5
#define WMSZ_BOTTOM         6
#define WMSZ_BOTTOMLEFT     7
#define WMSZ_BOTTOMRIGHT    8

/*
 * MessageBox() Flags
 */
#define MB_OK                       0x00000000L

static inline HWND WINAPI FindWindow(const char * lpClassName, 
									 const char * lpWindowName)
	{
	// return nonzero.  precise value not currently needed by WinDVD
	return (HWND)0xdeadbeef;
	}

typedef VOID (* TIMERPROC)(HWND, UINT, UINT, DWORD);

// thread messages, including timer messages

/*
 * PeekMessage() Options
 */
#define PM_NOREMOVE         0x0000
#define PM_REMOVE           0x0001
#define PM_NOYIELD          0x0002
BOOL   PeekMessage(LPMSG lpMsg, HWND hWnd ,
				   UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
BOOL   GetMessage(LPMSG lpMsg, HWND hWnd,
				  UINT wMsgFilterMin, UINT wMsgFilterMax);
BOOL   PostThreadMessage(DWORD idThread, UINT Msg, 
						 WPARAM wParam, LPARAM lParam);

UINT_PTR SetTimer (HWND hWnd, UINT_PTR uIDEvent, 
				   UINT uElapse, TIMERPROC lpTimerFunc);
BOOL     KillTimer(HWND hWnd,UINT_PTR uIDEvent);

VOID     IVI_KillTimerThread();                         // InterVideo extension
BOOL     IVI_IsValidTimer(WPARAM wParam, LPARAM lParam);// InterVideo extension

LONG     GetWindowLong(HWND hWnd, int nIndex);
void     PostQuitMessage(int nExitCode);
LRESULT  DefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
HCURSOR  LoadCursor(HINSTANCE hInstance, LPCSTR lpCursorName);
WORD     RegisterClass(const WNDCLASS* lpWndClass);
HWND     CreateWindow(LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int hHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
LONG     SetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong);
BOOL     ShowWindow(HWND hWnd, int nCmdShow);
BOOL     TranslateMessage(const MSG* lpMsg);
BOOL     DispatchMessage(const MSG* lpMsg);
BOOL     GetWindowRect(HWND hWnd, LPRECT lpRect);
int      GetSystemMetrics(int nIndex);
BOOL     SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);

#define WM_DESTROY 0x0002

// Window fiekd offsets for GetWindowLong()
#define GWL_STYLE (-16)
#define GWL_USERDATA (-21)

#define MAKEINTRESOURE(i) (LPSTR)((DWORD)((WORD)(i)))
#define IDC_ARROW MAKEINTRESOURCE(32512)

// Window Styles
#define WS_POPUP 0x80000000L
#define WS_VISIBLE 0x10000000L
#define WS_OVERLAPPEDWINDOW 0 //fake define

// Flags for ShowWindow
#define SW_SHOW 5

#define WM_LBUTTONDBLCLK 0x0203

// GetSystemMetrics() codes
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1

#define HWND_TOPMOST ((HWND)-1)
#define HWND_NOTOPMOST ((HWND)-2)

// SetWindowPos Flags
#define SWP_SHOWWINDOW 0x0040

// Class styles
#define CS_DBLCLKS 0x0008

#endif /* _WINUSER_H */
