// IMEEdit.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "IMEEdit.h"


// CIMEEdit

IMPLEMENT_DYNAMIC(CIMEEdit, CSimpleEdit)

CIMEEdit::CIMEEdit()
{

}

CIMEEdit::~CIMEEdit()
{
}


BEGIN_MESSAGE_MAP(CIMEEdit, CSimpleEdit)
	ON_CONTROL_REFLECT(EN_SETFOCUS, &CIMEEdit::OnEnSetfocus)
	ON_CONTROL_REFLECT(EN_KILLFOCUS, &CIMEEdit::OnEnKillfocus)
END_MESSAGE_MAP()



// CIMEEdit メッセージ ハンドラー




void CIMEEdit::OnEnSetfocus()
{
	if (through()) { return; }

	HWND hwnd = GetSafeHwnd();
	HIMC himc = ImmGetContext(hwnd);

	ImmSetOpenStatus(himc, value_.state);
	ImmReleaseContext(hwnd, himc);
}


void CIMEEdit::OnEnKillfocus()
{
	if (through()) { return; }

	HWND hwnd = GetSafeHwnd();
	HIMC himc = ImmGetContext(hwnd);

	value_.state = ImmGetOpenStatus(himc);
	ImmSetOpenStatus(himc, FALSE);
	ImmReleaseContext(hwnd, himc);
}
