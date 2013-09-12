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
#include "../../inc/GfxBuildDef.h"

#include <stdio.h>
#include <windows.h>
#include "../../inc/GfxPresent.h"
#include "GfxPresentPrepare.h"
#include "../../inc/dp.h"

extern bool hasNeon();

#if defined(PLATOFRM_ANDROID_ACERNETBOOK) || defined(PLATOFRM_DELL_MINI9)
#include "../../3rdparty/yuvrgb/y420rgbAPI.h"
#endif

#ifdef APOLLO_SAMSUNG2450
#include "../../3rdparty/APOLLO_SAMSUNG2450/RTKTunerDll.h"
#pragma comment(lib, "../../3rdparty/APOLLO_SAMSUNG2450/RTKTunerDll.lib")
#endif


#ifdef RENESAS
#define STATIC_IMGPROC_LIB
#pragma comment(lib, "CSCLib.lib")
#pragma comment(lib, "CSCLib_sh4.lib")
#endif

#ifdef GFX_QNX
#define STATIC_IMGPROC_LIB
#endif



#ifdef DUMPDATA
typedef struct {
	unsigned short BMP_id ; // 'B''M'
	unsigned int size; // size in bytes of the BMP file
	unsigned int zero_res; // 0
	unsigned int offbits; // 54
	unsigned int biSize; // 0x28
	unsigned int Width;  // X
	unsigned int Height;  // Y
	unsigned short  biPlanes; // 1
	unsigned short  biBitCount ; // 24
	unsigned int biCompression; // 0 = BI_RGB
	unsigned int biSizeImage; // 0
	unsigned int biXPelsPerMeter; // 0xB40
	unsigned int biYPelsPerMeter; // 0xB40
	unsigned int biClrUsed; //0
	unsigned int biClrImportant; //0
} BMPHeader;

#define RGB565_MASK_RED    0xF800
#define RGB565_MASK_GREEN  0x07E0
#define RGB565_MASK_BLUE   0x001F

void write_bmp_file_888(int width, int height, unsigned char *pRGB)
{
	static int count =0;

	char filename[MAX_PATH];
	sprintf(filename, "/sdcard/%dx%d_%d.bmp",width, height, count++);
	FILE *pf = fopen(filename,"wb");
	int tmp;
	BMPHeader BH;
	BH.BMP_id = 'M'*256+'B';							fwrite(&BH.BMP_id,2,1,pf);
	BH.size=66+height*width*3;							fwrite(&BH.size,4,1,pf);
	BH.zero_res = 0;									fwrite(&BH.zero_res,4,1,pf);
	BH.offbits = 66;									fwrite(&BH.offbits,4,1,pf);
	BH.biSize = 0x28;									fwrite(&BH.biSize,4,1,pf);
	BH.Width = width;									fwrite(&BH.Width,4,1,pf);
	BH.Height = (-1)*height;							fwrite(&BH.Height,4,1,pf);
	BH.biPlanes = 1;									fwrite(&BH.biPlanes,2,1,pf);
	BH.biBitCount = 24;									fwrite(&BH.biBitCount,2,1,pf);
	BH.biCompression = 0;								fwrite(&BH.biCompression,4,1,pf);
	BH.biSizeImage = height*width*3;					fwrite(&BH.biSizeImage,4,1,pf);
	BH.biXPelsPerMeter = 0;								fwrite(&BH.biXPelsPerMeter,4,1,pf);
	BH.biYPelsPerMeter = 0;								fwrite(&BH.biYPelsPerMeter,4,1,pf);
	BH.biClrUsed = 0;									fwrite(&BH.biClrUsed,4,1,pf);
	BH.biClrImportant = 0;								fwrite(&BH.biClrImportant,4,1,pf);
	tmp = RGB565_MASK_RED;								fwrite(&tmp,4,1,pf);
	tmp = RGB565_MASK_GREEN;							fwrite(&tmp,4,1,pf);
	tmp = RGB565_MASK_BLUE;								fwrite(&tmp,4,1,pf);
	fwrite(pRGB, width*height*3, 1, pf);
	fclose(pf);
}
#endif
#ifdef OPENCORE_CC

	#include <cczoomrotation16.h>

	ColorConvertBase * m_iColorConverter = NULL;
	LPBYTE m_iColorDstBuf = NULL;
	
	CI_IMAGEPROC_CREATEOPTION m_opencoreCC_open;
	
	CI_RESULT OPENCORE_IMAGEPROC_Create(
		CI_VOID **pProcessor,
		CI_VOID *pLicense,
		CI_IMAGEPROC_CREATEOPTION *pOption)
	{
		if(!m_iColorConverter)
	    		m_iColorConverter = ColorConvert16::NewL();
		
		m_opencoreCC_open = *pOption;

		if(m_iColorDstBuf)
		{
			free(m_iColorDstBuf);
			m_iColorDstBuf = 0;			
		}

		m_iColorDstBuf = (LPBYTE)malloc(m_opencoreCC_open.u32SrcWidth*m_opencoreCC_open.u32SrcHeight*3/2);
		if(!m_iColorDstBuf)	
		{
			return E_FAIL;
		}
		
		m_iColorConverter->Init(m_opencoreCC_open.u32SrcWidth, m_opencoreCC_open.u32SrcHeight, m_opencoreCC_open.u32SrcWidth, 
		m_opencoreCC_open.u32SrcWidth, m_opencoreCC_open.u32SrcHeight, m_opencoreCC_open.u32SrcWidth, CCROTATE_NONE);
		m_iColorConverter->SetMemHeight(m_opencoreCC_open.u32SrcHeight);
		m_iColorConverter->SetMode(1);
		return S_OK;
	}

	CI_RESULT OPENCORE_IMAGEPROC_Destroy(
		CI_VOID *pProcessor)
	{
		if(m_iColorDstBuf)
			free(m_iColorDstBuf);

		if (m_iColorConverter != NULL)
		{
			delete m_iColorConverter;
			m_iColorConverter = NULL;
		}
		return S_OK;
	}

	CI_RESULT OPENCORE_IMAGEPROC_ProcessFrame(
		CI_VOID *pProcessor,
		CI_U8 *pSrc[3],
		CI_U32 srcStride[3],
		CI_U8 *pDst[3],
		CI_U32 dstStride[3],
		CI_IMAGEPROC_PROCESSOPTION *pOption)
	{
//Remove stride, because opencore  CC doesn't support stride.
		LPBYTE pYBuf = m_iColorDstBuf ;
		LPBYTE pUBuf = m_iColorDstBuf + m_opencoreCC_open.u32SrcWidth*m_opencoreCC_open.u32SrcHeight;
		LPBYTE pVBuf = m_iColorDstBuf + m_opencoreCC_open.u32SrcWidth*m_opencoreCC_open.u32SrcHeight*5/4 ;
		for(unsigned int i=0;i<m_opencoreCC_open.u32SrcHeight;i++)
		{
			memcpy(pYBuf + i*m_opencoreCC_open.u32SrcWidth, pSrc[0]+ i*srcStride[0], 
				m_opencoreCC_open.u32SrcWidth );
		}
		
		for(unsigned int i=0;i<m_opencoreCC_open.u32SrcHeight/2;i++)
		{
			memcpy(pUBuf + i*m_opencoreCC_open.u32SrcWidth/2, pSrc[1] + i*srcStride[1], 
				m_opencoreCC_open.u32SrcWidth/2);
		}
		
		for(unsigned int i=0;i<m_opencoreCC_open.u32SrcHeight/2;i++)
		{
			memcpy(pVBuf + i*m_opencoreCC_open.u32SrcWidth/2, pSrc[2] + i*srcStride[2], 
				m_opencoreCC_open.u32SrcWidth/2);
		}
		
		m_iColorConverter->Convert(m_iColorDstBuf,pDst[0]);
		return S_OK;
	}

	CI_RESULT OPENCORE_IMAGEPROC_Set(
		IN OUT CI_VOID *pProcessor,
		IN CI_U32 u32PropId,
		IN CONST CI_VOID *pData,
		IN CI_U32 u32DataLen)
	{
		return S_OK;
	}


	CI_RESULT OPENCORE_IMAGEPROC_Get(
		IN OUT CI_VOID *pProcessor,
		IN CI_U32 u32PropId,
		OUT CI_VOID *pData,
		IN CI_U32 u32DataLen,
		OUT CI_U32 *pu32DataUsed)
	{
		return S_OK;
	}
#endif	

static void rgb565copybuf(unsigned char *pDst, unsigned char *pSrc, int iDstStride, int iSrcStride, int iWidth, int iHeight)
{
	int i;
	for(i =0;i<iHeight;i++)
	{
		memcpy(pDst,pSrc, iWidth<<1);
		pDst += iDstStride;
		pSrc += iSrcStride;
	}

	return;
}


static void avgcpy(LPBYTE dst, LPBYTE src1, LPBYTE src2, int num)
{
	DWORD *wd,*ws1,*ws2,s1,s2;

	wd = reinterpret_cast<DWORD *>(dst);
	ws1 = reinterpret_cast<DWORD *>(src1);
	ws2 = reinterpret_cast<DWORD *>(src2);
	num>>=2;
	do
	{
		s1 = *ws1++;
		s2 = *ws2++;
		s2 ^= s1;
		s1 |= s2;
		s2 &= 0xfefefefe;
		*wd++ = s1 - (s2>>1);
	}
	while(--num);
}


static const DWORD s1_matrix[4] = 
{
	0x01030002,
	0x00020103,
	0x03000201,
	0x02010300
};


static const DWORD s2_matrix[4] = 
{
	0x02060105,
	0x00040307,
	0x07010402,
	0x05030600
};

static inline void linecpy2_s0(LPBYTE dst1,LPBYTE src1, int num, int i, int lpad, int rpad, int fill)
{
	int j;
	for(j=0;j<lpad;j+=4)
	{
		*((DWORD *)dst1) = fill;
		dst1 += 4;
	}
	memcpy(dst1,src1,num);
	dst1 += num;
	for(j=0;j<rpad;j+=4)
	{
		*((DWORD *)dst1) = fill;
		dst1 += 4;
	}
}


static inline void avgcpy2_s0(LPBYTE dst1, LPBYTE dst2, LPBYTE src1, LPBYTE src2, int num, int i, int lpad, int rpad, int fill)
{
	int j;
	for(j=0;j<lpad;j+=4)
	{
		*((DWORD *)dst1) = fill;
		*((DWORD *)dst2) = fill;
		dst1 += 4;
		dst2 += 4;
	}
	memcpy(dst1,src1,num);
	avgcpy(dst2,src1,src2,num);
	dst1 += num;
	dst2 += num;
	for(j=0;j<rpad;j+=4)
	{
		*((DWORD *)dst1) = fill;
		*((DWORD *)dst2) = fill;
		dst1 += 4;
		dst2 += 4;
	}
}


