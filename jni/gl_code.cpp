/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// OpenGL ES 2.0 code

#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <windows.h>
#include "GfxPresent.h"
//#include "dp.h"

#define  LOG_TAG    "libgl2jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)



extern "C" {
/*
 * Class:     com_example_com_android_vrpresenttest_GL2JNILib
 * Method:    init
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_example_com_android_vrpresenttest_GL2JNILib_init
  (JNIEnv *, jobject, jint, jint);

/*
 * Class:     com_example_com_android_vrpresenttest_GL2JNILib
 * Method:    step
 * Signature: (III)V
 */
JNIEXPORT void JNICALL Java_com_example_com_android_vrpresenttest_GL2JNILib_step
  (JNIEnv *, jobject, jint);
};

typedef struct _JAVA_CALLBACK_FUN
{
	JNIEnv    						*env;
	JavaVM    						*jvm;
	jobject   						obj;
	jclass    						cls;
	jmethodID 					callbackFuns;
}JAVA_CALLBACK_FUN;

static JAVA_CALLBACK_FUN jCallbackFun;

IMcVideoPresenterAdv* m_pPresenterAdv = 0;
IMcVideoPresenterStream* m_pStream1 = NULL;
IMcVideoPresenterStream* m_pStream2 = NULL;

VP_CANVASINFO_PARAMS m_canvasInfo;
static HANDLE h_avengine = 0;
unsigned char* m_pbuff = NULL;

int surfwidth = 720;
int surfheight = 480;

//int surfwidth = 1280;
//int surfheight = 720;

//int surfwidth = 1920;
//int surfheight = 1080;

FILE* mFile = 0;
long mFileSize =0;
char filelocation[64];

VR_ANDROID_ISUR_PARAM m_android_param;

typedef enum
{
	E_INIT,
	E_RENDERING,
	E_WAITING,
	E_PAUSE,
	E_EXIT
} RENDERINGSTATE;

RENDERINGSTATE gstate = E_INIT;

DWORD AVENGINE_Thread(void* param)
{

//	m_pPresenterAdv->Set(IID_IMcVideoPresenter, MC_PROPID_VP_ANDROID_JVM, 0, 0, &jCallbackFun, sizeof(jCallbackFun));

	int status =jCallbackFun.jvm->GetEnv((void **) &jCallbackFun.env, JNI_VERSION_1_4);
	if(status< 0)
	{
		status =jCallbackFun.jvm->AttachCurrentThread(&jCallbackFun.env, NULL);
		if(status< 0)
			{
			LOGE("callback_handler: failed to attach "
				 "current thread");
			return -1;
		}
	}

	jCallbackFun.cls = jCallbackFun.env->GetObjectClass(jCallbackFun.obj);
	jCallbackFun.callbackFuns = jCallbackFun.env->GetMethodID(jCallbackFun.cls, "present", "(I)I");

	DWORD tick = 0;
	while(gstate != E_EXIT)
	{
		if(gstate == E_PAUSE)
		{
			usleep(30*1000);
			continue;
		}
		if(m_pbuff==0 || mFile == 0)
			continue;
		tick = GetTickCount();
	    int byteRead = fread(m_pbuff, 1, surfwidth*surfheight*3/2, mFile);
	    if(byteRead <surfwidth*surfheight*3/2)
	    {
	        fseek(mFile, 0, SEEK_SET);
	        byteRead = fread(m_pbuff, 1, surfwidth*surfheight*3/2, mFile);
	    }

	    unsigned char* pBuffArr[3];
	    pBuffArr[0] = m_pbuff;
	    pBuffArr[1] = m_pbuff + surfwidth*surfheight;
	    pBuffArr[2] = m_pbuff + surfwidth*surfheight*5/4;

	    DWORD strides[3] = {surfwidth,surfwidth/2,surfwidth/2};

	    if(m_pStream1)
	    {
	        HRESULT hr = m_pStream1->Present(pBuffArr, strides, 0);
	        if(FAILED(hr))
	            LOGE("error :%d \n",__LINE__);
	    }

	    if (m_pStream2) {
	        HRESULT hr = m_pStream2->Present(pBuffArr, strides, 0);
	        if(FAILED(hr))
	        	LOGE("error :%d \n",__LINE__);
	    }

	    if (m_pPresenterAdv) {
	        HRESULT hr = m_pPresenterAdv->Render(0);
	        if (hr == VR_FAIL_CANVASCHANGED) {
	            m_pPresenterAdv->Get(IID_IMcVideoPresenter, MC_PROPID_VP_CANVAS_INFO,
	                                 0, 0, &m_canvasInfo, sizeof(m_canvasInfo),0);
	        	LOGE("cavas changed to %d,%d \n",m_canvasInfo.dwWidth, m_canvasInfo.dwHeight);

	            RECT dstRect1;
	            SetRect(&dstRect1, 0, 0, m_canvasInfo.dwWidth, m_canvasInfo.dwHeight);
	            m_pStream1->Set(IID_IMcVideoPresenter, MC_PROPID_VP_RECTDST, 0, 0, &dstRect1, sizeof(dstRect1));
	            RECT dstRect2;
	            //SetRect(&dstRect2, 0, 0, m_canvasInfo.dwWidth/2, m_canvasInfo.dwHeight/2);
	            SetRect(&dstRect2, 0, 0, m_canvasInfo.dwWidth/2, m_canvasInfo.dwHeight/2);
	            if(m_pStream2)
	            m_pStream2->Set(IID_IMcVideoPresenter, MC_PROPID_VP_RECTDST, 0, 0, &dstRect2, sizeof(dstRect2));

	        }
	        if(FAILED(hr))
	        	LOGE("error :%d \n",__LINE__);
	        jCallbackFun.env->CallIntMethod(jCallbackFun.obj, jCallbackFun.callbackFuns,m_pPresenterAdv);

	    }
		DWORD rendertime = GetTickCount() -tick;
		if(rendertime<1000/30)
			usleep((1000/30 - rendertime)*1000);
	}
//	m_pPresenterAdv->Set(IID_IMcVideoPresenter, MC_PROPID_VP_ANDROID_JVM_DETACH, 0, 0, &jCallbackFun, sizeof(jCallbackFun));
	jCallbackFun.jvm->DetachCurrentThread();

	return 0;
}


