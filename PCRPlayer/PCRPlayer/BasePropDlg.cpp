// BasePropDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "BasePropDlg.h"
#include "afxdialogex.h"


// CBasePropDlg ダイアログ

IMPLEMENT_DYNAMIC(CBasePropDlg, CDialog)

CBasePropDlg::CBasePropDlg(utl::PropSetting& setting, CWnd* pParent /*=NULL*/)
	: CDialog(IDD_BASE_PROP, pParent)
	, setting_(setting)
	, prop_(m_xcProp)
{

}

CBasePropDlg::~CBasePropDlg()
{
}

void CBasePropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROP_POS, m_xcPropPos);
	DDX_Control(pDX, IDC_CHECK_DESC, m_xcCheckDesc);
}


BEGIN_MESSAGE_MAP(CBasePropDlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_ALL_DEFAULT, &CBasePropDlg::OnBnClickedBtnAllDefault)
	ON_BN_CLICKED(IDC_CHECK_DESC, &CBasePropDlg::OnBnClickedCheckDesc)
END_MESSAGE_MAP()


// CBasePropDlg メッセージ ハンドラー


BOOL CBasePropDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
	//CMFCVisualManager::(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
	//CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));

	//CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
	//CMFCButton::EnableWindowsTheming();

	// ダイアログリソースにPropertyGridを直接貼り付ける方法はバグがあり
	// Borderスタイルが設定できなかったりDescriptionAreaを表示すると描画が乱れる
	// PictureControlを貼り付けてその位置を元にしてCreateする

	CRect rc;
	m_xcPropPos.GetClientRect(rc);
	m_xcPropPos.MapWindowPoints(this, &rc);
	m_xcPropPos.ShowWindow(SW_HIDE);

	m_xcProp.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rc, this, (UINT)-1);

	m_xcProp.EnableHeaderCtrl(FALSE);
	m_xcProp.EnableDescriptionArea();
	m_xcProp.SetVSDotNetLook();
	m_xcProp.SetBoolLabels(L"有効", L"無効");

	// グループ文字色変更
	COLORREF c = -1;
	m_xcProp.SetCustomColors(c, c, c, GetSysColor(COLOR_BTNTEXT), c, c, c);

	if (setting_.width >= 0)
	{
		HDITEM hdItem;
		hdItem.mask = HDI_WIDTH;
		hdItem.cxy = setting_.width;
		m_xcProp.GetHeaderCtrl().SetItem(0, &hdItem);
	}

	if (::IsWindow(m_xcCheckDesc) && ::IsWindow(m_xcProp))
	{
		m_xcCheckDesc.SetCheck(setting_.check ? BST_CHECKED : BST_UNCHECKED);
		m_xcProp.EnableDescriptionArea(setting_.check ? TRUE : FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CBasePropDlg::PreTranslateMessage(MSG* pMsg)
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


void CBasePropDlg::OnDestroy()
{
	CDialog::OnDestroy();

	OnGetProp();

	HDITEM hdItem;
	hdItem.mask = HDI_WIDTH;
	if (m_xcProp.GetHeaderCtrl().GetItem(0, &hdItem))
	{
		setting_.width = hdItem.cxy;
	}
}


void CBasePropDlg::OnBnClickedBtnAllDefault()
{
	OnSetProp();

	m_xcProp.RedrawWindow();
}


void CBasePropDlg::OnBnClickedCheckDesc()
{
	if (::IsWindow(m_xcCheckDesc) && ::IsWindow(m_xcProp))
	{
		setting_.check = m_xcCheckDesc.GetCheck() == BST_CHECKED;
		m_xcCheckDesc.SetCheck(setting_.check ? BST_CHECKED : BST_UNCHECKED);
		m_xcProp.EnableDescriptionArea(setting_.check ? TRUE : FALSE);
	}
}
