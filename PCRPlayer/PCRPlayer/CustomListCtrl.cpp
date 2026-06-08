// CustomListCtrl.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "CustomListCtrl.h"


// CCustomListCtrl

IMPLEMENT_DYNAMIC(CCustomListCtrl, CListCtrl)

CCustomListCtrl::CCustomListCtrl()
	: drag_(false)
	, stretch_(-1)
	, stripe_(false)
{

}

CCustomListCtrl::~CCustomListCtrl()
{
}


BEGIN_MESSAGE_MAP(CCustomListCtrl, CListCtrl)
	ON_WM_SIZE()
	ON_WM_NCCALCSIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CAPTURECHANGED()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CCustomListCtrl::OnNMCustomdraw)
END_MESSAGE_MAP()


void CCustomListCtrl::SizeHelper()
{
	if (stretch_ >= 0)
	{
		ShowVScrollBar();

		// カラム幅調整
		CHeaderCtrl *header = GetHeaderCtrl();
		if (header)
		{
			int fixed = 0;
			int count = header->GetItemCount();
			for (int i = 0; i < count; ++i)
			{
				if (i != stretch_)
				{// タイトルのカラム以外の幅
					fixed += GetColumnWidth(i);
				}
			}

			CRect rc;
			GetClientRect(rc);
			int width = max(0, rc.Width() - fixed);

			SetColumnWidth(stretch_, width);
		}
	}
}

void CCustomListCtrl::SetHeaderText(int index, const std::wstring& text)
{
	HDITEM item;
	item.mask = HDI_TEXT;
	item.pszText = (LPWSTR)text.c_str();
	item.cchTextMax = (int)text.size();

	CHeaderCtrl* header = GetHeaderCtrl();
	if (header)
	{
		header->SetItem(index, &item);
	}
}

// CCustomListCtrl メッセージ ハンドラー



void CCustomListCtrl::OnSize(UINT nType, int cx, int cy)
{
	CListCtrl::OnSize(nType, cx, cy);

	SizeHelper();
}


void CCustomListCtrl::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	ShowVScrollBar();

	if (stretch_ >= 0)
	{
		EnableScrollBarCtrl(SB_HORZ, FALSE);
	}

	CListCtrl::OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CCustomListCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	CListCtrl::OnMouseMove(nFlags, point); // ツールチップの表示に必要

	if (!drag_ && DragDetect(point))
	{
		drag_ = true;
		SetCapture();
		return;
	}

	if (!(nFlags & MK_LBUTTON) || nFlags & MK_SHIFT || nFlags & MK_CONTROL)
	{
		drag_ = false;
		return;
	}

	if (drag_)
	{
		int headerHeight = 0;
		int headerHit = -1;
		CHeaderCtrl* header = GetHeaderCtrl();
		if (header)
		{
			HDHITTESTINFO info = { 0 };
			info.pt = point;
			headerHit = header->HitTest(&info);

			CRect hr;
			header->GetClientRect(&hr);
			headerHeight = hr.Height();
		}

		int hit = HitTest(point);

		if (hit >= 0 && headerHit < 0)
		{// カーソルがアイテム上にある
			// 選択する
			SetItemState(hit, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			SetSelectionMark(hit);
			int count = GetItemCount();
			for (int i = 0; i < count; ++i)
			{
				if (i == hit) { continue; }
				SetItemState(i, 0, LVIS_SELECTED | LVIS_FOCUSED);
			}
		}
		else
		{
			int index = -1;
			while ((index = GetNextItem(index, LVNI_ALL | LVNI_SELECTED)) != -1)
			{
				break;
			}

			int count = GetItemCount();

			CRect rc;
			GetClientRect(&rc);

			if (index > 0 && (point.y - headerHeight) < 0)
			{// クライアント領域より上にある
				SetItemState(index - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				SetItemState(index, 0, LVIS_SELECTED | LVIS_FOCUSED);
				EnsureVisible(index - 1, FALSE);
				SetSelectionMark(index - 1);
			}
			else if ((index + 1) < count && point.y > rc.bottom)
			{
				SetItemState(index + 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				SetItemState(index, 0, LVIS_SELECTED | LVIS_FOCUSED);
				EnsureVisible(index + 1, FALSE);
				SetSelectionMark(index + 1);
			}

			// 更新された部分のみ再描画する
			UpdateWindow();
		}
	}

	//CListCtrl::OnMouseMove(nFlags, point);
}


void CCustomListCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (drag_) { ReleaseCapture(); }

	CListCtrl::OnLButtonUp(nFlags, point);
}


void CCustomListCtrl::OnCaptureChanged(CWnd *pWnd)
{
	if (drag_) { drag_ = false; }

	CListCtrl::OnCaptureChanged(pWnd);
}

void CCustomListCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	if (pNMCD == NULL) { return; }

	if (stripe_)
	{
		LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNMCD;

		// 描画サイクル開始
		if (lplvcd->nmcd.dwDrawStage == CDDS_PREPAINT)
		{// カスタムドローを行うことを通知
			if (pResult) { *pResult = CDRF_NOTIFYITEMDRAW; }
			return;
		}

		// アイテム描画
		if (lplvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
		{
			// 文字色
			lplvcd->clrText = RGB(0, 0, 0);

			if (lplvcd->nmcd.dwItemSpec % 2 != 0)
			{// 奇数行の背景
				lplvcd->clrTextBk = RGB(255, 255, 255);
			}
			else
			{
				lplvcd->clrTextBk = RGB(230, 230, 230);
			}
		}
	}

	if (pResult) { *pResult = 0; }
}


BOOL CCustomListCtrl::SetItemCountEx(_In_ int iCount, _In_ DWORD dwFlags)
{
	SetRedraw(FALSE);
	BOOL res = CListCtrl::SetItemCountEx(iCount, dwFlags);
	ShowVScrollBar();
	SetRedraw(TRUE);

	return res;
}


void CCustomListCtrl::ShowVScrollBar()
{
	if (stretch_ >= 0)
	{
		EnableScrollBarCtrl(SB_VERT, TRUE);

		if (GetItemCount() > GetCountPerPage())
		{
			EnableScrollBar(SB_VERT, ESB_ENABLE_BOTH);
		}
		else
		{
			EnableScrollBar(SB_VERT, ESB_DISABLE_BOTH);
		}
	}
}



