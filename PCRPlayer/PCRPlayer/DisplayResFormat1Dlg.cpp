// DisplayResFormat1Dlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayResFormat1Dlg.h"
#include "afxdialogex.h"


// CDisplayResFormat1Dlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayResFormat1Dlg, CDialog)

//CDisplayResFormat1Dlg::CDisplayResFormat1Dlg(CWnd* pParent /*=NULL*/)
//	: CDialog(CDisplayResFormat1Dlg::IDD, pParent)
//{
//
//}

CDisplayResFormat1Dlg::CDisplayResFormat1Dlg(sl::ResConfig& res, CResSampleView& sample)
	: CDialog(CDisplayResFormat1Dlg::IDD, NULL)
	, res_(res)
	, m_xcSample(sample)
{

}

CDisplayResFormat1Dlg::~CDisplayResFormat1Dlg()
{
}

void CDisplayResFormat1Dlg::InitControls()
{
	if (res_.format.type < 0 || res_.format.type >= sl::ResConfig::FORMAT_COUNT)
	{
		res_.format.type = sl::ResConfig::FORMAT_NORMAL1;
	}
	m_xcCmbType.SetCurSel(res_.format.type);

	m_xcBtnFont.SetFontIndirect(res_.format.font);
	m_xcBtnFont.Invalidate(FALSE);

	m_xcSpinSpace.SetPos32(res_.format.space);
	m_xcSpinWidth.SetPos32(res_.format.width);

	m_xcSample.SetFontIndirect(res_.format.font);
}


void CDisplayResFormat1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_TYPE, m_xcCmbType);
	DDX_Control(pDX, IDC_BTN_FONT, m_xcBtnFont);
	DDX_Control(pDX, IDC_SPIN_SPACE, m_xcSpinSpace);
	DDX_Control(pDX, IDC_SPIN_WIDTH, m_xcSpinWidth);
}


BEGIN_MESSAGE_MAP(CDisplayResFormat1Dlg, CDialog)
	ON_WM_DESTROY()
	ON_CBN_SELCHANGE(IDC_CMB_TYPE, &CDisplayResFormat1Dlg::OnCbnSelchangeCmbType)
	ON_BN_CLICKED(IDC_BTN_FONT, &CDisplayResFormat1Dlg::OnBnClickedBtnFont)
	ON_EN_CHANGE(IDC_EDIT_SPACE, &CDisplayResFormat1Dlg::OnEnChangeEditSpace)
	ON_EN_CHANGE(IDC_EDIT_WIDTH, &CDisplayResFormat1Dlg::OnEnChangeEditWidth)
END_MESSAGE_MAP()


// CDisplayResFormat1Dlg メッセージ ハンドラー


BOOL CDisplayResFormat1Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_xcCmbType.InsertString(0, L"ノーマル");
	m_xcCmbType.InsertString(1, L"ノーマル (非デフォルト名のみ)");
	m_xcCmbType.InsertString(2, L"シンプル");
	m_xcCmbType.InsertString(3, L"シンプル (非デフォルト名のみ)");
	m_xcCmbType.InsertString(4, L"コンパクト");
	m_xcCmbType.InsertString(5, L"コンパクト (非デフォルト名のみ)");

	m_xcSpinSpace.SetRange32(0, 100);
	m_xcSpinWidth.SetRange32(0, INT_MAX);
	
	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayResFormat1Dlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayResFormat1Dlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
}



void CDisplayResFormat1Dlg::OnCbnSelchangeCmbType()
{
	res_.format.type = m_xcCmbType.GetCurSel();
	m_xcSample.SetFormatType(res_.format.type);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResFormat1Dlg::OnBnClickedBtnFont()
{
	LOGFONT lf = {};
	if (m_xcBtnFont.DoModal(lf))
	{
		res_.format.font = lf;
		m_xcSample.SetFontIndirect(lf);
		m_xcSample.Invalidate(FALSE);
	}
}

void CDisplayResFormat1Dlg::OnEnChangeEditSpace()
{
	if (!IsWindow(m_xcSpinSpace.GetSafeHwnd())) { return; }
	res_.format.space = m_xcSpinSpace.GetPos32();
	m_xcSample.SetFormatSpace(res_.format.space);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResFormat1Dlg::OnEnChangeEditWidth()
{
	if (!IsWindow(m_xcSpinWidth.GetSafeHwnd())) { return; }
	res_.format.width = m_xcSpinWidth.GetPos32();
	m_xcSample.SetFormatWidth(res_.format.width);
	m_xcSample.Invalidate(FALSE);
}
