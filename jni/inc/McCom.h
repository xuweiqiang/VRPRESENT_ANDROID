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
// 	Copyright (c) 2008, 2009 Cidana, Inc.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

#ifndef _MCCOM_H_
#define _MCCOM_H_

#ifndef E_PROP_SET_UNSUPPORTED
#define E_PROP_SET_UNSUPPORTED    ((HRESULT)0x80070492L)
#endif //E_PROP_SET_UNSUPPORTED
#ifndef E_PROP_ID_UNSUPPORTED
#define E_PROP_ID_UNSUPPORTED     ((HRESULT)0x80070490L)
#endif //E_PROP_ID_UNSUPPORTED

#ifndef CI_DEFINE_GUID

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

	#define CI_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
		 EXTERN_C DECLSPEC_SELECTANY const GUID FAR name ATTRIBUTE_WEAK \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
                
#endif //CI_DEFINE_GUID

/* the following is used for COM delegation.  It is not
directly called because it does not support delegation/
aggregation. */

DECLARE_INTERFACE(IMcNonDelegatingUnknown)
{
	STDMETHOD(NonDelegatingQueryInterface)(THIS_ REFIID, LPVOID *) PURE;
	STDMETHOD_(ULONG, NonDelegatingAddRef)(THIS) PURE;
	STDMETHOD_(ULONG, NonDelegatingRelease)(THIS) PURE;
};

/*
DECLARE_IUNKNOWN is required for the derived class exporting
COM functionality.  This ensures that the outermost class
which is aggregating the object gets the proper QueryInterface
request. */

#ifdef DECLARE_IUNKNOWN
#undef DECLARE_IUNKNOWN
#endif
#define DECLARE_IUNKNOWN                                        \
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) {      \
        return GetOwner()->QueryInterface(riid,ppv);            \
    };                                                          \
    STDMETHODIMP_(ULONG) AddRef() {                             \
        return GetOwner()->AddRef();                            \
    };                                                          \
    STDMETHODIMP_(ULONG) Release() {                            \
        return GetOwner()->Release();                           \
    };

class CMcUnknown : IMcNonDelegatingUnknown
{
public:
	CMcUnknown(LPCTSTR pName = 0, LPUNKNOWN pUnk = 0)
	{
		m_lRef = 1;
		m_pName = pName;
	    m_pUnk = pUnk ? pUnk : reinterpret_cast<IUnknown*>(static_cast<IMcNonDelegatingUnknown *>(this));
	}
	virtual ~CMcUnknown() {}
	STDMETHODIMP_(ULONG) NonDelegatingAddRef() 
	{
		return InterlockedIncrement(&m_lRef);
	}
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, LPVOID *ppvObj)
	{
		if(ppvObj==0)
			return E_POINTER;

#ifndef __linux__
		if(riid==IID_IUnknown)
			return GetInterface(reinterpret_cast<IUnknown*>(static_cast<IMcNonDelegatingUnknown*>(this)), ppvObj);
#else
static char data4[8] = { 0xc0, 0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x46 };
		if(riid.Data1 == 0 &&
			riid.Data2 == 0 &&
			riid.Data3 == 0 &&
			!memcmp(riid.Data4, data4, sizeof(data4))
			)

		{
			*ppvObj = static_cast<IMcNonDelegatingUnknown *>(this);
			return S_OK;
		}
#endif
		return E_NOINTERFACE;
	}
	STDMETHODIMP_(ULONG) NonDelegatingRelease()
	{
		LONG ref;
		
		ref = InterlockedDecrement(&m_lRef);
		if(ref<=0)
			delete this;
		return ref;
	}
	LPUNKNOWN GetOwner()
	{
		return m_pUnk;
	}
	STDMETHOD (GetInterface)(LPUNKNOWN pUnk, LPVOID *ppvObj)
	{
		if(ppvObj==0)
			return E_POINTER;
		NonDelegatingAddRef();
		*ppvObj = pUnk;
		return S_OK;
	}
protected:
	LPCTSTR		m_pName;
	LONG		m_lRef;
	LPUNKNOWN	m_pUnk;
};

template <typename T> inline HRESULT GetCom(T tValue, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
{
	if(pPropData==0)
		return E_POINTER;
	if(cbPropData<sizeof(T))
		return E_INVALIDARG;
	*reinterpret_cast<T *>(pPropData) = tValue;
	if(pcbReturned)
		*pcbReturned = sizeof(T);
	return S_OK;
}

template <typename T> inline HRESULT SetCom(T &tValue, LPVOID pPropData, DWORD cbPropData)
{
	if(pPropData==0)
		return E_POINTER;
	if(sizeof(T)<cbPropData)
		return E_INVALIDARG;
	tValue = *reinterpret_cast<T *>(pPropData);
	return S_OK;
}

template <typename T> inline HRESULT GetComString(T *tValue, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
{
	unsigned i;
	T *dst;

	if(pPropData==0)
		return E_POINTER;
	for(i=0,dst=reinterpret_cast<T *>(pPropData);*tValue && i<=cbPropData-sizeof(T);i+=sizeof(T))
		*dst++ = *tValue++;	// copy data
	*dst++ = 0;	// ensure zero terminated
	if(pcbReturned)
		*pcbReturned = i+sizeof(T);
	return *tValue ? E_FAIL : S_OK;
}

template <typename T> inline HRESULT SetComString(T *tValue, DWORD cbValue, LPVOID pPropData, DWORD cbPropData)
{
	unsigned i;
	T *src;

	if(pPropData==0)
		return E_POINTER;
	if(cbPropData>cbValue)
		return E_INVALIDARG;
	for(i=0,src=reinterpret_cast<T *>(pPropData);*src && i<=cbValue-sizeof(T);i+=sizeof(T))
		*tValue++ = *src++;	// copy data
	*tValue++ = 0;	// ensure zero terminated
	return *src ? E_FAIL : S_OK;
}

inline HRESULT ReturnComLong(LONG lValue, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
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

#endif //_MCCOM_H_
