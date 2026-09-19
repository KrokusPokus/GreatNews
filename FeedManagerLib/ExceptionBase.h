#pragma once

class CExceptionBase
{
public:
	CExceptionBase() : m_errorCode(0) {};
	CExceptionBase(UINT code, LPCTSTR msg) : m_errorCode(code), m_errorMsg(msg) {};
	virtual ~CExceptionBase() {};

public:
	inline LPCTSTR GetErrorMsg() const { return m_errorMsg; };
	inline UINT GetErrorCode() const { return m_errorCode; };

protected:
	UINT m_errorCode;
	CString m_errorMsg;

};

class CInvalidCharacterException : public CExceptionBase
{
public:
	CInvalidCharacterException(UINT code, LPCTSTR msg) : CExceptionBase(code, msg) {};
};

class CUniqueNameException : public CExceptionBase
{
public:
	CUniqueNameException(UINT code, LPCTSTR msg) : CExceptionBase(code, msg) {};
};
