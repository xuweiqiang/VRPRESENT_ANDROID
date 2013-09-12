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

#include "GfxStreamAndroidOpenGLES.h"
#include "dp.h"


CStreamAndroidOpenglES::CStreamAndroidOpenglES(LPBYTE pSurf, 	CRITICAL_SECTION* pCsDisplay,
	VP_OPEN_STREAM_PARAMS *pOpenParams, DWORD miWidth, 
    DWORD miHeight,ESRender* render,unsigned int textureId,bool* pCanvasChanged):
m_pBuf(pSurf),
m_bEnable(false),
m_pCsDisplay(pCsDisplay),
m_layerId(0),
m_deinterlace(VP_DEINTERLACE_None),
mMiWidth(miWidth),
mMiHeight(miHeight),
m_pVideoWnd(0),
m_pCanvasChanged(pCanvasChanged)
{
	m_glRender = render;
	if(pOpenParams)
		memcpy(&m_open_param, pOpenParams, sizeof(m_open_param));
	m_alpha = (DWORD)(m_open_param.fAlpha*MAX_ALPHA_VAL);
    Set(IID_IMcVideoPresenter, MC_PROPID_VP_ALPHA, NULL, 0,&m_alpha,sizeof(m_alpha));
    
	memset(&m_rectDst, 0 ,sizeof(m_rectDst));
	m_zoom_mode = VP_ZOOM_MODE_STRETCH;
    m_textureId = textureId;
	
//	flipstreamObj = [[NsFlipStream alloc]init];
//	flipstreamObj.pStreamObj = this;

	m_bOpen = false;
	
    if (m_open_param.type == eMAINSTREAM) {
//        mOutputFormat = MAKE_FOURCC('R','5','6','5');
        mOutputFormat = MAKE_FOURCC('Y','V','1','2');
        memset(m_pBuf, 0x00, mMiWidth*mMiHeight);
        memset(m_pBuf+mMiWidth*mMiHeight, 0x80, mMiWidth*mMiHeight/2);
    }
    else
//        mOutputFormat = MAKE_FOURCC('R','5','5','5');
    mOutputFormat = MAKE_FOURCC('Y','V','1','2');

	return;
}

CStreamAndroidOpenglES::~CStreamAndroidOpenglES()
{
//	[flipstreamObj release];
//	flipstreamObj = nil;

	return;
}

STDMETHODIMP CStreamAndroidOpenglES::Open()
{
	if(m_bOpen)	return E_UNEXPECTED;

    memset(&m_rectDst, 0 ,sizeof(m_rectDst));
	m_zoom_mode = VP_ZOOM_MODE_STRETCH;
	m_deinterlace  = VP_DEINTERLACE_None;
    m_SrcColorKey = INVALID_SRC_COLORKEY;
    m_bDirty = false;
    if(!m_pVideoWnd )
		m_pVideoWnd = CreateVideoWindow();
	if(!m_pVideoWnd )	return E_FAIL;

    VE_Options tVEOpen;
	ZeroMemory(&tVEOpen, sizeof(tVEOpen));
	tVEOpen.mode = (VE_MODE)m_zoom_mode;
	tVEOpen.rSrcClip.right = tVEOpen.rDst.right = tVEOpen.rSrc.right = m_open_param.dwWidth;
	tVEOpen.rSrcClip.bottom = tVEOpen.rDst.bottom =  tVEOpen.rSrc.bottom = m_open_param.dwHeight;
	tVEOpen.aspect.dwAspectX = m_open_param.dwAspectX;
	tVEOpen.aspect.dwAspectY = m_open_param.dwAspectY;
	HRESULT hr = m_pVideoWnd->Open(&tVEOpen,sizeof(tVEOpen));
	if(FAILED(hr)) 	return E_FAIL;

	m_pBuffer = (unsigned char*)malloc(m_open_param.dwWidth*m_open_param.dwHeight*4);
	if(m_pBuffer == 0)
		return E_FAIL;
	m_SrcColorKey = m_open_param.dwTranColor;
	m_needUpdate = true;
	m_bOpen = true;
	return S_OK;
};

STDMETHODIMP CStreamAndroidOpenglES::Close()
{
	if(!m_bOpen)	return E_UNEXPECTED;
	if(m_pBuffer)
	{
		free(m_pBuffer);
		m_pBuffer = 0;
	}
	if (m_pVideoWnd) {
        m_pVideoWnd->Close();
        m_pVideoWnd = NULL;
    }
	m_bOpen = false;
	return S_OK;
};
	
