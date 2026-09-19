#pragma once

#include "loki\SmartPtr.h"

#include "CppSQLite3.h"


class CDbUpgrader
{
public:
	CDbUpgrader(LPCTSTR oldver, LPCTSTR newver) :
		m_previousVer(oldver),
		m_finishVer(newver)
	{
	}

	virtual ~CDbUpgrader(void)
	{
	}

public:
	virtual bool Upgrade(LPCTSTR dbFilePath) = 0;

public:
	const CString m_previousVer;
	const CString m_finishVer;
	CString m_errorMsg;

};

typedef Loki::SmartPtr<CDbUpgrader> UpgraderPtr;
