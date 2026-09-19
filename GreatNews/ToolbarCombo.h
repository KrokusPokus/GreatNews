#pragma once

#include "resource.h"


class CToolbarCombo : public CWindowImpl<CToolbarCombo, CComboBox>
{
public:
	DECLARE_WND_SUPERCLASS(_T("GNToolbarComboBox"), CComboBox::GetWndClassName())

	BEGIN_MSG_MAP(CToolbarCombo)
		DEFAULT_REFLECTION_HANDLER() 
	END_MSG_MAP()
};

