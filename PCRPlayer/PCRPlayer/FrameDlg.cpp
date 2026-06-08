// FrameDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "FrameDlg.h"
#include "afxdialogex.h"


// CFrameDlg ダイアログ

IMPLEMENT_DYNAMIC(CFrameDlg, CDialog)

CFrameDlg::CFrameDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFrameDlg::IDD, pParent)
{

}

CFrameDlg::~CFrameDlg()
{
}

void CFrameDlg::SizeHelper()
{
	CRect rc;
	GetClientRect(&rc);

	// ビデオ描画
	SIZE aspect = {};
	gl_.getAspect(aspect);

	RECT dst = {};
	if (utl::getAspectRect(aspect, rc, dst))
	{
		rc = dst;
	}

	gl_.player.setVideoPosition(rc);
}

void CFrameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFrameDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_MESSAGE(WM_GRAPH_NOTIFY, &CFrameDlg::OnGraphNotify)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_MBUTTONDOWN()
END_MESSAGE_MAP()


// CFrameDlg メッセージ ハンドラー


BOOL CFrameDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SizeHelper();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CFrameDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。

	return CDialog::PreTranslateMessage(pMsg);
}


void CFrameDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
}


void CFrameDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	SizeHelper();
}


BOOL CFrameDlg::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	//return CDialog::OnEraseBkgnd(pDC);
}


void CFrameDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: ここにメッセージ ハンドラー コードを追加します。
					   // 描画メッセージで CDialog::OnPaint() を呼び出さないでください。

	CRect src;
	GetClientRect(&src);
	if (gl_.player.repaint(dc))
	{
		// 動画以外の場所を黒く塗りつぶす
		SIZE aspect = {};
		gl_.getAspect(aspect);

		RECT dst = {};
		if (utl::getAspectRect(aspect, src, dst))
		{
			CRgn r, r1, r2;
			r.CreateRectRgn(0, 0, 1, 1);
			r1.CreateRectRgnIndirect(&src);
			r2.CreateRectRgnIndirect(&dst);
			r.CombineRgn(&r1, &r2, RGN_DIFF);
#ifdef _DEBUG
			dc.FillRgn(&r, &CBrush(RGB(0, 255, 0)));
#else
			dc.FillRgn(&r, &CBrush(RGB(0, 0, 0)));
#endif
		}
		return;
	}

	// 背景を黒く塗りつぶす
	dc.FillRect(&src, &CBrush(RGB(0, 0, 0)));
}


afx_msg LRESULT CFrameDlg::OnGraphNotify(WPARAM wParam, LPARAM lParam)
{
	if (this_.wnd && ::IsWindow(*this_.wnd))
	{
		this_.wnd->PostMessage(WM_GRAPH_NOTIFY, wParam, lParam);
	}
	return 0;
}


void CFrameDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (this_.wnd && ::IsWindow(*this_.wnd))
	{
		ClientToScreen(&point);
		(static_cast<CMainDlg*>(this_.wnd))->ScreenToClient(&point);
		(static_cast<CMainDlg*>(this_.wnd))->OnRButtonDown(nFlags, point);
	}
	//CDialog::OnRButtonDown(nFlags, point);
}


void CFrameDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (this_.wnd && ::IsWindow(*this_.wnd))
	{
		ClientToScreen(&point);
		(static_cast<CMainDlg*>(this_.wnd))->ScreenToClient(&point);
		(static_cast<CMainDlg*>(this_.wnd))->OnRButtonUp(nFlags, point);
	}
	//CDialog::OnRButtonUp(nFlags, point);
}


void CFrameDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (this_.wnd && ::IsWindow(*this_.wnd))
	{
		if (gl_.value.window.show.edit && gl_.value.detail.board.focus)
		{
			(static_cast<CMainDlg*>(this_.wnd))->SetFocusBBSEdit(); // BBS popup
		}
		else
		{
			(static_cast<CMainDlg*>(this_.wnd))->SetFocus(); // BBS popup
		}

		ClientToScreen(&point);
		(static_cast<CMainDlg*>(this_.wnd))->ScreenToClient(&point);
		(static_cast<CMainDlg*>(this_.wnd))->OnLButtonDblClk(nFlags, point);
	}
	//CDialog::OnLButtonDblClk(nFlags, point);
}


void CFrameDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (this_.wnd && ::IsWindow(*this_.wnd))
	{
		if (gl_.value.window.show.edit && gl_.value.detail.board.focus)
		{
			(static_cast<CMainDlg*>(this_.wnd))->SetFocusBBSEdit(); // BBS popup
		}
		else
		{
			(static_cast<CMainDlg*>(this_.wnd))->SetFocus(); // BBS popup
		}

		ClientToScreen(&point);
		(static_cast<CMainDlg*>(this_.wnd))->ScreenToClient(&point);
		(static_cast<CMainDlg*>(this_.wnd))->OnLButtonDown(nFlags, point);
	}
	//CDialog::OnLButtonDown(nFlags, point);
}


void CFrameDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (this_.wnd && ::IsWindow(*this_.wnd))
	{
		ClientToScreen(&point);
		(static_cast<CMainDlg*>(this_.wnd))->ScreenToClient(&point);
		(static_cast<CMainDlg*>(this_.wnd))->OnLButtonUp(nFlags, point);
	}
	//CDialog::OnLButtonUp(nFlags, point);
}


void CFrameDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (this_.wnd && ::IsWindow(*this_.wnd))
	{
		ClientToScreen(&point);
		(static_cast<CMainDlg*>(this_.wnd))->ScreenToClient(&point);
		(static_cast<CMainDlg*>(this_.wnd))->OnMouseMove(nFlags, point);
	}
	//CDialog::OnMouseMove(nFlags, point);
}


void CFrameDlg::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	if (this_.wnd && ::IsWindow(*this_.wnd))
	{
		ClientToScreen(&point);
		(static_cast<CMainDlg*>(this_.wnd))->ScreenToClient(&point);
		(static_cast<CMainDlg*>(this_.wnd))->OnMButtonDblClk(nFlags, point);
	}
	//CDialog::OnMButtonDblClk(nFlags, point);
}


void CFrameDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	if (this_.wnd && ::IsWindow(*this_.wnd))
	{
		ClientToScreen(&point);
		(static_cast<CMainDlg*>(this_.wnd))->ScreenToClient(&point);
		(static_cast<CMainDlg*>(this_.wnd))->OnMButtonDown(nFlags, point);
	}
	//CDialog::OnMButtonDown(nFlags, point);
}

