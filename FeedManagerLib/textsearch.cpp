/*

	Super fast linear text search algorithms:
	searchi = search ignore case
	search = search case sensitive
	searchiw = search ignore case words only (e.g. words delimited by whitespace only,
				not words within words)
	searchw() = search case sensitive words only
	
	All functions return the number of matches for keyword in buffer, or -1 on error.

	by James Buchanan
	No license ristrictions on this code.
	
	Email: jamesb@northnet.com.au

*/

#include "StdAfx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "textsearch.h"

namespace textsearch
{

bool searchi(LPCTSTR buffer, LPCTSTR keyword)
{
	LPTSTR p = StrStrI(buffer, keyword);
	return (p!=NULL);
}

bool search(LPCTSTR buffer, LPCTSTR keyword)
{
	LPTSTR p = StrStr(buffer, keyword);
	return (p!=NULL);	
}

bool IsWholeWord(LPCTSTR pBegin, LPCTSTR pFound, int keywordLen)
{
	if((pFound == pBegin || *(pFound-1)>255 || !isalpha(*(pFound-1)))  // not have alphabetic in front
		&& (*(pFound+keywordLen)>255 || !isalpha(*(pFound+keywordLen))))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool searchiw(LPCTSTR buffer, LPCTSTR keyword)
{
	LPTSTR p = StrStrI(buffer, keyword);
	while(p != NULL)
	{
		int keywordLen = (int)_tcslen(keyword);
		if(IsWholeWord(buffer, p, keywordLen))
			break;
		else
			p = StrStrI(p+keywordLen, keyword);
	}
	return (p!=NULL);
}

bool searchw(LPCTSTR buffer, LPCTSTR keyword)
{
	LPTSTR p = StrStr(buffer, keyword);
	while(p != NULL)
	{
		int keywordLen = (int)_tcslen(keyword);
		if(IsWholeWord(buffer, p, keywordLen))
			break;
		else
			p = StrStr(p+keywordLen, keyword);
	}

	return (p!=NULL);	
}


} // end namespace
