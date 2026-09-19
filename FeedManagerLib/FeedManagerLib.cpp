#include "StdAfx.h"
#include "FeedManagerLib.h"
#include "FeedManager.h"
#include "FeedManagerLibHelper.h"
#include "Singleton.h"

namespace FeedManagerLib
{

void CompactDatabase()
{
	// vacuum cannot run within a transaction!
	CFeedManagerLibHelper::DBExec(_T("VACUUM;"), CFeedManagerLibHelper::NoTransaction);
	CFeedManagerLibHelper::DBExec(_T("ANALYZE;"), CFeedManagerLibHelper::NoTransaction);
}

CString GetDbVersion()
{
	CString version;

	try
		{
		CppSQLite3DB db;
		CFeedManager::OpenDatabase(db);
		CppSQLite3Query q;

		q = db.execQuery(_T("select value from configuration where key = 'db_version';"));
		if(!q.eof())
		{
			version.Format(_T("1.0.0.%d"), q.getIntField(0));
		}
		q.finalize();
		db.close();
	}
	catch(CppSQLite3Exception&)
	{
	}

	if (version.IsEmpty()) try
	{
		CppSQLite3DB db;
		CFeedManager::OpenDatabase(db);
		CppSQLite3Query q;

		q = db.execQuery(_T("select db_ver from configurations;"));
		if(!q.eof())
		{
			version.Format(_T("1.0.0.%d"), q.getIntField(0));
		}
		q.finalize();

		db.close();
	}
	catch(CppSQLite3Exception&)
	{
	}

	return version;
}

bool CheckLastShutdown()
{
	CppSQLite3DB db;

	CFeedManager::OpenDatabase(db);

	int lastStart, lastShutdown;
	lastStart = lastShutdown = 0;

	try
	{
		CppSQLite3Query q;

		q = db.execQuery(_T("select value from configuration where key = 'last_start'"));
		if(!q.eof())
		{
			lastStart = q.getIntField(0);
		}
		q.finalize();

		q = db.execQuery(_T("select value from configuration where key = 'last_shutdown'"));
		if(!q.eof())
		{
			lastShutdown = q.getIntField(0);
		}
		q.finalize();
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_CANNOTOPENDB, e.errorMessage());
	}

	return (lastShutdown>=lastStart);
}

void MarkTime(TimeMark mark)
{
	CppSQLite3DB db;
	CFeedManager::OpenDatabase(db);

	try
	{
		CTime now = CTime::GetCurrentTime();
		CString sql;
		sql.Format(_T("update configuration set value = %d where key = '%s'"),
			(int)now.GetTime(),
			(mark==StartupTime ? _T("last_start") : _T("last_shutdown")));

		db.execDML(sql);
	}
	catch(CppSQLite3Exception&)
	{
	}
}

#ifdef _DEBUG
CString DebugSql(const CString& sql)
{
	try
	{
		CppSQLite3DB db;
		CFeedManager::OpenDatabase(db);
		
		CString result = _T("\
<html><head><META http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\
<style>\
body {	font: 12px; 'Lucida Grande', Geneva, Arial, Verdana, sans-serif; } \
table { font: 12px 'Lucida Grande', Geneva, Verdana, Arial, sans-serif; border-collapse: collapse; border: black 1px solid; }\
th, td { border: black 1px solid; }\
</style>\
</head>\
<body><h3>");
		
		result += sql;
		result += _T("</h3><p><table>");

		CppSQLite3Query q = db.execQuery(sql);
		result += _T("<tr>");
		for (int fld = 0; fld < q.numFields(); ++fld)
		{
			result.AppendFormat(_T("<th>%s</th>"), q.fieldName(fld));
		}
		result += _T("</tr>");

		while(!q.eof())
		{
			result += _T("<tr>");
			for (int fld = 0; fld < q.numFields(); ++fld)
			{
				result.AppendFormat(_T("<td>%s</td>"), q.getStringField(fld));
			}
			result += _T("</tr>");
			q.nextRow();
		}
		result += _T("</table></body></html>");
		q.finalize();

		return result;
	}
	catch(CppSQLite3Exception& e)
	{
		throw CExceptionBase(ERR_FM_GENERICERR, e.errorMessage());
	}

	return _T("");
}
#endif

} // namespace
