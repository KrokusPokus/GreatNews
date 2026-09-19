#pragma once

#include <vector>
#include <map>
#include "loki\SmartPtr.h"
#include "NewsFilter.h"

#import "msxml3.dll"

class CContentGenerator
{
public:
	enum Page
	{
		FirstPage = 0,
		PrevPage = -2,
		NextPage = -3,
		LastPage = -4,
		AllPage = -5,
		CurrentPage = -10
	};

	CContentGenerator();
	virtual ~CContentGenerator();

	virtual void InitGenerator(int nPageSize)=0;
	virtual size_t GetNumOfItems()=0;
	virtual CString GeneratePageHTML(int nPage)=0;
	virtual CString GeneratePageContentHTML(int nPage)=0;
	virtual void UpdateUsage() {};
	virtual bool IsBatch() {return false;}	// really bad

	virtual CString GetContentID() const =0;
	virtual CString GetHomeURL() const=0;
	virtual CString GetTitle() const=0;
	virtual CString GetStyle() const { return _T(""); }

	virtual bool DisplayingLastPage() {return true;}

	static std::vector<ULONG_PTR> m_unreadItemIDs;
	static std::map<ULONG_PTR, int> m_channelUnreadCounts;
	static bool HasUnreadItem() {return !m_unreadItemIDs.empty();}

public:
	void SetNewsFilter(CNewsFilter* p) {m_pNewsFilter=p;};
	bool HasFilter() {return m_pNewsFilter!=NULL;}

	static void AddElement(MSXML2::IXMLDOMDocumentPtr& spDoc,
							MSXML2::IXMLDOMElementPtr& spElement,
							LPCTSTR elementName,
							LPCTSTR text);
	static void AddElementCData(MSXML2::IXMLDOMDocumentPtr& spDoc,
							MSXML2::IXMLDOMElementPtr& spElement,
							LPCTSTR elementName,
							LPCTSTR text);

protected:
	static CNewsFilter* m_pNewsFilter;
};

typedef Loki::SmartPtr<CContentGenerator, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> ContentGeneratorPtr;