STDMETHODIMP CStreamAndroidOpenglES::Get(IN REFGUID guidPropSet, IN DWORD dwPropID,
	IN LPVOID pInstanceData, IN DWORD cbInstanceData, IN LPVOID pPropData, IN DWORD cbPropData, OUT DWORD *pcbReturned)
{
	if(pPropData==0)
		return E_POINTER;

	switch(dwPropID)
	{
		case MC_PROPID_VP_RECTSRC:
			{
				*reinterpret_cast<RECT *>(pPropData) = m_rectSrc;
				if(pcbReturned)
					*pcbReturned = sizeof(RECT);
				return S_OK;
			}
		case MC_PROPID_VP_RECTDST:
			{
				if(pPropData)
					*reinterpret_cast<RECT *>(pPropData) = m_rectDst;
				if(pcbReturned)
					*pcbReturned = sizeof(RECT);
				return S_OK;
			}
		case MC_PROPID_VP_ENABLE:
			{
				*reinterpret_cast<bool *>(pPropData) = m_bEnable;
				if(pcbReturned)
					*pcbReturned = sizeof(RECT);
				return S_OK;
			}
		case MC_PROPID_VP_ALPHA:
			{
				*reinterpret_cast<DWORD*>(pPropData) = m_alpha;
				return S_OK;
			}
		case MC_PROPID_VP_ZOOM:
			{
				*reinterpret_cast<DWORD*>(pPropData) = m_zoom_mode ;
				return S_OK;
			}
		case MC_PROPID_VP_LAYER_ID:
			{
				*reinterpret_cast<DWORD*>(pPropData) = m_layerId;
				return S_OK;
			}
			break;
		case MC_PROPID_VP_DEINTERLACE_MODE:
			{
				*reinterpret_cast<VP_DEINTERLACE_MODE*>(pPropData) = m_deinterlace ;
				return S_OK;
			}
			break;
            
        case MC_PROPID_VP_COLORKEYSRC:
            {
                *reinterpret_cast<DWORD*>(pPropData) = m_SrcColorKey ;
                return S_OK;
            }
		default:
			break;
	}
	return E_NOTIMPL;
}

