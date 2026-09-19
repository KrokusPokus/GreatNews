#pragma once

#include "loki\SmartPtr.h"

class CBlogTool;
typedef Loki::SmartPtr<CBlogTool, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> BlogToolPtr;
typedef std::vector<BlogToolPtr> BlogToolVector;

class CBlogTool
{
public:
	CBlogTool(void);
	CBlogTool(LPCTSTR name, LPCTSTR type, LPCTSTR url);
	virtual ~CBlogTool(void);

public:
	static size_t GetAllBlogTools(BlogToolVector& allTools);

public:
	void Save();
	void Delete();

public:
	UINT_PTR m_id;
	CString m_name;
	CString m_type;
	CString m_url;
};

bool operator == (BlogToolPtr& type, LPCTSTR name);

