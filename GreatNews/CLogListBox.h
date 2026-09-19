#pragma once

class CLogListBox : public CListBox
{
public:
	CLogListBox() : m_nExtent(0) {}

	void operator=(HWND hWnd) {CListBox::operator=(hWnd);}

	int AddMessage(const CString& msg)
	{
		int n = AddString(msg);
		EnsureVisible(n);

		CDC dc(GetDC());
		CSize size;
		if(dc.GetTextExtent(msg,-1, &size) && size.cx > m_nExtent)
		{
			m_nExtent = size.cx;
			SetHorizontalExtent(m_nExtent);
		}
		return n;
	}
	
	void EnsureVisible(int n)
	{
		int top = GetTopIndex();
		if(n >= top + VisibleItems())
		{
			SetTopIndex(max(0, n-VisibleItems()+1));
		}
	}

	int VisibleItems()
	{
		CRect rc;
		GetClientRect(rc);
		return (rc.bottom-rc.top)/GetItemHeight(0);
	}

private:
	int m_nExtent;
};

