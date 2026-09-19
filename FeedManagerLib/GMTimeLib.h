#pragma once

#include <atltime.h>


#define DATEFMT_DISPLAY		_T("%x %I:%M %p")
#define DATEFMT_SQL			_T("%Y/%m/%d %H:%M:%S")

class CGMTimeHelper
{
public:
	static CTime DaysAgo(CTime& time, int nDays);
	static CTime GetCurrentSysTime();
	static CTime GmtToLocalTime(CTime& time);
	static CTime ParseDate(LPCTSTR);		// parse any ISO 8601 or RFC 822 format date
	static CTime StringToCTime(LPCTSTR);	// YYYY/MM/DD hh:mm:ss
	static CString FormatSqlDate(CTime& time);
	static CString FormatDisplayDate(CTime& time);
	static CString FormatRFC822Date(CTime& time);

public:
	static CTime RssDateToCTime(LPCTSTR);
	static CTime dcDateToCTime(LPCTSTR);

public: // public properties
	static LPCTSTR day[];
	static LPCTSTR mon[];
	static LPCTSTR zone[];
	static int zone_bias[];
	static LPCTSTR military_zone[];
	static int military_zone_bias[];
	static bool m_bShowTimeIn24Hr;

};