STDMETHODIMP CStreamAndroidOpenglES::Set(IN REFGUID guidPropSet, IN DWORD dwPropID,
	IN LPVOID pInstanceData, IN DWORD cbInstanceData, IN LPVOID pPropData, IN DWORD cbPropData)
{
	if(!m_bOpen && dwPropID != MC_PROPID_VP_ONESURF_MODE )	return E_UNEXPECTED;
	
	switch (dwPropID)
	{
	case MC_PROPID_VP_RECTSRC:
		{
			if(!pPropData)	return E_INVALIDARG;
			RECT *pr = reinterpret_cast<RECT *>(pPropData);
			HRESULT hr = m_pVideoWnd->Set(IID_IMcVideoPresenter, VE_COMMAND_SRCRECT,0,0,pr, sizeof(RECT));
			if(FAILED(hr))	return hr;
			m_needUpdate = true;
		}
		break;
	case MC_PROPID_VP_RECTDST:
		{
			if(!pPropData)	return E_INVALIDARG;
			RECT* pr = reinterpret_cast<RECT *>(pPropData);
			HRESULT hr = m_pVideoWnd->Set(IID_IMcVideoPresenter, VE_COMMAND_DSTRECT,0,0,pr, sizeof(RECT));
			if(FAILED(hr))	return hr;
			m_needUpdate = true;
		}
		break;
	case MC_PROPID_VP_ENABLE:
		{
			if(!pPropData)	return E_INVALIDARG;
			m_bEnable = *(reinterpret_cast<bool*>(pPropData));
            EnterCriticalSection(m_pCsDisplay);
            m_glRender->ShowHideTexture(m_textureId, m_bEnable);
            LeaveCriticalSection(m_pCsDisplay);
		}
		break;
	case MC_PROPID_VP_ALPHA:
		{
			if(!pPropData)	return E_INVALIDARG;
			m_alpha   = *(reinterpret_cast<DWORD*>(pPropData));
            float alpha = (float)m_alpha / (float)MAX_ALPHA_VAL;

            EnterCriticalSection(m_pCsDisplay);
            m_glRender->SetAlphaTexture(m_textureId,alpha);
            LeaveCriticalSection(m_pCsDisplay);
		}
		break;
	case MC_PROPID_VP_ZOOM:
		{
			if(!pPropData)	return E_INVALIDARG;
			m_zoom_mode = *reinterpret_cast<VP_ZOOM_MODE*>(pPropData);			
			m_pVideoWnd->Set(IID_IMcVideoPresenter, VE_COMMAND_MODE,0,0,pPropData, sizeof(VP_ZOOM_MODE));
            if(!m_bEnable)  break;
			m_needUpdate = true;
		}
		break;
	case MC_PROPID_VP_LAYER_ID:
		if(!pPropData)	return E_INVALIDARG;
		m_layerId = *(reinterpret_cast<DWORD*>(pPropData));
		break;
	case MC_PROPID_VP_CLEAR:
			if(mOutputFormat == MAKE_FOURCC('R','5','6','5'))
			{
				memset(m_pBuf, 0x0,m_open_param.dwWidth*m_open_param.dwHeight*2);

                EnterCriticalSection(m_pCsDisplay);
                m_glRender->RenderTexture(m_textureId,m_pBuf);
                LeaveCriticalSection(m_pCsDisplay);

				return S_OK;
			}
			else if(mOutputFormat == MAKE_FOURCC('R','5','5','5'))
			{
                DWORD miWidth =0;
                DWORD miHeight = 0;
                CalculateGLStride(m_open_param.dwWidth,&miWidth);
                CalculateGLStride(m_open_param.dwHeight,&miHeight);
				memset(m_pBuf, 0x0,miWidth*miHeight*2);

                EnterCriticalSection(m_pCsDisplay);
                m_glRender->RenderTexture(m_textureId,m_pBuf);
                LeaveCriticalSection(m_pCsDisplay);

				return S_OK;
			}
            else if (mOutputFormat == MAKE_FOURCC('Y','V','1','2')){
                memset(m_pBuf, 0x10,m_open_param.dwWidth*m_open_param.dwHeight);
                memset(m_pBuf+m_open_param.dwWidth*m_open_param.dwHeight
                       , 0x80,m_open_param.dwWidth*m_open_param.dwHeight/2);

                EnterCriticalSection(m_pCsDisplay);
                m_glRender->RenderTexture(m_textureId ,m_pBuf);
                LeaveCriticalSection(m_pCsDisplay);

            }
			else
				return E_NOTIMPL;
		break;
	case MC_PROPID_VP_DEINTERLACE_MODE:
		{
			if(!pPropData)	return E_INVALIDARG;
			m_deinterlace  = *(reinterpret_cast<VP_DEINTERLACE_MODE*>(pPropData));
		}
		break;
    case MC_PROPID_VP_COLORKEYSRC:
        {
			if(!pPropData)	return E_INVALIDARG;
			m_SrcColorKey  = *(reinterpret_cast<DWORD*>(pPropData));
            m_PrepareData.Set(MC_PROPID_GFXPREPARE_COLORKEY, &m_SrcColorKey, sizeof(m_SrcColorKey));
		}
		break;
    case MC_PROPID_VP_FLIP:
    	PresentREAL();
    	break;
	case MC_PROPID_VP_VM_CONTEXT:
		break;
	default:
		return E_FAIL;
	}
	return S_OK;
 }

STDMETHODIMP CStreamAndroidOpenglES::LockSurface(LPBYTE *ppData, DWORD *pdwStride)
{
	HRESULT hr  = S_OK;
	if(ppData)
		*ppData = (LPBYTE)m_pBuf;
	
	if(pdwStride)
	{
		DWORD bytePerPixel = 0;
		switch(mOutputFormat)
		{
			case MAKE_FOURCC('R', '5', '6', '5'):
			case MAKE_FOURCC('R', '5', '5', '5'):
				bytePerPixel = 2;
				break;
			case MAKE_FOURCC('Y', 'V', '1', '2'):
			case MAKE_FOURCC('N', 'V', '1', '2'):
				bytePerPixel = 1;
				break;
			default:
				return E_FAIL;
		}
//		*pdwStride = mMiWidth * bytePerPixel;
		*pdwStride = m_open_param.dwWidth * bytePerPixel;
	}

	return hr;

}
STDMETHODIMP CStreamAndroidOpenglES::UnlockSurface()
{
	return  S_OK;
}


