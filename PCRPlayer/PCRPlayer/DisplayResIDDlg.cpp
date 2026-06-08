// DisplayResIDDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayResIDDlg.h"
#include "afxdialogex.h"


// CDisplayResIDDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayResIDDlg, CDialog)

//CDisplayResIDDlg::CDisplayResIDDlg(CWnd* pParent /*=NULL*/)
//	: CDialog(CDisplayResIDDlg::IDD, pParent)
//{
//
//}

CDisplayResIDDlg::CDisplayResIDDlg(sl::ResConfig& res, CResSampleView& sample)
	: CDialog(CDisplayResIDDlg::IDD, NULL)
	, res_(res)
	, m_xcSample(sample)
{

}

CDisplayResIDDlg::~CDisplayResIDDlg()
{
}

void CDisplayResIDDlg::InitControls()
{
	m_xcCheckEnable.SetCheck(res_.id.enable ? BST_CHECKED : BST_UNCHECKED);
	m_xcSpinThreshold.SetPos32(res_.id.threshold);
}


void CDisplayResIDDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_ENABLE, m_xcCheckEnable);
	DDX_Control(pDX, IDC_SPIN_THRESHOLD, m_xcSpinThreshold);
}


BEGIN_MESSAGE_MAP(CDisplayResIDDlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_ENABLE, &CDisplayResIDDlg::OnBnClickedCheckEnable)
	ON_EN_CHANGE(IDC_EDIT_THRESHOLD, &CDisplayResIDDlg::OnEnChangeEditThreshold)
END_MESSAGE_MAP()


// CDisplayResIDDlg メッセージ ハンドラー


BOOL CDisplayResIDDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_xcSpinThreshold.SetRange32(3, INT_MAX);

	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayResIDDlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayResIDDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
}

void CDisplayResIDDlg::OnBnClickedCheckEnable()
{
	if (!IsWindow(m_xcCheckEnable.GetSafeHwnd())) { return; }
	res_.id.enable = m_xcCheckEnable.GetCheck() == BST_CHECKED;
	m_xcSample.SetIDEnable(res_.id.enable);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayResIDDlg::OnEnChangeEditThreshold()
{
	if (!IsWindow(m_xcSpinThreshold.GetSafeHwnd())) { return; }
	res_.id.threshold = m_xcSpinThreshold.GetPos32();
	m_xcSample.SetIDThreshold(res_.id.threshold);
	m_xcSample.Invalidate(FALSE);
}

