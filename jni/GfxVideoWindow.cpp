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
#include "GfxBuildDef.h"


#include <stdio.h>
#include <windows.h>
#include "GfxVideoWindow.h"
#include "GfxPresent.h"
#include "GfxContext.h"
#include "dp.h"

static HRESULT ReturnComLong(LONG lValue, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
{
	if(pPropData==0)
		return E_POINTER;
	if(cbPropData<sizeof(LONG))
		return E_INVALIDARG;
	*reinterpret_cast<LONG *>(pPropData) = lValue;
	if(pcbReturned)
		*pcbReturned = sizeof(LONG);
	return S_OK;
}


class CVideoWindow: 
	public IMcVideoWindow
{
public:
	CVideoWindow();
	virtual ~CVideoWindow();
	STDMETHODIMP Open(VE_Options *pOptions, DWORD dwOptionSize);
	STDMETHODIMP Close();
	STDMETHODIMP Reset();
	STDMETHODIMP Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned);
	STDMETHODIMP Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);
	STDMETHODIMP GetCaps(DWORD *pCaps)
	{
		if(pCaps==0)
			return E_POINTER;
		*pCaps = VE_MODE_CROPPING | VE_MODE_LETTERBOX | VE_MODE_STRETCH;
		return S_OK;
	}
	STDMETHODIMP SetInterface(DWORD dwRoute, REFIID riid, LPUNKNOWN pUnk);
protected:
	STDMETHODIMP SetDisplayMode(RECT* pSrc, RECT* pDst);
	CRITICAL_SECTION m_csEffect; 

	VE_Options			 m_VEOptions;

	RECT	m_rcScreen;

};

#define VIDEO_WINDOW_NAME TEXT("Video Window")

CVideoWindow::CVideoWindow()
{
	ZeroMemory(&m_VEOptions, sizeof(VE_Options));
	ZeroMemory(&m_rcScreen,  sizeof(m_rcScreen));
	InitializeCriticalSection(&m_csEffect);
}

CVideoWindow::~CVideoWindow()
{
	DeleteCriticalSection(&m_csEffect);
}

STDMETHODIMP CVideoWindow::Open(VE_Options *pOptions, DWORD dwOptionSize)
{
	if(pOptions)
		memcpy(&m_VEOptions, pOptions, min(dwOptionSize,sizeof(m_VEOptions)));
	
	return S_OK;
}

STDMETHODIMP CVideoWindow::Close()
{
	return S_OK;
}

STDMETHODIMP CVideoWindow::Reset()
{
	Close();
	return Open(&m_VEOptions,sizeof(m_VEOptions));
}

STDMETHODIMP CVideoWindow::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
{
	switch (dwPropID)
	{
		case MC_PROPID_VP_ZOOM:
			return ReturnComLong(m_VEOptions.mode,pPropData,cbPropData,pcbReturned);
		case VE_COMMAND_SRCRECT:
			RECT src;
			if(SUCCEEDED(SetDisplayMode(&src,NULL)))
			{
				*reinterpret_cast<RECT *>(pPropData) = src;
				return S_OK;
			}
			break;
		case VE_COMMAND_DSTRECT:
			RECT dst;
			if(SUCCEEDED(SetDisplayMode(NULL, &dst)))
			{
				*reinterpret_cast<RECT *>(pPropData) = dst;
				return S_OK;
			}
			break;
		default:
			break;
	}
	return E_PROP_ID_UNSUPPORTED;
}

STDMETHODIMP CVideoWindow::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData)
{
	switch (dwPropID)
	{
		case VE_COMMAND_SRCRECT:
			if(sizeof(m_VEOptions.rSrc) == cbPropData)
			{
				RECT* pInput = (RECT*) pPropData;
				if(pInput->left == 0&& pInput->top == 0 && pInput->right== 0 &&pInput->bottom== 0 )	return E_FAIL;
				if(pPropData)
					memcpy(&m_VEOptions.rSrc, pPropData,sizeof(m_VEOptions.rSrc));
			}
			return S_OK;
		case VE_COMMAND_DSTRECT:
			if(sizeof(m_VEOptions.rDst) == cbPropData)
			{
				RECT* pInput = (RECT*) pPropData;
				if(pInput->left == 0&& pInput->top == 0 && pInput->right== 0 &&pInput->bottom== 0 )	return E_FAIL;
				if(pPropData)
					memcpy(&m_VEOptions.rDst, pPropData,sizeof(m_VEOptions.rDst));
			}
			return S_OK;		
		case VE_COMMAND_ASPECTRATIO:
			if(sizeof(m_VEOptions.aspect) == cbPropData)
			{
				if(pPropData)
					memcpy(&m_VEOptions.aspect, pPropData,sizeof(m_VEOptions.aspect));
			}
			return S_OK;
		case VE_COMMAND_MODE:
			if(sizeof(m_VEOptions.mode) == cbPropData)
			{
				if(pPropData)
					memcpy(&m_VEOptions.mode, pPropData,sizeof(m_VEOptions.mode));
			}
			return S_OK;
			
	}
	return E_PROP_ID_UNSUPPORTED;
}

STDMETHODIMP CVideoWindow::SetInterface(DWORD dwRoute, REFIID riid, LPUNKNOWN pUnk)
{
	return S_OK;
}

