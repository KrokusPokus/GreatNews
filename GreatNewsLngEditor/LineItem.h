#pragma once

#include <vector>
#include "loki\SmartPtr.h"

class CLineItem
{
public:
	enum Status
	{
		Same,
		Missing,
		Deleted
	};

	CLineItem(void) : m_status(Same)
	{
	}

	CLineItem(LPCTSTR left, LPCTSTR right, Status status=Same)
		:m_left(left), m_right(right), m_status(status)
	{
	}

	~CLineItem(void)
	{
	}

public:
	CString m_left;
	CString m_right;
	Status m_status;
};

typedef Loki::SmartPtr<CLineItem, Loki::RefCountedMTAdj<Loki::ClassLevelLockable>::RefCountedMT> LineItemPtr;
typedef std::vector<LineItemPtr> LineItemVector;
