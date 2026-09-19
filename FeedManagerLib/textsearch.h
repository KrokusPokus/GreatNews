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

#ifndef __TEXTSEARCH_H
#define __TEXTSEARCH_H

namespace textsearch
{

bool searchi(LPCTSTR buffer, LPCTSTR keyword);
bool search(LPCTSTR buffer, LPCTSTR keyword);
bool searchiw(LPCTSTR buffer, LPCTSTR keyword);
bool searchw(LPCTSTR buffer, LPCTSTR keyword);

}

#endif
