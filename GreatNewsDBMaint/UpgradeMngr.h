#pragma once

#include "DbUpgrader.h"
#include <vector>

class CUpgradeMngr
{
public:
	CUpgradeMngr(void);
	~CUpgradeMngr(void);

public:
	CString GetHighestVersion();
	CString GetBeginVersion();
	bool Upgrade(LPCTSTR dbPath, CString& err);

protected:
	void SetDb(LPCTSTR dbPath);
	CString m_dbPath;
	TCHAR m_dbFileName[MAX_PATH];
	std::vector<UpgraderPtr> m_upgraders;
};
