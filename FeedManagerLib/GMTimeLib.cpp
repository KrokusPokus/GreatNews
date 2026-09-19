#include "StdAfx.h"
#include "GMTimeLib.h"
#include "GNUtil.h"

bool CGMTimeHelper::m_bShowTimeIn24Hr = false;


LPCTSTR CGMTimeHelper::mon[] = {_T("Jan"), _T("Feb"), _T("Mar"), _T("Apr"),
								_T("May"), _T("Jun"), _T("Jul"), _T("Aug"),
								_T("Sep"), _T("Oct"), _T("Nov"), _T("Dec")};

LPCTSTR CGMTimeHelper::day[]  =  {	_T("Sun"),
									_T("Mon"), 
									_T("Tue"),
									_T("Wed"),
									_T("Thu"),
									_T("Fri"),
									_T("Sat")};

LPCTSTR CGMTimeHelper::zone[] = {
								_T("UT"), _T("GMT"),
								_T("EST"), _T("EDT"),
								_T("CST"), _T("CDT"),
								_T("MST"), _T("MDT"),
								_T("PST"), _T("PDT")
								};
int CGMTimeHelper::zone_bias[] = {
								0,	0,
								-5,	-4,
								-6,	-5,
								-7,	-6,
								-8,	-7
								};
LPCTSTR CGMTimeHelper::military_zone[] = {_T("Z"), _T("A"), _T("M"), _T("N"), _T("Y")};
int CGMTimeHelper::military_zone_bias[] = {0, -1, -12, +1, +12};



CTime CGMTimeHelper::DaysAgo(CTime& time, int nDays)
{
	SYSTEMTIME sysTime;
	time.GetAsSystemTime(sysTime);
	DATE dtDate;
	::SystemTimeToVariantTime(&sysTime, &dtDate);

	dtDate = (long)dtDate-nDays;
	VariantTimeToSystemTime(dtDate, &sysTime);

	return CTime(sysTime);
}

CString CGMTimeHelper::FormatSqlDate(CTime& time)
{
	if(time == CTime())
		return _T("");
	else
		return time.Format(DATEFMT_SQL);
}

CTime CGMTimeHelper::GmtToLocalTime(CTime& time)
{
	if(time == 0)
		return CTime();

	SYSTEMTIME st, local;
	if(!time.GetAsSystemTime(st))
		return CTime();

	if(!SystemTimeToTzSpecificLocalTime(NULL, &st, &local))
		return CTime();

	return CTime(local);
}

CString CGMTimeHelper::FormatDisplayDate(CTime& time)
{
	if(time != 0)
	{
		CTime localTime = GmtToLocalTime(time);
		CTime currentTime = CTime::GetCurrentTime();
		if(localTime.GetYear() == currentTime.GetYear()
			&& localTime.GetMonth() == currentTime.GetMonth()
			&& localTime.GetDay() ==  currentTime.GetDay())
		{
			if(m_bShowTimeIn24Hr)
				return localTime.Format(_T("Today %H:%M"));
			else
				return localTime.Format(_T("Today %I:%M %p"));
		}
		else
		{
			SYSTEMTIME sysTime;
			localTime.GetAsSystemTime(sysTime);
			TCHAR buffer[50];
			int n = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &sysTime, NULL, buffer, 49);

			CString str;
			if(m_bShowTimeIn24Hr)
				str.Format(_T("%s %s"), buffer, localTime.Format(_T("%H:%M")));
			else
				str.Format(_T("%s %s"), buffer, localTime.Format(_T("%I:%M %p")));

			return str;
			// return localTime.Format(DATEFMT_DISPLAY);
		}
	}

	return _T("");
}


CTime CGMTimeHelper::StringToCTime(LPCTSTR str)
{
	if(_tcslen(str)==0)
		return CTime();

	// YYYY/MM/DD hh:mm:ss
	int m, d, Y, H, M, S;
	_stscanf(str, _T("%d/%d/%d %d:%d:%d"), &Y, &m, &d, &H, &M, &S);

	return CTime(Y, m, d, H, M, S); 
}

CTime CGMTimeHelper::ParseDate(LPCTSTR str)
{
	if(!str || _tcslen(str)==0)
		return CTime();
	
	try
	{
		LPCTSTR pDash = _tcschr(str,_T('-'));
		if(pDash != NULL && pDash-str<=4)
		{
			// ISO 8601 format
			// 2004-09-11T16:22:17-08:00
			// 2004-09-11T16:22Z
			return dcDateToCTime(str);
		}
		else
		{
			// RFC 822 format
			// Sat, 07 Sep 2002 09:42:31 GMT
			return RssDateToCTime(str);
		}
	}
	catch(...)
	{
		//ATLASSERT(FALSE);
		return CTime(); // don't understand the date format
	}
}

