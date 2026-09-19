#pragma once

#define CATCH_ALL_ERROR() \
	catch(const CExceptionBase& e)\
	{\
		::MessageBox(::GetActiveWindow(), e.GetErrorMsg(), _T("GreatNews"), MB_OK);\
	}\
	catch(const _com_error& e)\
	{\
		::MessageBox(::GetActiveWindow(), e.ErrorMessage(), _T("GreatNews"), MB_OK);\
	}\
	catch(...)\
	{\
		::MessageBox(::GetActiveWindow(), _T("Unknown error!"), _T("GreatNews"), MB_OK);\
	}

#define FORWARD_COMMANDS() \
	if(uMsg == WM_COMMAND && GetParent() != NULL) \
	{ \
		::PostMessage(GetParent(), uMsg, wParam, lParam); \
		return TRUE; \
	}

#define FORWARD_COMMAND_ID(id) \
	if(uMsg == WM_COMMAND && id == LOWORD(wParam) && GetParent() != NULL) \
	{ \
		::PostMessage(GetParent(), uMsg, wParam, lParam); \
		return TRUE;\
	}

#define FORWARD_COMMAND_ID_RANGE(idLow, idHigh) \
	if(uMsg == WM_COMMAND && idLow <= LOWORD(wParam) && LOWORD(wParam)<=idHigh && GetParent() != NULL) \
	{ \
		::PostMessage(GetParent(), uMsg, wParam, lParam); \
		return TRUE;\
	}

#define REFLECT_NOTIFY_CODE(cd) \
	if(uMsg == WM_NOTIFY && cd == ((LPNMHDR)lParam)->code) \
	{ \
		bHandled = TRUE; \
		lResult = ReflectNotifications(uMsg, wParam, lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

#define REFLECT_MESSAGE(msg) \
	if(uMsg == msg) \
	{ \
		bHandled = TRUE; \
		lResult = ReflectNotifications(uMsg, wParam, lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}


#ifdef DEBUG
inline void PrintID(ULONG n)
{
	ATLTRACE("id = %d\n", n);
}
#endif