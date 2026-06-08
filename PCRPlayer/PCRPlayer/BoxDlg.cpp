// BoxDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "BoxDlg.h"
//#include "afxdialogex.h"


// CBoxDlg ダイアログ

IMPLEMENT_DYNAMIC(CBoxDlg, CDialog)

CBoxDlg::CBoxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBoxDlg::IDD, pParent)
	, click_(false)
	, hoverMin_(false)
	, hoverMax_(false)
	, hoverClose_(false)
{

}

CBoxDlg::~CBoxDlg()
{
	if (bmp_.GetSafeHandle()) { bmp_.DeleteObject(); }
}

void CBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void CBoxDlg::showWindow(bool show)
{
	if (show)
	{
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING);
		ShowWindow(SW_SHOWNORMAL);
	}
	else
	{
		// wndBottomやwndTopだとちらつきが発生する
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOSENDCHANGING);
		ShowWindow(SW_HIDE);
	}
}


BEGIN_MESSAGE_MAP(CBoxDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONDBLCLK()
END_MESSAGE_MAP()


// CBoxDlg メッセージ ハンドラー


BOOL CBoxDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (bmp_.GetSafeHandle()) { bmp_.DeleteObject(); }
	bmp_.LoadBitmap(IDB_BOX);

	SetLayeredWindowAttributes(0, 255-63, LWA_ALPHA);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CBoxDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		return FALSE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL CBoxDlg::OnEraseBkgnd(CDC* pDC)
{
	CBrush backBrush(RGB(0, 0, 0));

	// Save old brush
	CBrush* pOldBrush = pDC->SelectObject(&backBrush);

	CRect rect;
	pDC->GetClipBox(&rect); // Erase the area needed

	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
	pDC->SelectObject(pOldBrush);
	return TRUE;
	//return CDialog::OnEraseBkgnd(pDC);
}


void CBoxDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: ここにメッセージ ハンドラー コードを追加します。
	// 描画メッセージで CDialog::OnPaint() を呼び出さないでください。

	bool zoom = reinterpret_cast<CMainDlg*>(AfxGetMainWnd())->IsZoomed() == TRUE || gl_.screen.full || gl_.screen.maximize;

	CRect org;
	GetClientRect(&org);
	

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	CBitmap* oldBmp = memDC.SelectObject(&bmp_);

	///////////////////////////////////////////////////////////////

	if (hoverMin_)
	{
		dc.BitBlt(org.right - 72, 0, 24, 24, &memDC, 0, 24, SRCCOPY);
	}
	else
	{
		dc.BitBlt(org.right - 72, 0, 24, 24, &memDC, 0, 0, SRCCOPY);
	}

	if (hoverMax_)
	{
		dc.BitBlt(org.right - 48, 0, 24, 24, &memDC, zoom ? 48 : 24, 24, SRCCOPY);
	}
	else
	{
		dc.BitBlt(org.right - 48, 0, 24, 24, &memDC, zoom ? 48 : 24, 0, SRCCOPY);
	}

	if (hoverClose_)
	{
		dc.BitBlt(org.right - 24, 0, 24, 24, &memDC, 72, 24, SRCCOPY);
	}
	else
	{
		dc.BitBlt(org.right - 24, 0, 24, 24, &memDC, 72, 0, SRCCOPY);
	}

	///////////////////////////////////////////////////////////////

	memDC.SelectObject(oldBmp);
}


void CBoxDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// マウス監視開始(OnMouseLeave)
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(tme);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = GetSafeHwnd();
	tme.dwHoverTime = HOVER_DEFAULT;
	TrackMouseEvent(&tme);

	CRect rc;
	GetClientRect(&rc);

	CRect rcMin(rc);
	rcMin.right = rc.left + 24;

	CRect rcMax(rc);
	rcMax.left = rcMin.right;
	rcMax.right = rcMax.left + 24;


	if (rcMin.PtInRect(point))
	{
		hoverMin_ = true;
		hoverMax_ = false;
		hoverClose_ = false;
		Invalidate(FALSE);
	}
	else if (rcMax.PtInRect(point))
	{
		hoverMin_ = false;
		hoverMax_ = true;
		hoverClose_ = false;
		Invalidate(FALSE);
	}
	else
	{
		hoverMin_ = false;
		hoverMax_ = false;
		hoverClose_ = true;
		Invalidate(FALSE);
	}


	CDialog::OnMouseMove(nFlags, point);
}


void CBoxDlg::OnMouseLeave()
{
	ShowWindow(SW_HIDE);
	hoverMin_ = false;
	hoverMax_ = false;
	hoverClose_ = false;
	CDialog::OnMouseLeave();
}


void CBoxDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (click_)
	{
		//bool zoom = reinterpret_cast<CMainDlg*>(GetParent())->IsZoomed() == TRUE || gl_.screen.full || gl_.screen.maximize;

		CRect rc;
		GetClientRect(&rc);

		CRect rcMin(rc);
		rcMin.right = rc.left + 24;

		CRect rcMax(rc);
		rcMax.left = rcMin.right;
		rcMax.right = rcMax.left + 24;


		if (rcMin.PtInRect(point))
		{
			ShowWindow(SW_HIDE);
			reinterpret_cast<CMainDlg*>(AfxGetMainWnd())->SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
		}
		else if (rcMax.PtInRect(point))
		{
			ShowWindow(SW_HIDE);
			reinterpret_cast<CMainDlg*>(AfxGetMainWnd())->SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
		}
		else
		{
			ShowWindow(SW_HIDE);
			reinterpret_cast<CMainDlg*>(AfxGetMainWnd())->SendMessage(WM_SYSCOMMAND, SC_CLOSE);
		}
	}
	
	click_ = false;
	//CDialog::OnLButtonUp(nFlags, point);
}


void CBoxDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	click_ = true;
	//CDialog::OnLButtonDown(nFlags, point);
}


void CBoxDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	reinterpret_cast<CMainDlg*>(AfxGetMainWnd())->ScreenToClient(&point);
	reinterpret_cast<CMainDlg*>(AfxGetMainWnd())->OnMButtonDown(nFlags, point);
	//CDialog::OnMButtonDown(nFlags, point);
}

void CBoxDlg::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	OnMButtonDown(nFlags, point);

	//CDialog::OnMButtonDblClk(nFlags, point);
}

