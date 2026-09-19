#include "StdAfx.h"
#include "resource.h"
#include "GNCommand.h"

CGNDispatchCommand::CGNDispatchCommand() 
	: m_wndNotify(NULL)
{
}

void CGNDispatchCommand::SetNotifyWnd(HWND wnd)
{
	m_wndNotify = wnd;
}

VARIANT_BOOL __stdcall CGNDispatchCommand::GotoPage(SHORT in)
{
	if(::IsWindow(m_wndNotify))
	{
		::PostMessage(m_wndNotify, WM_COMMAND, MAKELONG(ID_IE_GOTO_PAGE, in), 0);
	}

	return VARIANT_FALSE;
}

VARIANT_BOOL __stdcall CGNDispatchCommand::GotoChannel(int channelID)
{
	if(::IsWindow(m_wndNotify))
	{
		// open the channel and auto-update
		::PostMessage(m_wndNotify, WM_COMMAND, MAKELONG(ID_IE_OPENCHANNEL,1), (LPARAM)channelID);
	}

	return VARIANT_FALSE;
}

VARIANT_BOOL __stdcall CGNDispatchCommand::ShowAllItems(void)
{
	PostCommand(ID_FILTER_SHOWALL);

	return VARIANT_FALSE;

}

VARIANT_BOOL __stdcall CGNDispatchCommand::NextUnread(void)
{
	PostCommand(ID_VIEW_NEXTUNREADNEWS);
	return VARIANT_FALSE;
}

VARIANT_BOOL __stdcall CGNDispatchCommand::UpdateAll(void)
{
	PostCommand(ID_CHANNEL_UPDATEALLCHANNEL);
	return VARIANT_FALSE;
}
VARIANT_BOOL __stdcall CGNDispatchCommand::ImportChannels(void)
{
	PostCommand(ID_TOOLS_IMPORT);
	return VARIANT_FALSE;
}

VARIANT_BOOL __stdcall CGNDispatchCommand::AddChannel(void)
{
	PostCommand(ID_CHANNEL_ADDCHANNEL);
	return VARIANT_FALSE;
}

void CGNDispatchCommand::PostCommand(UINT cmdID)
{
	if(::IsWindow(m_wndNotify))
	{
		::PostMessage(m_wndNotify, WM_COMMAND, (WPARAM)cmdID, 0L);
	}
}

VARIANT_BOOL __stdcall CGNDispatchCommand::LabelItem(int itemID)
{
	if(::IsWindow(m_wndNotify))
	{
		::PostMessage(m_wndNotify, WM_COMMAND, CMD_ID_LABELITEM, (LPARAM)itemID);
	}

	return VARIANT_FALSE;
}

VARIANT_BOOL __stdcall CGNDispatchCommand::EmailItem(int itemID)
{
	if(::IsWindow(m_wndNotify))
	{
		::PostMessage(m_wndNotify, WM_COMMAND, CMD_ID_EMAILITEM, (LPARAM)itemID);
	}

	return VARIANT_FALSE;
}

VARIANT_BOOL __stdcall CGNDispatchCommand::AddDelicious(int itemID)
{
	if(::IsWindow(m_wndNotify))
	{
		::PostMessage(m_wndNotify, WM_COMMAND, CMD_ID_ADDDELICIOUS, (LPARAM)itemID);
	}

	return VARIANT_FALSE;
}

VARIANT_BOOL __stdcall CGNDispatchCommand::AddFurl(int itemID)
{
	if(::IsWindow(m_wndNotify))
	{
		::PostMessage(m_wndNotify, WM_COMMAND, CMD_ID_ADDFURL, (LPARAM)itemID);
	}

	return VARIANT_FALSE;
}

VARIANT_BOOL __stdcall CGNDispatchCommand::BlogThis(int itemID)
{
	if(::IsWindow(m_wndNotify))
	{
		::PostMessage(m_wndNotify, WM_COMMAND, CMD_ID_BLOGTHIS, (LPARAM)itemID);
	}

	return VARIANT_FALSE;
}

