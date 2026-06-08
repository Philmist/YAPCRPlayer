// DisplayResBoxDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayResBoxDlg.h"
#include "afxdialogex.h"


// CDisplayResBoxDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayResBoxDlg, CDialog)

//CDisplayResBoxDlg::CDisplayResBoxDlg(CWnd* pParent /*=NULL*/)
//	: CDialog(CDisplayResBoxDlg::IDD, pParent)
//{
//
//}

CDisplayResBoxDlg::CDisplayResBoxDlg(sl::ResConfig& res, CResSampleView& sample)
	: CDialog(CDisplayResBoxDlg::IDD, NULL)
	, res_(res)
	, m_xcSample(sample)
{

}

CDisplayResBoxDlg::~CDisplayResBoxDlg()
{
}

void CDisplayResBoxDlg::InitControls()
{
	for (int i = 0; i < 6; ++i)
	{
		m_xcBtnColor[i].SetColor(res_.box.color[i]);
		m_xcBtnColor[i].Invalidate(FALSE);
	}

	m_xcSpinMarginX.SetPos32(res_.box.marginX);
	m_xcSpinMarginY.SetPos32(res_.box.marginY);
	m_xcSpinSpace.SetPos32(res_.box.space);
	m_xcCheckPunct.SetCheck(res_.box.punct ? BST_CHECKED : BST_UNCHECKED);
	m_xcSpinSeparate.SetPos32(res_.box.separate);
}

void CDisplayResBoxDlg::OnBnClickedBtnColor(int index)
{
	res_.box.color[index] = m_xcBtnColor[index].GetColor();
	m_xcSample.SetBoxColor(index, res_.box.color[index]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResBoxDlg::DoDataExchange(CDataExchange* pDX)
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
	DDX_Control(pDX, IDC_SPIN_SPACE, m_xcSpinSpace);
	DDX_Control(pDX, IDC_CHECK_PUNCT, m_xcCheckPunct);
	DDX_Control(pDX, IDC_SPIN_SEPARATE, m_xcSpinSeparate);
}


BEGIN_MESSAGE_MAP(CDisplayResBoxDlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_COLOR1, &CDisplayResBoxDlg::OnBnClickedBtnColor1)
	ON_BN_CLICKED(IDC_BTN_COLOR2, &CDisplayResBoxDlg::OnBnClickedBtnColor2)
	ON_BN_CLICKED(IDC_BTN_COLOR3, &CDisplayResBoxDlg::OnBnClickedBtnColor3)
	ON_BN_CLICKED(IDC_BTN_COLOR4, &CDisplayResBoxDlg::OnBnClickedBtnColor4)
	ON_BN_CLICKED(IDC_BTN_COLOR5, &CDisplayResBoxDlg::OnBnClickedBtnColor5)
	ON_BN_CLICKED(IDC_BTN_COLOR6, &CDisplayResBoxDlg::OnBnClickedBtnColor6)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_X, &CDisplayResBoxDlg::OnEnChangeEditMarginX)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_Y, &CDisplayResBoxDlg::OnEnChangeEditMarginY)
	ON_EN_CHANGE(IDC_EDIT_SPACE, &CDisplayResBoxDlg::OnEnChangeEditSpace)
	ON_BN_CLICKED(IDC_CHECK_PUNCT, &CDisplayResBoxDlg::OnBnClickedCheckPunct)
	ON_EN_CHANGE(IDC_EDIT_SEPARATE, &CDisplayResBoxDlg::OnEnChangeEditSeparate)
END_MESSAGE_MAP()


// CDisplayResBoxDlg メッセージ ハンドラー


BOOL CDisplayResBoxDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_xcSpinMarginX.SetRange32(0, 100);
	m_xcSpinMarginY.SetRange32(0, 100);
	m_xcSpinSpace.SetRange32(0, 100);
	m_xcSpinSeparate.SetRange32(0, 100);

	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayResBoxDlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayResBoxDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
}


void CDisplayResBoxDlg::OnBnClickedBtnColor1()
{
	OnBnClickedBtnColor(0);
}


void CDisplayResBoxDlg::OnBnClickedBtnColor2()
{
	OnBnClickedBtnColor(1);
}


void CDisplayResBoxDlg::OnBnClickedBtnColor3()
{
	OnBnClickedBtnColor(2);
}


void CDisplayResBoxDlg::OnBnClickedBtnColor4()
{
	OnBnClickedBtnColor(3);
}


void CDisplayResBoxDlg::OnBnClickedBtnColor5()
{
	OnBnClickedBtnColor(4);
}


void CDisplayResBoxDlg::OnBnClickedBtnColor6()
{
	OnBnClickedBtnColor(5);
}


void CDisplayResBoxDlg::OnEnChangeEditMarginX()
{
	if (!IsWindow(m_xcSpinMarginX.GetSafeHwnd())) { return; }
	res_.box.marginX = m_xcSpinMarginX.GetPos32();
	m_xcSample.SetBoxMarginX(res_.box.marginX);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayResBoxDlg::OnEnChangeEditMarginY()
{
	if (!IsWindow(m_xcSpinMarginY.GetSafeHwnd())) { return; }
	res_.box.marginY = m_xcSpinMarginY.GetPos32();
	m_xcSample.SetBoxMarginY(res_.box.marginY);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayResBoxDlg::OnEnChangeEditSpace()
{
	if (!IsWindow(m_xcSpinSpace.GetSafeHwnd())) { return; }
	res_.box.space = m_xcSpinSpace.GetPos32();
	m_xcSample.SetBoxSpace(res_.box.space);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayResBoxDlg::OnBnClickedCheckPunct()
{
	if (!IsWindow(m_xcCheckPunct.GetSafeHwnd())) { return; }
	res_.box.punct = m_xcCheckPunct.GetCheck() == BST_CHECKED;
	m_xcSample.SetBoxPunct(res_.box.punct);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayResBoxDlg::OnEnChangeEditSeparate()
{
	if (!IsWindow(m_xcSpinSeparate.GetSafeHwnd())) { return; }
	res_.box.separate = m_xcSpinSeparate.GetPos32();
	m_xcSample.SetBoxSeparate(res_.box.separate);
	m_xcSample.Invalidate(FALSE);
}