extern "C" void avg_s0(unsigned char * pDst, unsigned char * pSrc1, unsigned char * pSrc2, unsigned int width, unsigned int lstride, unsigned int rstride, unsigned int fill);
extern "C" void avg_s1(unsigned char * pDst1, unsigned char * pDst2, unsigned char * pSrc1, unsigned char * pSrc2, unsigned int width, unsigned int d1, unsigned int d2, unsigned int lstride, unsigned int rstride, unsigned int fill);
extern "C" void avg_s2(unsigned char * pDst1, unsigned char * pDst2, unsigned char * pSrc1, unsigned char * pSrc2, unsigned int width, unsigned int d1, unsigned int d2, unsigned int lstride, unsigned int rstride, unsigned int fill);
extern "C" void cpy_s1(unsigned char * pDst1, unsigned char * pSrc1, unsigned int num, unsigned int dither, unsigned int lstride, unsigned int rstride, unsigned int fill);
extern "C" void cpy_s2(unsigned char * pDst1, unsigned char * pSrc1, unsigned int num, unsigned int dither, unsigned int lstride, unsigned int rstride, unsigned int fill);

static inline void linecpy2_s1(LPBYTE dst1, LPBYTE src1, int num, int i, int lpad, int rpad, int fill)
{
#ifdef TITAN
	cpy_s1(dst1,src1,num,s1_matrix[i&3],lpad,rpad,fill);
#else
	linecpy2_s0(dst1,src1,num,i,lpad,rpad,fill);
#endif
}

static inline void linecpy2_s2(LPBYTE dst1, LPBYTE src1, int num, int i, int lpad, int rpad, int fill)
{
#ifdef TITAN
	cpy_s2(dst1,src1,num,s2_matrix[i&3],lpad,rpad,fill);
#else
	linecpy2_s0(dst1,src1,num,i,lpad,rpad,fill);
#endif
}
static inline void avgcpy2_s1(LPBYTE dst1, LPBYTE dst2, LPBYTE src1, LPBYTE src2, int num, int i, int lpad, int rpad, int fill)
{
#ifdef TITAN
	avg_s1(dst1,dst2,src1,src2,num,s1_matrix[i&3],s1_matrix[i+1&3],lpad,rpad,fill);
#else
	avgcpy2_s0(dst1,dst2,src1,src2,num,i,lpad,rpad,fill);
#endif
}

static inline void avgcpy2_s2(LPBYTE dst1, LPBYTE dst2, LPBYTE src1, LPBYTE src2, int num, int i, int lpad, int rpad, int fill)
{
#ifdef TITAN
	avg_s2(dst1,dst2,src1,src2,num,s2_matrix[i&3],s2_matrix[i+1&3],lpad,rpad,fill);
#else
	avgcpy2_s0(dst1,dst2,src1,src2,num,i,lpad,rpad,fill);
#endif
}

static void copybuf(unsigned char *dst, unsigned char *src, int d_stride, int s_stride, int width, int height)
{
	int i;
	for(i=0;i<height;i++)
	{
		memcpy(dst,src,width);
		dst += d_stride;
		src += s_stride;
	}
}

static void cpybuf_s0(unsigned char *dst, unsigned char *src, int d_stride, int s_stride, int width, int height, int fill)
{
	int i,lpad,rpad;

	rpad = d_stride - width;
	lpad = 0;
	for(i=0;i<height;i++)
	{
		linecpy2_s0(dst,src,width,i,lpad,rpad,fill);
		dst += d_stride;
		src += s_stride;
	}
}

static void cpybuf_s1(unsigned char *dst, unsigned char *src, int d_stride, int s_stride, int width, int height, int fill)
{
	int i,lpad,rpad;

	rpad = d_stride - width;
	lpad = 0;
	for(i=0;i<height;i++)
	{
		linecpy2_s1(dst,src,width,i,lpad,rpad,fill);
		dst += d_stride;
		src += s_stride;
	}
}

static void cpybuf_s2(unsigned char *dst, unsigned char *src, int d_stride, int s_stride, int width, int height, int fill)
{
	int i,lpad,rpad;

	rpad = d_stride - width;
	lpad = 0;
	for(i=0;i<height;i++)
	{
		linecpy2_s2(dst,src,width,i,lpad,rpad,fill);
		dst += d_stride;
		src += s_stride;
	}
}

// this expands the height by 2
static void avgbufh2_s0(unsigned char *dst, unsigned char *src, int d_stride, int s_stride, int width, int height, int fill)
{
	int i,lpad,rpad;

	rpad = d_stride - width;
	lpad = 0;
	for(i=0;i<height-1;i++)
	{
		avgcpy2_s0(dst,dst+d_stride,src,src+s_stride,width,i,lpad,rpad,fill);
		dst += d_stride<<1;
		src += s_stride;
	}
	if(height)
		avgcpy2_s0(dst,dst+d_stride,src,src,width,i,lpad,rpad,fill);
}

// this expands the height by 2
static void avgbufh2_s1(unsigned char *dst, unsigned char *src, int d_stride, int s_stride, int width, int height, int fill)
{
	int i,lpad,rpad;

	rpad = d_stride - width;
	lpad = 0;
	for(i=0;i<height-1;i++)
	{
		avgcpy2_s1(dst,dst+d_stride,src,src+s_stride,width,i,lpad,rpad,fill);
		dst += d_stride<<1;
		src += s_stride;
	}
	if(height)
		avgcpy2_s1(dst,dst+d_stride,src,src,width,i,lpad,rpad,fill);
}

// this expands the height by 2
static void avgbufh2_s2(unsigned char *dst, unsigned char *src, int d_stride, int s_stride, int width, int height, int fill)
{
	int i,lpad,rpad;

	rpad = d_stride - width;
	lpad = 0;
	for(i=0;i<height-1;i++)
	{
		avgcpy2_s2(dst,dst+d_stride,src,src+s_stride,width,i,lpad,rpad,fill);
		dst += d_stride<<1;
		src += s_stride;
	}
	if(height)
		avgcpy2_s2(dst,dst+d_stride,src,src,width,i,lpad,rpad,fill);
}

static void avgbufo_s0(unsigned char *dst, unsigned char *src, int d_stride, int s_stride, int width, int height, int fill)
{
	if(height==0)
		return;
	if(height>>1)
		avgbufh2_s0(dst,src,d_stride,s_stride<<1,width,height>>1,fill);
	if(height&1)	// odd height (will be missed with the above code)
	{
		int lpad,rpad;

		rpad = d_stride - width;
		lpad = 0;
		linecpy2_s0(dst+d_stride*(height-1),src+s_stride*(height-1),width,0,lpad,rpad,fill);
	}
}

static void avgbufe_s0(unsigned char *dst, unsigned char *src, int d_stride, int s_stride, int width, int height, int fill)
{
	int i,lpad,rpad;

	if(height==0)
		return;
	rpad = d_stride - width;
	lpad = 0;
	linecpy2_s0(dst,src+s_stride,width,0,lpad,rpad,fill);
	i = (height-1)>>1;
	if(i>0)
		avgbufh2_s0(dst+d_stride,src+s_stride,d_stride,s_stride<<1,width,i,fill);
	i = (i<<1) + 1;
	if(i<height)
	{	// i will be even (will be missed with the above code)
		linecpy2_s0(dst+d_stride*i,src+s_stride*i,width,0,lpad,rpad,fill);
	}
}

#ifdef LOADLIBRARY_ONCE
HMODULE GfxPrepareDataV2::h_dll = 0;
#endif

GfxPrepareDataV2::GfxPrepareDataV2() :
	m_imageHnd(NULL)
{
	ZeroMemory(&m_openParams, sizeof(m_openParams));
	m_openParams.u32Size = sizeof(m_openParams);
	
	ZeroMemory(&m_colorParams, sizeof(m_colorParams));
	m_colorParams.s32Brightness = m_colorParams.s32Contrast = m_colorParams.s32Saturation = 128;

	ZeroMemory(&m_processParams, sizeof(m_processParams));
	m_processParams.u32Size = sizeof(m_processParams);
	m_processParams.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;
	
	
	#ifdef APOLLO_SAMSUNG2450
	m_apoloo_inputBuf = NULL;
	m_apoloo_outputBuf = NULL;
	ZeroMemory(&m_apoloo_lastsrcSize, sizeof(m_apoloo_lastsrcSize));
	ZeroMemory(&m_apoloo_lastdstSize, sizeof(m_apoloo_lastdstSize));
	#endif
	
	#if defined(PLATOFRM_ANDROID_ACERNETBOOK) ||  defined(PLATOFRM_DELL_MINI9)
	m_yuvrgb_hnd = NULL;
	#endif

#ifndef  LOADLIBRARY_ONCE
	h_dll = 0;
#endif
	m_pFunc = NULL;
}

GfxPrepareDataV2::~GfxPrepareDataV2()
{
	Uninit();
#ifdef STATIC_IMGPROC_LIB
	if(m_pFunc)
		free(m_pFunc);
	m_pFunc = NULL;
#else

	#ifndef  LOADLIBRARY_ONCE
	FreeLibrary(h_dll);
	h_dll = 0;
	#endif

#endif
}


