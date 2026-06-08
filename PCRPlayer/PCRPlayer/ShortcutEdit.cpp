// ShortcutEdit.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "ShortcutEdit.h"


// CShortcutEdit

IMPLEMENT_DYNAMIC(CShortcutEdit, CSimpleEdit)

CShortcutEdit::CShortcutEdit()
{

}

CShortcutEdit::~CShortcutEdit()
{
}


BEGIN_MESSAGE_MAP(CShortcutEdit, CSimpleEdit)
END_MESSAGE_MAP()



// CGestureEdit メッセージ ハンドラー


BOOL CShortcutEdit::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		{
			CPoint pt(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
			ClientToScreen(&pt);
			GetParent()->ScreenToClient(&pt);
			GetParent()->SendMessage(pMsg->message, pMsg->wParam, MAKELPARAM(pt.x, pt.y));
			return TRUE;
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}
