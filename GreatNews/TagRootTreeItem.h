#pragma once

#include "FeedTreeItem.h"

class CTagRootTreeItem :
	public CFeedTreeItem
{
public:
	CTagRootTreeItem(void);
	virtual ~CTagRootTreeItem(void);

public:
	virtual Type GetType() {return Group;}
	virtual ULONG_PTR GetId() { return 0;}

	virtual CString GetTreeNodeText();

	virtual CString GetName();
	virtual NewsSourcePtr GetNewsSource();
	virtual BatchContentGeneratorPtr GetContentGenerator();
	virtual int GetIcon();
	virtual INT_PTR OnProperties(HWND hWndParent) {return IDCANCEL;} 

public:
	virtual bool ShowInBold() {return false;}

};