HRESULT GfxPrepareDataV2::LoadImgProcLibrary()
{

#ifdef STATIC_IMGPROC_LIB
	if(!m_pFunc)
		m_pFunc = (CI_IMAGEPROC_FUNCTIONS*)malloc(sizeof(CI_IMAGEPROC_FUNCTIONS));
	m_pFunc->Create = CI_IMAGEPROC_Create;
	m_pFunc->Destroy = CI_IMAGEPROC_Destroy;
	m_pFunc->ProcessFrame = CI_IMAGEPROC_ProcessFrame;
	m_pFunc->Set = CI_IMAGEPROC_Set;
	m_pFunc->Get = CI_IMAGEPROC_Get;

#elif OPENCORE_CC
	if(!m_pFunc)
		m_pFunc = (CI_IMAGEPROC_FUNCTIONS*)malloc(sizeof(CI_IMAGEPROC_FUNCTIONS));
	
	m_pFunc->Create = OPENCORE_IMAGEPROC_Create;
	m_pFunc->Destroy = OPENCORE_IMAGEPROC_Destroy;
	m_pFunc->ProcessFrame = OPENCORE_IMAGEPROC_ProcessFrame;
	m_pFunc->Set = OPENCORE_IMAGEPROC_Set;
	m_pFunc->Get = OPENCORE_IMAGEPROC_Get;
#else
	#if defined(__linux__) && !defined(__QNX__)
	TCHAR dllpath[256];
	memset(dllpath,0,sizeof(dllpath));
	GetDllDirectory(sizeof(dllpath)/sizeof(char), dllpath);
	if(hasNeon())
		strcat(dllpath, _T("libciviproc_arm.so"));
	else
		strcat(dllpath, _T("libciviproc_neon.so"));
	if(h_dll == 0)
		h_dll = LoadLibrary(dllpath);

//		h_dll = LoadLibrary(_T("/data/data/com.example.com.android.vrpresenttest/lib/libciviproc_arm.so"));
#endif
	static bool bprinted = false;


	if(!h_dll)
	{
		if(bprinted ==false)
		{
				bprinted = true;
				DP("%s is not found \n",dllpath);
		}
		return E_FAIL;
	}

	if(bprinted ==false)
	{
			bprinted = true;
			DP("%s found \n",dllpath);
	}

	LPFN_GetDllInterface fpgetInterface = NULL;

#ifdef _WIN32_WCE
	fpgetInterface = (LPFN_GetDllInterface)GetProcAddress(h_dll, _T("GetDllInterface"));
#else
	fpgetInterface = (LPFN_GetDllInterface)GetProcAddress(h_dll, "GetDllInterface");
#endif
	if(fpgetInterface==0)
	{
		DP("Entry function is not found \n");
		return E_FAIL;
	}
	HRESULT ret = fpgetInterface(IID_CI_IMAGEPROC_C,(void**)&m_pFunc/*pFunc*/);
	if(FAILED(ret))
	{
		DP("[IMGLIB]fpgetInterface is not found \n");
		return ret;
	}
#endif

	if(!m_pFunc->Create || !m_pFunc->Destroy || !m_pFunc->ProcessFrame || !m_pFunc->Set || !m_pFunc->Get)
	{
		DP("[IMGLIB] incomplete \n");
		return E_FAIL;
	}

	return S_OK;	
}

HRESULT GfxPrepareDataV2::Init(CI_IMAGEPROC_CREATEOPTION *pOption,DWORD *pSrcStrides_arr , DWORD *pDstStrides_arr  )
{
	if(FAILED(LoadImgProcLibrary()))
		return E_FAIL;

	if(pOption && (memcmp(&m_openParams,pOption,sizeof(m_openParams)) == 0) )
		return S_OK;
/*
  DP("m_openParams: (%d,%d) ->(%d,%d) u32Interpolation:%d\n",m_openParams.u32SrcWidth,m_openParams.u32SrcHeight,
    m_openParams.u32DstWidth,m_openParams.u32DstHeight,m_openParams.u32Interpolation);
  DP("pOption: (%d,%d) ->(%d,%d) u32Interpolation:%d\n",pOption->u32SrcWidth,pOption->u32SrcHeight,
    pOption->u32DstWidth,pOption->u32DstHeight,pOption->u32Interpolation);

  switch(pOption->u32ColorSpaceConversion)
  {
  	case CI_IMAGEPROC_CSC_YCBCR420_TO_RGB565: 
      DP("CI_IMAGEPROC_CSC_YCBCR420_TO_RGB565 \n");
    	break;
	case CI_IMAGEPROC_CSC_YCBCR420_TO_ARGB32:
      DP("CI_IMAGEPROC_CSC_YCBCR420_TO_ARGB32\n");
    	break;
	case CI_IMAGEPROC_CSC_YCBCR420_TO_YCBCR420:
      DP("CI_IMAGEPROC_CSC_YCBCR420_TO_YCBCR420\n");
    	break;
	case CI_IMAGEPROC_CSC_RGB565_TO_ARGB32:
      DP("CI_IMAGEPROC_CSC_RGB565_TO_ARGB32\n");
    	break;
	case CI_IMAGEPROC_CSC_YCBCR444_TO_RGB565:
      DP("CI_IMAGEPROC_CSC_YCBCR444_TO_RGB565\n");
    	break;
	case CI_IMAGEPROC_CSC_YCBCR420_TO_YUY2:
      DP("CI_IMAGEPROC_CSC_YCBCR420_TO_YUY2\n");
    	break;
	case CI_IMAGEPROC_CSC_YCBCR420_TO_UYVY:
      DP("CI_IMAGEPROC_CSC_YCBCR420_TO_UYVY\n");
    	break;
	case CI_IMAGEPROC_CSC_RGB565_TO_RGB565:
      DP("CI_IMAGEPROC_CSC_RGB565_TO_RGB565\n");
    	break;
	case CI_IMAGEPROC_CSC_RGB888_TO_RGB565 :
      DP("CI_IMAGEPROC_CSC_RGB888_TO_RGB565 \n");
    	break;
	case CI_IMAGEPROC_CSC_YCBCR420_TO_RGB888:
      DP("CI_IMAGEPROC_CSC_YCBCR420_TO_RGB888\n");
    	break;
	case CI_IMAGEPROC_CSC_YCBCR422_TO_RGB565:
      DP("CI_IMAGEPROC_CSC_YCBCR422_TO_RGB565\n");
    	break;
	case CI_IMAGEPROC_CSC_YCBCR420_TO_NV12:
      DP("CI_IMAGEPROC_CSC_YCBCR420_TO_NV12\n");
    	break;
  default:
    break;
  }
*/

	Uninit();

	memcpy(&m_openParams, pOption, sizeof(CI_IMAGEPROC_CREATEOPTION));

#ifdef DUMPDATA
	char filename[MAX_PATH];
	sprintf(filename, "/SDMMC/DumpInput_%dx%d.yuv",m_openParams.u32SrcWidth, m_openParams.u32SrcHeight);
    DP("DUMP input DATA to %s \n",filename);
	pFileDumpInput = fopen(filename,"wb");
	sprintf(filename, "/SDMMC/DumpOutput_%dx%d.yuv",m_openParams.u32DstWidth, m_openParams.u32DstHeight);
    DP("DUMP output DATA to %s \n",filename);
	pFileDumpOutput = fopen(filename,"wb");
#endif


#if defined(PLATOFRM_ANDROID_ACERNETBOOK) || defined(PLATOFRM_DELL_MINI9)

	CC_RECT src,dst,dstStride;
	int bpp = 0;
	if(m_convertion == CICscYCbCr420ToRGB565)
		bpp = 2;
	else if(m_convertion == CICscYCbCr420ToAlphaRGB)
		bpp = 4;

	src.left = 0;
	src.top = 0;
	src.right = srcSize.width;
	src.bottom = srcSize.height;
	if((pDstStrides == NULL)||(pSrcStrides == NULL)) 
	{
		return E_FAIL;
	}
	dst.left = 0;
	dst.top = 0;
	dst.right = *pDstStrides/bpp ;
	dst.bottom = dstSize.height;


	dstStride.left = 0;
	dstStride.top = 0;
	dstStride.right = dstSize.width;
	dstStride.bottom = dstSize.height;


	if(I420_RGB_Init(&m_yuvrgb_hnd,&src, (int*)pSrcStrides, &dst, &dstStride, bpp) == 0)
		return S_OK;
	else
		return E_FAIL;
#else
	HRESULT ret =m_pFunc->Create(&m_imageHnd, NULL, &m_openParams);
	if(FAILED(ret))
	{
		DP("[GfxPrepareData]Init failed %d \n",ret);
		return ret;
	}

	#ifndef ANDROID
	 ret = m_pFunc->Set(
		m_imageHnd,
		CI_IMAGEPROC_PROPID_COLOROPTION,
		&m_colorParams,
		sizeof(m_colorParams));
	if(FAILED(ret))
	{
		DP("[GfxPrepareData]Set failed %d \n",ret);
		return ret;
	}
	#endif

#endif

#ifdef APOLLO_SAMSUNG2450
	m_apoloo_inputBuf = NULL;
	m_apoloo_outputBuf = NULL;
	ZeroMemory(&m_apoloo_lastsrcSize, sizeof(m_apoloo_lastsrcSize));
	ZeroMemory(&m_apoloo_lastdstSize, sizeof(m_apoloo_lastdstSize));
	init_buffer(&m_apoloo_inputBuf , &m_apoloo_outputBuf);
#endif
	return (S_OK);
}

HRESULT GfxPrepareDataV2::Uninit()
{
#if defined(PLATOFRM_ANDROID_ACERNETBOOK) || defined(PLATOFRM_DELL_MINI9)
	if(m_yuvrgb_hnd == NULL)
		return S_OK;
	I420_RGB_unInit(m_yuvrgb_hnd);
	m_yuvrgb_hnd = NULL;
	return S_OK;
#else
	if(m_imageHnd == NULL)
		return S_OK;

	HRESULT ret = m_pFunc->Destroy(m_imageHnd);
	if(FAILED(ret))
	{
		DP("[GfxPrepareData]UnInit failed %d \n",ret);
		return ret;
	}
	m_imageHnd = NULL;

	ZeroMemory(&m_openParams, sizeof(m_openParams));
	m_openParams.u32Size = sizeof(m_openParams);


#ifdef DUMPDATA
	if(pFileDumpInput)
	{
		fclose(pFileDumpInput);
		pFileDumpInput = NULL;
	}
	if(pFileDumpOutput)
	{
		fclose(pFileDumpOutput);
		pFileDumpOutput= NULL;
	}
#endif

	return ret;
#endif
}

HRESULT GfxPrepareDataV2::Set(DWORD dwPropID, LPVOID pPropData, DWORD cbPropData) 
{
#ifdef ANDROID
	return S_OK;
#endif
	switch(dwPropID)
	{

		case MC_PROPID_GFXPREPARE_VIDEOEFFECT:
			{
				if(!m_imageHnd)
					return E_FAIL;
				m_colorParams = *reinterpret_cast<CI_IMAGEPROC_COLOROPTION*>(pPropData);
				return (m_pFunc->Set(	m_imageHnd,	CI_IMAGEPROC_PROPID_COLOROPTION,
					&m_colorParams,sizeof(m_colorParams)));
			}
		default:
			return E_FAIL;
		
	}
	return S_OK;
}

