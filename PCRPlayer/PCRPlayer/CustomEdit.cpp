// CustomEdit.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "CustomEdit.h"


// CCustomEdit

IMPLEMENT_DYNAMIC(CCustomEdit, CSimpleEdit)

CCustomEdit::CCustomEdit()
	: focus_(false)
{

}

CCustomEdit::~CCustomEdit()
{
}


BEGIN_MESSAGE_MAP(CCustomEdit, CSimpleEdit)
	ON_WM_LBUTTONDOWN()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()



// CCustomEdit メッセージ ハンドラー




void CCustomEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
	CEdit::OnLButtonDown(nFlags, point);

	if (!focus_)
	{
		SetSel(0, -1);
		focus_ = true;
	}
}


void CCustomEdit::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);

	if (pNewWnd)
	{
		focus_ = false;
	}
}
