#include "StdAfx.h"
#include "NewsSource.h"
#include "FeedManager.h"
#include "FeedManagerErrorCode.h"
#include "ExceptionBase.h"

CNewsSource::CNewsSource(void) : m_numOfUnread(0)
{
}

CNewsSource::~CNewsSource(void)
{
}


int CNewsSource::CheckUnread(const CString& criteria)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CString sql;
		sql.Format(_T("select count(*) from news_item where %s and readtime is null"), (LPCTSTR)criteria);
		m_numOfUnread = db.execScalar(sql);
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

	return m_numOfUnread;
}

void CNewsSource::CountUnread(std::map<ULONG_PTR, int>& mapUnread, LPCTSTR szSql)
{
	mapUnread.clear();

	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CppSQLite3Query q = db.execQuery(szSql);
		while(!q.eof())
		{
			if(q.getIntField(0) > 0) // filter some bad data
				mapUnread.insert(std::map<ULONG_PTR, int>::value_type(q.getIntField(0), q.getIntField(1)));
			q.nextRow();
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_DBERROR, e.errorMessage());
	}

}

int CNewsSource::GetUnread() const
{
	return m_numOfUnread;
}
