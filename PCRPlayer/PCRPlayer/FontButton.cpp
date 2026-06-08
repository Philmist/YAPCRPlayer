// FontButton.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "FontButton.h"


// CFontButton

IMPLEMENT_DYNAMIC(CFontButton, CButton)

CFontButton::CFontButton()
{
	utl::createFont(font_);
}

CFontButton::~CFontButton()
{
	font_.DeleteObject();
}

void CFontButton::SetFontIndirect(const LOGFONT& lf)
{
	font_.DeleteObject();
	font_.CreateFontIndirect(&lf);
	SetFont(&font_);

	SetWindowText(lf.lfFaceName);
	RedrawWindow();
}

BOOL CFontButton::GetLogFont(LOGFONT& lf)
{
	return font_.GetLogFont(&lf);
}

BOOL CFontButton::DoModal(LOGFONT& lfont)
{
	LOGFONT lf = {};
	memset(&lf, 0, sizeof(LOGFONT));
	font_.GetLogFont(&lf);
	lf.lfFaceName[LF_FACESIZE - 1] = L'\0';

	CFontDialog dlg(&lf, CF_SCREENFONTS);
	
	if(dlg.DoModal() == IDOK)
	{
		dlg.GetCurrentFont(&lf);
		lf.lfFaceName[LF_FACESIZE - 1] = L'\0';

		font_.DeleteObject();
		font_.CreateFontIndirect(&lf);

		SetFont(&font_);
		
		SetWindowText(lf.lfFaceName);
		RedrawWindow();
		return font_.GetLogFont(&lfont);
	}
	return FALSE;
}

BEGIN_MESSAGE_MAP(CFontButton, CButton)
	//ON_CONTROL_REFLECT_EX(BN_CLICKED, &CFontButton::OnBnClicked)
END_MESSAGE_MAP()



// CFontButton メッセージ ハンドラー

void CFontButton::PreSubclassWindow()
{
	SetFont(&font_);

	LOGFONT lf = {};
	font_.GetLogFont(&lf);
	lf.lfFaceName[LF_FACESIZE - 1] = L'\0';
	SetWindowText(lf.lfFaceName);

	CButton::PreSubclassWindow();
}


void CFontButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* dc = CDC::FromHandle(lpDrawItemStruct->hDC);
	if (!dc) { return; }
	UINT nState = lpDrawItemStruct->itemState;

	CRect rc(lpDrawItemStruct->rcItem);
	CRect focus(rc);
	focus.DeflateRect(4, 4);
	
	dc->FillRect(&rc, &CBrush(GetSysColor(COLOR_BTNFACE)));
	dc->SetTextColor(RGB(0, 0, 0));
	dc->SetBkColor(RGB(255, 255, 255));
	dc->SetBkMode(TRANSPARENT);

	CString text;
	GetWindowText(text);
	dc->DrawText(text, -1, &rc, DT_NOPREFIX|DT_SINGLELINE|DT_CENTER|DT_VCENTER);
	
	dc->DrawEdge(&rc, BDR_RAISEDOUTER, BF_RECT);

	//フォーカスを得たときや選択されているとき
	if (nState & ODS_FOCUS)
	{
		if (nState & ODS_SELECTED)
		{
			dc->DrawEdge(&rc, BDR_SUNKENOUTER, BF_RECT);
		}
		dc->DrawFocusRect(&focus);
	}
	else if(nState & ODS_DISABLED)//ボタンが無効であるとき
	{
		//DrawCaption(pDC, btnRect, GetDisabledColor(), strCaption);
	}
}


/*
BOOL CFontButton::OnBnClicked()
{
	LOGFONT lf = {};
	memset(&lf, 0, sizeof(LOGFONT));
	font_.GetLogFont(&lf);
	lf.lfFaceName[LF_FACESIZE - 1] = L'\0';

	CFontDialog dlg(&lf, CF_SCREENFONTS);
	
	if(dlg.DoModal() == IDOK)
	{
		dlg.GetCurrentFont(&lf);
		lf.lfFaceName[LF_FACESIZE - 1] = L'\0';

		font_.DeleteObject();
		font_.CreateFontIndirect(&lf);

		SetFont(&font_);
		
		SetWindowText(lf.lfFaceName);
		RedrawWindow();
	}
	return FALSE;
}


*/
