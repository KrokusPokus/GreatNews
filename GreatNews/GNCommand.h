#pragma once

#include "atldispa.h"

class CGNDispatchCommand :
	public IDispDynImpl<CGNDispatchCommand>
{
public:
	CGNDispatchCommand();

public:
	BEGIN_DISPATCH_MAP(CGNDispatchCommand)
		DISP_METHOD1(&CGNDispatchCommand::GotoPage, VT_BOOL, VT_I2)
		DISP_METHOD0(&CGNDispatchCommand::ShowAllItems, VT_BOOL)
		DISP_METHOD0(&CGNDispatchCommand::NextUnread, VT_BOOL)
		DISP_METHOD0(&CGNDispatchCommand::UpdateAll, VT_BOOL)
		DISP_METHOD0(&CGNDispatchCommand::AddChannel, VT_BOOL)
		DISP_METHOD0(&CGNDispatchCommand::ImportChannels, VT_BOOL)
		DISP_METHOD1(&CGNDispatchCommand::GotoChannel, VT_BOOL, VT_I4)
		DISP_METHOD1(&CGNDispatchCommand::LabelItem, VT_BOOL, VT_I4)
		DISP_METHOD1(&CGNDispatchCommand::EmailItem, VT_BOOL, VT_I4)
		DISP_METHOD1(&CGNDispatchCommand::AddDelicious, VT_BOOL, VT_I4)
		DISP_METHOD1(&CGNDispatchCommand::AddFurl, VT_BOOL, VT_I4)
		DISP_METHOD1(&CGNDispatchCommand::BlogThis, VT_BOOL, VT_I4)
	END_DISPATCH_MAP()

	VARIANT_BOOL __stdcall GotoPage(SHORT in);
	VARIANT_BOOL __stdcall ShowAllItems(void);
	VARIANT_BOOL __stdcall NextUnread(void);
	VARIANT_BOOL __stdcall UpdateAll(void);
	VARIANT_BOOL __stdcall ImportChannels(void);
	VARIANT_BOOL __stdcall AddChannel(void);
	VARIANT_BOOL __stdcall GotoChannel(int channelID);
	VARIANT_BOOL __stdcall LabelItem(int itemID);
	VARIANT_BOOL __stdcall EmailItem(int itemID);
	VARIANT_BOOL __stdcall AddDelicious(int itemID);
	VARIANT_BOOL __stdcall AddFurl(int itemID);
	VARIANT_BOOL __stdcall BlogThis(int itemID);

public:
	void SetNotifyWnd(HWND wnd);

private:
	HWND m_wndNotify;
	void PostCommand(UINT cmdID);
};
