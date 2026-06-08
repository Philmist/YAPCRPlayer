// OpenFileDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "OpenFileDlg.h"
//#include "afxdialogex.h"


// COpenFileDlg ダイアログ

IMPLEMENT_DYNAMIC(COpenFileDlg, CDialog)

COpenFileDlg::COpenFileDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COpenFileDlg::IDD, pParent)
{

}

COpenFileDlg::~COpenFileDlg()
{
}

void COpenFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PATH, m_xcEditPath);
	DDX_Control(pDX, IDC_BTN_PATH, m_xcBtnPath);
	DDX_Control(pDX, IDOK, m_xcBtnOk);
	DDX_Control(pDX, IDCANCEL, m_xcBtnCancel);
}


BEGIN_MESSAGE_MAP(COpenFileDlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_PATH, &COpenFileDlg::OnBnClickedBtnPath)
	ON_WM_SIZE()
	ON_WM_SIZING()
END_MESSAGE_MAP()


// COpenFileDlg メッセージ ハンドラー


BOOL COpenFileDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	status_.CreateEx(this, SBARS_SIZEGRIP);

	if (::IsWindow(status_.GetSafeHwnd()))
	{
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	}

	// サイズ変更枠
	this->ModifyStyle(WS_MAXIMIZEBOX, WS_THICKFRAME);

	// スタイル変更/サイズの適用
	CRect rc;
	GetCalcRect(rc);
	cfg_.height = rc.Height();
	SetWindowPos(NULL, 0, 0, cfg_.width, cfg_.height, SWP_NOZORDER|SWP_NOMOVE|SWP_FRAMECHANGED);

	m_xcEditPath.SetWindowText(gl_.latest.path.c_str());

	SizeHelper();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL COpenFileDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。

	return CDialog::PreTranslateMessage(pMsg);
}


void COpenFileDlg::OnDestroy()
{
	CDialog::OnDestroy();

	CString path;
	m_xcEditPath.GetWindowText(path);
	path_ = path;

	CRect rc;
	GetWindowRect(&rc);
	cfg_.width = rc.Width();
	cfg_.height = rc.Height();
}


void COpenFileDlg::OnBnClickedBtnPath()
{
	CString filter("すべてのファイル (*.*)|*.*||\0\0");
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);

	if (dlg.DoModal() == IDOK)
	{
		CString csTmp;
		csTmp = dlg.GetPathName();
		m_xcEditPath.SetWindowText(csTmp);
	}
}


void COpenFileDlg::SizeHelper()
{
	if (::IsWindow(status_) &&
		::IsWindow(m_xcEditPath) &&
		::IsWindow(m_xcBtnPath) &&
		::IsWindow(m_xcBtnOk) &&
		::IsWindow(m_xcBtnCancel))
	{
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

		CRect sp(1, 1, 3, 3);
		MapDialogRect(sp);

		CRect rc;
		GetClientRect(rc);
		rc.DeflateRect(sp.right, sp.right);

		CRect br;
		m_xcBtnPath.GetWindowRect(br);

		m_xcEditPath.MoveWindow(rc.left, rc.top, rc.Width() - br.Width() - sp.right, br.Height());
		m_xcBtnPath.SetWindowPos(NULL, rc.left + (rc.Width() - br.Width()), rc.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);

		CRect sr;
		status_.GetWindowRect(sr);

		CRect cr;
		m_xcBtnCancel.GetWindowRect(cr);
		m_xcBtnCancel.SetWindowPos(NULL, rc.right - cr.Width(), rc.bottom - cr.Height() - sr.Height(), 0, 0, SWP_NOSIZE|SWP_NOZORDER);

		CRect or;
		m_xcBtnOk.GetWindowRect(or);
		m_xcBtnOk.SetWindowPos(NULL, rc.right - cr.Width() - sp.right - or.Width(), rc.bottom - cr.Height() - sr.Height(), 0, 0, SWP_NOSIZE|SWP_NOZORDER);

		m_xcBtnCancel.RedrawWindow();
		m_xcBtnOk.RedrawWindow();
	}
}

void COpenFileDlg::GetCalcRect(LPRECT pRect)
{
	if (!pRect) { return; }

	if (::IsWindow(status_) &&
		::IsWindow(m_xcEditPath) &&
		::IsWindow(m_xcBtnPath) &&
		::IsWindow(m_xcBtnOk) &&
		::IsWindow(m_xcBtnCancel))
	{
		CRect sp(1, 1, 3, 3);
		MapDialogRect(sp);

		CRect sr;
		status_.GetWindowRect(sr);

		CRect br;
		m_xcBtnPath.GetWindowRect(br);

		CRect or;
		m_xcBtnOk.GetWindowRect(or);

		CRect cr;
		m_xcBtnCancel.GetWindowRect(cr);

		CRect rc(0, 0, or.Width() + cr.Width() + (sp.right * 3), br.Height() + or.Height() + sr.Height() + (sp.right * 3));

		utl::inflateWindowRect(rc, true, true);

		*pRect = rc;
	}
}

void COpenFileDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	SizeHelper();
}


void COpenFileDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);

	CRect rc;
	GetCalcRect(rc);

	switch (fwSide)
	{
	case WMSZ_BOTTOM:
	case WMSZ_BOTTOMRIGHT:
	case WMSZ_RIGHT:
	{
		pRect->bottom = pRect->top + rc.Height();
		if ((pRect->right - pRect->left) < rc.Width())
		{
			pRect->right = pRect->left + rc.Width();
		}
		break;
	}
	case WMSZ_TOP:
	case WMSZ_TOPLEFT:
	case WMSZ_LEFT:
	{
		pRect->top = pRect->bottom - rc.Height();
		if ((pRect->right - pRect->left) < rc.Width())
		{
			pRect->left = pRect->right - rc.Width();
		}
		break;
	}
	case WMSZ_TOPRIGHT:
	{
		pRect->top = pRect->bottom - rc.Height();
		if ((pRect->right - pRect->left) < rc.Width())
		{
			pRect->right = pRect->left + rc.Width();
		}
		break;
	}
	case WMSZ_BOTTOMLEFT:
	{
		pRect->bottom = pRect->top + rc.Height();
		if ((pRect->right - pRect->left) < rc.Width())
		{
			pRect->left = pRect->right - rc.Width();
		}
		break;
	}
	default:
		break;
	}
}
