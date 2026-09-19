#pragma once

class CGNCommandParser
{
public:
	CGNCommandParser(void);
	~CGNCommandParser(void);

public:
	void SetNotifyWnd(HWND wnd);
	bool ProcessCommand(CString strCommand);

private:
	HWND m_wndNotify;
	void PostCommand(UINT cmdID);
};
