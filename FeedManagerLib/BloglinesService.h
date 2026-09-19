#pragma once

#include "Singleton.h"

#import "msxml3.dll"

class CBloglinesService
{
public:
	virtual ~CBloglinesService(void);

public:
	MSXML2::IXMLDOMDocumentPtr ListChannels(LPCTSTR account, LPCTSTR password);	
	bool GetAccountInfo(CString& account, CString& password);
	bool SetAccountInfo(const CString& account, const CString& password);

	bool GetMarkReadFlag();
	void SetMarkReadFlag(bool dontMark);

	CString GetSyncChannelUrl(int subsId);
	CString& GetErrorMessage() {return m_errMsg;}

	bool SaveParam();

protected:
	CBloglinesService(void);
	bool LoadParameters();

protected:
	CString m_account;
	CString m_password;
	bool m_dontMarkRead;

	CString m_errMsg;

	friend CGNSingleton<CBloglinesService>;
};
