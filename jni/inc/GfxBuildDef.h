//#define BUILD_ALL

#ifndef __linux__
#pragma warning(disable:4995)
#pragma warning(disable:4996)
#endif

#define _CRT_SECURE_NO_DEPRECATE

#if defined(RTK_TITAN_II) || defined(FY_TITAN_II) 
#define TITAN_II
#endif

#ifdef SAMSUNG_2451_DDNAV
#define SAMSUNG_2450_LEADER_INTL
#endif

#if defined(SAMSUNG_2416) || defined(SAMSUNG_2450) || defined(SAMSUNG_2450_LEADER_INTL)
#define SAMSUNG
#endif

#ifdef  SIRF_A4_PVEGPS
#define SIRF_A4_NEWOS
#endif

#ifdef  GFX_FB_LINUX
#define GFX_FB
#endif

#if defined(FREESCALE_IMX31) || defined(MT3351_DBG) || defined(TITAN_II) ||	defined(APOLLO_SAMSUNG2450) || defined(SIRF_A4_NEWOS) || defined(SIRF_A4_OLDOS)	|| defined(SAMSUNG6443) || defined(NAVI) 
#ifndef GFX_DDRAW
#define GFX_DDRAW
#endif
#endif

#if defined(SAMSUNG_PC100) || defined(ST_SAVECOPY) || defined(OMAP3530) || defined(TI3630)
#ifndef GFX_DDRAW
#define GFX_DDRAW
#endif
#endif

#if defined(MT3351_DBG) 
#define GFX_DDRAW_OSD
#endif

#ifdef BUILD_ALL
	#ifdef WIN32
	#define GFX_SDL
	#define GFX_D3D
	#endif

	#ifdef _WIN32_WCE
	#define TITAN
	#define TITAN_II
	#define RMI
	#endif
#define GFX_DDRAW
#define PXA
#define GFX_GDI
#define GFX_GX
#endif

