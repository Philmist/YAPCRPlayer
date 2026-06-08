#include "stdafx.h"
#include "CustomEventHandler.h"


CCustomEventHandler::CCustomEventHandler(const HandlerFunc& func)
	: ref_(0)
	, func_(func)
{
}

CCustomEventHandler::~CCustomEventHandler()
{
	//gl_.addLog(L"CCustomEventHandler::~CCustomEventHandler");
}


STDMETHODIMP CCustomEventHandler::QueryInterface(REFIID riid, void** ppvObject)
{
	*ppvObject = nullptr;

	if (::IsEqualIID(riid, IID_IUnknown) || ::IsEqualIID(riid, IID_IDispatch))
	{
		//gl_.addLog(L"CCustomEventHandler::QueryInterface");
		*ppvObject = static_cast<IDispatch*>(this);
	}
	else
	{
		return E_NOINTERFACE;
	}

	AddRef();

	return S_OK;
}

STDMETHODIMP_(ULONG) CCustomEventHandler::AddRef()
{
	return ::InterlockedIncrement(&ref_);
}

STDMETHODIMP_(ULONG) CCustomEventHandler::Release()
{
	if (::InterlockedDecrement(&ref_) == 0)
	{
		delete this;
		return 0;
	}

	return ref_;
}

STDMETHODIMP CCustomEventHandler::GetTypeInfoCount(UINT* pctinfo)
{
	*pctinfo = 0;

	return S_OK;
}

STDMETHODIMP CCustomEventHandler::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
{
	return E_NOTIMPL;
}

STDMETHODIMP CCustomEventHandler::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgDispId)
{
	return E_NOTIMPL;
}

STDMETHODIMP CCustomEventHandler::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
	//ATLTRACE(_T(__FUNCTION__) _T("\n") _T("Now custom-handling an event...\n"));
	//gl_.addLog(L"CCustomEventHandler::Invoke");
	
	// イベントのキャンセル。
	// JavaScript (JScript) で、
	// onXXX = "return false;"
	// と指定することに相当する。
	// http://support.microsoft.com/kb/246247/ja

	// 何らかのメソッドがバインドされており、さらにメソッドが true を返したとき。
	if (func_ && func_())
	{
		//ATLTRACE(_T(__FUNCTION__) _T("\n") _T("Event is cancelled.\n"));
		// ハンドル済みとしてマーク。
		pVarResult->vt = VT_BOOL;
		pVarResult->boolVal = VARIANT_FALSE;
	}
	return S_OK;
}

