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

#include <jni.h>

#include <stdio.h>
#include <windows.h>
#include "GfxPresent.h"

#include "dp.h"
#include "tchar.h"
#include "GfxPresentPrepare.h"
#include "ESRender.h"
#include "McCom.h"
#define MAX_STREAM 8


typedef struct _JAVA_CALLBACK_FUN
{
	JNIEnv    						*env;
	JavaVM    						*jvm;
	jobject   						obj;
	jclass    						cls;
	jmethodID 					callbackFuns;
}VR_JAVA_CALLBACK_FUN;

class CAndroidProcOpenglES :
	public IMcVideoPresenterAdv,
	public CMcUnknown
{
public:
	CAndroidProcOpenglES ();
	virtual ~CAndroidProcOpenglES ();

	DECLARE_IUNKNOWN
	//Interface 3.0
	STDMETHODIMP Open(IN VP_OpenOptionsAdv *pOpenParams, IN DWORD dwOpenSize);
	STDMETHODIMP Close();
	STDMETHODIMP Render(DWORD dwRenderFlags);
	STDMETHODIMP Get(IN REFGUID guidPropSet, IN DWORD dwPropID, IN LPVOID pInstanceData, 
		IN DWORD cbInstanceData, IN LPVOID pPropData, IN DWORD cbPropData, OUT DWORD *pcbReturned);
	STDMETHODIMP Set(IN REFGUID guidPropSet, IN DWORD dwPropID, IN LPVOID pInstanceData, 
		IN DWORD cbInstanceData, IN LPVOID pPropData, IN DWORD cbPropData);

	STDMETHODIMP CreateStream(IN VP_OPEN_STREAM_PARAMS *pOpenParams,
					 IN DWORD dwOpenSize ,OUT IMcVideoPresenterStream **ppStream);
	STDMETHODIMP DestroyStream(IN IMcVideoPresenterStream *ppStream);

	STDMETHODIMP OpenGLESInit();
	STDMETHODIMP OpenGLESUnInit();
    STDMETHODIMP ResizeScreenInternal(int width, int height);
    STDMETHODIMP CreateTextureInternal(int i);

private:

	STDMETHODIMP CalculateGLStride(DWORD input, DWORD* pOut);
	int GetStreamNumber();

	bool m_Init;

	typedef struct
	{
		bool valid;
		LPBYTE pBuf;
		VP_OPEN_STREAM_PARAMS param;
		IMcVideoPresenterStream *pStream;
        unsigned int textureId;
	}SSTREAM_INFO;
	SSTREAM_INFO m_pStmArrar[MAX_STREAM];

	CRITICAL_SECTION m_csDisplay; 
	bool m_bcanChanged;
	ICivmVideoMixer* m_pVideoMixer;
	VR_ANDROID_ISUR_PARAM m_android_param;
//    VR_IOS_PARAMS m_surf_param;
	int m_wndwidth ;
	int m_wndheight ;
	ESRender* m_glRender;
	VR_JAVA_CALLBACK_FUN m_JVM;
};



