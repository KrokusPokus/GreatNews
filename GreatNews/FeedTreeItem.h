#pragma once

#include "resource.h"
#include "NewsFeed.h"
#include "FeedGroup.h"
#include "NewsWatch.h"
#include "GNResourceManager.h"

enum NodeIcon{	ChannelRootIcon=0,
				ChannelGroupIcon = 1,
				TagIcon = 3,
				WatchIcon = 4,
				ChannelIcon = 5,
				ChannelWaitingUpdatingIcon = 6,
				ChannelUpdatingIcon = 7,
				SearchChannelIcon = 8,
				SearchChannelWaitingUpdatingIcon = 9,
				SearchChannelUpdatingIcon = 10,
				BloglinesChannelIcon = 11,
				BloglinesChannelWaitingUpdateIcon = 12,
				BloglineChannelUpdatingIcon = 13,
				UpdatingOverlay = 14,
				WaitingOverlay = 15
			};

enum ChannelStatus
{
	ChannelIdle = 0,
	ChannelDownloading = 1,
	ChannelQueued = 2
};


class CFeedTreeItem
{
public:
	enum Type
	{
		Group = 0,
		Channel = 1,
		Watch = 2,
		Tag = 3
	};
	enum Color
	{
		DefaultColor = RGB(111,111,111)
	};

	CFeedTreeItem();
	virtual ~CFeedTreeItem();

	INT_PTR GetUnreadCount() { return m_unread; }
	void SetUnreadCount(INT_PTR n) { m_unread = n; }
	void AdjustUnreadCount(INT_PTR delta) { m_unread += delta; }
	void ClearUnreadCount() { m_unread = 0; };
	virtual void UpdateUIUnreadCount() { };
	virtual bool IsUnreadCountChanged() { return false; }

	virtual CString GetTreeNodeText();

	virtual Type GetType() = 0;
	virtual ULONG_PTR GetId() = 0;
	virtual bool Disabled() { return false; }  
	virtual bool Delete() { return false; }
	virtual bool Rename(LPCTSTR newName) { return false; }
	virtual CString GetName() = 0;
	virtual NewsSourcePtr GetNewsSource() = 0;
	virtual BatchContentGeneratorPtr GetContentGenerator() = 0;
	virtual int GetIcon() = 0;
	virtual INT_PTR OnProperties(HWND hWndParent) = 0;
	virtual UINT GetContextMenuID() { return m_ContextMenuID; };
	virtual bool IsItem(INT_PTR id, Type t) { return false; }
	virtual FeedGroupPtr GetFeedGroup() { return NULL; };
	virtual COLORREF GetColor() { return (COLORREF)DefaultColor; }
	virtual COLORREF GetBkColor() { return (COLORREF)DefaultColor; }

public:
	virtual bool CanDrag() { return false; }
	virtual bool CanDrop(CFeedTreeItem* pItem) { return false; }
	virtual bool CanRename() { return false; }
	virtual bool ShowInBold() { return m_unread > 0; }

public:
	bool m_bUpdateFailed;
	UINT m_ContextMenuID;

protected:
	INT_PTR m_unread;
};
