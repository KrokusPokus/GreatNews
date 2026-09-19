#pragma once

#include "ContentGenerator.h"
#include "NewsSource.h"
#include "loki\SmartPtr.h"

class CBatchContentGenerator :
	public CContentGenerator,
	public CNewsSource
{
public:
	CBatchContentGenerator(void);
	virtual ~CBatchContentGenerator(void);

	virtual bool IsBatch() {return true;}	// really bad method

	virtual void InitGenerator(int nPageSize);
	virtual void SetPageSize(int nPageSize);
	virtual size_t GetNumOfItems();
	virtual CString GeneratePageHTML(int nPage)=0;
	virtual CString GeneratePageContentHTML(int nPage);
	virtual void ExportRss(MSXML2::IXMLDOMElementPtr& spChannel);
	virtual bool DisplayingLastPage() {return m_nCurrentPage==m_nTotalPage-1;}

	static std::vector<ULONG_PTR> m_vectNewsIDs;

	static int GetFirstItemOnPage() {return m_nPageSize*m_nCurrentPage;}
	static int GetLastItemOnPage() {return max(0, m_nPageSize*(m_nCurrentPage+1)-1);}

protected:
	void GetCurrentPageItems(NewsItemVector& newsItems, int nPage);
	int CalcTotalPage();
	CString GeneratePager();

	// pagination support.
	static int m_nPageSize;
	static int m_nCurrentPage;
	static int m_nTotalPage;
	static bool m_b10x;
};

typedef Loki::SmartPtr<CBatchContentGenerator, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> BatchContentGeneratorPtr;
typedef std::vector<BatchContentGeneratorPtr> BatchContentGeneratorVector;
