// FavoriteDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "FavoriteDlg.h"
#include "afxdialogex.h"

// CFavoriteDlg ダイアログ

IMPLEMENT_DYNAMIC(CFavoriteDlg, CDialog)

CFavoriteDlg::CFavoriteDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_FAVORITE, pParent)
	, list_(links_)
	, m_xcList(list_)
{

}

CFavoriteDlg::~CFavoriteDlg()
{
}

void CFavoriteDlg::EnableControls(BOOL enable)
{
	m_xcEditTitle.EnableWindow(enable);
	m_xcEditURL.EnableWindow(enable);
}

void CFavoriteDlg::SetControls(int index)
{
	if (links_.empty() || index < 0)
	{
		EnableControls(FALSE);
		//--------------------------------
		m_xcEditTitle.SetWindowText(L"");
		m_xcEditURL.SetWindowText(L"");
	}
	else if (index < static_cast<int>(links_.size()))
	{
		try {
			bbs::Link link = links_.at(index);

			EnableControls(TRUE);
			//--------------------------------
			m_xcEditTitle.SetWindowText(link.title.c_str());
			m_xcEditURL.SetWindowText(link.url.c_str());
		}
		catch (std::exception const&) {}
	}
}

void CFavoriteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_xcList);
	DDX_Control(pDX, IDC_EDIT_TITLE, m_xcEditTitle);
	DDX_Control(pDX, IDC_EDIT_URL, m_xcEditURL);
}


BEGIN_MESSAGE_MAP(CFavoriteDlg, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CFavoriteDlg::OnLvnItemchangedList)
	ON_BN_CLICKED(IDC_BTN_ADD, &CFavoriteDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, &CFavoriteDlg::OnBnClickedBtnDel)
	ON_BN_CLICKED(IDC_BTN_UP, &CFavoriteDlg::OnBnClickedBtnUp)
	ON_BN_CLICKED(IDC_BTN_DOWN, &CFavoriteDlg::OnBnClickedBtnDown)
	ON_EN_CHANGE(IDC_EDIT_TITLE, &CFavoriteDlg::OnEnChangeEditTitle)
	ON_EN_CHANGE(IDC_EDIT_URL, &CFavoriteDlg::OnEnChangeEditUrl)
END_MESSAGE_MAP()


// CFavoriteDlg メッセージ ハンドラー
BOOL CFavoriteDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rc(120, 0, 150, 0);
	MapDialogRect(&rc);

	// リスト
	m_xcList.InsertColumn(0, L"タイトル", LVCFMT_LEFT, rc.left);
	m_xcList.InsertColumn(1, L"URL", LVCFMT_LEFT, rc.right);

	m_xcList.Init();
	SetControls(0);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CFavoriteDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。

	return CDialog::PreTranslateMessage(pMsg);
}


void CFavoriteDlg::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	SetControls(m_xcList.GetIndex());

	if (pResult) { *pResult = 0; }
}


void CFavoriteDlg::OnBnClickedBtnAdd()
{
	m_xcList.AddItem();
}


void CFavoriteDlg::OnBnClickedBtnDel()
{
	m_xcList.DelItem();

	OnLvnItemchangedList(0, 0);
}


void CFavoriteDlg::OnBnClickedBtnUp()
{
	m_xcList.UpItem();
}


void CFavoriteDlg::OnBnClickedBtnDown()
{
	m_xcList.DownItem();
}


void CFavoriteDlg::OnEnChangeEditTitle()
{
	if (!IsWindow(m_xcList.GetSafeHwnd())) { return; }

	int i = m_xcList.GetIndex();
	if (i < 0 || i >= (int)links_.size()) { return; }

	if (!IsWindow(m_xcEditTitle.GetSafeHwnd())) { return; }

	CString str;
	m_xcEditTitle.GetWindowText(str);

	links_.at(i).title = std::wstring(str);
	m_xcList.Update(i);
}


void CFavoriteDlg::OnEnChangeEditUrl()
{
	if (!IsWindow(m_xcList.GetSafeHwnd())) { return; }

	int i = m_xcList.GetIndex();
	if (i < 0 || i >= (int)links_.size()) { return; }

	if (!IsWindow(m_xcEditURL.GetSafeHwnd())) { return; }

	CString str;
	m_xcEditURL.GetWindowText(str);

	links_.at(i).url = std::wstring(str);
	m_xcList.Update(i);
}



