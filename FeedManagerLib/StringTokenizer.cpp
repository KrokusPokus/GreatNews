#include "StdAfx.h"
#include "StringTokenizer.h"

StringTokenizer::StringTokenizer(LPCTSTR p, LPCTSTR separators)
{
	m_index = 0;
	CString str = p;
	str.Trim(separators);

	CString sep = separators;
	LPTSTR pBegin = str.LockBuffer();
	LPTSTR pEnd = pBegin+str.GetLength();
	LPTSTR pTokenBegin = pBegin;
	while(pBegin < pEnd)
	{
		if(*pBegin == '"' || *pBegin == '\'')
		{
			TCHAR quote = *pBegin;
			pBegin++; // skip quote
			pTokenBegin = pBegin;
			for(;pBegin < pEnd; ++pBegin) // find closing quote
			{
				if(*pBegin == quote)
					break;
			}
			*pBegin = 0;
		}
		else
		{
			for(;pBegin < pEnd; ++pBegin)
			{
				if(sep.Find(*pBegin) >= 0)
				{
					*pBegin = 0;
					break;
				}
			}		
		}

		// found token?
		if(*pBegin == 0)
		{
			m_tokens.push_back(pTokenBegin);
			pBegin++;

			// skip additional delimiters
			for(;pBegin < pEnd; ++pBegin) 
			{
				if(sep.Find(*pBegin) < 0)
					break;
			}	

			pTokenBegin = pBegin;
		}
	}
	str.ReleaseBuffer();
}


StringTokenizer::~StringTokenizer(void)
{
}

CString StringTokenizer::NextToken()
{
	if(m_index >= m_tokens.size())
		return _T("");
	else
		return m_tokens[m_index++];

}