CTime CGMTimeHelper::dcDateToCTime(LPCTSTR str)
{
	// ISO 8601 format
	
	// YYYY-MM-DDThh:mm:ssTZD (eg 1997-07-16T19:20:30+01:00)
	//where:
	//
	//     YYYY = four-digit year
	//     MM   = two-digit month (01=January, etc.)
	//     DD   = two-digit day of month (01 through 31)
	//     hh   = two digits of hour (00 through 23) (am/pm NOT allowed)
	//     mm   = two digits of minute (00 through 59)
	//     ss   = two digits of second (00 through 59)
	//     s    = one or more digits representing a decimal fraction of a second
	//     TZD  = time zone designator (Z or +hh:mm or -hh:mm)

	// 2004-09-11T16:22:17-08:00
	// 2004-09-11T16:22Z

	
	int m, d, Y, H, M, S;
	m=d=Y=H=M=S=0;

	// date part
	CString strDateIn = str;
	CString date, time, zone;
	int posT, posZ;
	posT = strDateIn.FindOneOf(_T("T "));
	if(posT<0)
	{
		// only date part was found
		if(EOF == _stscanf((LPCTSTR)strDateIn, _T("%d-%d-%d"), &Y, &m, &d))
			return CTime();

		if(Y<1970)
			return CTime();
		else
			return CTime(Y, m, d, 0, 0, 0); 
	}
	date = strDateIn.Left(posT);
	if(EOF == _stscanf((LPCTSTR)date, _T("%d-%d-%d"), &Y, &m, &d))
		return CTime();

	// time part
	strDateIn = strDateIn.Right(strDateIn.GetLength()-posT-1);
	posZ = strDateIn.FindOneOf(_T("Z+-"));
	if(posZ>=0)
	{
		time = strDateIn.Left(posZ);
		zone = strDateIn.Mid(posZ);
	}
	else
		time = strDateIn;

	LPCTSTR pTime =(LPCTSTR)time;
	if(_tcschr(pTime, _T(':')) != _tcsrchr(pTime, _T(':')))
	{
		if(EOF == _stscanf(pTime, _T("%d:%d:%d"), &H, &M, &S))
			return CTime();
	}
	else
	{
		if(EOF == _stscanf(pTime, _T("%d:%d"), &H, &M))
			return CTime();
	}

	CTime parsedTime(Y, m, d, H, M, S); 

	//
	// adjust time zone to get GMT time
	//
	if(zone.GetLength())
	{
		//  TZD  = time zone designator (Z or +hh:mm or -hh:mm)
		if(zone[0] == _T('Z'))
		{
			// nothing to do
		}
		else
		{
			int hr, min;
			hr=min=0;
			if(2 == _stscanf((LPCTSTR)zone, _T("%d:%d"), &hr, &min)
				&& abs(hr)<=11
				&& min >=0 && min <= 59)
			{
				int bias = hr*60+min;
				if(bias != 0)
				{
					CTimeSpan span(0,0,bias,0);
					parsedTime = parsedTime - span;
				}
			}
		}
	}

	return parsedTime;
}

