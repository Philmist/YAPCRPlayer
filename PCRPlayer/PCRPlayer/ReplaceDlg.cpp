// ReplaceDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "ReplaceDlg.h"
#include "afxdialogex.h"


// CReplaceDlg ダイアログ

IMPLEMENT_DYNAMIC(CReplaceDlg, CDialog)

//CReplaceDlg::CReplaceDlg(CWnd* pParent /*=NULL*/)
//	: CDialog(CReplaceDlg::IDD, pParent)
//{
//
//}

CReplaceDlg::CReplaceDlg(std::vector<utl::ReplaceElement>& list, const boost::function<void(utl::ReplaceElementList&)>& reset)
	: CDialog(CReplaceDlg::IDD, NULL)
	, list_(list)
	, reset_(reset)
	, m_xcList(list_, true)
{

}

CReplaceDlg::~CReplaceDlg()
{
}

void CReplaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_xcList);
	DDX_Control(pDX, IDC_EDIT_DESC, m_xcEditDesc);
	DDX_Control(pDX, IDC_EDIT_REGEX, m_xcEditRegex);
	DDX_Control(pDX, IDC_EDIT_FORMAT, m_xcEditFormat);
	DDX_Control(pDX, IDC_CHECK_ICASE, m_xcCheckIcase);
}

void CReplaceDlg::InitControls()
{
	int index = m_xcList.GetIndex();
	if (!list_.get(index, [this](const utl::ReplaceElement& elem) {
		EnableControls(TRUE);
		//--------------------------------
		m_xcEditDesc.SetWindowText(elem.desc.c_str());
		m_xcEditRegex.SetWindowText(elem.regex.c_str());
		m_xcEditFormat.SetWindowText(elem.format.c_str());
		m_xcCheckIcase.SetCheck(elem.icase ? BST_CHECKED : BST_UNCHECKED);
	}))
	{
		EnableControls(FALSE);
		//--------------------------------
		m_xcEditDesc.SetWindowText(L"");
		m_xcEditRegex.SetWindowText(L"");
		m_xcEditFormat.SetWindowText(L"");
		m_xcCheckIcase.SetCheck(BST_UNCHECKED);
	}
}

void CReplaceDlg::EnableControls(BOOL enable)
{
	m_xcEditDesc.EnableWindow(enable);
	m_xcEditRegex.EnableWindow(enable);
	m_xcEditFormat.EnableWindow(enable);
	m_xcCheckIcase.EnableWindow(enable);
}

BEGIN_MESSAGE_MAP(CReplaceDlg, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CReplaceDlg::OnLvnItemchangedList)
	ON_BN_CLICKED(IDC_BTN_ADD, &CReplaceDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_COPY, &CReplaceDlg::OnBnClickedBtnCopy)
	ON_BN_CLICKED(IDC_BTN_DEL, &CReplaceDlg::OnBnClickedBtnDel)
	ON_BN_CLICKED(IDC_BTN_UP, &CReplaceDlg::OnBnClickedBtnUp)
	ON_BN_CLICKED(IDC_BTN_DOWN, &CReplaceDlg::OnBnClickedBtnDown)
	ON_EN_CHANGE(IDC_EDIT_DESC, &CReplaceDlg::OnEnChangeEditDesc)
	ON_EN_CHANGE(IDC_EDIT_REGEX, &CReplaceDlg::OnEnChangeEditRegex)
	ON_EN_CHANGE(IDC_EDIT_FORMAT, &CReplaceDlg::OnEnChangeEditFormat)
	ON_BN_CLICKED(IDC_CHECK_ICASE, &CReplaceDlg::OnBnClickedCheckIcase)
	ON_BN_CLICKED(IDC_BTN_ALL_DEFAULT, &CReplaceDlg::OnBnClickedBtnAllDefault)
END_MESSAGE_MAP()


// CReplaceDlg メッセージ ハンドラー


BOOL CReplaceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rc(18, 0, 70, 0);
	MapDialogRect(&rc);

	m_xcList.InsertColumn(0, L"", LVCFMT_LEFT, rc.left);
	m_xcList.InsertColumn(1, L"説明", LVCFMT_LEFT, rc.right);
	m_xcList.InsertColumn(2, L"正規表現", LVCFMT_LEFT, rc.right);
	m_xcList.InsertColumn(3, L"置換規則", LVCFMT_LEFT, rc.right);
	m_xcList.InsertColumn(4, L"大小文字区別なし", LVCFMT_LEFT, rc.left);

	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CReplaceDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			return FALSE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void CReplaceDlg::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	InitControls();

	if (pResult) { *pResult = 0; }
}

void CReplaceDlg::OnBnClickedBtnAdd()
{
	m_xcList.AddItem();
	InitControls();
}

void CReplaceDlg::OnBnClickedBtnCopy()
{
	m_xcList.CopyItem();
	InitControls();
}

void CReplaceDlg::OnBnClickedBtnDel()
{
	m_xcList.DelItem();
	InitControls();
}


void CReplaceDlg::OnBnClickedBtnUp()
{
	m_xcList.UpItem();
	InitControls();
}


void CReplaceDlg::OnBnClickedBtnDown()
{
	m_xcList.DownItem();
	InitControls();
}


void CReplaceDlg::OnEnChangeEditDesc()
{
	if (!IsWindow(m_xcEditDesc.GetSafeHwnd())) { return; }

	int index = m_xcList.GetIndex();
	if (list_.get(index, [this](utl::ReplaceElement& elem) {
		CString str;
		m_xcEditDesc.GetWindowText(str);
		elem.desc = std::wstring(str);
	}))
	{
		m_xcList.Update(index);
	}
}


void CReplaceDlg::OnEnChangeEditRegex()
{
	if (!IsWindow(m_xcEditRegex.GetSafeHwnd())) { return; }

	int index = m_xcList.GetIndex();
	if (list_.get(index, [this](utl::ReplaceElement& elem) {
		CString str;
		m_xcEditRegex.GetWindowText(str);
		elem.regex = std::wstring(str);
	}))
	{
		m_xcList.Update(index);
	}
}

void CReplaceDlg::OnEnChangeEditFormat()
{
	if (!IsWindow(m_xcEditFormat.GetSafeHwnd())) { return; }

	int index = m_xcList.GetIndex();
	if (list_.get(index, [this](utl::ReplaceElement& elem) {
		CString str;
		m_xcEditFormat.GetWindowText(str);
		elem.format = std::wstring(str);
	}))
	{
		m_xcList.Update(index);
	}
}

void CReplaceDlg::OnBnClickedCheckIcase()
{
	if (!IsWindow(m_xcCheckIcase.GetSafeHwnd())) { return; }

	int index = m_xcList.GetIndex();
	if (list_.get(index, [this](utl::ReplaceElement& elem) {
		elem.icase = m_xcCheckIcase.GetCheck() == BST_CHECKED;
	}))
	{
		m_xcList.Update(index);
	}
}

void CReplaceDlg::OnBnClickedBtnAllDefault()
{
	reset_(list_);

	m_xcList.Init();
	m_xcList.RedrawWindow();
	InitControls();
}



