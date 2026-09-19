#include "StdAfx.h"
#include ".\exceptionbase.h"

CExceptionBase::CExceptionBase(void) : m_errorCode(0)
{
}

CExceptionBase::~CExceptionBase(void)
{
}

CExceptionBase::CExceptionBase(UINT code, LPCTSTR msg) 
	: m_errorCode(code), m_errorMsg(msg)
{
}

UINT CExceptionBase::GetErrorCode(void) const
{
	return m_errorCode;
}

LPCTSTR CExceptionBase::GetErrorMsg(void) const
{
	return m_errorMsg;
}