JNIEXPORT void JNICALL Java_com_example_com_android_vrpresenttest_GL2JNILib_init
  (JNIEnv * env, jobject obj, jint width, jint height)
{

	gstate = E_EXIT;
	if(WaitForSingleObject(h_avengine, 3000) == WAIT_TIMEOUT)
	{
		TerminateThread(h_avengine,0);
		CloseHandle(h_avengine);
	}
	m_android_param.scnWidth = (DWORD)width;
	m_android_param.scnHeight = (DWORD)height;


    if(m_pStream2)
    {
    	m_pStream2->Close();
    	m_pStream2->Release();
    	m_pStream2 = 0;
    }
    if(m_pStream1)
    {
    	m_pStream1->Close();
    	m_pPresenterAdv->DestroyStream(m_pStream1);
    	m_pStream1 = 0;
    }
	if(m_pPresenterAdv)
	{
		m_pPresenterAdv->Release();
		m_pPresenterAdv = 0;
	}

	if(m_pPresenterAdv == 0)
	{
		HMODULE h_dll = LoadLibrary("/data/data/com.example.com.android.vrpresenttest/lib/libvrpresent.so");
		fpCreateVideoPresenterAdv fpCreate = (fpCreateVideoPresenterAdv)GetProcAddress(h_dll, ENTRY_FUNC_NAME_V2);
	    m_pPresenterAdv = fpCreate();
	    VP_OpenOptionsAdv option;
	    memset(&option, 0, sizeof(option));
	    m_pPresenterAdv->Open(&option, sizeof(option));

	    m_pPresenterAdv->Get(IID_IMcVideoPresenter, MC_PROPID_VP_CANVAS_INFO,
	                         0, 0, &m_canvasInfo, sizeof(m_canvasInfo),0);


	    TCHAR path[256] = _T("/data/data/com.example.com.android.vrpresenttest/lib/");
	    m_pPresenterAdv->Set(IID_IMcVideoPresenter,
	    		MC_PROPID_VP_DLL_DIRECTORY, 0, 0,path,0);


	    bool bEnable = true;
	    VP_ZOOM_MODE mode = VP_ZOOM_MODE_LETTERBOX;

	    VP_OPEN_STREAM_PARAMS stream_parms;
	    memset(&stream_parms, 0, sizeof(stream_parms));
	    stream_parms.type = eMAINSTREAM;
	    stream_parms.dwWidth = surfwidth;
	    stream_parms.dwHeight = surfheight;
	    stream_parms.dwFourCC = 'Y'|'V'<<8|'1'<<16|'2'<<24;
	    stream_parms.fAlpha = 1.0f;
	//        stream_parms.dwAspectX = 25;
	//        stream_parms.dwAspectY = 9;
	    m_pPresenterAdv->CreateStream(&stream_parms, sizeof(stream_parms), &m_pStream1);
	    m_pStream1->Open();

	    RECT dstRect1;
	    SetRect(&dstRect1, 10, 0, m_canvasInfo.dwWidth-10, m_canvasInfo.dwHeight-10);
	    m_pStream1->Set(IID_IMcVideoPresenter, MC_PROPID_VP_RECTDST, 0, 0, &dstRect1, sizeof(dstRect1));

	    m_pStream1->Set(IID_IMcVideoPresenter, MC_PROPID_VP_ENABLE, 0, 0, &bEnable, sizeof(bEnable));
	    m_pStream1->Set(IID_IMcVideoPresenter, MC_PROPID_VP_ZOOM, NULL, 0,&mode,sizeof(mode));

//	    stream_parms.type = eSUBSTREAMS;
//	    m_pPresenterAdv->CreateStream(&stream_parms, sizeof(stream_parms), &m_pStream2);
	    if(m_pStream2)
	    {
	    	 m_pStream2->Open();
	    	    RECT dstRect2;
	    	    SetRect(&dstRect2, 20, 20, m_canvasInfo.dwWidth/2, m_canvasInfo.dwHeight/2);
	    	    m_pStream2->Set(IID_IMcVideoPresenter, MC_PROPID_VP_RECTDST, 0, 0, &dstRect2, sizeof(dstRect2));
	    	    m_pStream2->Set(IID_IMcVideoPresenter, MC_PROPID_VP_ENABLE, 0, 0, &bEnable, sizeof(bEnable));

	    	    DWORD colorkey = 0x0;
	    	//        m_pStream1->Set(IID_IMcVideoPresenter, MC_PROPID_VP_COLORKEYSRC, 0, 0, &colorkey, sizeof(colorkey));
	    	    m_pStream2->Set(IID_IMcVideoPresenter, MC_PROPID_VP_COLORKEYSRC, 0, 0, &colorkey, sizeof(colorkey));

	    	    mode = VP_ZOOM_MODE_STRETCH;
	    	      m_pStream2->Set(IID_IMcVideoPresenter, MC_PROPID_VP_ZOOM, 0, 0,&mode,sizeof(mode));
	    }



	//        DWORD alpha = MAX_ALPHA_VAL/2;
	//        m_pStream2->Set(IID_IMcVideoPresenter, MC_PROPID_VP_ALPHA, NULL, NULL,&alpha,sizeof(alpha));

	    if(m_pbuff == NULL)
	    {
	        m_pbuff = (unsigned char*)malloc(surfwidth*surfheight*3/2);
	        if(m_pbuff == NULL)
	            LOGE("error :%d \n",__LINE__);

	    }

	    strcpy(filelocation,"/sdcard/test.yuv");
	    if(mFile == 0)
	    {
	        mFile = fopen(filelocation, "r");
	       if(mFile == 0)
	       {
	    	   LOGE("OPEN FILE %s failed \n", filelocation);
	           return;
	       }
	        fseek(mFile, 0, SEEK_END);
	        mFileSize = ftell(mFile);

	    }

	    m_pPresenterAdv->Set(IID_IMcVideoPresenter,
	                    MC_PROPID_VP_RECT_SCREEN, 0, 0,&m_android_param,0);

	    if(jCallbackFun.jvm == 0)
	    	env->GetJavaVM(&jCallbackFun.jvm);

	    /*
	    if(jCallbackFun.obj)
	    {
	    	env->DeleteGlobalRef(jCallbackFun.obj);
	    }
	    */
	    jCallbackFun.obj = env->NewGlobalRef(obj);


		//if(h_avengine == 0)
		{
			gstate = E_INIT;
			h_avengine = CreateThread(0,0,AVENGINE_Thread,0,0,0);
		}


	}
}

JNIEXPORT void JNICALL Java_com_example_com_android_vrpresenttest_GL2JNILib_step
  (JNIEnv * env, jobject obj,jint presenter)
{
	IMcVideoPresenterAdv* pThisPresenter = (IMcVideoPresenterAdv*) presenter;

	if(pThisPresenter)
		pThisPresenter->Set(IID_IMcVideoPresenter, MC_PROPID_VP_FLIP, 0, 0, 0, 0);


}
