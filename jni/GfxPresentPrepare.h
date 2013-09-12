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

#ifndef _GFXPREPARE_PREPARE_H_
#define _GFXPREPARE_PREPARE_H_
#include "../../inc/GfxContext.h"

#ifdef GFX_ANDROID_OPENGL_ES_SHADER
#include "ci_codec_type.h"
#include "ci_imageproc.h"
#else
#include "../../3rdparty/imglib/ci_codec_type.h"
#include "../../3rdparty/imglib/ci_imageproc.h"
#endif

#ifndef STATIC_IMGPROC_LIB
#include "Civiproc.h"
#endif

#ifndef EXPORT
#define EXPORT extern "C" _declspec(dllexport)
#endif

//#define PXA310
//#define DUMPDATA

#ifdef PXA310
#define USE_IPP_ROTATION
#endif

enum
{
	MC_PROPID_GFXPREPARE_VIDEOEFFECT,
	MC_PROPID_GFXPREPARE_COLORKEY
};

class GfxPrepareDataV2
{
public:
	GfxPrepareDataV2();
	virtual ~GfxPrepareDataV2();
	
	HRESULT PrepareBufferYUY2(DWORD dwInputType,LPBYTE pBuf, 
		LONG dstStride, LPBYTE *ppbPlanes, DWORD *pSrcStrides, DWORD *pdwWidth, 
		DWORD *pdwHeight, DWORD dwFlags,LONG lRotation);

	HRESULT PrepareBufferYV12(LPBYTE pBuf, LONG dstStride, LPBYTE *ppbPlanes, 
		DWORD *pSrcStrides_arr, DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr, 
		DWORD dwFlags ,LONG lRotation, BOOL bScale, RECT* dstRect,
		VP_DEINTERLACE_MODE mode);

	HRESULT PrepareBufferYV12_Test( LPBYTE *pSrcPlanes_arr, LPBYTE* pDstPlanes_arr,
		DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr,
		 DWORD *pSrcWidthStride_arr, DWORD* pDstWidthStride_arr,
		DWORD dwFlags ,LONG lRotation,
		VP_DEINTERLACE_MODE mode);

	HRESULT PrepareBufferYV12Scaled(unsigned char *pSrc[3], int srcStride[3], 
		CI_IMAGEPROC_SIZE srcSize, unsigned char *pDes[3], int desStride[3], 
		CI_IMAGEPROC_SIZE desSize, CI_IMAGEPROC_ROTATION rotationtype);

	HRESULT PrepareBufferNV12(LPBYTE pBuf, LONG lStride, LPBYTE *ppbPlanes, DWORD *pdwStrides, DWORD *pdwWidth, DWORD *pdwHeight, DWORD dwFlags, DWORD* pdwBufStrides);

	HRESULT PrepareBufferYU24(LPBYTE pBuf, LONG lStride, LPBYTE *ppbPlanes, DWORD *pdwStrides, DWORD *pdwWidth, DWORD *pdwHeight, DWORD dwFlags);

	HRESULT PrepareBufferRGB565(LPBYTE pBuf, LONG lStride, LPBYTE *ppbPlanes, 
		DWORD *pdwStrides, DWORD *pdwWidth, DWORD *pdwHeight, DWORD dwFlags,
		LONG lRotation,VP_DEINTERLACE_MODE mode);

	HRESULT PrepareBufferRGB565Scaled(LPBYTE pBuf, LONG dstStride, 
		LPBYTE *ppbPlanes, DWORD *pSrcStrides, RECT *prSrc, RECT *prDst, 
		DWORD dwFlags , LONG lRotation,VP_DEINTERLACE_MODE mode);

	HRESULT PrepareBufferXRGB(LPBYTE pBuf, LONG dstStride, LPBYTE *ppbPlanes, DWORD *pSrcStrides_arr, DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr , DWORD dwFlags, LONG lRotation);

	HRESULT PrepareBufferRGB888(LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 	
		DWORD *pSrcStrides_arr, RECT *prSrc, RECT *prDst, DWORD dwFlags , LONG lRotation);

	HRESULT PrepareBufferR565ToR565Scaled(LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 
		DWORD *pSrcStrides_arr, RECT *prSrc, RECT *prDst, DWORD dwFlags , LONG lRotation);

	HRESULT PrepareBufferYV12ToARGB(LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 
		DWORD *pSrcStrides_arr, RECT *prSrc, RECT *prDst, DWORD dwFlags , LONG lRotation);
	
	HRESULT PrepareBufferYV12ToNV12(LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 
		DWORD *pSrcStrides_arr, DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr, DWORD dwFlags ,LONG lRotation);

HRESULT PrepareBufferNV12ToYV12(LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 
	DWORD *pSrcStrides_arr, DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr, DWORD dwFlags ,LONG lRotation);

	HRESULT Set(DWORD dwPropID, LPVOID pPropData, DWORD cbPropData);

HRESULT PrepareBufferUYVY(DWORD	dwInputType,LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 
	DWORD *pSrcStrides_arr, DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr, DWORD dwFlags,LONG lRotation);

	HRESULT PrepareBufferARGB2YV12(LPBYTE pBuf, LONG dstStride, LPBYTE *ppbPlanes, 
		DWORD *pdwStrides, DWORD *pdwWidth, DWORD *pdwHeight, DWORD dwFlags,
		LONG lRotation,VP_DEINTERLACE_MODE mode);


	HRESULT PrepareBufferRGB5652YV12(LPBYTE pBuf, LONG dstStride, LPBYTE *ppbPlanes, 
		DWORD *pdwStrides, DWORD *pdwWidth, DWORD *pdwHeight, DWORD dwFlags,
		LONG lRotation,VP_DEINTERLACE_MODE mode);

private:
	HRESULT LoadImgProcLibrary();
	HRESULT Init(CI_IMAGEPROC_CREATEOPTION *pOption,DWORD *pSrcStrides_arr = NULL, DWORD *pDstStrides_arr  = NULL);
	HRESULT Uninit();
	HRESULT Process(unsigned char *pSrc[3], CI_U32 srcStride[3], CI_IMAGEPROC_SIZE srcSize, unsigned char *pDst[3],  CI_U32 dstStride[3], CI_IMAGEPROC_SIZE dstSize);
	HRESULT SetDeinterlaceMode(VP_DEINTERLACE_MODE mode);
	CI_IMAGEPROC_CREATEOPTION m_openParams;
	CI_VOID* m_imageHnd; 
	CI_IMAGEPROC_COLOROPTION  m_colorParams;
	CI_IMAGEPROC_PROCESSOPTION m_processParams;
	#ifdef APOLLO_SAMSUNG2450
	char* m_apoloo_inputBuf;
	char* m_apoloo_outputBuf;
	CI_IMAGEPROC_SIZE m_apoloo_lastsrcSize;
	CI_IMAGEPROC_SIZE m_apoloo_lastdstSize;
	#endif

	#ifdef GFX_ANDROID_SF
	void* m_yuvrgb_hnd;
	#endif

#ifdef DUMPDATA
	FILE* pFileDumpInput;
	FILE* pFileDumpOutput;
#endif

#ifdef  LOADLIBRARY_ONCE
	static HMODULE h_dll;
#else
	HMODULE h_dll;
#endif
	CI_IMAGEPROC_FUNCTIONS* m_pFunc;


};

#endif //_VRPRESENT_COMM_H_
