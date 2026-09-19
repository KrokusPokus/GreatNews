#pragma once

#define DDX_COMBO_INDEX(nID, var) \
	if(nCtlID == (UINT)-1 || nCtlID == nID) \
	{ \
		if(!DDX_Combo_Index(nID, var, TRUE, bSaveAndValidate)) \
		return FALSE; \
	}

#define DDX_COMBO_ITEMDATA(nID, var) \
	if(nCtlID == (UINT)-1 || nCtlID == nID) \
	{ \
		if(!DDX_Combo_ItemData(nID, var, TRUE, bSaveAndValidate)) \
		return FALSE; \
	}

template <class Type>
class CMyWinDataExchange : public CWinDataExchange<Type>
{
public:
	BOOL DDX_Combo_Index(UINT nID, 
						INT_PTR& nVal, 
						BOOL bSigned, 
						BOOL bSave, 
						BOOL bValidate = FALSE, 
						int nMin = 0, 
						int nMax = 0)
	{
		Type* pT = static_cast<Type*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			nVal = ::SendMessage((HWND) pT->GetDlgItem(nID),
				CB_GETCURSEL,
				(WPARAM) 0,
				(LPARAM) 0);
			bSuccess = (nVal == CB_ERR ? false : true);	
		}
		else
		{
			ATLASSERT(!bValidate || nVal >= nMin && nVal <= nMax);
			INT_PTR iRet = 	::SendMessage((HWND)pT->GetDlgItem(nID),
				CB_SETCURSEL,
				(WPARAM) nVal,
				(LPARAM) 0);

			bSuccess = (iRet == CB_ERR ? false : true);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	// validation
		{
			ATLASSERT(nMin != nMax);
			if(nVal < nMin || nVal > nMax)
			{
				_XData data;
				data.nDataType = ddxDataInt;
				data.intData.nVal = (long)nVal;
				data.intData.nMin = (long)nMin;
				data.intData.nMax = (long)nMax;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}

	BOOL DDX_Combo_ItemData(UINT nID, 
						int& nVal, 
						BOOL bSigned, 
						BOOL bSave, 
						BOOL bValidate = FALSE, 
						int nMin = 0, 
						int nMax = 0)
	{
		Type* pT = static_cast<Type*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			INT_PTR selected = ::SendMessage((HWND) pT->GetDlgItem(nID),
				CB_GETCURSEL,
				(WPARAM) 0,
				(LPARAM) 0);
			if(selected >= 0)
			{
				nVal = (int)::SendMessage((HWND) pT->GetDlgItem(nID), CB_GETITEMDATA, selected, 0L);
				bSuccess = (nVal == CB_ERR ? false : true);	
			}
			else // nothing selected
			{
				nVal = -1;
			}
		}
		else
		{
			::SendMessage((HWND) pT->GetDlgItem(nID), CB_SETCURSEL, -1, 0L);	// clear current selection

			INT_PTR count = ::SendMessage((HWND)pT->GetDlgItem(nID),(UINT) CB_GETCOUNT,0,0);
			for(INT_PTR i = 0; i < count; ++i)
			{
				INT_PTR id = (int)::SendMessage((HWND) pT->GetDlgItem(nID), CB_GETITEMDATA, i, 0L);
				if(id == nVal)
				{
					LRESULT iRet = 	::SendMessage((HWND)pT->GetDlgItem(nID),
						CB_SETCURSEL,
						(WPARAM) i,
						(LPARAM) 0);
					bSuccess = (iRet == CB_ERR ? false : true);
				}
			}
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	// validation
		{
		}
		return bSuccess;
	}
};