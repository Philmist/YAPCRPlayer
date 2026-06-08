// DisplayResFormat2Dlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayResFormat2Dlg.h"
#include "afxdialogex.h"


// CDisplayResFormat2Dlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayResFormat2Dlg, CDialog)

//CDisplayResFormat2Dlg::CDisplayResFormat2Dlg(CWnd* pParent /*=NULL*/)
//	: CDialog(CDisplayResFormat2Dlg::IDD, pParent)
//{
//
//}

CDisplayResFormat2Dlg::CDisplayResFormat2Dlg(sl::ResConfig& res, CResSampleView& sample)
	: CDialog(CDisplayResFormat2Dlg::IDD, NULL)
	, res_(res)
	, m_xcSample(sample)
{

}

CDisplayResFormat2Dlg::~CDisplayResFormat2Dlg()
{
}

void CDisplayResFormat2Dlg::InitControls()
{
	if (res_.format.punct < 0 || res_.format.punct >= sl::ResConfig::PUNCT_COUNT)
	{
		res_.format.punct = sl::ResConfig::PUNCT_COLON;
	}
	m_xcCmbPunct.SetCurSel(res_.format.punct);

	m_xcCheckSecond.SetCheck(res_.format.second ? BST_CHECKED : BST_UNCHECKED);

	m_xcSample.SetFontIndirect(res_.format.font);
}


void CDisplayResFormat2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_PUNCT, m_xcCmbPunct);
	DDX_Control(pDX, IDC_CHECK_SECOND, m_xcCheckSecond);
}


BEGIN_MESSAGE_MAP(CDisplayResFormat2Dlg, CDialog)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_CMB_PUNCT, &CDisplayResFormat2Dlg::OnCbnSelchangeCmbPunct)
	ON_BN_CLICKED(IDC_CHECK_SECOND, &CDisplayResFormat2Dlg::OnBnClickedCheckSecond)
END_MESSAGE_MAP()


// CDisplayResFormat2Dlg メッセージ ハンドラー


BOOL CDisplayResFormat2Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_xcCmbPunct.InsertString(0, L"なし");
	m_xcCmbPunct.InsertString(1, L"スペース");
	m_xcCmbPunct.InsertString(2, L"コロン");
	
	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayResFormat2Dlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayResFormat2Dlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
}

void CDisplayResFormat2Dlg::OnCbnSelchangeCmbPunct()
{
	res_.format.punct = m_xcCmbPunct.GetCurSel();
	m_xcSample.SetFormatPunct(res_.format.punct);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResFormat2Dlg::OnBnClickedCheckSecond()
{
	if (!IsWindow(m_xcCheckSecond.GetSafeHwnd())) { return; }
	res_.format.second = m_xcCheckSecond.GetCheck() == BST_CHECKED;
	m_xcSample.SetFormatSecond(res_.format.second);
	m_xcSample.Invalidate(FALSE);
}
