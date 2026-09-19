#pragma once
#include "atlcomcli.h"

class CSafeArrayVariant : public CComVariant
{
public:
	CSafeArrayVariant(void)
	{
		parray    = 0;
		m_nLength = 0;
	}

	CSafeArrayVariant(const CComVariant& var)
	{
		parray   = 0;
		operator =(var);
	}
	CSafeArrayVariant(BYTE * szBytes,int nLength)
	{
		ReadBytes(szBytes,nLength);
	}

	~CSafeArrayVariant(void) {};

public:
	void Clear()
	{
		if((vt & VT_ARRAY) && parray)
		{
			SafeArrayDestroy(parray);
			parray = 0;
		}
	}

	void ReadBytes(BYTE * pBytes,int nLength)
	{
		Clear();
		vt = VT_ARRAY|VT_UI1;
		BYTE * pBytes2 = 0;
		m_nLength = (nLength > 0) ? nLength : 0;
		parray = SafeArrayCreateVector(VT_UI1,1,m_nLength);
		SafeArrayAccessData(parray,(void**)&pBytes2);
		memcpy(pBytes2,pBytes,m_nLength);
		SafeArrayUnaccessData(parray);
	}

	CSafeArrayVariant& operator = (VARIANT var)
	{
		m_nLength = 0;
		if(var.vt == (VT_ARRAY|VT_UI1))
		{
			long lBound,uBound;
			SafeArrayGetLBound(var.parray,1,&lBound);
			SafeArrayGetUBound(var.parray,1,&uBound);
			m_nLength = uBound - lBound;
		}
		CComVariant::operator =(var);
		return *this;
	}
	int GetLength()const \
	{
		return m_nLength;
	}

protected:
	int m_nLength;

};
