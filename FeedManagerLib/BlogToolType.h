#pragma once

#include "loki\SmartPtr.h"
#include "NewsItem.h"

class CBlogToolType;
typedef Loki::SmartPtr<CBlogToolType, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> BlogToolTypePtr;
typedef std::vector<BlogToolTypePtr> BlogToolTypeVector;

typedef LPCTSTR (CALLBACK* GET_NAME_PROC)(void);
typedef void (CALLBACK* BLOG_THIS_PROC)(LPCTSTR, LPCTSTR);

class CBlogToolType
{
public:
	enum Type
	{
		Unknown,
		URL,
		Plugin,
		GenericExternal
	};

	CBlogToolType(void);
	CBlogToolType(LPCTSTR name, LPCTSTR url, Type type=URL);
	CBlogToolType(LPCTSTR filePath);
	~CBlogToolType(void);

	virtual void BlogThis(NewsItemPtr& item, LPCTSTR pluginOptions);

public:
	static size_t GetAllBlogToolTyes(BlogToolTypeVector& m_allTypes, const CString& pluginPath);
	static bool IsGreatNewsPlugin(LPCTSTR filePath);

public:
	CString m_name;
	CString m_defaultURL;
	Type m_type;

	HINSTANCE m_hPlugin; 
	GET_NAME_PROC m_pGetNameProc;
	BLOG_THIS_PROC m_pBlogThisProc;
};

class CBlogToolGenericExternalType : public CBlogToolType
{
public:
	CBlogToolGenericExternalType();
	virtual void BlogThis(NewsItemPtr& item, LPCTSTR pluginOptions);
};


//bool operator == (BlogToolTypePtr& type, LPCTSTR name);

