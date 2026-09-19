// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions
//#define WINVER			0x0410
//#define _WIN32_WINDOWS	0x0410
#define _WIN32_WINNT		0x501
#define _WIN32_IE			_WIN32_IE_IE60SP2
#define _RICHEDIT_VER		0x0300

#include <atlstr.h>
#include <atlbase.h>

#if (_ATL_VER >= 0x0700)
#include <atlstr.h>
#include <atltypes.h>
#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES
#define _WTL_NO_UNION_CLASSES
#endif

#include <atlapp.h>
#include <comdef.h>

extern CAppModule _Module;

#include <atlcom.h>
#include <atlhost.h>
#include <atlwin.h>
#include <atlctl.h>

#include <atltime.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>

#include <atlmisc.h>
#include <atlcrack.h>
#include <atlsplit.h>
#include <shlobj.h>
#include <shlguid.h>
#include <atlctrlx.h>
#include <atlddx.h>

#include <atlfile.h>

#include <atltheme.h>

#include <memory>

#include "WtlExt.h"

#ifdef BEGIN_MSG_MAP
#undef BEGIN_MSG_MAP
#endif
#define BEGIN_MSG_MAP	BEGIN_MSG_MAP_EX

#if _MSC_VER>=1400
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' \
    name='Microsoft.Windows.Common-Controls'  version='6.0.0.0' \
    processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' \
    language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' \
    name='Microsoft.Windows.Common-Controls'  version='6.0.0.0' \
    processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' \
    language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' \
    name='Microsoft.Windows.Common-Controls'  version='6.0.0.0' \
    processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' \
    language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' \
    name='Microsoft.Windows.Common-Controls'  version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' \
    language='*'\"")
#endif
#endif
