// ColorButton.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "ColorButton.h"


// CColorButton

IMPLEMENT_DYNAMIC(CColorButton, CButton)

CColorButton::CColorButton()
	: color_(RGB(0, 0, 0))
{

}

CColorButton::~CColorButton()
{
}


BEGIN_MESSAGE_MAP(CColorButton, CButton)
	ON_CONTROL_REFLECT_EX(BN_CLICKED, &CColorButton::OnBnClicked)
END_MESSAGE_MAP()



// CColorButton メッセージ ハンドラー




void CColorButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* dc = CDC::FromHandle(lpDrawItemStruct->hDC);
	if (!dc) { return; }
	UINT nState = lpDrawItemStruct->itemState;

	CRect rc(lpDrawItemStruct->rcItem);
	CRect focus(rc);
	focus.DeflateRect(4, 4);

	dc->FillRect(&rc, &CBrush(color_));
	dc->DrawEdge(&rc, BDR_RAISEDOUTER, BF_RECT);

	//フォーカスを得たときや選択されているとき
	if (nState & ODS_FOCUS)
	{
		if (nState & ODS_SELECTED)
		{
			dc->DrawEdge(&rc, EDGE_SUNKEN, BF_RECT);
		}
		dc->DrawFocusRect(&focus);
	}
	else if(nState & ODS_DISABLED)//ボタンが無効であるとき
	{
		//DrawCaption(pDC, btnRect, GetDisabledColor(), strCaption);
	}
}


BOOL CColorButton::OnBnClicked()
{
	CColorDialog dlg(color_);

	dlg.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT;

	if (dlg.DoModal() == IDOK)
	{
		color_ = dlg.GetColor();

		RedrawWindow();
	}
	return FALSE;
}