STDMETHODIMP CVideoWindow::SetDisplayMode(RECT* pSrc, RECT* pDst)
{

	DWORD dwSrcWidth = m_VEOptions.rSrc.right - m_VEOptions.rSrc.left;
	DWORD dwSrcHight = m_VEOptions.rSrc.bottom- m_VEOptions.rSrc.top;
	DWORD dwDstWidth = m_VEOptions.rDst.right - m_VEOptions.rDst.left;
	DWORD dwDstHight = m_VEOptions.rDst.bottom- m_VEOptions.rDst.top;
	DWORD dwOffsetX  = 0;//m_VEOptions.p.x;
	DWORD dwOffsetY	 = 0;//m_VEOptions.p.y;
	int   j = 0, 
		  k = 0;
	RECT  src, dst;

	ZeroMemory(&src, sizeof(src));
	ZeroMemory(&dst, sizeof(dst));
	
	dwOffsetX = m_VEOptions.rDst.left;
	dwOffsetY = m_VEOptions.rDst.top;

	switch(m_VEOptions.mode)
	{
	case VE_MODE_STRETCH:
		{
			src.top		=  0;
			src.bottom	=  dwSrcHight;
			src.left	=  0;
			src.right	=  dwSrcWidth;

			dst.top		=  dwOffsetY;
			dst.bottom	=  dwOffsetY + dwDstHight;
			dst.left	=  dwOffsetX;
			dst.right	=  dwOffsetX + dwDstWidth;

			break;
		}
	case VE_MODE_LETTERBOX:
		{
			float aspectSrc = 0;
			float aspectDst = 0;

			if(m_VEOptions.aspect.dwAspectX*m_VEOptions.aspect.dwAspectY)
				aspectSrc = (float)m_VEOptions.aspect.dwAspectX/m_VEOptions.aspect.dwAspectY;
			else
				aspectSrc =  (float)dwSrcWidth/dwSrcHight;

			aspectDst = (float)dwDstWidth/dwDstHight;

			if(aspectSrc > aspectDst )
			{
				//fill black strip on top and bottom

				if(m_VEOptions.aspect.dwAspectX*m_VEOptions.aspect.dwAspectY)
				{
					j = dwDstWidth * m_VEOptions.aspect.dwAspectY/m_VEOptions.aspect.dwAspectX;
					j = dwDstHight - j;
				}
				else
				{
					j = dwDstWidth * dwSrcHight/dwSrcWidth;
					j = dwDstHight - j;
				}
				j = (j+1)/2;
			}
			else
			{
				if(m_VEOptions.aspect.dwAspectX*m_VEOptions.aspect.dwAspectY)
				{
					k = dwDstHight * m_VEOptions.aspect.dwAspectX/m_VEOptions.aspect.dwAspectY;
					k = dwDstWidth - k;
				}
				else
				{
					k = dwDstHight * dwSrcWidth/dwSrcHight;
					k = dwDstWidth - k;
				}
				k = (k+1)/2;
			}

			src.top		=  0;
			src.bottom	=  dwSrcHight;
			src.left	=  0;
			src.right	=  dwSrcWidth;

			dst.top		=  dwOffsetY + j;
			dst.bottom	=  dwOffsetY + dwDstHight -j;
			dst.left	=  dwOffsetX + k;
			dst.right	=  dwOffsetX + dwDstWidth -k;

			break;
		}
	case VE_MODE_CROPPING:
		{
			//if((dwDstHight < dwSrcHight) || (dwDstWidth < dwSrcWidth))
			//	break;
			float aspectSrc = 0;
			float aspectDst = 0;

			float fScale;
 			if(m_VEOptions.aspect.dwAspectX*m_VEOptions.aspect.dwAspectY)
 			{
 				aspectSrc = (float)m_VEOptions.aspect.dwAspectX/m_VEOptions.aspect.dwAspectY;
				fScale = ((float)m_VEOptions.aspect.dwAspectX/(float)m_VEOptions.aspect.dwAspectY);

 			}
 			else
			{
				aspectSrc =  (float)dwSrcWidth/dwSrcHight;
				fScale = (float)dwSrcWidth/dwSrcHight;
			}

			aspectDst = (float)dwDstWidth/dwDstHight;


			if(aspectSrc > aspectDst)
			{
				k = (int)(dwSrcWidth - dwSrcWidth * aspectDst /fScale );
				k = (k+1)/2;
				if(k%2 == 1)
					k++;
			}
			else
			{
				j = (int)(dwSrcHight - dwSrcHight / aspectDst * fScale);
				j = (j+1)/2;
				if(j%2 == 1)
					j++;

			}
			src.top		=  j;
			src.bottom	=  dwSrcHight -j;
			src.left	=  k;
			src.right	=  dwSrcWidth -k;

			dst.top		=  dwOffsetY ;
			dst.bottom	=  dwOffsetY + dwDstHight;
			dst.left	=  dwOffsetX ;
			dst.right	=  dwOffsetX + dwDstWidth;
			break;
		}
	default:
		return E_FAIL;
	}

	if(pSrc)
		*pSrc = src;
	if(pDst)
		*pDst = dst;

	return S_OK;
}

IMcVideoWindow *CreateVideoWindow()
{
	return new CVideoWindow();
}
