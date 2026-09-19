#include "StdAfx.h"
#include <atltime.h>
#include "NewsFilter.h"
#include "ExceptionBase.h"
#include "FeedmanagerErrorCode.h"
#include "GMTimeLib.h"

CNewsFilter::CNewsFilter(NewsStatus ns, DateRange dr, SortItems sort, bool desc)
: m_newsStatus(ns), m_dateRange(dr), m_sort(sort), m_bSortDesc(desc)
{
}

CNewsFilter::CNewsFilter(void)
: m_newsStatus(All), m_dateRange(AllDates), m_sort(SortNone), m_bSortDesc(true)
{
}

CNewsFilter::~CNewsFilter(void)
{
}

CString CNewsFilter::GenOrderByClause()
{
	CString orderBy;
	switch(m_sort)
	{
	case SortNone:
		break;
	case SortByTitle:
		orderBy = _T(" title COLLATE GNUNICODE ");
		break;
	case SortByChannel:
		orderBy = _T(" feed_id ");
		break;
	case SortByDate:
		orderBy = _T(" retrieved ");
		break;
	case SortByAuthor:
		orderBy = _T(" author ");
	}

	if(orderBy.GetLength() && m_bSortDesc)
		orderBy.Append(_T(" desc "));

	return orderBy;
}

CString CNewsFilter::GenerateWhereClause()
{
	CString where = GenDateWhereClause();
	CString statusWhere = GenStatusWhereClause();
	if(statusWhere.GetLength()>0)
	{
		if(where.GetLength()>0)
			where += _T(" AND ");

		where += statusWhere;
	}
	return where;
}

CString CNewsFilter::GenStatusWhereClause()
{
	switch(m_newsStatus)
	{
	case All:
		return _T("");
	case Unread:
		return _T(" readtime is null ");
	case Flagged:
		return _T(" mark > 0 ");
	default:
		throw CExceptionBase(ERR_FM_GENERICERR, _T("Unknown status filter"));
	}

	return "";
}

CString CNewsFilter::GenDateWhereClause()
{
	CTime time = CGMTimeHelper::GetCurrentSysTime();

	switch(m_dateRange)
	{
	case Today:
		time = CTime(time.GetYear(), time.GetMonth(), time.GetDay(), 0,0,0,0);
		break;
	case SinceYesterday:
		time = CGMTimeHelper::DaysAgo(time, 1);
		break;
	case Last7Days:
		time = CGMTimeHelper::DaysAgo(time, 7);
		break;
	case Last30Days:
		time = CGMTimeHelper::DaysAgo(time, 30);
		break;
	case AllDates:
		return _T("");
	default:
		throw CExceptionBase(ERR_FM_GENERICERR, _T("Unknown date range"));
	}

	CString where;
	where.Format(_T(" retrieved>=%d "), (int)time.GetTime());

	return where;
}

