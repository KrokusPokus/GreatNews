#pragma once

#include "ngmtsync.h"
#include "gnthread.h"
#include "BatchContentGenerator.h"

#define MM_SEARCH_FOUND		(WM_APP + 200)
#define MM_SEARCH_NOTFOUND	(WM_APP + 202)	

class CSearchThread :
	public CGNThread
{
public:
	CSearchThread(void);
	virtual ~CSearchThread(void);

	virtual DWORD Run();

public:
	BatchContentGeneratorPtr m_content;
	CString m_searchText;
	int m_itemFound;
	int m_nSearching;

public:
	bool m_bStop;
	HWND m_hwndToNotify;
	CNGEvent m_stopEvent;
	CString m_matchedString;
};
