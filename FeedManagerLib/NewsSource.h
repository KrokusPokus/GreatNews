#pragma once

#include <vector>
#include <map>
#include "NewsItem.h"
#include "NewsFilter.h"

class CNewsSource
{
public:
	CNewsSource(void);
	virtual ~CNewsSource(void);

public:
	virtual int RetrieveNewsToDB()=0;
	virtual size_t GetNewsItemIDs(std::vector<ULONG_PTR>& newsIDs, CNewsFilter* pNewsFilter) const = 0;
	virtual CString GetNewsSourceName() const = 0;
	virtual int CheckUnread()=0;
	virtual int GetUnread() const;
	virtual void MarkRead()=0;
	virtual void MarkUnread()=0;
	virtual void Delete()=0;
	virtual void Save()=0;

protected:
	virtual int CheckUnread(const CString& criteria);
	static void CountUnread(std::map<ULONG_PTR, int>& mapUnread, LPCTSTR szSql);

protected:
	int m_numOfUnread;
};

typedef Loki::SmartPtr<CNewsSource, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> NewsSourcePtr;
typedef std::vector<NewsSourcePtr> NewsSourceVector;