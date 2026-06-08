// TemplateListCtrl.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "TemplateListCtrl.h"


// CTemplateListCtrl

IMPLEMENT_DYNAMIC(CTemplateListCtrl, CCustomListCtrl)

CTemplateListCtrl::CTemplateListCtrl(utl::ITemplateList& list, bool check)
	: list_(list)
	, check_(check)
{

}

CTemplateListCtrl::~CTemplateListCtrl()
{
}

void CTemplateListCtrl::Init()
{
	Apply();
	SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	EnsureVisible(0, FALSE);
}

void CTemplateListCtrl::Apply()
{
	SetItemCountEx(static_cast<int>(list_.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
}

BOOL CTemplateListCtrl::IsSelectLast()
{
	if (list_.empty()) { return TRUE; }
	int i = GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if ((list_.size() - 1) == i)
	{
		return TRUE;
	}
	return FALSE;
}

void CTemplateListCtrl::SelectLast()
{
	int count = list_.size();
	SetItemState(count - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	EnsureVisible(count - 1, FALSE);
}

int CTemplateListCtrl::GetIndex()
{
	return GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
}

void CTemplateListCtrl::AddItem()
{
	list_.add();

	Apply();
	SelectLast();
}

void CTemplateListCtrl::CopyItem()
{
	int i = GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (list_.copy(i))
	{
		Apply();
		SelectLast();
	}
}


void CTemplateListCtrl::DelItem()
{
	int i = GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (list_.del(i))
	{
		Apply();

		if (list_.size() > i)
		{
			SetItemState(i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			EnsureVisible(i, FALSE);
		}
		RedrawWindow();
	}
}

void CTemplateListCtrl::UpItem()
{
	int i = GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (list_.up(i))
	{
		SetItemState(i - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		EnsureVisible(i - 1, FALSE);

		RedrawWindow();
	}
}

void CTemplateListCtrl::DownItem()
{
	int i = GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (list_.down(i))
	{
		SetItemState(i + 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		EnsureVisible(i + 1, FALSE);

		RedrawWindow();
	}
}


BEGIN_MESSAGE_MAP(CTemplateListCtrl, CCustomListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &CTemplateListCtrl::OnLvnGetdispinfo)
	ON_NOTIFY_REFLECT(NM_CLICK, &CTemplateListCtrl::OnNMClick)
END_MESSAGE_MAP()



// CTemplateListCtrl メッセージ ハンドラー
void CTemplateListCtrl::PreSubclassWindow()
{
	if (check_)
	{
		SetExtendedStyle(GetExtendedStyle() |
			LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP | LVS_EX_DOUBLEBUFFER | LVS_EX_CHECKBOXES);
		SetCallbackMask(LVIS_STATEIMAGEMASK);
	}
	else
	{
		SetExtendedStyle(GetExtendedStyle() |
			LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP | LVS_EX_DOUBLEBUFFER);
	}

	Init();

	CCustomListCtrl::PreSubclassWindow();
}

void CTemplateListCtrl::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if (!pDispInfo) { return; }
	int i = pDispInfo->item.iItem;

	if (check_ && (pDispInfo->item.mask & LVIF_STATE))
	{
		pDispInfo->item.state = LVIS_STATEIMAGEMASK;

		if (list_.check(i))
		{
			pDispInfo->item.stateMask = INDEXTOSTATEIMAGEMASK(2);
		}
		else
		{
			pDispInfo->item.stateMask = INDEXTOSTATEIMAGEMASK(1);
		}
	}

	if (pDispInfo->item.mask & LVIF_TEXT)
	{
		CString csLine;
		csLine = list_.text(i, pDispInfo->item.iSubItem).c_str();

		wcsncpy_s(
			pDispInfo->item.pszText,
			pDispInfo->item.cchTextMax,
			csLine, _TRUNCATE);
	}

	if (pResult) { *pResult = 0; }
}

void CTemplateListCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	CPoint pt;
	::GetCursorPos(&pt);
	ScreenToClient(&pt);

	LVHITTESTINFO lvHit;
	lvHit.pt = pt;
	int i = SubItemHitTest(&lvHit);

	if (lvHit.iSubItem == 0 && lvHit.flags & LVHT_ONITEMSTATEICON)
	{
		list_.toggle(i);
		CRect rc;
		GetItemRect(i, &rc, LVIR_BOUNDS);
		InvalidateRect(&rc, FALSE);
	}

	if (pResult) { *pResult = 0; }
}