CTime CGMTimeHelper::RssDateToCTime(LPCTSTR str)
{
	// RFC 822 format
	// Sat, 07 Sep 2002 09:42:31 GMT


//5.1 SYNTAX
//
//     date-time   =  [ day "," ] date time        ; dd mm yy
//                                                 ;  hh:mm:ss zzz
//
//     day         =  "Mon"  / "Tue" /  "Wed"  / "Thu"
//                 /  "Fri"  / "Sat" /  "Sun"
//
//     date        =  1*2DIGIT month 2DIGIT        ; day month year
//                                                 ;  e.g. 20 Jun 82
//
//     month       =  "Jan"  /  "Feb" /  "Mar"  /  "Apr"
//                 /  "May"  /  "Jun" /  "Jul"  /  "Aug"
//                 /  "Sep"  /  "Oct" /  "Nov"  /  "Dec"
//
//     time        =  hour zone                    ; ANSI and Military
//
//     hour        =  2DIGIT ":" 2DIGIT [":" 2DIGIT]
//                                                 ; 00:00:00 - 23:59:59
//
//     zone        =  "UT"  / "GMT"                ; Universal Time
//                                                 ; North American : UT
//                 /  "EST" / "EDT"                ;  Eastern:  - 5/ - 4
//                 /  "CST" / "CDT"                ;  Central:  - 6/ - 5
//                 /  "MST" / "MDT"                ;  Mountain: - 7/ - 6
//                 /  "PST" / "PDT"                ;  Pacific:  - 8/ - 7
//                 /  1ALPHA                       ; Military: Z = UT;
//                                                 ;  A:-1; (J not used)
//                                                 ;  M:-12; N:+1; Y:+12
//                 / ( ("+" / "-") 4DIGIT )        ; Local differential
//                                                 ;  hours+min. (HHMM)

	
	// check our internal data structures are correct
	ATLASSERT(GN_ARRAYSIZE(zone) == GN_ARRAYSIZE(zone_bias));
	ATLASSERT(GN_ARRAYSIZE(military_zone) == GN_ARRAYSIZE(military_zone_bias));

	// TCHAR weekday[5];
	TCHAR month[30]={0};
	TCHAR timezone[10]={0};
	int m, d, Y, H, M, S;
	m=d=Y=H=M=S=0;
	
	LPCTSTR pComma = _tcschr(str,_T(','));
	if(pComma)
	{
		// remove the weekday part
		str = pComma+1;
	}

	
	if(_tcschr(str,_T(':')) == _tcsrchr(str,_T(':')))
	{
		// only one ":", no seconds part
		if(EOF == _stscanf(str, _T("%d %s %d %d:%d %s"), &d, month, &Y,
			&H, &M, timezone))
		{
			return CTime();
		}
	}
	else
	{
		if(EOF == _stscanf(str, _T("%d %s %d %d:%d:%d %s"), &d, month, &Y,
			&H, &M, &S, timezone))
		{
			return CTime();
		}
	}

	// adjust 2 digits year
	if(Y<50) Y+=2000;

	m = 0;
	for(int i=0; i<GN_ARRAYSIZE(mon);++i)
	{
		if(_tcsnicmp(mon[i],month,3)==0)
		{
			m = i+1;
			break;
		}
	}

	CTime parsedTime;
	if(Y>1969 && Y<2038 && m>0)
		parsedTime = CTime(Y, m, d, H, M, S); 
	else
		return CTime();

	//
	// now adjust timezone to get GMT time
	//
	int bias = 0;
	int timezoneLen = (int)_tcslen(timezone);
	if(timezoneLen == 1)
	{
		// military format
		for(int i=0; i<GN_ARRAYSIZE(military_zone); ++i)
		{
			if(_tcsicmp(military_zone[i], timezone)==0)
			{
				bias = military_zone_bias[i]*60;
				break;
			}
		}
	}
	else if(timezoneLen >= 4)
	{
		// ( ("+" / "-") 4DIGIT )
		LPCTSTR p = timezone;
		int sign = 1;
		if(p[0]==_T('+'))
		{
			p++;
		}
		else if(p[0]==_T('-'))
		{
			p++;
			sign = -1;
		}

		if(isdigit(*p)) // make sure it's a number
		{
			int n=_ttoi(p);
			int hr = n/100;
			int min = n%100;

			if(abs(hr)<= 23 || abs(min)<=59) // do some sanity check
			{
				bias = sign*(hr*60+min);
			}
		}
	}
	else
	{
		for(int i=0; i<GN_ARRAYSIZE(zone); ++i)
		{
			if(_tcsicmp(zone[i], timezone)==0)
			{
				bias = zone_bias[i]*60;
				break;
			}
		}
	}

	if(bias != 0)
	{
		CTimeSpan span(0,0,bias,0);
		parsedTime = parsedTime-span;
	}

	return parsedTime;
}

CString CGMTimeHelper::FormatRFC822Date(CTime& time)
{
	//Sat, 07 Sep 2002 00:00:01 GMT
	CString temp;
	temp.Format(_T("%s, %02d %s %04d %02d:%02d:%02d GMT"), 
		day[time.GetDayOfWeek()-1],
		time.GetDay(),
		mon[time.GetMonth()-1],
		time.GetYear(),
		time.GetHour(),
		time.GetMinute(),
		time.GetDay());

	return temp;
}


CTime CGMTimeHelper::GetCurrentSysTime()
{
	SYSTEMTIME sysTime;
	::GetSystemTime(&sysTime);
	return CTime(sysTime);
}
