#pragma once

#include <atlstr.h>
#include <vector>

class StringTokenizer
{
public:
	StringTokenizer(LPCTSTR p, LPCTSTR tokens=_T(" "));
	~StringTokenizer(void);

public:
	CString NextToken();

private:
	std::vector<CString> m_tokens;
	size_t m_index;
};