/*
Workaround for missing export in msvcrt.dll.
This is only necessary for WDK (DDK) builds.
*/

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <time.h>

#pragma warning(disable: 4273)

errno_t __cdecl _localtime64_s(struct tm* _tm, const __time64_t *time)
{
    struct tm *tm = _localtime64(time);
    *_tm = *tm;
    return 0;
}
