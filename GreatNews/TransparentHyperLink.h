#pragma once

class CTransparentHyperLink : public CHyperLinkImpl<CTransparentHyperLink>
{
public:
	DECLARE_WND_CLASS(_T("GN_HyperLink"))

	CTransparentHyperLink(void);
	~CTransparentHyperLink(void);

public:
	void DoEraseBackground(CDCHandle dc);
};