HRESULT GfxPrepareDataV2::Process(unsigned char *pSrc[3], CI_U32 srcStride[3], 
	CI_IMAGEPROC_SIZE srcSize,  unsigned char *pDst[3],  CI_U32 dstStride[3], 
	CI_IMAGEPROC_SIZE dstSize)
{
	HRESULT ret = S_OK;
//	DP("srcSize:%d,%d dstSize:%d,%d  \n",srcSize.s32Width,srcSize.s32Height, 
//		dstSize.s32Width,dstSize.s32Height);
#ifdef APOLLO_SAMSUNG2450
	if(m_convertion == CICscYCbCr420ToRGB565)
	{
		if((memcmp(&srcSize, &m_apoloo_lastsrcSize ,sizeof(srcSize)) != 0) || (memcmp(&dstSize, &m_apoloo_lastdstSize ,sizeof(dstSize)) != 0)) 
		{
			m_apoloo_lastsrcSize = srcSize;
			m_apoloo_lastdstSize = dstSize;
			change_yuv420_rgb565_size(srcSize.width, srcSize.height, dstSize.width, dstSize.height);
		}

		memcpy(m_apoloo_inputBuf,pSrc[0], srcSize.width*srcSize.height);
		memcpy(m_apoloo_inputBuf + srcSize.width*srcSize.height,pSrc[1], srcSize.width*srcSize.height>>2);
		memcpy(m_apoloo_inputBuf + ((srcSize.width*srcSize.height*5)>>2) ,pSrc[2], srcSize.width*srcSize.height>>2);
		
		convert_yuv420_rgb565(m_apoloo_inputBuf , m_apoloo_outputBuf,srcSize.width, srcSize.height, dstSize.width, dstSize.height);

		memcpy(pDst[0],m_apoloo_outputBuf,dstSize.width*dstSize.height*2);

		return S_OK;

	}
#endif

#if defined(PLATOFRM_ANDROID_ACERNETBOOK) || defined(PLATOFRM_DELL_MINI9)
	if(m_convertion == CICscYCbCr420ToRGB565)
	{
		I420_R5G6B5_SSE2(m_yuvrgb_hnd,pSrc[0], pSrc[1], pSrc[2], (uint16_t*)pDst[0]);
	}
#else
	ret =  m_pFunc->ProcessFrame(m_imageHnd,pSrc, srcStride,pDst, dstStride,&m_processParams);
	if(FAILED(ret))
  {
		DP("[GfxPrepareData]ProcessFrame failed %p,%p\n",pSrc,pDst);
		DP("[GfxPrepareData]ProcessFrame failed %d srcbuff:%p,%p,%p, srcStride:%d,%d,%d\n"
      ,ret,pSrc[0],pSrc[1],pSrc[2],srcStride[0],srcStride[1],    srcStride[2]);
		DP("[GfxPrepareData]ProcessFrame failed %d dstbuff:%p,%p,%p, dstStride:%d,%d,%d\n"
      ,ret,pDst[0],pDst[1],pDst[2],dstStride[0],dstStride[1],    dstStride[2]);
  } 
#endif

#ifdef DUMPDATA
    DP("conv:%p pFileDumpInput:%p ,%p \n",m_openParams.u32ColorSpaceConversion,pFileDumpInput ,
      pFileDumpOutput);
	if(m_openParams.u32ColorSpaceConversion == CI_IMAGEPROC_CSC_YCBCR420_TO_YCBCR420
		|| m_openParams.u32ColorSpaceConversion == CI_IMAGEPROC_CSC_YCBCR420_TO_NV12
		|| m_openParams.u32ColorSpaceConversion == CI_IMAGEPROC_CSC_YCBCR420_TO_RGB565
 	    && pFileDumpInput && pFileDumpOutput)
	{
		for(int i=0;i<m_openParams.u32SrcHeight;i++)
			fwrite(pSrc[0] + i*srcStride[0],sizeof(BYTE),m_openParams.u32SrcWidth,pFileDumpInput);
		for(int i=0;i<m_openParams.u32SrcHeight/2;i++)
			fwrite(pSrc[1] + i*srcStride[1],sizeof(BYTE),m_openParams.u32SrcWidth/2 ,pFileDumpInput);
		for(int i=0;i<m_openParams.u32SrcHeight/2;i++)
			fwrite(pSrc[2] + i*srcStride[2],sizeof(BYTE),m_openParams.u32SrcWidth/2,pFileDumpInput);
	}
	else if(m_openParams.u32ColorSpaceConversion == CI_IMAGEPROC_CSC_ARGB32_TO_YCBCR420
		&& pFileDumpInput && pFileDumpOutput)
	{
		fwrite(pSrc[0] ,sizeof(BYTE),m_openParams.u32SrcWidth*m_openParams.u32SrcHeight*4,
			pFileDumpInput);
	}
	else if(m_openParams.u32ColorSpaceConversion == CI_IMAGEPROC_CSC_RGB565_TO_RGB565)
	{
		fwrite(pSrc[0] ,sizeof(BYTE),m_openParams.u32SrcWidth*m_openParams.u32SrcHeight*2,
			pFileDumpInput);
	}


	
	if(m_openParams.u32ColorSpaceConversion == CI_IMAGEPROC_CSC_YCBCR420_TO_YCBCR420
		|| m_openParams.u32ColorSpaceConversion == CI_IMAGEPROC_CSC_YCBCR420_TO_NV12
		|| m_openParams.u32ColorSpaceConversion == CI_IMAGEPROC_CSC_ARGB32_TO_YCBCR420
	    && pFileDumpInput && pFileDumpOutput)
 	{
		for(int i=0;i<m_openParams.u32DstHeight;i++)
			fwrite(pDst[0] + i*dstStride[0],sizeof(BYTE),m_openParams.u32DstWidth ,pFileDumpOutput);
		for(int i=0;i<m_openParams.u32DstHeight/2;i++)
			fwrite(pDst[1] + i*dstStride[1],sizeof(BYTE),m_openParams.u32DstWidth/2 ,pFileDumpOutput);
		for(int i=0;i<m_openParams.u32DstHeight/2;i++)
			fwrite(pDst[2] + i*dstStride[2],sizeof(BYTE),m_openParams.u32DstWidth/2,pFileDumpOutput);
	}
	else if(m_openParams.u32ColorSpaceConversion == CI_IMAGEPROC_CSC_YCBCR420_TO_RGB565
 	    && pFileDumpInput && pFileDumpOutput)
	{
			fwrite(pDst[0] ,sizeof(BYTE),m_openParams.u32DstWidth*m_openParams.u32DstHeight*2
				,pFileDumpOutput);
 	}
	else if(m_openParams.u32ColorSpaceConversion == CI_IMAGEPROC_CSC_YCBCR420_TO_RGB888)
		write_bmp_file_888(m_openParams.u32DstWidth ,m_openParams.u32DstHeight,pDst[0]);
	else if(m_openParams.u32ColorSpaceConversion == CI_IMAGEPROC_CSC_YCBCR420_TO_YUY2)
	{
		for(int i=0;i<m_openParams.u32SrcHeight;i++)
			fwrite(pSrc[0] + i*srcStride[0],sizeof(BYTE),m_openParams.u32SrcWidth,pFileDumpInput);
		for(int i=0;i<m_openParams.u32SrcHeight/2;i++)
			fwrite(pSrc[1] + i*srcStride[1],sizeof(BYTE),m_openParams.u32SrcWidth/2 ,pFileDumpInput);
		for(int i=0;i<m_openParams.u32SrcHeight/2;i++)
			fwrite(pSrc[2] + i*srcStride[2],sizeof(BYTE),m_openParams.u32SrcWidth/2,pFileDumpInput);

	fwrite(pDst[0],sizeof(BYTE),m_openParams.u32DstWidth*m_openParams.u32DstHeight*2,pFileDumpOutput);

	}
	else if(m_openParams.u32ColorSpaceConversion == CI_IMAGEPROC_CSC_RGB565_TO_RGB565)
	{
		fwrite(pDst[0],sizeof(BYTE),m_openParams.u32DstWidth*m_openParams.u32DstHeight*2,pFileDumpOutput);
	}
#endif

	return ret;
}


HRESULT GfxPrepareDataV2::PrepareBufferXRGB(LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 
	DWORD *pSrcStrides_arr, DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr , DWORD dwFlags, LONG lRotation)
{
	if((pDstBuf == NULL) || (ppbPlanes==NULL)|| (ppbPlanes[0]==NULL)|| (ppbPlanes[1]==NULL)|| (ppbPlanes[2]==NULL)
		|| (pSrcStrides_arr==NULL) ||(pSrcWidth_arr==NULL) ||(pSrcHeight_arr == NULL))
	{
		DP("[GfxPrepareDataV2]Invalid param \n");
		return E_FAIL;
	}

	CI_IMAGEPROC_CREATEOPTION params;
	memset(&params, 0,sizeof(params));
	params.u32Size = sizeof(params);
	params.u32Alpha = 0xff;
	params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_RGB565_TO_ARGB32;
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;


	params.u32SrcWidth= pSrcWidth_arr[0];
	params.u32SrcHeight= pSrcHeight_arr[0];
	
	if(lRotation == VP_ROTATION_0)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	else if(lRotation == VP_ROTATION_90)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
	else if(lRotation == VP_ROTATION_180)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
	else if(lRotation == VP_ROTATION_270)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;

	if((params.u32Rotation == CI_IMAGEPROC_ROTATION_90L) || (params.u32Rotation == CI_IMAGEPROC_ROTATION_90R))
	{
		params.u32DstWidth = params.u32SrcHeight;
		params.u32DstHeight = params.u32SrcWidth;
	}
	else
	{
		params.u32DstWidth = params.u32SrcWidth;
		params.u32DstHeight = params.u32SrcHeight;
	}
	
	if(params.u32SrcWidth<=0 || params.u32SrcHeight<=0 || params.u32DstWidth<=0 || params.u32DstHeight<=0)
	{
		DP("[GfxPrepareDataV2]Invalid param (%d,%d) (%d,%d) \n", params.u32SrcWidth,params.u32SrcHeight,params.u32DstWidth,params.u32DstHeight);
		return E_FAIL;
	}

	int ret = Init(&params);
	if(FAILED(ret))
		return ret;

	CI_U32 srcStride_arr[3];
	srcStride_arr[0] = (int)pSrcStrides_arr[0];
	srcStride_arr[1] = (int)pSrcStrides_arr[1];
	srcStride_arr[2] = (int)pSrcStrides_arr[2];

	unsigned char* pDst[3];
	pDst[0] = pDstBuf;

	CI_U32 dstStride_arr[3];
	dstStride_arr[0] = (int)dstStride;
	dstStride_arr[1] = (int)dstStride;
	dstStride_arr[2] = (int)dstStride;

	CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
	CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};
	
	return Process(ppbPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);

}


