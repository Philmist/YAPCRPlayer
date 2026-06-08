// DisplayResBackDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayResBackDlg.h"
#include "afxdialogex.h"


// CDisplayResBackDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayResBackDlg, CDialog)

//CDisplayResBackDlg::CDisplayResBackDlg(CWnd* pParent /*=NULL*/)
//	: CDialog(CDisplayResBackDlg::IDD, pParent)
//{
//
//}

CDisplayResBackDlg::CDisplayResBackDlg(sl::ResConfig& res, CResSampleView& sample)
	: CDialog(CDisplayResBackDlg::IDD, NULL)
	, res_(res)
	, m_xcSample(sample)
{

}

CDisplayResBackDlg::~CDisplayResBackDlg()
{
}

void CDisplayResBackDlg::InitControls()
{
	for (int i = 0; i < 6; ++i)
	{
		m_xcBtnColor[i].SetColor(res_.back.color[i]);
		m_xcBtnColor[i].Invalidate(FALSE);
	}
	m_xcSpinMarginX.SetPos32(res_.back.marginX);
	m_xcSpinMarginY.SetPos32(res_.back.marginY);

	m_xcCheckOpaque.SetCheck(res_.back.opaque.enable ? BST_CHECKED : BST_UNCHECKED);

	m_xcSpinOpaqueText.SetPos32(res_.back.opaque.text);
	m_xcSpinOpaqueBack.SetPos32(res_.back.opaque.back);

	BOOL enable = res_.back.opaque.enable ? TRUE : FALSE;
	m_xcEditOpaqueText.EnableWindow(enable);
	m_xcSpinOpaqueText.EnableWindow(enable);
}

void CDisplayResBackDlg::OnBnClickedBtnColor(int index)
{
	res_.back.color[index] = m_xcBtnColor[index].GetColor();
	m_xcSample.SetBackColor(index, res_.back.color[index]);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayResBackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_COLOR1, m_xcBtnColor[0]);
	DDX_Control(pDX, IDC_BTN_COLOR2, m_xcBtnColor[1]);
	DDX_Control(pDX, IDC_BTN_COLOR3, m_xcBtnColor[2]);
	DDX_Control(pDX, IDC_BTN_COLOR4, m_xcBtnColor[3]);
	DDX_Control(pDX, IDC_BTN_COLOR5, m_xcBtnColor[4]);
	DDX_Control(pDX, IDC_BTN_COLOR6, m_xcBtnColor[5]);
	DDX_Control(pDX, IDC_SPIN_MARGIN_X, m_xcSpinMarginX);
	DDX_Control(pDX, IDC_SPIN_MARGIN_Y, m_xcSpinMarginY);
	DDX_Control(pDX, IDC_CHECK_OPAQUE, m_xcCheckOpaque);
	DDX_Control(pDX, IDC_EDIT_OPAQUE_TEXT, m_xcEditOpaqueText);
	DDX_Control(pDX, IDC_SPIN_OPAQUE_TEXT, m_xcSpinOpaqueText);
	DDX_Control(pDX, IDC_EDIT_OPAQUE_BACK, m_xcEditOpaqueBack);
	DDX_Control(pDX, IDC_SPIN_OPAQUE_BACK, m_xcSpinOpaqueBack);
}


BEGIN_MESSAGE_MAP(CDisplayResBackDlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_COLOR1, &CDisplayResBackDlg::OnBnClickedBtnColor1)
	ON_BN_CLICKED(IDC_BTN_COLOR2, &CDisplayResBackDlg::OnBnClickedBtnColor2)
	ON_BN_CLICKED(IDC_BTN_COLOR3, &CDisplayResBackDlg::OnBnClickedBtnColor3)
	ON_BN_CLICKED(IDC_BTN_COLOR4, &CDisplayResBackDlg::OnBnClickedBtnColor4)
	ON_BN_CLICKED(IDC_BTN_COLOR5, &CDisplayResBackDlg::OnBnClickedBtnColor5)
	ON_BN_CLICKED(IDC_BTN_COLOR6, &CDisplayResBackDlg::OnBnClickedBtnColor6)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_X, &CDisplayResBackDlg::OnEnChangeEditMarginX)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_Y, &CDisplayResBackDlg::OnEnChangeEditMarginY)
	ON_BN_CLICKED(IDC_CHECK_OPAQUE, &CDisplayResBackDlg::OnBnClickedCheckOpaque)
	ON_EN_CHANGE(IDC_EDIT_OPAQUE_TEXT, &CDisplayResBackDlg::OnEnChangeEditOpaqueText)
	ON_EN_CHANGE(IDC_EDIT_OPAQUE_BACK, &CDisplayResBackDlg::OnEnChangeEditOpaqueBack)
END_MESSAGE_MAP()


// CDisplayResBackDlg メッセージ ハンドラー


BOOL CDisplayResBackDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_xcSpinMarginX.SetRange32(0, 100);
	m_xcSpinMarginY.SetRange32(0, 100);
	m_xcSpinOpaqueText.SetRange32(0, 255);
	m_xcSpinOpaqueBack.SetRange32(0, 255);

	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayResBackDlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayResBackDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
}


void CDisplayResBackDlg::OnBnClickedBtnColor1()
{
	OnBnClickedBtnColor(0);
}


void CDisplayResBackDlg::OnBnClickedBtnColor2()
{
	OnBnClickedBtnColor(1);
}


void CDisplayResBackDlg::OnBnClickedBtnColor3()
{
	OnBnClickedBtnColor(2);
}


void CDisplayResBackDlg::OnBnClickedBtnColor4()
{
	OnBnClickedBtnColor(3);
}


void CDisplayResBackDlg::OnBnClickedBtnColor5()
{
	OnBnClickedBtnColor(4);
}


void CDisplayResBackDlg::OnBnClickedBtnColor6()
{
	OnBnClickedBtnColor(5);
}

void CDisplayResBackDlg::OnEnChangeEditMarginX()
{
	if (!IsWindow(m_xcSpinMarginX.GetSafeHwnd())) { return; }
	res_.back.marginX = m_xcSpinMarginX.GetPos32();
	m_xcSample.SetBackMarginX(res_.back.marginX);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayResBackDlg::OnEnChangeEditMarginY()
{
	if (!IsWindow(m_xcSpinMarginY.GetSafeHwnd())) { return; }
	res_.back.marginY = m_xcSpinMarginY.GetPos32();
	m_xcSample.SetBackMarginY(res_.back.marginY);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResBackDlg::OnBnClickedCheckOpaque()
{
	res_.back.opaque.enable = m_xcCheckOpaque.GetCheck() == BST_CHECKED;
	BOOL enable = res_.back.opaque.enable ? TRUE : FALSE;
	m_xcEditOpaqueText.EnableWindow(enable);
	m_xcSpinOpaqueText.EnableWindow(enable);
	m_xcSample.SetBackOpaqueEnable(res_.back.opaque.enable);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResBackDlg::OnEnChangeEditOpaqueText()
{
	if (!IsWindow(m_xcSpinOpaqueText.GetSafeHwnd())) { return; }
	res_.back.opaque.text = m_xcSpinOpaqueText.GetPos32();
	m_xcSample.SetBackOpaqueText(res_.back.opaque.text);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResBackDlg::OnEnChangeEditOpaqueBack()
{
	if (!IsWindow(m_xcSpinOpaqueBack.GetSafeHwnd())) { return; }
	res_.back.opaque.back = m_xcSpinOpaqueBack.GetPos32();
	m_xcSample.SetBackOpaqueBack(res_.back.opaque.back);
	m_xcSample.Invalidate(FALSE);
}