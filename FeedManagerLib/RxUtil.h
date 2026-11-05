// (c) 2000 Microsoft Corporation
//Regular Expression helper class
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Classes Reference and related electronic
// documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft C++ Libraries products.

#pragma once

#define S_PARSE_FAILED 0x2
#define S_NO_MATCH 0x3
#define S_INVALID_REPLACE_EXP 0x4

#include <atlrx.h>
template <class CharTraits=CAtlRECharTraits>
class CRxUtil
{
private:

	
CAtlRegExp<CharTraits> atlRegExp;


	/*
		Helper function for s(), substitutes variables($N)
	*/
	HRESULT SubstituteVars(CString &original,CAtlREMatchContext<CharTraits> &atlRematchContext){
		CAtlREMatchContext<CharTraits>::MatchGroup tempGroup;
		HRESULT hr=S_OK;
		CString temp,variableid;
		UINT i,y;
		for(i=1;i<=atlRematchContext.m_uNumGroups;++i){
			CString number;
			number.Format(_T("%u"), i);
			temp="";
			atlRematchContext.GetMatch(i-1,&tempGroup);
			for(y=0;y+tempGroup.szStart<tempGroup.szEnd;++y){
				temp=temp+tempGroup.szStart[y];
			}

			variableid="{$";
			variableid=variableid+number+"}";
			hr=s(original,variableid,temp,true,false);
			if(hr!=S_OK&&hr!=S_NO_MATCH)
				return hr;
		}
		return hr;


	}

	/*
		Helper for FindAllInstances
	*/
	CString getFoundat(int pos,CAtlREMatchContext<CharTraits> &atlRematchContext){
		CString temp;
		temp="";
		int y;
		for(y=0;y+atlRematchContext.m_pMatches[pos].szStart<atlRematchContext.m_pMatches[pos].szEnd;++y){
				temp=temp+atlRematchContext.m_pMatches[pos].szStart[y];
			}
		return temp;
	}

public:
	typedef typename CharTraits::RECHARTYPE RECHAR;

	/*
	Regular expression substitution:

	baseStr:

		Original String,

	oldexp:
		
		Regular expression to look for inside 'baseStr'

	newexp:
	
		Instances of 'oldexp' inside 'baseStr' will be replaced by this

	global:

		Substitute all instances of 'oldexp' found or only the first one

	usingvars:
		
		Use variables in replacements, you can specify variables using $(var#) (see example)

	casesensitive:

		Case sensitive or insesitive search for 'oldexp'


	Example:

		baseStr			="Hello World! Goodbye World!"
		oldexp			="{.*?}World{.}"
		newexp			="<b>$1</b>World?"
		global			=true
		usingvars		=true
		casesensitive	=true

		the output would be:

			"<b>Hello </b>World?<b> Goodbye </b>World?"

	
	*/
	HRESULT s(CString &baseStr,CString oldexp,CString newexp,const bool &global=false,const bool &usingvars=false,const bool &casesensitive=true){

		// iterative implementation (the former recursive one overflowed the stack on large inputs)
		if(atlRegExp.Parse(oldexp,casesensitive)!=REPARSE_ERROR_OK)
			return S_PARSE_FAILED;

		CAtlREMatchContext<CharTraits> atlRematchContext;
		CString source = baseStr;	// keep the original alive while we scan it
		CString result;
		const RECHAR* p = (LPCTSTR)source;
		bool any = false;

		while(atlRegExp.Match(p,&atlRematchContext)){
			any = true;
			const RECHAR* mStart = atlRematchContext.m_Match.szStart;
			const RECHAR* mEnd = atlRematchContext.m_Match.szEnd;

			result.Append(p, (int)(mStart-p));
			if(usingvars){
				CString substitutedexpr = newexp;
				SubstituteVars(substitutedexpr,atlRematchContext);
				result += substitutedexpr;
				// SubstituteVars reuses the shared regexp object, restore ours
				if(atlRegExp.Parse(oldexp,casesensitive)!=REPARSE_ERROR_OK)
					return S_PARSE_FAILED;
			}
			else
				result += newexp;

			p = mEnd;
			if(!global)
				break;
			if(mEnd==mStart){	// empty match: avoid endless loop
				if(*p==0)
					break;
				result.AppendChar(*p);
				++p;
			}
		}

		if(!any)
			return S_NO_MATCH;

		result += p;
		baseStr = result;
		return S_OK;
	}
	/*
		Will return in 'allinstances' every occurrance it finds of 'oldexp' inside 'basestring'
	*/	
	int FindAllInstances(CAtlArray<CString> &allinstances,CString basestring,CString oldexp){
		int count=0;
		atlRegExp.Parse(oldexp);
		CAtlREMatchContext<CharTraits> atlRematchContext;
		while(atlRegExp.Match(basestring,&atlRematchContext)){
			count++;
			allinstances.Add(getFoundat(0,atlRematchContext));
			basestring=atlRematchContext.m_Match.szEnd;
		}
		return count;

	}

	/*
		Will return the occurance number 'pos' of 'oldexp' in finds inside 'basestring'
	*/
	CString GetInstanceNum(int pos,CString basestring,CString oldexp){

		atlRegExp.Parse(oldexp);
		CAtlREMatchContext<CharTraits> atlRematchContext;
		for(int i=1;i<=pos;++i){
			if(atlRegExp.Match(basestring,&atlRematchContext)){
				if(i==pos)
					return getFoundat(0,atlRematchContext);
				else basestring=atlRematchContext.m_Match.szEnd;
			}
			else break;
		}
		return "";
		

	}


	CRxUtil(void){
	}
	~CRxUtil(void){

		}



};