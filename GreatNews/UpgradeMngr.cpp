#include "StdAfx.h"
#include "UpgradeMngr.h"
#include "Upgraders.h"
#include "FeedManagerLib.h"
#include <strsafe.h>

CUpgradeMngr::CUpgradeMngr(void)
{
	m_upgraders.push_back(new CUpgrader1_0_0_102());
	m_upgraders.push_back(new CUpgrader1_0_0_103());
	m_upgraders.push_back(new CUpgrader1_0_0_104());
	m_upgraders.push_back(new CUpgrader1_0_0_105());
	m_upgraders.push_back(new CUpgrader1_0_0_106());
	m_upgraders.push_back(new CUpgrader1_0_0_107());
	m_upgraders.push_back(new CUpgrader1_0_0_108());
	m_upgraders.push_back(new CUpgrader1_0_0_109());

}

CUpgradeMngr::~CUpgradeMngr(void)
{
}

CString CUpgradeMngr::GetHighestVersion()
{
	UpgraderPtr lastUpgrader = *(m_upgraders.end()-1);
	return lastUpgrader->m_finishVer;

}

CString CUpgradeMngr::GetBeginVersion()
{
	UpgraderPtr lastUpgrader = m_upgraders[0];
	return lastUpgrader->m_previousVer;
}

void CUpgradeMngr::SetDb(LPCTSTR dbPath)
{
	m_dbPath = dbPath;

	StringCchCopy(m_dbFileName, MAX_PATH, dbPath);
}

bool CUpgradeMngr::Upgrade(LPCTSTR dbPath, CString& err)
{
	SetDb(dbPath);

	CString ver = FeedManagerLib::GetDbVersion();

	std::vector<UpgraderPtr>::iterator it;
	for(it=m_upgraders.begin(); it!= m_upgraders.end(); ++it)
	{
		UpgraderPtr upgrader = *it;
		if(upgrader->m_previousVer == ver)
			break;
	}

	if(it==m_upgraders.end())
	{
		err.Format(_T("No upgrader was found for %s"), (LPCTSTR)ver);
		return false;
	}

	for(; it!= m_upgraders.end(); ++it)
	{
		UpgraderPtr upgrader = *it;
		try
		{
			if(!upgrader->Upgrade(m_dbFileName))
			{
				err = upgrader->m_errorMsg;
				return false;
			}
		}
		catch(...)
		{
			err.Format(_T("failed to upgrade data file"));
			return false;
		}
	}

	return true;
}