STDMETHODIMP CStreamAndroidOpenglES::Present(LPBYTE *ppbPlanes, DWORD *pdwStrides, DWORD dwFlags)
{
	DWORD srcWidth[3];
	DWORD srcHeight[3];

	srcWidth[0] = m_open_param.dwWidth;
	srcWidth[1] = srcWidth[2] = m_open_param.dwWidth>>1;

	srcHeight[0] = m_open_param.dwHeight;
	srcHeight[1] = srcHeight[2] = m_open_param.dwHeight>>1;

	if(m_open_param.dwFourCC == MAKE_FOURCC('Y','V','1','2'))
	{
//		if(pdwStrides[0] == m_open_param.dwWidth)
//		{
//			memcpy(m_pBuffer,ppbPlanes[0],m_open_param.dwWidth*m_open_param.dwHeight*3/2);
//			return S_OK;
//		}
//		else
//		{
//			for(int i=0;i<m_open_param.dwHeight;i++)
//				memcpy(m_pBuffer+ i*m_open_param.dwWidth,ppbPlanes[0]+ i*pdwStrides[0],m_open_param.dwWidth);
//			for(int i=0;i<m_open_param.dwHeight/2;i++)
//				memcpy(m_pBuffer+m_open_param.dwWidth*m_open_param.dwHeight,ppbPlanes[1]+ i*pdwStrides[1],m_open_param.dwWidth/2);
//			for(int i=0;i<m_open_param.dwHeight/2;i++)
//				memcpy(m_pBuffer+m_open_param.dwWidth*m_open_param.dwHeight*5/4,ppbPlanes[2]+ i*pdwStrides[2],m_open_param.dwWidth/2);
//
//			return S_OK;
//		}

		LPBYTE pBuf = NULL;
		DWORD stride = 0;
		HRESULT hr  = S_OK;

		hr = LockSurface(&pBuf , &stride);
		if(FAILED(hr))	return hr;



		BYTE* pDstPlanes_arr[3] =
		{
				pBuf,
				pBuf + m_open_param.dwWidth*m_open_param.dwHeight*5/4,
				pBuf + m_open_param.dwWidth*m_open_param.dwHeight
		};

		DWORD pDstWidthStride_arr[3] =
		{
				m_open_param.dwWidth,
				m_open_param.dwWidth/2,
				m_open_param.dwWidth/2
		};


		m_PrepareData.PrepareBufferYV12_Test(ppbPlanes, pDstPlanes_arr,srcWidth, srcHeight,
				pdwStrides, pDstWidthStride_arr, dwFlags, 0,m_deinterlace);

		hr = UnlockSurface();
		if(FAILED(hr))	return hr;
		m_bDirty = true;
		return S_OK;
	}
	return E_NOTIMPL;
}
STDMETHODIMP CStreamAndroidOpenglES::PresentREAL()
{
	if (*m_pCanvasChanged) {
		return E_FAIL;
	}
    if (m_bEnable == false) {
        return S_OK;
    }

    if(m_needUpdate == true)
    {
		m_needUpdate = false;
		Update();

    }
//
//    unsigned char* ppbPlanes[3];
//    DWORD pdwStrides[3];
//	DWORD dwFlags = 0;
//
//	if(m_open_param.dwFourCC == MAKE_FOURCC('Y','V','1','2'))
//	{
//		ppbPlanes[0] = m_pBuffer;
//		ppbPlanes[1] = m_pBuffer + m_open_param.dwWidth*m_open_param.dwHeight;
//		ppbPlanes[2] = m_pBuffer +m_open_param.dwWidth*m_open_param.dwHeight*5/4;
//
//		pdwStrides[0] = m_open_param.dwWidth;
//		pdwStrides[1] = m_open_param.dwWidth>>1;
//		pdwStrides[2] = m_open_param.dwWidth>>1;
//	}
//	else
//		return E_NOTIMPL;
//
//	LPBYTE pBuf = NULL;
//	DWORD stride = 0;
//	HRESULT hr  = S_OK;
//
//	hr = LockSurface(&pBuf , &stride);
//	if(FAILED(hr))	return hr;
//
////	DWORD srcWidth[3];
////	DWORD srcHeight[3];
////	srcWidth[0] = m_open_param.dwWidth;
////	srcWidth[1] = srcWidth[2] = m_open_param.dwWidth>>1;
////
////	srcHeight[0] = m_open_param.dwHeight;
////	srcHeight[1] = srcHeight[2] = m_open_param.dwHeight>>1;
////
////
////	if(mOutputFormat == MAKE_FOURCC('R','5','6','5'))
////	{
////		if(m_open_param.dwFourCC == MAKE_FOURCC('Y','V','1','2'))
////		{
////			HRESULT res = 	m_PrepareData.PrepareBufferRGB565(pBuf,
////				stride, ppbPlanes, pdwStrides, srcWidth, srcHeight, dwFlags, VP_ROTATION_0);
////			if(FAILED(res))	return res;
////		}
////		else if(m_open_param.dwFourCC == MAKE_FOURCC('R','5','6','5'))
////		{
////			for(int i=0;i<m_open_param.dwHeight;i++)
////				memcpy(pBuf + i*mMiWidth, ppbPlanes[0],m_open_param.dwWidth*2);
////		}
////
////	}
////	else if(mOutputFormat == MAKE_FOURCC('R','5','5','5'))
////	{
////		if(m_open_param.dwFourCC == MAKE_FOURCC('Y','V','1','2'))
////		{
////			HRESULT res = 	m_PrepareData.PrepareYV12ToRGB555(pBuf,
////                        stride, ppbPlanes, pdwStrides, srcWidth, srcHeight,
////                        dwFlags, VP_ROTATION_0,m_SrcColorKey);
////			if(FAILED(res))	return res;
////		}
////    }
////    else if(mOutputFormat == MAKE_FOURCC('Y','V','1','2'))
////    {
////        if(m_open_param.dwFourCC == MAKE_FOURCC('Y','V','1','2'))
////        {
////            HRESULT res = 	m_PrepareData.PrepareBufferYV12(pBuf,
////                         stride,mMiHeight, ppbPlanes, pdwStrides, srcWidth, srcHeight,
////                         dwFlags, VP_ROTATION_0,false,0,m_deinterlace);
////            if(FAILED(res))	return  res;
////        }
////    }
//	hr = UnlockSurface();
//	if(FAILED(hr))	return hr;

    if(m_bDirty == false)
    	return S_OK;
	EnterCriticalSection(m_pCsDisplay);
    m_glRender->RenderTexture(m_textureId,m_pBuf);
	LeaveCriticalSection(m_pCsDisplay);
	return S_OK;
}

