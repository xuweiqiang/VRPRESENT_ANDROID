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
#include "GfxPresent.h"

#include "dp.h"
#include "tchar.h"
#include "GfxProcAndroidOpenGLES.h"
#include "GfxStreamAndroidOpenGLES.h"
 

static HRESULT ConvertPrecision(LONG& lSrcVal, const LONG lDstMax, const LONG lDstMin, const LONG lSrcMax,const LONG lSrcMin)
{
	LONG lSrcNum = lSrcMax - lSrcMin;
	LONG lDstNum = lDstMax - lDstMin;
    
	if(lSrcVal>lSrcMax)
		lSrcVal = lDstMax;
	else if(lSrcVal < lSrcMin)
		lSrcVal = lDstMin;
	else		
		lSrcVal = lDstMin + (lSrcVal-lSrcMin)*lDstNum/lSrcNum;
	return S_OK;
}



CAndroidProcOpenglES ::CAndroidProcOpenglES () :
	m_Init(false),
	m_bcanChanged(true),
	m_pVideoMixer(NULL),
    m_wndwidth(0),
    m_wndheight(0)
{
	ZeroMemory(&m_pStmArrar, sizeof(m_pStmArrar));	
	InitializeCriticalSection(&m_csDisplay);
	memset(&m_android_param, 0,sizeof(m_android_param));

}

CAndroidProcOpenglES ::~CAndroidProcOpenglES ()
{
	DeleteCriticalSection(&m_csDisplay);
}	


STDMETHODIMP CAndroidProcOpenglES ::Open(IN VP_OpenOptionsAdv *pOpenParams, IN DWORD dwOpenSize)
{
	HRESULT hr = S_OK;
	if(pOpenParams==NULL )	return E_INVALIDARG;
	if(m_Init)
		return E_UNEXPECTED;


	OpenGLESInit();
	memset(&m_JVM, 0 ,sizeof(m_JVM));
	m_Init = true;
	return hr;
}

STDMETHODIMP CAndroidProcOpenglES ::Close()
{

	if(!m_Init)
		return E_UNEXPECTED;
    for(int i=0;i<MAX_STREAM ;i++)
	{
		if(m_pStmArrar[i].valid)
            DestroyStream(m_pStmArrar[i].pStream);
    }
    
	OpenGLESUnInit();

	m_Init = false;
    
    
	return S_OK;
}


STDMETHODIMP CAndroidProcOpenglES::Render(DWORD dwRenderFlags)
{
	if(!m_Init)	return VR_FAIL_UNOPEN;
//	if(m_JVM.jvm == 0)
//		return VR_FAIL_USEVM;

    int window_width = 0;
    int window_height = 0;
    int ret = m_glRender->getWindowWidth(&window_width,&window_height);
    if (ret == 0 &&(window_width!=0) && (window_height != 0)) {
        if((m_wndwidth != window_width) || (m_wndheight != window_height))
        {
            m_wndwidth = window_width;
            m_wndheight  = window_height;
            m_bcanChanged = true;
        }
    }
    if(m_bcanChanged)
		return VR_FAIL_CANVASCHANGED;


//    LOGE("this %p \n",this);
//	m_JVM.env->CallIntMethod(m_JVM.obj, m_JVM.callbackFuns,this);

    return S_OK;
}


STDMETHODIMP CAndroidProcOpenglES ::Get(IN REFGUID guidPropSet, IN DWORD dwPropID, IN LPVOID pInstanceData,
	IN DWORD cbInstanceData, IN LPVOID pPropData, IN DWORD cbPropData, OUT DWORD *pcbReturned)
{
	switch (dwPropID)
	{
		case MC_PROPID_VP_CANVAS_INFO:
			{
				if(pPropData == NULL)	return E_INVALIDARG;
				if(!m_Init)	return VR_FAIL_UNOPEN;
				VP_CANVASINFO_PARAMS* pArgs = (VP_CANVASINFO_PARAMS*)pPropData;
				pArgs->dwWidth = m_wndwidth;
				pArgs->dwHeight = m_wndheight;
				m_bcanChanged = false;
				return S_OK;
			}
		case MC_PROPID_VP_ALLSTREAMS:
		{
			IMcVideoPresenterStream** pArray = (IMcVideoPresenterStream**)pPropData;
			int intputArraySize = cbPropData/ sizeof(IMcVideoPresenterStream*);
			if(pArray == 0)
				return E_INVALIDARG;

			if(GetStreamNumber()> intputArraySize)
				return E_OUTOFMEMORY;

			int i = 0;
			int j = 0;
			for(i=0;i<MAX_STREAM ;i++)
			{
				if(m_pStmArrar[i].valid)
				{
					pArray[j++] = m_pStmArrar[i].pStream;
				}
			}
			if(pcbReturned)
				*pcbReturned = j;
			return S_OK;
		}

		default:
			break;
	}
	return E_PROP_ID_UNSUPPORTED;
}

