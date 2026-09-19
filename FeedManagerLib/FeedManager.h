#pragma once

#include "CppSQLite3.h"

class CFeedManager
{
public:
	CFeedManager(void);
	~CFeedManager(void);

public:
	static void OpenDatabase(CppSQLite3DB& db);
	static void Init(LPCTSTR dbFileName);

private:
	static TCHAR m_dbFileName[MAX_PATH];
};