HRESULT GfxPrepareDataV2::PrepareBufferRGB565(LPBYTE pDstBuf, 
	LONG dstStride, LPBYTE *ppbPlanes, DWORD *pSrcStrides_arr, 
	DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr, DWORD dwFlags, 
	LONG lRotation,VP_DEINTERLACE_MODE mode)
{
	SetDeinterlaceMode(mode);

	CI_IMAGEPROC_CREATEOPTION params;
	memset(&params, 0,sizeof(params));
	params.u32Size = sizeof(params);
	params.u32Alpha = 0xff;
	params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_YCBCR420_TO_RGB565;
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

	params.u32SrcWidth= pSrcWidth_arr[0];
	params.u32SrcHeight= pSrcHeight_arr[0];

	if(lRotation == VP_ROTATION_0)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	else if(lRotation == VP_ROTATION_90)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
	else if(lRotation == VP_ROTATION_180)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
	else if(lRotation == VP_ROTATION_270)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;

	if((params.u32Rotation == CI_IMAGEPROC_ROTATION_90L) || (params.u32Rotation == CI_IMAGEPROC_ROTATION_90R))
	{
		params.u32DstWidth = params.u32SrcHeight;
		params.u32DstHeight = params.u32SrcWidth;
	}
	else
	{
		params.u32DstWidth = params.u32SrcWidth;
		params.u32DstHeight = params.u32SrcHeight;
	}


	if(params.u32SrcWidth<=0 || params.u32SrcHeight<=0 || params.u32DstWidth<=0 || params.u32DstHeight<=0)
	{
		DP("[GfxPrepareDataV2]Invalid param (%d,%d) (%d,%d) \n", params.u32SrcWidth,params.u32SrcHeight,params.u32DstWidth,params.u32DstHeight);
		return E_FAIL;
	}

	int ret = Init(&params);
	if(FAILED(ret))
		return ret;



	CI_U32 srcStride_arr[3];
	srcStride_arr[0] = (int)pSrcStrides_arr[0];
	srcStride_arr[1] = (int)pSrcStrides_arr[1];
	srcStride_arr[2] = (int)pSrcStrides_arr[2];

	unsigned char* pDst[3];
	pDst[0] = pDstBuf;
	
	CI_U32 dstStride_arr[3];
	dstStride_arr[0] = (int)dstStride;
	dstStride_arr[1] = (int)dstStride;
	dstStride_arr[2] = (int)dstStride;
	
	CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
	CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};

	return Process(ppbPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);
}

HRESULT GfxPrepareDataV2::PrepareBufferRGB565Scaled(LPBYTE pDstBuf, 
  LONG dstStride, LPBYTE *ppbPlanes, DWORD *pSrcStrides_arr, RECT *prSrc, 
  RECT *prDst, DWORD dwFlags , LONG lRotation,VP_DEINTERLACE_MODE mode)
{

	LPBYTE pSrcPlanes[3];

	if((pDstBuf == NULL) || (ppbPlanes==NULL)|| (ppbPlanes[0]==NULL)|| (ppbPlanes[1]==NULL)|| (ppbPlanes[2]==NULL)
		|| (pSrcStrides_arr==NULL) ||(prSrc==NULL) ||(prDst== NULL))
	{
		DP("[GfxPrepareDataV2]Invalid param \n");
		return E_FAIL;
	}

	SetDeinterlaceMode(mode);

	CI_IMAGEPROC_CREATEOPTION params;
	memset(&params, 0,sizeof(params));
	params.u32Size = sizeof(params);
	params.u32Alpha = 0xff;
	params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_YCBCR420_TO_RGB565;
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;


	if(lRotation == VP_ROTATION_0)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	else if(lRotation == VP_ROTATION_90)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
	else if(lRotation == VP_ROTATION_180)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
	else if(lRotation == VP_ROTATION_270)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;

	params.u32SrcWidth = prSrc->right-prSrc->left;
	params.u32SrcHeight = prSrc->bottom-prSrc->top;
	params.u32DstWidth = prDst->right-prDst->left;
	params.u32DstHeight = prDst->bottom-prDst->top;
	if(params.u32SrcWidth<=0 || params.u32SrcHeight<=0 || params.u32DstWidth<=0 || params.u32DstHeight<=0)
	{
		DP("[GfxPrepareDataV2]Invalid param (%d,%d) (%d,%d) \n", params.u32SrcWidth,params.u32SrcHeight,params.u32DstWidth,params.u32DstHeight);
		return E_FAIL;
	}

	int ret = Init(&params);
	if(FAILED(ret))
		return ret;

	CI_U32 srcStride_arr[3];
	srcStride_arr[0] = (int)pSrcStrides_arr[0];
	srcStride_arr[1] = (int)pSrcStrides_arr[1];
	srcStride_arr[2] = (int)pSrcStrides_arr[2];

	pSrcPlanes[0] = ppbPlanes[0] + prSrc->left + prSrc->top * srcStride_arr[0];
	pSrcPlanes[1] = ppbPlanes[1] + (prSrc->left>>1) + (prSrc->top>>1) * srcStride_arr[0];
	pSrcPlanes[2] = ppbPlanes[2] + (prSrc->left>>1) + (prSrc->top>>1) * srcStride_arr[0];


	unsigned char* pDst[3];
	pDst[0] = (unsigned char *)pDstBuf;
#ifndef NO_ROT_SCL
	pDst[0] += prDst->left*2 + prDst->top * dstStride;
#endif

	CI_U32 dstStride_arr[3];
	dstStride_arr[0] = (int)dstStride;
	dstStride_arr[1] = (int)dstStride;
	dstStride_arr[2] = (int)dstStride;
	
	CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
	CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};

	return Process(pSrcPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);
}



HRESULT GfxPrepareDataV2::PrepareBufferYUY2(DWORD	dwInputType,LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 
	DWORD *pSrcStrides_arr, DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr, DWORD dwFlags,LONG lRotation)
{
	if((pDstBuf == NULL) || (ppbPlanes==NULL)|| (ppbPlanes[0]==NULL)|| (ppbPlanes[1]==NULL)|| (ppbPlanes[2]==NULL)
		|| (pSrcStrides_arr==NULL) ||(pSrcWidth_arr==NULL) ||(pSrcHeight_arr == NULL))
	{
		DP("[GfxPrepareDataV2]Invalid param \n");
		return E_FAIL;
	}

	if(dwInputType == MAKE_FOURCC('Y','V','1','2'))
	{
		CI_IMAGEPROC_CREATEOPTION params;
		memset(&params, 0,sizeof(params));
		params.u32Size = sizeof(params);
		params.u32Alpha = 0xff;
#if defined(MT3351_DBG) && !defined(MT3351_YECON)		
		params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_YCBCR420_TO_UYVY;
#else
		params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_YCBCR420_TO_YUY2;
#endif
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

		if(lRotation == VP_ROTATION_0)
			params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
		else if(lRotation == VP_ROTATION_90)
			params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
		else if(lRotation == VP_ROTATION_180)
			params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
		else if(lRotation == VP_ROTATION_270)
			params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;

		params.u32SrcWidth= pSrcWidth_arr[0];
		params.u32SrcHeight= pSrcHeight_arr[0];

		if((params.u32Rotation == CI_IMAGEPROC_ROTATION_90L) || (params.u32Rotation == CI_IMAGEPROC_ROTATION_90R))
		{
			params.u32DstWidth = params.u32SrcHeight;
			params.u32DstHeight = params.u32SrcWidth;
		}
		else
		{
			params.u32DstWidth = params.u32SrcWidth;
			params.u32DstHeight = params.u32SrcHeight;
		}

		int ret = Init(&params);
		if(FAILED(ret))
			return ret;

		CI_U32 srcStride_arr[3];
		srcStride_arr[0] = (int)pSrcStrides_arr[0];
		srcStride_arr[1] = (int)pSrcStrides_arr[1];
		srcStride_arr[2] = (int)pSrcStrides_arr[2];

		unsigned char* pDst[3];
		pDst[0] = pDstBuf;
		
		CI_U32 dstStride_arr[3];
		dstStride_arr[0] = (int)dstStride;
		dstStride_arr[1] = (int)dstStride;
		dstStride_arr[2] = (int)dstStride;

		CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
		CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};

		return Process(ppbPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);
	}
	else if(dwInputType == MAKE_FOURCC('Y','U','Y','2'))
	{
		copybuf(pDstBuf,ppbPlanes[0],dstStride,pSrcStrides_arr[0],pSrcWidth_arr[0],pSrcHeight_arr[0]);
		pDstBuf += dstStride*pSrcHeight_arr[0];
		copybuf(pDstBuf,ppbPlanes[1],dstStride,pSrcStrides_arr[1],pSrcWidth_arr[1],pSrcHeight_arr[1]);
		pDstBuf += dstStride*pSrcHeight_arr[1];
		copybuf(pDstBuf,ppbPlanes[2],dstStride,pSrcStrides_arr[2],pSrcWidth_arr[2],pSrcHeight_arr[2]);
		return S_OK;
	}
	return E_FAIL;
}


