#include "StdAfx.h"
#include "GNCommandParser.h"
#include "resource.h"

#include <vector>

CGNCommandParser::CGNCommandParser(void)
	: m_wndNotify(NULL)
{
}

CGNCommandParser::~CGNCommandParser(void)
{
}


void CGNCommandParser::SetNotifyWnd(HWND wnd)
{
	m_wndNotify = wnd;
}

// e.g.: #GreatNewsTag_GotoPage_-3
bool CGNCommandParser::ProcessCommand(CString strCommand)
{
	int pos = strCommand.Find(_T("#GreatNewsTag"));
	if(pos==-1)
		return false;
	strCommand = strCommand.Mid(pos);

	try
	{
		std::vector<CString> commands;
		int curPos = 0;
		CString token = strCommand.Tokenize(_T("_"), curPos);
		while( token != "")
		{
			commands.push_back(token);
			token = strCommand.Tokenize(_T("_"), curPos);
		}

		// sanity check
		if(commands.size() <2 || !::IsWindow(m_wndNotify))
		{
			// no idea what it is
			ATLASSERT(false);
			return true; // ignore it
		}

		CString cmd = commands[1];
		
		if(cmd == _T("GotoPage"))
		{
			int page=_ttol(commands[2]);
			::PostMessage(m_wndNotify, WM_COMMAND, MAKELONG(ID_IE_GOTO_PAGE, page), 0);
		}
		else if(cmd == _T("NextUnread"))
		{
			::PostMessage(m_wndNotify, WM_COMMAND, ID_VIEW_NEXTUNREADNEWS, 0);
		}
		else if(cmd == _T("AddChannel"))
		{
			::PostMessage(m_wndNotify, WM_COMMAND, ID_CHANNEL_ADDCHANNEL, 0);
		}
		else if(cmd == _T("UpdateAll"))
		{
			::PostMessage(m_wndNotify, WM_COMMAND, ID_CHANNEL_UPDATEALLCHANNEL, 0);
		}
		else if(cmd == _T("ImportChannels"))
		{
			::PostMessage(m_wndNotify, WM_COMMAND, ID_TOOLS_IMPORT, 0);
		}
		else if(cmd == _T("ShowAllItems"))
		{
			::PostMessage(m_wndNotify, WM_COMMAND, ID_FILTER_SHOWALL, 0);
		}
		else if(cmd == _T("LabelItem"))
		{
			int itemID = _ttol(commands[2]);
			::PostMessage(m_wndNotify, WM_COMMAND, CMD_ID_LABELITEM, (LPARAM)itemID);
		}
		else if(cmd == _T("EmailItem"))
		{
			int itemID = _ttol(commands[2]);
			::PostMessage(m_wndNotify, WM_COMMAND, CMD_ID_EMAILITEM, (LPARAM)itemID);
		}
		else if(cmd == _T("AddDelicious"))
		{
			int itemID = _ttol(commands[2]);
			::PostMessage(m_wndNotify, WM_COMMAND, CMD_ID_ADDDELICIOUS, (LPARAM)itemID);
		}
		else if(cmd == _T("AddFurl"))
		{
			int itemID = _ttol(commands[2]);
			::PostMessage(m_wndNotify, WM_COMMAND, CMD_ID_ADDFURL, (LPARAM)itemID);
		}
		else if(cmd == _T("BlogThis"))
		{
			int itemID = _ttol(commands[2]);
			::PostMessage(m_wndNotify, WM_COMMAND, CMD_ID_BLOGTHIS, (LPARAM)itemID);
		}
		else if(cmd == _T("GotoChannel"))
		{
			int channelID = _ttol(commands[2]);
			// open the channel and auto-update
			::PostMessage(m_wndNotify, WM_COMMAND, MAKELONG(ID_IE_OPENCHANNEL,1), (LPARAM)channelID);
		}
		else if(cmd == _T("TrackComments"))
		{
			int itemID = _ttol(commands[2]);
			::PostMessage(m_wndNotify, WM_COMMAND, CMD_ID_TRACKCOMMENTS, (LPARAM)itemID);
		}
		else if(cmd == _T("ToggleRead"))
		{
			int itemID = _ttol(commands[2]);
			::PostMessage(m_wndNotify, WM_COMMAND, CMD_ID_TOGGLEREADUNREAD, (LPARAM)itemID);
		}
	}
	catch(...)
	{
	}

	return true;
}


