#pragma once

class CNewsFilter
{
public:
	enum NewsStatus
	{
		Unread,
		Flagged,
		All
	};
	enum DateRange
	{
		Today,
		SinceYesterday,
		Last7Days,
		Last30Days,
		AllDates
	};
	enum SortItems
	{
		SortNone = 0,
		SortByTitle = 2,
		SortByChannel = 3,
		SortByDate = 4,
		SortByAuthor = 5
	};

public:
	CNewsFilter(NewsStatus ns, DateRange dr, SortItems sort, bool desc);
	CNewsFilter(void);
	~CNewsFilter(void);

public:
	CString GenerateWhereClause();
	CString GenOrderByClause();

protected:
	CString GenStatusWhereClause();
	CString GenDateWhereClause();

public:
	NewsStatus m_newsStatus;
	DateRange m_dateRange;
	SortItems m_sort;
	bool m_bSortDesc;
};
