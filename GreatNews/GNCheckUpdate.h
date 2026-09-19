#pragma once

//
// Helper class to check GreatNews program update
//

#define VERSION_URL _T("http://www.curiostudio.com/version.ini")
enum VersionCheckStatus {NoNewVersion, NewVersionAvailable, Failed};

class CGNCheckUpdate
{
public:
	CGNCheckUpdate(void);
	~CGNCheckUpdate(void);

public:
	static VersionCheckStatus IsUpdateAvailable(CString& newVersion);

};
