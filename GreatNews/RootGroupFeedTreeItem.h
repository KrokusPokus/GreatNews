#pragma once

#include "FeedGroupTreeItem.h"

class CRootGroupFeedTreeItem :
	public CFeedGroupTreeItem
{
public:
	CRootGroupFeedTreeItem(FeedGroupPtr feedGroup);
	~CRootGroupFeedTreeItem(void);

	virtual bool Delete();
	virtual CString GetName();
	virtual NewsSourcePtr GetNewsSource();
	virtual BatchContentGeneratorPtr GetContentGenerator();
	virtual int GetIcon();
};

