#include "StdAfx.h"
#include "ContentGenerator.h"

std::vector<ULONG_PTR> CContentGenerator::m_unreadItemIDs;
std::map<ULONG_PTR, int> CContentGenerator::m_channelUnreadCounts;
CNewsFilter* CContentGenerator::m_pNewsFilter = NULL;

CContentGenerator::CContentGenerator()
{
}

CContentGenerator::~CContentGenerator()
{
}

void CContentGenerator::AddElement(MSXML2::IXMLDOMDocumentPtr& spDoc,
									MSXML2::IXMLDOMElementPtr& spElement,
									LPCTSTR elementName,
									LPCTSTR text)
{
	MSXML2::IXMLDOMElementPtr spChildElement = spDoc->createElement(elementName);
	spChildElement->text = text;
	spElement->appendChild(spChildElement);
}

void CContentGenerator::AddElementCData(MSXML2::IXMLDOMDocumentPtr& spDoc,
									MSXML2::IXMLDOMElementPtr& spElement,
									LPCTSTR elementName,
									LPCTSTR text)
{
	MSXML2::IXMLDOMElementPtr spChildElement = spDoc->createElement(elementName);
	MSXML2::IXMLDOMCDATASectionPtr spCData = spDoc->createCDATASection(text);
	spChildElement->appendChild(spCData);
	spElement->appendChild(spChildElement);
}