STDMETHODIMP CStreamAndroidOpenglES::Update()
{
	HRESULT ret = S_OK;
	RECT rSrc, rDst;
	ret = m_pVideoWnd->Get(IID_IMcVideoPresenter, VE_COMMAND_SRCRECT,0,0,&rSrc, sizeof(rSrc),NULL);
	if(FAILED(ret))
		return ret;
	m_rectSrc = rSrc;
	
	ret = m_pVideoWnd->Get(IID_IMcVideoPresenter, VE_COMMAND_DSTRECT,0,0,&rDst, sizeof(rDst),NULL);
	if(FAILED(ret))
		return ret;
    
	m_rectDst = rDst;
    
    EnterCriticalSection(m_pCsDisplay);
    Rect src;
    src.left = m_rectSrc.left;
    src.top = m_rectSrc.top;
    src.right = m_rectSrc.right;
    src.bottom = m_rectSrc.bottom;
    m_glRender->SetSrcRectTexture(m_textureId,&src);

    Rect dst;
    dst.left = m_rectDst.left;
    dst.top = m_rectDst.top;
    dst.right = m_rectDst.right;
    dst.bottom = m_rectDst.bottom;
    m_glRender->SetDestRectTexture(m_textureId,&dst);
    LeaveCriticalSection(m_pCsDisplay);

	return S_OK;
}

STDMETHODIMP CStreamAndroidOpenglES::ZoomInternal()
{
	m_pVideoWnd->Set(IID_IMcVideoPresenter, VE_COMMAND_MODE,0,0,&m_zoom_mode, sizeof(VP_ZOOM_MODE));
	HRESULT hr = Update();
	if(FAILED(hr))	return hr;
	m_glRender->RenderTexture(m_textureId,m_pBuf);
	return S_OK;
}

STDMETHODIMP CStreamAndroidOpenglES::CalculateGLStride(DWORD input, DWORD* pOut)
{
	if(!pOut ||input>1024 )	return E_FAIL;
	if(input >512)	*pOut = 512<<1;
	else if(input >512>>1) 	*pOut = 512;
	else if(input >512>>2) 	*pOut = 512>>1;
	else if(input >512>>3) 	*pOut = 512>>2;
	else if(input >512>>4) 	*pOut = 512>>3;
	else 
		*pOut = 512>>4;
	
	return S_OK;
}

