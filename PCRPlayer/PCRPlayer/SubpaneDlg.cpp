// SubpaneDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "SubpaneDlg.h"
//#include "afxdialogex.h"


// CSubpaneDlg ダイアログ

IMPLEMENT_DYNAMIC(CSubpaneDlg, CDialog)

CSubpaneDlg::CSubpaneDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSubpaneDlg::IDD, pParent)
{

}

CSubpaneDlg::~CSubpaneDlg()
{
}

void CSubpaneDlg::SizeHelper()
{
	CRect rc;
	GetClientRect(&rc);

	int status = 0, seek = 0, bbs = 0;
	if (bbs_.isValid())
	{
		bbs = bbs_.getHeight();
		bbs_.SetWindowPos(&CWnd::wndBottom, 0, 0, rc.Width(), bbs, SWP_NOACTIVATE);
	}

	if (seek_.isValid())
	{
		seek = seek_.getHeight();
		seek_.SetWindowPos(&CWnd::wndBottom, 0, bbs, rc.Width(), seek, SWP_NOACTIVATE);
	}

	if (status_.isValid())
	{
		status = status_.getHeight();
		status_.SetWindowPos(&CWnd::wndBottom, 0, bbs + seek, rc.Width(), status, SWP_NOACTIVATE);
	}
}

bool CSubpaneDlg::isPopup()
{
	return !!(GetStyle() & WS_POPUP);
}


void CSubpaneDlg::showBBS(bool show, bool edit)
{
	bbs_.setShow(show, edit);
}

void CSubpaneDlg::showSeek(bool show)
{
	seek_.setShow(show);
}

void CSubpaneDlg::showStatus(bool show)
{
	status_.setShow(show);
}

int CSubpaneDlg::getHeight()
{
	int bbs = bbs_.isValid() ? bbs_.getHeight() : 0;
	int seek = seek_.isValid() ? seek_.getHeight() : 0;
	int status = status_.isValid() ? status_.getHeight() : 0;
	return bbs + seek + status;
}

void CSubpaneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSubpaneDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


void CSubpaneDlg::getVideoRect(CRect& rc)
{
	CMainDlg *main = reinterpret_cast<CMainDlg*>(GetParent());
	main->GetVideoRect(rc);

	//CMainDlg *main = reinterpret_cast<CMainDlg*>(GetParent());
	//
	//main->GetClientRect(rc);

	//if (!main->IsExcludeSubpane())
	//{// サブペイン表示時
	//	rc.DeflateRect(0, 0, 0, getHeight());
	//}

	//SIZE aspect = {};
	//gl_.getAspect(aspect);

	//RECT dst = {};
	//if (utl::getAspectRect(aspect, rc, dst))
	//{
	//	rc = dst;
	//}
}

// CSubpaneDlg メッセージ ハンドラー


BOOL CSubpaneDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	bbs_.Create(CBBSDlg::IDD, this);
	seek_.Create(CSeekDlg::IDD, this);
	status_.Create(CStatusDlg::IDD, this);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CSubpaneDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	// キーメッセージを親に渡す
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		return FALSE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CSubpaneDlg::OnSize(UINT nType, int cx, int cy)
{
	//CDialog::OnSize(nType, cx, cy);

	SizeHelper();
}


void CSubpaneDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	reinterpret_cast<CMainDlg*>(GetParent())->ScreenToClient(&point);
	reinterpret_cast<CMainDlg*>(GetParent())->OnLButtonDown(nFlags, point);

	//CDialog::OnLButtonDown(nFlags, point);
}

void CSubpaneDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	reinterpret_cast<CMainDlg*>(GetParent())->ScreenToClient(&point);
	reinterpret_cast<CMainDlg*>(GetParent())->OnLButtonDblClk(nFlags, point);

	//CDialog::OnLButtonDblClk(nFlags, point);
}

void CSubpaneDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	reinterpret_cast<CMainDlg*>(GetParent())->ScreenToClient(&point);
	reinterpret_cast<CMainDlg*>(GetParent())->OnMButtonDown(nFlags, point);

	//CDialog::OnMButtonDown(nFlags, point);
}

void CSubpaneDlg::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	reinterpret_cast<CMainDlg*>(GetParent())->ScreenToClient(&point);
	reinterpret_cast<CMainDlg*>(GetParent())->OnMButtonDblClk(nFlags, point);

	//CDialog::OnMButtonDblClk(nFlags, point);
}



BOOL CSubpaneDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rc;
	GetClientRect(&rc);
	pDC->FillRect(&rc, &CBrush(RGB(0,0,0)));

	return TRUE;
	//return CDialog::OnEraseBkgnd(pDC);
}



void CSubpaneDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// サイズ変更カーソル表示のためメインウィンドウに渡す
	// ポップアップのときは無効にする
	if (GetStyle() & WS_CHILD)
	{
		ClientToScreen(&point);
		reinterpret_cast<CMainDlg*>(GetParent())->ScreenToClient(&point);
		reinterpret_cast<CMainDlg*>(GetParent())->OnMouseMove(nFlags, point);
	}

	//CDialog::OnMouseMove(nFlags, point);
}