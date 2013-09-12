//===================================== ========================================
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

#include "GfxBuildDef.h"
#include <stdio.h>
#include <windows.h>
#include "GfxPresent.h"
#include "GfxVideoWindow.h"
#include "GfxPresentPrepare.h"
#include "ESRender.h"
//#include "../../inc/VideoMixer.h"
#include "McCom.h"

//#define DUMPSTREAM
#define INVALID_SRC_COLORKEY    0xff00ff00

class CStreamAndroidOpenglES :
	public IMcVideoPresenterStream,
	public CMcUnknown
{
public:
		CStreamAndroidOpenglES(LPBYTE pSurf, CRITICAL_SECTION* pCsDisplay,
		VP_OPEN_STREAM_PARAMS *pOpenParams, DWORD miWidth, 
                      DWORD miHeight,ESRender* render,unsigned int textureId,
					  bool* pCanvasChanged);
	virtual ~CStreamAndroidOpenglES ();

	DECLARE_IUNKNOWN
	STDMETHODIMP Open();
	STDMETHODIMP Close();
	STDMETHODIMP Get(IN REFGUID guidPropSet, IN DWORD dwPropID, 
                     IN LPVOID pInstanceData, IN DWORD cbInstanceData, 
                     IN LPVOID pPropData, IN DWORD cbPropData, OUT DWORD *pcbReturned);
    
	STDMETHODIMP Set(IN REFGUID guidPropSet, IN DWORD dwPropID, 
                     IN LPVOID pInstanceData, IN DWORD cbInstanceData, 
                     IN LPVOID pPropData, IN DWORD cbPropData);
    
	STDMETHODIMP LockSurface(LPBYTE *ppData, DWORD *pdwStride);
	STDMETHODIMP UnlockSurface();
	STDMETHODIMP Present(LPBYTE *ppbPlanes, DWORD *pdwStrides, DWORD dwFlags);

	STDMETHODIMP StartAnimation(VR_ANIMATION_PARAMS* pParms){return E_NOTIMPL;};
	STDMETHODIMP StopAnimation(){return E_NOTIMPL;};
    STDMETHODIMP ZoomInternal();

	VP_OPEN_STREAM_PARAMS m_open_param;
	DWORD mMiWidth;
	DWORD mMiHeight;
	VP_ZOOM_MODE m_zoom_mode;

private:
    STDMETHODIMP Update();
    STDMETHODIMP CalculateGLStride(DWORD input, DWORD* pOut);
    STDMETHODIMP PresentREAL();


	LPBYTE m_pBuf;;
	GfxPrepareDataV2 m_PrepareData;
	RECT m_rectSrc, m_rectDst, m_rectScn;
	bool m_bEnable;
	bool m_bDirty;
	CRITICAL_SECTION* m_pCsDisplay; 
	DWORD m_alpha;
	DWORD m_layerId;
	bool m_bOpen;
	VP_DEINTERLACE_MODE m_deinterlace;
	IMcVideoWindow * m_pVideoWnd;

	DWORD mOutputFormat;
    unsigned int m_textureId;
	bool* m_pCanvasChanged;
	DWORD m_SrcColorKey;
	ESRender* m_glRender;

	unsigned char* m_pBuffer;
	bool m_needUpdate;

};