STDMETHODIMP CAndroidProcOpenglES::ResizeScreenInternal(int width, int height)
{
    m_glRender->resizeFromLayer(width,height);
    return S_OK;
}

STDMETHODIMP CAndroidProcOpenglES::CreateTextureInternal(int i)
{
    DWORD fourcc  = 0;
    if (m_pStmArrar[i].param.type == eMAINSTREAM)
    {
//        fourcc = MAKE_FOURCC('R','5','6','5');
        fourcc = MAKE_FOURCC('Y','V','1','2');
    }
    else
        fourcc = MAKE_FOURCC('Y','V','1','2');
//        fourcc = MAKE_FOURCC('R','5','5','5');
	m_pStmArrar[i].textureId = m_glRender->createTextureWidth(m_pStmArrar[i].param.dwWidth,
			m_pStmArrar[i].param.dwHeight,fourcc);

    return S_OK;
}

STDMETHODIMP CAndroidProcOpenglES::Set(IN REFGUID guidPropSet, IN DWORD dwPropID, IN LPVOID pInstanceData,
	IN DWORD cbInstanceData, IN LPVOID pPropData, IN DWORD cbPropData)
{
	if(!pPropData && (dwPropID!=MC_PROPID_VP_FLIP))
		return E_FAIL;

	switch (dwPropID)
	{
		case MC_PROPID_VP_VP_HND:			
			break;
		case MC_PROPID_VP_DLL_DIRECTORY:
			{
				HRESULT hr = SetDllDirectory((LPCSTR)pPropData);
				if(FAILED(hr)) return hr;
			}
			break;
		case MC_PROPID_VP_FLIP:
		{
			UINT i;
			for(i=0;i<MAX_STREAM ;i++)
			{
				if(m_pStmArrar[i].valid)
				{
					m_pStmArrar[i].pStream->Set(IID_IMcVideoPresenter, MC_PROPID_VP_FLIP, 0, 0, 0, 0);
				}
			}
		}
			break;
        case MC_PROPID_VP_RECT_SCREEN:
        	m_android_param = *reinterpret_cast<VR_ANDROID_ISUR_PARAM*>(pPropData);
            ResizeScreenInternal(m_android_param.scnWidth, m_android_param.scnHeight);
            break;
        case MC_PROPID_VP_ANDROID_JVM:
        {
//        	m_JVM = *reinterpret_cast<VR_JAVA_CALLBACK_FUN*>(pPropData);
//        	if(m_JVM.jvm == 0 || m_JVM.obj == 0)	return E_INVALIDARG;
//        	int status =m_JVM.jvm->GetEnv((void **) &m_JVM.env, JNI_VERSION_1_4);
//        	if(status< 0)
//        	{
//        		status =m_JVM.jvm->AttachCurrentThread(&m_JVM.env, NULL);
//        		if(status< 0)
//        			{
//        			LOGE("callback_handler: failed to attach "
//        				 "current thread");
//        			return -1;
//        		}
//        	}
//        	m_JVM.cls = m_JVM.env->GetObjectClass(m_JVM.obj);
//        	m_JVM.callbackFuns = m_JVM.env->GetMethodID(m_JVM.cls, "present", "(I)I");

        	break;
        }
        case MC_PROPID_VP_ANDROID_JVM_DETACH:
//        	if(m_JVM.jvm)
//        		m_JVM.jvm->DetachCurrentThread();

			break;
        default:
			return E_NOTIMPL;
	}

	return S_OK;
}