HRESULT GfxPrepareDataV2::PrepareBufferUYVY(DWORD	dwInputType,LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 
	DWORD *pSrcStrides_arr, DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr, DWORD dwFlags,LONG lRotation)
{
	if((pDstBuf == NULL) || (ppbPlanes==NULL)|| (ppbPlanes[0]==NULL)|| (ppbPlanes[1]==NULL)|| (ppbPlanes[2]==NULL)
		|| (pSrcStrides_arr==NULL) ||(pSrcWidth_arr==NULL) ||(pSrcHeight_arr == NULL))
	{
		DP("[GfxPrepareDataV2]Invalid param \n");
		return E_FAIL;
	}

	if(dwInputType == MAKE_FOURCC('Y','V','1','2'))
	{
		CI_IMAGEPROC_CREATEOPTION params;
		memset(&params, 0,sizeof(params));
		params.u32Size = sizeof(params);
		params.u32Alpha = 0xff;
		params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_YCBCR420_TO_UYVY;
		params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

		if(lRotation == VP_ROTATION_0)
			params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
		else if(lRotation == VP_ROTATION_90)
			params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
		else if(lRotation == VP_ROTATION_180)
			params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
		else if(lRotation == VP_ROTATION_270)
			params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;

		params.u32SrcWidth= pSrcWidth_arr[0];
		params.u32SrcHeight= pSrcHeight_arr[0];

		if((params.u32Rotation == CI_IMAGEPROC_ROTATION_90L) || (params.u32Rotation == CI_IMAGEPROC_ROTATION_90R))
		{
			params.u32DstWidth = params.u32SrcHeight;
			params.u32DstHeight = params.u32SrcWidth;
		}
		else
		{
			params.u32DstWidth = params.u32SrcWidth;
			params.u32DstHeight = params.u32SrcHeight;
		}

		int ret = Init(&params);
		if(FAILED(ret))
			return ret;

		CI_U32 srcStride_arr[3];
		srcStride_arr[0] = (int)pSrcStrides_arr[0];
		srcStride_arr[1] = (int)pSrcStrides_arr[1];
		srcStride_arr[2] = (int)pSrcStrides_arr[2];

		unsigned char* pDst[3];
		pDst[0] = pDstBuf;
		
		CI_U32 dstStride_arr[3];
		dstStride_arr[0] = (int)dstStride;
		dstStride_arr[1] = (int)dstStride;
		dstStride_arr[2] = (int)dstStride;

		CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
		CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};

		return Process(ppbPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);
	}
	else if(dwInputType == MAKE_FOURCC('Y','U','Y','2'))
	{
		copybuf(pDstBuf,ppbPlanes[0],dstStride,pSrcStrides_arr[0],pSrcWidth_arr[0],pSrcHeight_arr[0]);
		pDstBuf += dstStride*pSrcHeight_arr[0];
		copybuf(pDstBuf,ppbPlanes[1],dstStride,pSrcStrides_arr[1],pSrcWidth_arr[1],pSrcHeight_arr[1]);
		pDstBuf += dstStride*pSrcHeight_arr[1];
		copybuf(pDstBuf,ppbPlanes[2],dstStride,pSrcStrides_arr[2],pSrcWidth_arr[2],pSrcHeight_arr[2]);
		return S_OK;
	}
	return E_FAIL;
}
HRESULT GfxPrepareDataV2::PrepareBufferYV12_Test( LPBYTE *pSrcPlanes_arr, LPBYTE* pDstPlanes_arr,
	DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr,
	 DWORD *pSrcWidthStride_arr, DWORD* pDstWidthStride_arr,
	DWORD dwFlags ,LONG lRotation,
	VP_DEINTERLACE_MODE mode)
{

	if((pSrcPlanes_arr == NULL) || (pDstPlanes_arr==NULL)|| (pDstPlanes_arr==NULL)||
			(pSrcHeight_arr==NULL)|| (pSrcWidthStride_arr == NULL)|| (pDstWidthStride_arr==NULL) )
	{
		return E_FAIL;
	}
	SetDeinterlaceMode(mode);


	CI_IMAGEPROC_CREATEOPTION params;
	memset(&params, 0,sizeof(params));
	params.u32Size = sizeof(params);
	params.u32Alpha = 0xff;
	params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_YCBCR420_TO_YCBCR420;
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

	if(lRotation == VP_ROTATION_0)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	else if(lRotation == VP_ROTATION_90)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
	else if(lRotation == VP_ROTATION_180)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
	else if(lRotation == VP_ROTATION_270)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;

	params.u32SrcWidth= pSrcWidth_arr[0];
	params.u32SrcHeight= pSrcHeight_arr[0];

	if((params.u32Rotation == CI_IMAGEPROC_ROTATION_90L) || (params.u32Rotation == CI_IMAGEPROC_ROTATION_90R))
	{
		params.u32DstWidth = params.u32SrcHeight;
		params.u32DstHeight = params.u32SrcWidth;
	}
	else
	{
		params.u32DstWidth = params.u32SrcWidth;
		params.u32DstHeight = params.u32SrcHeight;
	}

	CI_U32 srcStride_arr[3];
	srcStride_arr[0] = (int)pSrcWidthStride_arr[0];
	srcStride_arr[1] = (int)pSrcWidthStride_arr[1];
	srcStride_arr[2] = (int)pSrcWidthStride_arr[2];

	CI_U32 dstStride_arr[3];
	dstStride_arr[0] = (int)pDstWidthStride_arr[0];
	dstStride_arr[1] = (int)pDstWidthStride_arr[1];
	dstStride_arr[2] = (int)pDstWidthStride_arr[2];

	unsigned char *pSrcPlanes[3];
	pSrcPlanes[0] = pSrcPlanes_arr[0];
	pSrcPlanes[1] = pSrcPlanes_arr[1];
	pSrcPlanes[2] = pSrcPlanes_arr[2];

	unsigned char *pDst[3];
	pDst[0] = pDstPlanes_arr[0];
	pDst[1] = pDstPlanes_arr[1];
	pDst[2] = pDstPlanes_arr[2];

	int ret = Init(&params);
	if(FAILED(ret))
		return ret;

	CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
	CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};

	return Process(pSrcPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);

}

HRESULT GfxPrepareDataV2::PrepareBufferYV12(LPBYTE pDstBuf, LONG dstStride, 
	LPBYTE *ppbPlanes, 	DWORD *pSrcStrides_arr, DWORD *pSrcWidth_arr, 
	DWORD *pSrcHeight_arr, DWORD dwFlags ,LONG lRotation, BOOL bScale, 
	RECT* dstRect,VP_DEINTERLACE_MODE mode)
{
	if((pDstBuf == NULL) || (ppbPlanes==NULL)|| (ppbPlanes[0]==NULL)|| (ppbPlanes[1]==NULL)|| (ppbPlanes[2]==NULL)
		|| (pSrcStrides_arr==NULL) ||(pSrcWidth_arr==NULL) ||(pSrcHeight_arr == NULL))
	{
		DP("[GfxPrepareDataV2]Invalid param pDstBuf :%p ppbPlanes:%p pSrcStrides_arr:%p 	pSrcWidth_arr:%p pSrcHeight_arr :%p \n",
		pDstBuf ,ppbPlanes,	  pSrcStrides_arr,pSrcWidth_arr,pSrcHeight_arr );
		return E_FAIL;
	}

	SetDeinterlaceMode(mode);
	CI_IMAGEPROC_CREATEOPTION params;
	memset(&params, 0,sizeof(params));
	params.u32Size = sizeof(params);
	params.u32Alpha = 0xff;
	params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_YCBCR420_TO_YCBCR420;
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

	if(lRotation == VP_ROTATION_0)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	else if(lRotation == VP_ROTATION_90)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
	else if(lRotation == VP_ROTATION_180)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
	else if(lRotation == VP_ROTATION_270)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;

	params.u32SrcWidth= pSrcWidth_arr[0];
	params.u32SrcHeight= pSrcHeight_arr[0];

	
	if((params.u32Rotation == CI_IMAGEPROC_ROTATION_90L) || (params.u32Rotation == CI_IMAGEPROC_ROTATION_90R))
	{
		if(!bScale || !dstRect)
		{
			params.u32DstWidth = params.u32SrcHeight;
			params.u32DstHeight = params.u32SrcWidth;
		}
		else
		{
			params.u32DstWidth = dstRect->bottom - dstRect->top;
			params.u32DstHeight = dstRect->right - dstRect->left;
		}
		//---TO DO Deinterlace while cropping.				
	}
	else
	{
		if(!bScale|| !dstRect)
		{
			params.u32DstWidth = params.u32SrcWidth;
			params.u32DstHeight = params.u32SrcHeight;
		}
		else
		{
			params.u32DstWidth = dstRect->right - dstRect->left;
			params.u32DstHeight = dstRect->bottom - dstRect->top;
		}
	}

	CI_U32 srcStride_arr[3]; 
	srcStride_arr[0] = (int)pSrcStrides_arr[0];
	srcStride_arr[1] = (int)pSrcStrides_arr[1];
	srcStride_arr[2] = (int)pSrcStrides_arr[2];

	CI_U32 dstStride_arr[3]; 
	dstStride_arr[0] = (int)dstStride;
	dstStride_arr[1] = (int)dstStride>>1;
	dstStride_arr[2] = (int)dstStride>>1;
	
	unsigned char *pSrcPlanes[3];
	pSrcPlanes[0] = ppbPlanes[0];
	pSrcPlanes[1] = ppbPlanes[1];
	pSrcPlanes[2] = ppbPlanes[2];


	DWORD pdwBufStrides_arr[2];
	pdwBufStrides_arr[0] = dstStride*params.u32DstHeight;
	pdwBufStrides_arr[1] = (dstStride*params.u32DstHeight)>>2;
	

	unsigned char *pDst[3];
	pDst[0] = pDstBuf;
#if  (!defined(TITAN_II) && !defined(PXA) && !defined(PXA_LINUX) && !defined(ST_LINUX)  \
&& !defined(RMI_CE6) )
	pDst[2] = pDstBuf + pdwBufStrides_arr[0];
	pDst[1] = pDstBuf + pdwBufStrides_arr[0] + pdwBufStrides_arr[1];
#else
	pDst[1] = pDstBuf + pdwBufStrides_arr[0];
	pDst[2] = pDstBuf + pdwBufStrides_arr[0] + pdwBufStrides_arr[1];
#endif	

	int ret = Init(&params);
	if(FAILED(ret))
		return ret;
	
	CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
	CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};

	return Process(pSrcPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);
}

HRESULT GfxPrepareDataV2::PrepareBufferYV12Scaled(unsigned char *pSrc[3], 
  int srcStride[3], CI_IMAGEPROC_SIZE srcSize, unsigned char *pDes[3], int desStride[3],
  CI_IMAGEPROC_SIZE desSize, CI_IMAGEPROC_ROTATION rotationtype)
{

	CI_IMAGEPROC_CREATEOPTION params;
	memset(&params, 0,sizeof(params));
	params.u32Size = sizeof(params);
	params.u32Alpha = 0xff;
	params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_YCBCR420_TO_YCBCR420;
	params.u32SrcWidth= srcSize.s32Width;
	params.u32SrcHeight=srcSize.s32Height;
	params.u32DstWidth= desSize.s32Width;
	params.u32DstHeight=desSize.s32Height;
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

	int ret = Init(&params);
	if(FAILED(ret))
		return ret;

	CI_U32 uSrcStrides[3] = {srcStride[0],srcStride[1],srcStride[2]};
	CI_U32 uDstStrides[3] = {desStride[0],desStride[1],desStride[2]};
	
	return Process(pSrc,uSrcStrides,srcSize, pDes, uDstStrides, desSize);
}

HRESULT GfxPrepareDataV2::PrepareBufferNV12(LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, DWORD *pSrcStrides_arr, DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr, DWORD dwFlags, DWORD* pdwBufStrides)
{
	if((pDstBuf == NULL) || (ppbPlanes==NULL)|| (ppbPlanes[0]==NULL)|| (ppbPlanes[1]==NULL)|| (ppbPlanes[2]==NULL)
		|| (pSrcStrides_arr==NULL) ||(pSrcWidth_arr==NULL) ||(pSrcHeight_arr == NULL))
	{
		DP("[GfxPrepareDataV2]Invalid param \n");
		return E_FAIL;
	}

	copybuf(pDstBuf,ppbPlanes[0],dstStride,pSrcStrides_arr[0],pSrcWidth_arr[0],pSrcHeight_arr[0]);
	pDstBuf += pdwBufStrides[0];
	copybuf(pDstBuf,ppbPlanes[1],dstStride,pSrcStrides_arr[1],pSrcWidth_arr[1],pSrcHeight_arr[1]);
	return S_OK;
}

