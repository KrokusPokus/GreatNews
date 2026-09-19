#pragma once

class CUIUtil
{
public:
	inline static HFONT GetDefaultiFont()
	{
		return (HFONT)::GetStockObject(SYSTEM_FONT);
	}

	static void ShowMyWindow(HWND hwnd)
	{
		// Show window if necessary
		if (!::IsWindowVisible(hwnd))
			::ShowWindow(hwnd, SW_SHOW);

		// Restore if window if minimized
		if (::IsIconic(hwnd))
			::ShowWindow(hwnd, SW_RESTORE);
		else
			::BringWindowToTop(hwnd);

		// Make this the active window
		::SetForegroundWindow(hwnd);
	}

};