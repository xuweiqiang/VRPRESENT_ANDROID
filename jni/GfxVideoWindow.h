//=============================================================================
//  CIDANA CONFIDENTIAL INFORMATION
//
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO CIDANA, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 2008  Cidana, Inc.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _VIDEO_WINDOW_H_
#define _VIDEO_WINDOW_H_

#define EXPORT extern "C" _declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif 

#ifdef __linux__
		#if defined(ST) || defined(GFX_FB_LINUX) || defined(ST_LINUX) || defined(PXA_LINUX) || defined(ANDROID) ||defined(__QNX__)
	#else
//	#include <SDL/SDL_syswm.h>
//	#include <X11/Xlib.h>
	#endif
#endif

/*! Video stream attributes.
*/
enum VE_MODE
{
	VE_MODE_STRETCH		= 0x00000001,  /// stretching the source to fulfill the dst
	VE_MODE_LETTERBOX	= 0x00000010,  /// adding black border to keep the aspect ratio
	VE_MODE_CROPPING	= 0x00000100   /// cropping source to keep the aspect ratio
};

enum
{
	VE_COMMAND_SRCRECT,
	VE_COMMAND_DSTRECT,
	VE_COMMAND_ASPECTRATIO,
	VE_COMMAND_MODE,
};

typedef struct
{
	DWORD	dwAspectX;
	DWORD	dwAspectY;
}VR_GFXVIDWIN_ASPECT_STRUCT;

struct VE_Options
{
	RECT	rSrc;			/// source rectangle
	RECT	rDst;			/// destination rectangle
	RECT	rSrcClip;		/// source clipping rectangle. 

	VE_MODE mode;			/// display mode
	VR_GFXVIDWIN_ASPECT_STRUCT aspect;
};

DECLARE_INTERFACE(IMcVideoWindow)
{
	STDMETHOD(Open)(VE_Options *pOptions, DWORD dwOptionSize) PURE;
	STDMETHOD(Close)() PURE;
	STDMETHOD(Reset)() PURE;
	STDMETHOD(Get)(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned) PURE;
	STDMETHOD(Set)(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData) PURE;
	STDMETHOD(SetInterface)(DWORD dwRoute, REFIID riid, LPUNKNOWN pUnk) PURE;
	STDMETHOD(GetCaps)(DWORD *pCaps) PURE;
};

IMcVideoWindow *CreateVideoWindow();

#ifdef __cplusplus
}
#endif 


#endif //_VIDEO_WINDOW_H_