STDMETHODIMP CAndroidProcOpenglES ::CreateStream(IN VP_OPEN_STREAM_PARAMS *pOpenParams,
					 IN DWORD dwOpenSize ,OUT IMcVideoPresenterStream **ppStream)
{
	HRESULT hr = S_OK;
	if(!pOpenParams || !ppStream )
		return E_INVALIDARG;

	if(!m_Init)	return VR_FAIL_UNOPEN;
    
//Init a free stream	
	UINT i;
	for(i=0;i<MAX_STREAM ;i++)
	{
		if(!m_pStmArrar[i].valid)	break;
	}

	if(i>= MAX_STREAM)	return E_UNEXPECTED;
	memcpy(&m_pStmArrar[i].param, pOpenParams, sizeof(VP_OPEN_STREAM_PARAMS));


	DWORD miWidth = 0;
	DWORD miHeight = 0;
	
    hr = CalculateGLStride(pOpenParams->dwWidth, &miWidth);	
    if(FAILED(hr))
        return E_INVALIDARG;
    hr = CalculateGLStride(pOpenParams->dwHeight, &miHeight);
    if(FAILED(hr))
        return E_INVALIDARG;

    m_pStmArrar[i].pBuf = (LPBYTE)malloc(miWidth*miHeight*2);
    if(!m_pStmArrar[i].pBuf ) return E_FAIL;
    memset(m_pStmArrar[i].pBuf, 0x00,miWidth*miHeight*2);


	CProtect proc(&m_csDisplay);
    CreateTextureInternal(i);

    m_pStmArrar[i].pStream = new CStreamAndroidOpenglES(m_pStmArrar[i].pBuf,
		&m_csDisplay, &m_pStmArrar[i].param, miWidth, miHeight,m_glRender,
		m_pStmArrar[i].textureId,&m_bcanChanged);
	if(!m_pStmArrar[i].pStream)	goto RET;

	m_pStmArrar[i].valid = true;

	if(ppStream)
		*ppStream = static_cast<IMcVideoPresenterStream*>(m_pStmArrar[i].pStream);


	return hr;
	
RET:

	m_pStmArrar[i].pStream->Release();
    m_pStmArrar[i].pStream = 0;

	if(m_pStmArrar[i].pBuf)
	{
		free(m_pStmArrar[i].pBuf);
		m_pStmArrar[i].pBuf = NULL;
	}
	ZeroMemory(&m_pStmArrar[i], sizeof(SSTREAM_INFO));
	return E_FAIL;
}


STDMETHODIMP CAndroidProcOpenglES ::DestroyStream(IN IMcVideoPresenterStream *ppStream)
{
	if(!ppStream)	return E_FAIL;
	for(int i=0;i<MAX_STREAM;i++)
	{
		if(m_pStmArrar[i].pStream == ppStream)
		{
		CProtect proc(&m_csDisplay);
            m_glRender->DestroyTexture(m_pStmArrar[i].textureId);
            m_pStmArrar[i].pStream->Release();
			if(m_pStmArrar[i].pBuf)
				free(m_pStmArrar[i].pBuf);
			ZeroMemory(&m_pStmArrar[i], sizeof(SSTREAM_INFO));
			return S_OK;
		}
	}
	return E_NOTIMPL;
}


STDMETHODIMP CAndroidProcOpenglES::OpenGLESInit()
{
    m_glRender = new ESRender();
    if (m_glRender == 0) {
        DP("OPENGLES 2.0 isn't supported !!!! \n");
		return E_FAIL;
    }
	m_glRender->getWindowWidth(&m_wndwidth ,&m_wndheight);
	m_bcanChanged = true;

	return S_OK;
}

STDMETHODIMP CAndroidProcOpenglES::OpenGLESUnInit()
{
    if (m_glRender) {
    	delete m_glRender;
        m_glRender = 0;
    }
	return S_OK;
}

STDMETHODIMP CAndroidProcOpenglES::CalculateGLStride(DWORD input, DWORD* pOut)
{

	if (input == 0) {
		return -1;
	}
	GLuint mi =0;

	switch (input) {
	case 2:
	case 4:
	case 8:
	case 16:
	case 32:
	case 64:
	case 128:
	case 256:
	case 512:
	case 1024:
	case 2048:
		return input;
	default:
		break;
	}

	while(input >0)
	{
		mi++;
		input >>= 1;
	}
	*pOut = (2 <<(mi-1));

	return S_OK;
}

int CAndroidProcOpenglES::GetStreamNumber()
{
	int streamNum = 0;
	for(int i=0;i<MAX_STREAM ;i++)
	{
		if(m_pStmArrar[i].valid)
			streamNum++;
	}
	return streamNum;
}


