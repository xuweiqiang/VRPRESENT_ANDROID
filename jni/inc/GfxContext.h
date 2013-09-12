//=============================================================================
//	THIS SOURCE CODE IS PROPRIETARY INFORMATION BELONGING TO INTERVIDEO, INC.
// 	ANY USE INCLUDING BUT NOT LIMITED TO COPYING OF CODE, CONCEPTS, AND/OR
//	ALGORITHMS IS PROHIBITED EXCEPT WITH EXPRESS WRITTEN PERMISSION BY THE 
//	COMPANY.
//
// 	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// 	KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// 	IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// 	PURPOSE.
//
// 	Copyright (c) 2004 - 2005  InterVideo Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _GFXCONTEXT_H_
#define _GFXCONTEXT_H_

struct GFX_OPEN_OPTIONS
{
	HWND hWnd;
};

struct GraphicsProperties {
	DWORD	cxWidth;
	DWORD	cyHeight;			// notice lack of 'th' in the word height.
	LONG	cbxPitch;			// number of bytes to move right one x pixel - can be negative.
	LONG	cbyPitch;			// number of bytes to move down one y pixel - can be negative.
	LONG	cBPP;				// # of bits in each pixel
	DWORD	ffFormat;			// format flags.
};

enum
{
	GUI_KEY_UNDEFINED,
	GUI_KEY_UP,
	GUI_KEY_DOWN,
	GUI_KEY_LEFT,
	GUI_KEY_RIGHT,
	GUI_KEY_START,
};

enum
{
	RGB_UNDEFINED = 0x0,
	RGB_565 = 0x1,
	RGB_555 = 0x2,
	RGB_888 = 0x4,
	RGB_444 = 0x8,
};

/*! Graphics context attributes.
 */
struct EMC_GRAPHICS_INFO
{
	DWORD	dwFourCC;
	DWORD	dwAspectX;
	DWORD	dwAspectY;
	DWORD	dwCopyProtect;
	DWORD	dwInterlace;
};

// from guid.h
#ifndef iviDEFINE_GUID

#ifdef __GNUG__  /* gcc/g++ compiler (Linux, Symbian ARMI) does the equivalent of _declspec(selectany) */
#define ATTRIBUTE_WEAK     __attribute__ ((weak))
#define DECLSPEC_SELECTANY
#elif defined(__ADS__)
#define ATTRIBUTE_WEAK  
#define DECLSPEC_SELECTANY 
#else
#define ATTRIBUTE_WEAK
#if !defined(DECLSPEC_SELECTANY)
#define DECLSPEC_SELECTANY _declspec(selectany)
#endif
#endif

	#define iviDEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
		 EXTERN_C DECLSPEC_SELECTANY const GUID FAR name ATTRIBUTE_WEAK \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
                
#endif

iviDEFINE_GUID(EMC_PROPSET_GFXCTXT,
0x9eccdcca, 0xeb44, 0x11d4, 0xb3, 0x65, 0x00, 0x01, 0x02, 0x2a, 0x4a, 0x87);

iviDEFINE_GUID(IID_IGraphicsContext,
0x9eccecca, 0xeb44, 0x11d4, 0xb3, 0x65, 0x00, 0x01, 0x02, 0x2a, 0x4a, 0x87);

#ifndef EMC_PROPID_OFFSET
#define EMC_PROPID_OFFSET(groupid)	(groupid<<8)
#endif

enum EMC_PROPID_GFXCTXT
{
	EMC_PROPID_GFXCTXT_TYPE = EMC_PROPID_OFFSET(0),
	EMC_PROPID_GFXCTXT_GFXINFO,
	EMC_PROPID_GFXCTXT_DSTRECT,
};

#ifndef IEmcPropertySet_DEFINED_
#define IEmcPropertySet_DEFINED_
/** The IEmcPropertySet interface is an way to set and retrieve component properties */
DECLARE_INTERFACE_(IEmcPropertySet,IUnknown)
{
/*! The QuerySupported method determines whether an object supports a specified property set. 
  \param guidPropSet [in] Property set GUID.
  \param dwPropID [in] Identifier of the property within the property set. 
  \param pTypeSupport [out] Pointer to a value in which to store flags indicating the support provided by the driver. 
  \return Returns an HRESULT value.
*/
	STDMETHOD(QuerySupported)(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport) PURE;
/*! The Get method retrieves a property identified by a property set GUID and a property ID.
  \param guidPropSet [in] Property set GUID.
  \param dwPropID [in] Identifier of the property within the property set. 
  \param pInstanceData [out, size_is(cbInstanceData)] Pointer to instance data for the property.
  \param cbInstanceData [in] Number of bytes in the buffer to which pInstanceData points.
  \param pPropData [out, size_is(cbPropData)] Pointer to the retrieved buffer, which contains the value of the property. 
  \param cbPropData [in] Number of bytes in the buffer to which pPropData points.
  \param pcbReturned [out] Pointer to a variable that receives the number of bytes the method returned in the pPropData array.
  \return Returns an HRESULT value.
*/
	STDMETHOD(Get)(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned) PURE;
/*! The Set method sets a property identified by a property set GUID and a property ID. 
  \param guidPropSet [in] Property set GUID.
  \param dwPropID [in] Identifier of the property within the property set. 
  \param pInstanceData [out, size_is(cbInstanceData)] Pointer to instance data for the property.
  \param cbInstanceData [in] Number of bytes in the buffer to which pInstanceData points.
  \param pPropData [out, size_is(cbPropData)] Pointer to the retrieved buffer, which contains the value of the property. 
  \param cbPropData [in] Number of bytes in the buffer to which pPropData points.
  \return Returns an HRESULT value.
*/
	STDMETHOD(Set)(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData) PURE;
};
#endif

DECLARE_INTERFACE_(IGraphicsContext,IEmcPropertySet)
{
	STDMETHOD(Open)(GFX_OPEN_OPTIONS *pOptions, DWORD dwSize) PURE;
	STDMETHOD(Close)() PURE;
	STDMETHOD(BeginDraw)(void ** pFrameBuffer, bool flip) PURE;
	STDMETHOD(BeginDraw)(void ** pY, void ** pU, void ** pV, int * pitch, RECT *rtSrcView, RECT *rtDstView, int rot) PURE;
	STDMETHOD(EndDraw)() PURE;
	STDMETHOD(EndDraw)(bool flip) PURE;
	STDMETHOD(SetFocus)(BOOL bFocusOn) PURE;
	STDMETHOD(SetViewport)( RECT *rtView ) PURE;
	STDMETHOD(GetGuiKey)( DWORD dwIn, DWORD * pOut ) PURE;
	STDMETHOD(GetGraphicsProperties)( GraphicsProperties * pGraphicsProperties ) PURE;
};

IGraphicsContext *CreateGraphicsContext();

#endif