HRESULT GfxPrepareDataV2::PrepareBufferYU24(LPBYTE pDstBuf, LONG lStride, LPBYTE *ppbPlanes, DWORD *pSrcStrides_arr, DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr, DWORD dwFlags)
{
	if((pDstBuf == NULL) || (ppbPlanes==NULL)|| (ppbPlanes[0]==NULL)|| (ppbPlanes[1]==NULL)|| (ppbPlanes[2]==NULL)
		|| (pSrcStrides_arr==NULL) ||(pSrcWidth_arr==NULL) ||(pSrcHeight_arr == NULL))
	{
		DP("[GfxPrepareDataV2]Invalid param \n");
		return E_FAIL;
	}

	avgbufh2_s2(pDstBuf,ppbPlanes[0],lStride,pSrcStrides_arr[0],pSrcWidth_arr[0],pSrcHeight_arr[0],0);
	pDstBuf += lStride*pSrcHeight_arr[0]<<1;
	lStride >>= 1;
	avgbufh2_s0(pDstBuf,ppbPlanes[1],lStride,pSrcStrides_arr[1],pSrcWidth_arr[1],pSrcHeight_arr[1],0x80808080);
	pDstBuf += lStride*pSrcHeight_arr[1]<<1;
	avgbufh2_s0(pDstBuf,ppbPlanes[2],lStride,pSrcStrides_arr[2],pSrcWidth_arr[2],pSrcHeight_arr[2],0x80808080);
	return S_OK;
}


HRESULT GfxPrepareDataV2::PrepareBufferRGB888(LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 
	DWORD *pSrcStrides_arr, RECT *prSrc, RECT *prDst, DWORD dwFlags , LONG lRotation)
{
	LPBYTE pSrcPlanes[3];

	if((pDstBuf == NULL) || (ppbPlanes==NULL)|| (ppbPlanes[0]==NULL)|| (ppbPlanes[1]==NULL)|| (ppbPlanes[2]==NULL)
		|| (pSrcStrides_arr==NULL) ||(prSrc==NULL) ||(prDst== NULL))
	{
		DP("[GfxPrepareDataV2]Invalid param \n");
		return E_FAIL;
	}


	CI_IMAGEPROC_CREATEOPTION params;
	memset(&params, 0,sizeof(params));
	params.u32Size = sizeof(params);
	params.u32Alpha = 0xff;
	params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_YCBCR420_TO_RGB888;
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

	if(lRotation == VP_ROTATION_0)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	else if(lRotation == VP_ROTATION_90)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
	else if(lRotation == VP_ROTATION_180)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
	else if(lRotation == VP_ROTATION_270)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;

	params.u32SrcWidth = prSrc->right-prSrc->left;
	params.u32SrcHeight = prSrc->bottom-prSrc->top;
	params.u32DstWidth = prDst->right-prDst->left;
	params.u32DstHeight = prDst->bottom-prDst->top;

	int ret = Init(&params);
	if(FAILED(ret))
		return ret;

	CI_U32 srcStride_arr[3];
	srcStride_arr[0] = (int)pSrcStrides_arr[0];
	srcStride_arr[1] = (int)pSrcStrides_arr[1];
	srcStride_arr[2] = (int)pSrcStrides_arr[2];

	pSrcPlanes[0] = ppbPlanes[0] + prSrc->left + prSrc->top * srcStride_arr[0];
	pSrcPlanes[1] = ppbPlanes[1] + (prSrc->left>>1) + (prSrc->top>>1) * srcStride_arr[0];
	pSrcPlanes[2] = ppbPlanes[2] + (prSrc->left>>1) + (prSrc->top>>1) * srcStride_arr[0];


	unsigned char* pDst[3];
	pDst[0] = (unsigned char *)pDstBuf;
#ifndef NO_ROT_SCL
	pDst[0] += prDst->left*3 + prDst->top * dstStride;
#endif

	CI_U32 dstStride_arr[3];
	dstStride_arr[0] = (int)dstStride;
	dstStride_arr[1] = (int)dstStride;
	dstStride_arr[2] = (int)dstStride;

	CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
	CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};

	return Process(pSrcPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);
}

HRESULT GfxPrepareDataV2::PrepareBufferR565ToR565Scaled(LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 
	DWORD *pSrcStrides_arr, RECT *prSrc, RECT *prDst, DWORD dwFlags , LONG lRotation)
{
	LPBYTE pSrcPlanes[3];

	if((pDstBuf == NULL) || (ppbPlanes==NULL)|| (ppbPlanes[0]==NULL)|| (ppbPlanes[1]==NULL)|| (ppbPlanes[2]==NULL)
		|| (pSrcStrides_arr==NULL) ||(prSrc==NULL) ||(prDst== NULL))
	{
		DP("[GfxPrepareDataV2]Invalid param \n");
		return E_FAIL;
	}

	CI_IMAGEPROC_CREATEOPTION params;
	memset(&params, 0,sizeof(params));
	params.u32Size = sizeof(params);
	params.u32Alpha = 0xff;
	params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_RGB565_TO_RGB565;
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

	if(lRotation == VP_ROTATION_0)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	else if(lRotation == VP_ROTATION_90)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
	else if(lRotation == VP_ROTATION_180)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
	else if(lRotation == VP_ROTATION_270)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;

	params.u32SrcWidth = prSrc->right-prSrc->left;
	params.u32SrcHeight = prSrc->bottom-prSrc->top;
	params.u32DstWidth = prDst->right-prDst->left;
	params.u32DstHeight = prDst->bottom-prDst->top;


	int ret = Init(&params);
	if(FAILED(ret))
		return ret;

	CI_U32 srcStride_arr[3];
	srcStride_arr[0] = (int)pSrcStrides_arr[0];
	srcStride_arr[1] = (int)pSrcStrides_arr[1];
	srcStride_arr[2] = (int)pSrcStrides_arr[2];

	pSrcPlanes[0] = ppbPlanes[0] + prSrc->left + prSrc->top * srcStride_arr[0];
	pSrcPlanes[1] = ppbPlanes[1] + (prSrc->left>>1) + (prSrc->top>>1) * srcStride_arr[0];
	pSrcPlanes[2] = ppbPlanes[2] + (prSrc->left>>1) + (prSrc->top>>1) * srcStride_arr[0];

	unsigned char* pDst[3];
	pDst[0] = (unsigned char *)pDstBuf;
#ifndef NO_ROT_SCL
	pDst[0] += prDst->left*2 + prDst->top * dstStride;
#endif

	CI_U32 dstStride_arr[3];
	dstStride_arr[0] = (int)dstStride;
	dstStride_arr[1] = (int)dstStride;
	dstStride_arr[2] = (int)dstStride;

	CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
	CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};

	return Process(pSrcPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);
}

HRESULT GfxPrepareDataV2::PrepareBufferYV12ToARGB(LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 
	DWORD *pSrcStrides_arr, RECT *prSrc, RECT *prDst, DWORD dwFlags , LONG lRotation)
{
	CI_IMAGEPROC_CREATEOPTION params;
	memset(&params, 0,sizeof(params));
	params.u32Size = sizeof(params);
	params.u32Alpha = 0xff;
	params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_YCBCR420_TO_ARGB32;
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

	if(lRotation == VP_ROTATION_0)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	else if(lRotation == VP_ROTATION_90)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
	else if(lRotation == VP_ROTATION_180)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
	else if(lRotation == VP_ROTATION_270)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;
	
	params.u32SrcWidth = prSrc->right-prSrc->left;
	params.u32SrcHeight = prSrc->bottom-prSrc->top;
	params.u32DstWidth = prDst->right-prDst->left;
	params.u32DstHeight = prDst->bottom-prDst->top;

	int ret = Init(&params);
	if(FAILED(ret))
		return ret;

	CI_U32 srcStride_arr[3];
	srcStride_arr[0] = (int)pSrcStrides_arr[0];
	srcStride_arr[1] = (int)pSrcStrides_arr[1];
	srcStride_arr[2] = (int)pSrcStrides_arr[2];

	LPBYTE pSrcPlanes[3];
	pSrcPlanes[0] = ppbPlanes[0] + prSrc->left + prSrc->top * srcStride_arr[0];
	pSrcPlanes[1] = ppbPlanes[1] + (prSrc->left>>1) + (prSrc->top>>1) * srcStride_arr[0];
	pSrcPlanes[2] = ppbPlanes[2] + (prSrc->left>>1) + (prSrc->top>>1) * srcStride_arr[0];

	unsigned char* pDst[3] = {0};
	pDst[0] = (unsigned char *)pDstBuf;
	
	CI_U32 dstStride_arr[3];
	dstStride_arr[0] = (int)dstStride;
	dstStride_arr[1] = (int)dstStride;
	dstStride_arr[2] = (int)dstStride;

	CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
	CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};

	return Process(pSrcPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);
}

HRESULT GfxPrepareDataV2::PrepareBufferYV12ToNV12(LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 
	DWORD *pSrcStrides_arr, DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr, DWORD dwFlags ,LONG lRotation)
{

	if((pDstBuf == NULL) || (ppbPlanes==NULL)|| (ppbPlanes[0]==NULL)|| (ppbPlanes[1]==NULL)|| (ppbPlanes[2]==NULL)
		|| (pSrcStrides_arr==NULL) ||(pSrcWidth_arr==NULL) ||(pSrcHeight_arr == NULL))
	{
		DP("[GfxPrepareDataV2]Invalid param \n");
		return E_FAIL;
	}

	CI_IMAGEPROC_CREATEOPTION params;
	memset(&params, 0,sizeof(params));
	params.u32Size = sizeof(params);
	params.u32Alpha = 0xff;
	params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_YCBCR420_TO_NV12;
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

	if(lRotation == VP_ROTATION_0)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	else if(lRotation == VP_ROTATION_90)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
	else if(lRotation == VP_ROTATION_180)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
	else if(lRotation == VP_ROTATION_270)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;

	params.u32SrcWidth= pSrcWidth_arr[0];
	params.u32SrcHeight= pSrcHeight_arr[0];

	
	if((params.u32Rotation == CI_IMAGEPROC_ROTATION_90L) || (params.u32Rotation == CI_IMAGEPROC_ROTATION_90R))
	{
		params.u32DstWidth = params.u32SrcHeight;
		params.u32DstHeight = params.u32SrcWidth;
	}
	else
	{
		params.u32DstWidth = params.u32SrcWidth;
		params.u32DstHeight = params.u32SrcHeight;
	}

	CI_U32 srcStride_arr[3]; 
	srcStride_arr[0] = (int)pSrcStrides_arr[0];
	srcStride_arr[1] = (int)pSrcStrides_arr[1];
	srcStride_arr[2] = (int)pSrcStrides_arr[2];

	CI_U32 dstStride_arr[3]; 
	dstStride_arr[0] = (int)dstStride;
	dstStride_arr[1] = (int)dstStride;
	dstStride_arr[2] = (int)dstStride;
	
	unsigned char *pSrcPlanes[3];
	pSrcPlanes[0] = ppbPlanes[0];
	pSrcPlanes[1] = ppbPlanes[1];
	pSrcPlanes[2] = ppbPlanes[2];

	unsigned char *pDst[3];
	pDst[0] = pDstBuf;
	pDst[1] = pDstBuf + dstStride*params.u32DstHeight;
	pDst[2] = pDstBuf + dstStride*params.u32DstHeight;

	int ret = Init(&params);
	if(FAILED(ret))
		return ret;
	
	CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
	CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};

	return Process(pSrcPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);
}


