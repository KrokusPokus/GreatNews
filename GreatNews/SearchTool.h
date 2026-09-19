#pragma once

#include <atlrx.h>

#include "loki\SmartPtr.h"
#include "NewsItem.h"

class CSearchTool;
typedef Loki::SmartPtr<CSearchTool, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> SearchToolPtr;

class CSearchTool
{
public:
	static SearchToolPtr CreateTool(const CString& searchString);
	virtual bool Match(NewsItemPtr& item) = 0;

public:
	CString m_matchedString;
};


class CTextSearchTool : public CSearchTool
{
protected:
	CTextSearchTool(const CString& searchString);

public:
	bool Match(NewsItemPtr& item);

private:
	CString m_searchString;

	friend class CSearchTool;
};

class CRegExpSearchTool : public CSearchTool
{
protected:
	CRegExpSearchTool(const CString& searchString);

public:
	bool Match(NewsItemPtr& item);

private:
	CAtlRegExp<> m_re;

	friend class CSearchTool;
};