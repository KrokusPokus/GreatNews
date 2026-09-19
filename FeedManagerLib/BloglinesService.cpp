#include "StdAfx.h"
#include "BloglinesService.h"
#include "CppSQLite3.h"
#include "ExceptionBase.h"
#include "FeedManagerErrorCode.h"
#include "GNUtil.h"
#include "FeedManager.h"
#include "FeedManagerLibHelper.h"

CBloglinesService::CBloglinesService(void) : m_dontMarkRead(false)
{
}

CBloglinesService::~CBloglinesService(void)
{
}

MSXML2::IXMLDOMDocumentPtr CBloglinesService::ListChannels(LPCTSTR account, LPCTSTR password)
{
	MSXML2::IXMLDOMDocumentPtr spChannelDoc;

	try
	{
		MSXML2::IXMLHTTPRequestPtr spRequest(_T("MSXML2.XMLHTTP.3.0"));

		spRequest->open(_T("GET"), _T("http://rpc.bloglines.com/listsubs"), false, account, password);
		spRequest->send();

		if(spRequest->status == 401)
		{
			CString msg;
			msg.Format(_T("Incorrect email address or password."));
			throw CExceptionBase(ERR_FM_GENERICERR, msg);
		}
		else if(spRequest->status != 200)
		{
			CString msg;
			msg.Format(_T("Error occured while accessing bloglines.com.[%d]"), spRequest->status);
			throw CExceptionBase(ERR_FM_GENERICERR, msg);
		}

		spChannelDoc.CreateInstance("MSXML2.DOMDocument.3.0");
		spChannelDoc->validateOnParse = false; // do not validate DTD/Schema
		spChannelDoc->resolveExternals = false;
		spChannelDoc->load(spRequest->responseStream);
		CGNUtil::CheckDomError(spChannelDoc);
	}
	catch(CExceptionBase& e)
	{
		spChannelDoc = NULL;
		m_errMsg = e.GetErrorMsg();
	}
	catch(...)
	{
		spChannelDoc = NULL;
		m_errMsg = _T("Unknown error occured while downloading bloglines.com subscriptions");
	}

	return spChannelDoc;
}

bool CBloglinesService::GetMarkReadFlag()
{
	if(!LoadParameters())
		return false;

	return m_dontMarkRead;
}

void CBloglinesService::SetMarkReadFlag(bool dontMark)
{
	m_dontMarkRead = dontMark;
}

bool CBloglinesService::GetAccountInfo(CString& account, CString& password)
{
	if(!LoadParameters())
		return false;

	account = m_account;
	password = m_password;

	return true;
}

bool CBloglinesService::SetAccountInfo(const CString& account, const CString& password)
{
	m_account = account;
	m_password = password;
	
	return true;
}

bool CBloglinesService::SaveParam()
{
	CString sql;

	sql.Format(_T("update configuration set value = '%s' where key = 'bloglines_account'"), (LPCTSTR)m_account);
	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

	sql.Format(_T("update configuration set value = '%s' where key = 'bloglines_password'"), (LPCTSTR)m_password);
	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

	sql.Format(_T("update configuration set value = '%d' where key = 'bloglines_dontmarkread'"), (int)m_dontMarkRead);
	CFeedManagerLibHelper::DBExec(sql, CFeedManagerLibHelper::UseTransaction);

	return true;
}


CString CBloglinesService::GetSyncChannelUrl(int subsId)
{
	CString url;
	url.Format(_T("http://rpc.bloglines.com/getitems?s=%d&n=%d")
		,subsId
		,(m_dontMarkRead ? 0 : 1));
	return url;
}

bool CBloglinesService::LoadParameters()
{
	if(m_account.GetLength() == 0)
	{
		try
		{
			CppSQLite3DB db;
			CString sql;
			CppSQLite3Query q;

			CFeedManager::OpenDatabase(db);

			q = db.execQuery(_T("select value from configuration where key = 'bloglines_account'"));
			if(!q.eof())
				m_account = q.getStringField(0);
			q.finalize();

			q = db.execQuery(_T("select value from configuration where key = 'bloglines_password'"));
			if(!q.eof())
				m_password = q.getStringField(0);
			q.finalize();

			q = db.execQuery(_T("select value from configuration where key = 'bloglines_dontmarkread'"));
			if(!q.eof())
				m_dontMarkRead = (q.getIntField(0) != 0);
			q.finalize();

			db.close();
		}
		catch(CppSQLite3Exception&)
		{
			return false;
		}
	}

	return true;

}