HRESULT GfxPrepareDataV2::PrepareBufferNV12ToYV12(LPBYTE pDstBuf, LONG dstStride, LPBYTE *ppbPlanes, 
	DWORD *pSrcStrides_arr, DWORD *pSrcWidth_arr, DWORD *pSrcHeight_arr, DWORD dwFlags ,LONG lRotation)
{

	if((pDstBuf == NULL) || (ppbPlanes==NULL)|| (ppbPlanes[0]==NULL)|| (ppbPlanes[1]==NULL)|| (ppbPlanes[2]==NULL)
		|| (pSrcStrides_arr==NULL) ||(pSrcWidth_arr==NULL) ||(pSrcHeight_arr == NULL))
	{
		DP("[GfxPrepareDataV2]Invalid param \n");
		return E_FAIL;
	}

	CI_IMAGEPROC_CREATEOPTION params;
	memset(&params, 0,sizeof(params));
	params.u32Size = sizeof(params);
	params.u32Alpha = 0xff;
	params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_YCBCR420_TO_NV12;
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

	if(lRotation == VP_ROTATION_0)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	else if(lRotation == VP_ROTATION_90)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
	else if(lRotation == VP_ROTATION_180)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
	else if(lRotation == VP_ROTATION_270)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;

	params.u32SrcWidth= pSrcWidth_arr[0];
	params.u32SrcHeight= pSrcHeight_arr[0];

	
	if((params.u32Rotation == CI_IMAGEPROC_ROTATION_90L) || (params.u32Rotation == CI_IMAGEPROC_ROTATION_90R))
	{
		params.u32DstWidth = params.u32SrcHeight;
		params.u32DstHeight = params.u32SrcWidth;
	}
	else
	{
		params.u32DstWidth = params.u32SrcWidth;
		params.u32DstHeight = params.u32SrcHeight;
	}

	CI_U32 srcStride_arr[3]; 
	srcStride_arr[0] = (int)pSrcStrides_arr[0];
	srcStride_arr[1] = (int)pSrcStrides_arr[1];
	srcStride_arr[2] = (int)pSrcStrides_arr[2];

	CI_U32 dstStride_arr[3]; 
	dstStride_arr[0] = (int)dstStride;
	dstStride_arr[1] = (int)dstStride;
	dstStride_arr[2] = (int)dstStride;
	
	unsigned char *pSrcPlanes[3];
	pSrcPlanes[0] = ppbPlanes[0];
	pSrcPlanes[1] = ppbPlanes[1];
	pSrcPlanes[2] = ppbPlanes[2];

	unsigned char *pDst[3];
	pDst[0] = pDstBuf;
	pDst[1] = pDstBuf + dstStride*params.u32DstHeight;
	pDst[2] = pDstBuf + dstStride*params.u32DstHeight;

	int ret = Init(&params);
	if(FAILED(ret))
		return ret;
	
	CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
	CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};

	return Process(pSrcPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);
}

HRESULT GfxPrepareDataV2::PrepareBufferARGB2YV12(LPBYTE pBuf, LONG dstStride, 
	LPBYTE *ppbPlanes, DWORD *pdwStrides, DWORD *pdwWidth, DWORD *pdwHeight, 
	DWORD dwFlags,	LONG lRotation,VP_DEINTERLACE_MODE mode)
{
	SetDeinterlaceMode(mode);

	CI_IMAGEPROC_CREATEOPTION params;
	memset(&params, 0,sizeof(params));
	params.u32Size = sizeof(params);
	params.u32Alpha = 0xff;
	params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_ARGB32_TO_YCBCR420;
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

	params.u32SrcWidth= pdwWidth[0];
	params.u32SrcHeight= pdwHeight[0];

	if(lRotation == VP_ROTATION_0)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	else if(lRotation == VP_ROTATION_90)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
	else if(lRotation == VP_ROTATION_180)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
	else if(lRotation == VP_ROTATION_270)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;

	params.u32DstWidth = params.u32SrcWidth;
	params.u32DstHeight = params.u32SrcHeight;


	if(params.u32SrcWidth<=0 || params.u32SrcHeight<=0 || params.u32DstWidth<=0 || params.u32DstHeight<=0)
	{
		DP("[GfxPrepareDataV2]Invalid param (%d,%d) (%d,%d) \n", params.u32SrcWidth,params.u32SrcHeight,params.u32DstWidth,params.u32DstHeight);
		return E_FAIL;
	}

	int ret = Init(&params);
	if(FAILED(ret))
		return ret;



	CI_U32 srcStride_arr[3];
	srcStride_arr[0] = (int)pdwStrides[0];
	srcStride_arr[1] = (int)pdwStrides[1];
	srcStride_arr[2] = (int)pdwStrides[2];

	unsigned char* pDst[3];
	pDst[0] = pBuf;

	 #if defined(VNCVIEWER)
	pDst[2] = pBuf+dstStride*params.u32DstHeight ;
	pDst[1] = pBuf+dstStride*params.u32DstHeight + dstStride*params.u32DstHeight/4;
	 #else
	pDst[1] = pBuf+dstStride*params.u32DstHeight ;
	pDst[2] = pBuf+dstStride*params.u32DstHeight + dstStride*params.u32DstHeight/4;
	#endif
	CI_U32 dstStride_arr[3];
	dstStride_arr[0] = (int)dstStride;
	dstStride_arr[1] = (int)dstStride/2;
	dstStride_arr[2] = (int)dstStride/2;
	
	CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
	CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};

	return Process(ppbPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);
}

HRESULT GfxPrepareDataV2::PrepareBufferRGB5652YV12(LPBYTE pBuf, LONG dstStride, LPBYTE *ppbPlanes, 
		DWORD *pdwStrides, DWORD *pdwWidth, DWORD *pdwHeight, DWORD dwFlags,
		LONG lRotation,VP_DEINTERLACE_MODE mode)
{
	SetDeinterlaceMode(mode);

	CI_IMAGEPROC_CREATEOPTION params;
	memset(&params, 0,sizeof(params));
	params.u32Size = sizeof(params);
	params.u32Alpha = 0xff;
	params.u32ColorSpaceConversion = CI_IMAGEPROC_CSC_RGB565_TO_YCBCR420;
	params.u32Interpolation = CI_IMAGEPROC_INTERPOLATION_LINEAR;

	params.u32SrcWidth= pdwWidth[0];
	params.u32SrcHeight= pdwHeight[0];

	if(lRotation == VP_ROTATION_0)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_DISABLE;
	else if(lRotation == VP_ROTATION_90)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90L;
	else if(lRotation == VP_ROTATION_180)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_180;
	else if(lRotation == VP_ROTATION_270)
		params.u32Rotation = CI_IMAGEPROC_ROTATION_90R;

	params.u32DstWidth = params.u32SrcWidth;
	params.u32DstHeight = params.u32SrcHeight;


	if(params.u32SrcWidth<=0 || params.u32SrcHeight<=0 || params.u32DstWidth<=0 || params.u32DstHeight<=0)
	{
		DP("[GfxPrepareDataV2]Invalid param (%d,%d) (%d,%d) \n", params.u32SrcWidth,params.u32SrcHeight,params.u32DstWidth,params.u32DstHeight);
		return E_FAIL;
	}

	int ret = Init(&params);
	if(FAILED(ret))
		return ret;


	CI_U32 srcStride_arr[3];
	srcStride_arr[0] = (int)pdwStrides[0];
	srcStride_arr[1] = (int)pdwStrides[1];
	srcStride_arr[2] = (int)pdwStrides[2];

	unsigned char* pDst[3];
	pDst[0] = pBuf;
	pDst[1] = pBuf+dstStride*params.u32DstHeight ;
	pDst[2] = pBuf+dstStride*params.u32DstHeight + dstStride*params.u32DstHeight/4;

	CI_U32 dstStride_arr[3];
	dstStride_arr[0] = (int)dstStride;
	dstStride_arr[1] = (int)dstStride/2;
	dstStride_arr[2] = (int)dstStride/2;
	
	CI_IMAGEPROC_SIZE iSrcSize = {params.u32SrcWidth,params.u32SrcHeight};
	CI_IMAGEPROC_SIZE idstSize = {params.u32DstWidth,params.u32DstHeight};

	return Process(ppbPlanes,srcStride_arr,iSrcSize, pDst, dstStride_arr, idstSize);
}

HRESULT GfxPrepareDataV2::SetDeinterlaceMode(VP_DEINTERLACE_MODE mode)
{
    CI_U32 deinterlaced = CI_IMAGEPROC_DeInterlace_None;
    switch (mode) {
        case VP_DEINTERLACE_None:
            deinterlaced = CI_IMAGEPROC_DeInterlace_None;
            break;
        case VP_DEINTERLACE_Fast:
           deinterlaced = CI_IMAGEPROC_DeInterlace_Fast;
           break;
        case VP_DEINTERLACE_Discard:
            deinterlaced = CI_IMAGEPROC_DeInterlace_Discard;
            break;
        case VP_DEINTERLACE_Bob:
            deinterlaced = CI_IMAGEPROC_DeInterlace_Bob;
            break;
        case VP_DEINTERLACE_Blend:
            deinterlaced = CI_IMAGEPROC_DeInterlace_Blend;
            break;
        case VP_DEINTERLACE_Mean:
            deinterlaced = CI_IMAGEPROC_DeInterlace_Mean;
            break;
        case VP_DEINTERLACE_Linear:
            deinterlaced = CI_IMAGEPROC_DeInterlace_Linear;
            break;
        case VP_DEINTERLACE_X:
            deinterlaced = CI_IMAGEPROC_DeInterlace_X;
            break;
        default:
            deinterlaced = CI_IMAGEPROC_DeInterlace_None;
            break;
    }

    m_processParams.u32DeInterlace = deinterlaced;
	return S_OK;
}
	
