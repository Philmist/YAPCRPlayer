// ExtractDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "ExtractDlg.h"
#include "afxdialogex.h"


std::vector<boost::shared_ptr<CExtractDlg>> CExtractDlg::vec_;
sl::DialogConfig::Extract CExtractDlg::cfg_;


void CExtractDlg::erase(CExtractDlg* p)
{
	if (!p) { return; }
	for (auto it = vec_.begin(); it != vec_.end(); ++it)
	{
		if (it->get() == p)
		{
			vec_.erase(it);
			break;
		}
	}
}

CExtractDlg* CExtractDlg::make(CWnd* pParent)
{
	CExtractDlg* ptr = new CExtractDlg(pParent);
	if (ptr)
	{
		vec_.emplace_back(ptr);
		return ptr;
	}
	return NULL;
}

void CExtractDlg::destroy()
{
	auto it = vec_.begin();
	while (it != vec_.end())
	{
		if (it->get() && ::IsWindow(it->get()->GetSafeHwnd()))
		{
			it->get()->DestroyWindow();
			it = vec_.begin();
		}
	}
}

void CExtractDlg::setfont(const std::wstring& name, int point)
{
	for (auto it = vec_.begin(); it != vec_.end(); ++it)
	{
		if ((*it))
		{
			(*it)->setFont(name, point);
		}
	}
}


// CExtractDlg ダイアログ

IMPLEMENT_DYNAMIC(CExtractDlg, CDialog)

CExtractDlg::CExtractDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_EXTRACT, pParent)
{
}

CExtractDlg::~CExtractDlg()
{
}


void CExtractDlg::setFont(const std::wstring& name, int point)
{
	if (::IsWindow(GetSafeHwnd()) &&
		::IsWindow(status_))
	{
		if (font_.GetSafeHandle()) { font_.DeleteObject(); }
		font_.CreatePointFont(point * 10, name.c_str());

		status_.SetFont(&font_);

		SizeHelper();
	}
}

void CExtractDlg::SizeHelper()
{
	if (::IsWindow(status_) && ::IsWindow(browser_))
	{
		RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

		CSize status;
		status = status_.CalcFixedLayout(TRUE, TRUE);

		CRect rc;
		GetClientRect(rc);
		rc.DeflateRect(0, 0, 0, status.cy);
		browser_.MoveWindow(rc);
	}
}

void CExtractDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CExtractDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CExtractDlg メッセージ ハンドラー


BOOL CExtractDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ステータスバー
	if (status_.CreateEx(this, SBARS_SIZEGRIP))
	{
		const static UINT ind[1] = {ID_SEPARATOR};
		if (status_.SetIndicators(ind, 1))
		{
			status_.SetPaneInfo(0, 0, SBPS_STRETCH, 0);
		}
	}

	// サイズ変更枠
	ModifyStyle(0, WS_THICKFRAME);

	// スタイル変更/サイズの適用
	SetWindowPos(NULL, 0, 0, cfg_.width, cfg_.height, SWP_NOZORDER|SWP_NOMOVE|SWP_FRAMECHANGED);

	browser_.setStatusBar(&status_);
	browser_.Create(IDD_EXTRACT_BROWSER, this);

	setFont(gl_.value.detail.font.extract.name, gl_.value.detail.font.extract.point);

	CenterWindow(AfxGetMainWnd());

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CExtractDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。

	return CDialog::PreTranslateMessage(pMsg);
}


void CExtractDlg::OnDestroy()
{
	CDialog::OnDestroy();

	CRect rc;
	WINDOWPLACEMENT wp = {};
	wp.length = sizeof(WINDOWPLACEMENT);
	if (GetWindowPlacement(&wp))
	{
		rc = wp.rcNormalPosition;
	}
	else
	{
		GetWindowRect(rc);
	}

	cfg_.width = rc.Width();
	cfg_.height = rc.Height();
}


void CExtractDlg::PostNcDestroy()
{
	erase(this);
	//CDialog::PostNcDestroy();
}


void CExtractDlg::OnOK()
{
	DestroyWindow();

	//CDialog::OnOK();
}


void CExtractDlg::OnCancel()
{
	DestroyWindow();

	//CDialog::OnCancel();
}


void CExtractDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	SizeHelper();
}

