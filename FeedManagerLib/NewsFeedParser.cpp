#include "StdAfx.h"
#include "NewsFeedParser.h"
#include "AsyncDownloadManager.h"
#include "NewsFeedParserImplMSXML.h"
#include "NewsFeedParserImplPugXml.h"
#include "GNUtil.h"

bool CNewsFeedParser::m_bKeepOriginalLink = false;

CNewsFeedParser::CNewsFeedParser(void)
{
}

CNewsFeedParser::~CNewsFeedParser(void)
{
}

CString CNewsFeedParser::GetFormatName(int format)
{
	switch(format)
	{
	case RSS200:
		return _T("RSS 2.0");
	case RSS09x:
		return _T("RSS 0.91");
	case RSS100:
		return _T("RSS 1.0");
	case ATOM030:
		return _T("Atom 0.3");
	case ATOM100:
		return _T("Atom 1.0");
	}

	return _T("Unknown Format");
}

CNewsFeedParser::RSSFormat CNewsFeedParser::GetFormat(const CString& rootTagName, CString version)
{
	if(rootTagName == _T("rss"))
	{
		//if(version.GetLength()==0)
		//	version = _T("2.0"); // default to 2.0 if the channel doesn't specify one

		if(version.Find(_T("2"))==0)
		{
			return RSS200;
		}
		else if(version.Find(_T("0.9")) == 0)
		{
			return RSS09x;
		}
		else // if we don't know what it is, try RSS2.0
		{
			return RSS200;
		}
	}
	else if(rootTagName == _T("rdf:RDF"))
	{
		return RSS100;
	}
	else if(rootTagName == _T("feed"))
	{
		if(version.GetLength() == 0)
			return ATOM100;
		else
			return ATOM030;
	}
	
	return Unknown;
}

NewsFeedParserPtr CNewsFeedParser::CreateParserFromURL(LPCTSTR url, LPCTSTR user, LPCTSTR password, ParserImplType implType)
{
	CString normalizedUrl = CGNUtil::NormalizeURL(url);
	DownloadContextPtr context(new CDownloadContext());
	context->SetDownloadParam(NULL, normalizedUrl, user, password);
	if(!CGNSingleton<CAsyncDownloadManager>::Instance()->Download(context))
	{
		throw CExceptionBase(ERR_NFP_INVALIDNEWSFEED, _T("Cannot connect to channel"));
	}

	NewsFeedParserPtr spParser = CreateParser(url, context->m_data, implType);
	return spParser;
}

NewsFeedParserPtr CNewsFeedParser::CreateParser(LPCTSTR url, std::vector<BYTE>& stream, ParserImplType implType)
{
	NewsFeedParserPtr spParser;

	if(implType == ImplMSXML)
	{
		spParser = CNewsFeedParserImplMSXML::CreateParser(stream);
	}
	else if(implType == ImplPug)
	{
		spParser = CNewsFeedParserImplPugXml::CreateParser(stream);
	}
	else // auto decide
	{
		// try to create parser use MSXML first	
		try
		{
			spParser = CNewsFeedParserImplMSXML::CreateParser(stream);
		}
		catch(_com_error& e)
		{
			DBG_UNREFERENCED_LOCAL_VARIABLE(e);
			ATLTRACE(e.ErrorMessage());
		}
		catch(CExceptionBase& e)
		{
			DBG_UNREFERENCED_LOCAL_VARIABLE(e);
			ATLTRACE(e.GetErrorMsg());
		}
		catch(...)
		{
			ATLTRACE(_T("Unknown error when creating parser...\n"));
		}

		// if failed to use MSXML, we try to use PugXml parser
		if(spParser == NULL)
			spParser = CNewsFeedParserImplPugXml::CreateParser(stream);
	}

	// we have to fix the website url as some feeds use relative path
	if(spParser != NULL)
	{
		spParser->m_newsFeed.m_url = url;
		spParser->m_newsFeed.NormalizeHomeUrl();
	}

	return spParser;
}
