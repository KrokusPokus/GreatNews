#pragma once

#include "FeedTreeItem.h"

class CWatchRootTreeItem :
	public CFeedTreeItem
{
public:
	CWatchRootTreeItem(void);
	~CWatchRootTreeItem(void);

public:
	virtual Type GetType() {return Watch;}
	virtual ULONG_PTR GetId() { return 0;}

	virtual CString GetName();
	virtual NewsSourcePtr GetNewsSource();
	virtual BatchContentGeneratorPtr GetContentGenerator();
	virtual int GetIcon();
	virtual INT_PTR OnProperties(HWND hWndParent) {return IDCANCEL;} 
};
