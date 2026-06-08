// DisplayResColor3Dlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayResColor3Dlg.h"
#include "afxdialogex.h"

#define TOP_INDEX 8

// CDisplayResColor3Dlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayResColor3Dlg, CDialog)

//CDisplayResColor3Dlg::CDisplayResColor3Dlg(CWnd* pParent /*=NULL*/)
//	: CDialog(CDisplayResColor3Dlg::IDD, pParent)
//{
//
//}

CDisplayResColor3Dlg::CDisplayResColor3Dlg(sl::ResConfig& res, CResSampleView& sample)
	: CDialog(CDisplayResColor3Dlg::IDD, NULL)
	, res_(res)
	, m_xcSample(sample)
{

}

CDisplayResColor3Dlg::~CDisplayResColor3Dlg()
{
}

void CDisplayResColor3Dlg::InitControls()
{
	for (int i = 0; i < 3; ++i)
	{
		int j = i + TOP_INDEX;
		m_xcBtnColorText[i].SetColor(res_.color.text[j]);
		m_xcBtnColorText[i].Invalidate(FALSE);

		m_xcBtnColorLink[i].SetColor(res_.color.link[j]);
		m_xcBtnColorLink[i].Invalidate(FALSE);

		m_xcBtnColorSage[i].SetColor(res_.color.sage[j]);
		m_xcBtnColorSage[i].Invalidate(FALSE);

		m_xcCheckBold[i].SetCheck(res_.color.bold[j] ? BST_CHECKED : BST_UNCHECKED);
	}
}

void CDisplayResColor3Dlg::OnBnClickedBtnColorText(int index)
{
	int i = index + TOP_INDEX;
	res_.color.text[i] = m_xcBtnColorText[index].GetColor();
	m_xcSample.SetColorText(i, res_.color.text[i]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResColor3Dlg::OnBnClickedBtnColorLink(int index)
{
	int i = index + TOP_INDEX;
	res_.color.link[i] = m_xcBtnColorLink[index].GetColor();
	m_xcSample.SetColorLink(i, res_.color.link[i]);
	m_xcSample.Invalidate(FALSE);
}
void CDisplayResColor3Dlg::OnBnClickedBtnColorSage(int index)
{
	int i = index + TOP_INDEX;
	res_.color.sage[i] = m_xcBtnColorSage[index].GetColor();
	m_xcSample.SetColorSage(i, res_.color.sage[i]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResColor3Dlg::OnBnClickedCheckBold(int index)
{
	int i = index + TOP_INDEX;
	res_.color.bold[i] = m_xcCheckBold[index].GetCheck() == BST_CHECKED ? TRUE : FALSE;
	m_xcSample.SetColorBold(i, res_.color.bold[i]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResColor3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_COLOR_TEXT1, m_xcBtnColorText[0]);
	DDX_Control(pDX, IDC_BTN_COLOR_TEXT2, m_xcBtnColorText[1]);
	DDX_Control(pDX, IDC_BTN_COLOR_TEXT3, m_xcBtnColorText[2]);
	DDX_Control(pDX, IDC_BTN_COLOR_LINK1, m_xcBtnColorLink[0]);
	DDX_Control(pDX, IDC_BTN_COLOR_LINK2, m_xcBtnColorLink[1]);
	DDX_Control(pDX, IDC_BTN_COLOR_LINK3, m_xcBtnColorLink[2]);
	DDX_Control(pDX, IDC_BTN_COLOR_SAGE1, m_xcBtnColorSage[0]);
	DDX_Control(pDX, IDC_BTN_COLOR_SAGE2, m_xcBtnColorSage[1]);
	DDX_Control(pDX, IDC_BTN_COLOR_SAGE3, m_xcBtnColorSage[2]);
	DDX_Control(pDX, IDC_CHECK_BOLD1, m_xcCheckBold[0]);
	DDX_Control(pDX, IDC_CHECK_BOLD2, m_xcCheckBold[1]);
	DDX_Control(pDX, IDC_CHECK_BOLD3, m_xcCheckBold[2]);
}


BEGIN_MESSAGE_MAP(CDisplayResColor3Dlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_COLOR_TEXT1, &CDisplayResColor3Dlg::OnBnClickedBtnColorText1)
	ON_BN_CLICKED(IDC_BTN_COLOR_TEXT2, &CDisplayResColor3Dlg::OnBnClickedBtnColorText2)
	ON_BN_CLICKED(IDC_BTN_COLOR_TEXT3, &CDisplayResColor3Dlg::OnBnClickedBtnColorText3)
	ON_BN_CLICKED(IDC_BTN_COLOR_LINK1, &CDisplayResColor3Dlg::OnBnClickedBtnColorLink1)
	ON_BN_CLICKED(IDC_BTN_COLOR_LINK2, &CDisplayResColor3Dlg::OnBnClickedBtnColorLink2)
	ON_BN_CLICKED(IDC_BTN_COLOR_LINK3, &CDisplayResColor3Dlg::OnBnClickedBtnColorLink3)
	ON_BN_CLICKED(IDC_BTN_COLOR_SAGE1, &CDisplayResColor3Dlg::OnBnClickedBtnColorSage1)
	ON_BN_CLICKED(IDC_BTN_COLOR_SAGE2, &CDisplayResColor3Dlg::OnBnClickedBtnColorSage2)
	ON_BN_CLICKED(IDC_BTN_COLOR_SAGE3, &CDisplayResColor3Dlg::OnBnClickedBtnColorSage3)
	ON_BN_CLICKED(IDC_CHECK_BOLD1, &CDisplayResColor3Dlg::OnBnClickedCheckBold1)
	ON_BN_CLICKED(IDC_CHECK_BOLD2, &CDisplayResColor3Dlg::OnBnClickedCheckBold2)
	ON_BN_CLICKED(IDC_CHECK_BOLD3, &CDisplayResColor3Dlg::OnBnClickedCheckBold3)
END_MESSAGE_MAP()


// CDisplayResColor3Dlg メッセージ ハンドラー


BOOL CDisplayResColor3Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayResColor3Dlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayResColor3Dlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
}

void CDisplayResColor3Dlg::OnBnClickedBtnColorText1()
{
	OnBnClickedBtnColorText(0);
}

void CDisplayResColor3Dlg::OnBnClickedBtnColorText2()
{
	OnBnClickedBtnColorText(1);
}

void CDisplayResColor3Dlg::OnBnClickedBtnColorText3()
{
	OnBnClickedBtnColorText(2);
}


void CDisplayResColor3Dlg::OnBnClickedBtnColorLink1()
{
	OnBnClickedBtnColorLink(0);
}

void CDisplayResColor3Dlg::OnBnClickedBtnColorLink2()
{
	OnBnClickedBtnColorLink(1);
}

void CDisplayResColor3Dlg::OnBnClickedBtnColorLink3()
{
	OnBnClickedBtnColorLink(2);
}


void CDisplayResColor3Dlg::OnBnClickedBtnColorSage1()
{
	OnBnClickedBtnColorSage(0);
}

void CDisplayResColor3Dlg::OnBnClickedBtnColorSage2()
{
	OnBnClickedBtnColorSage(1);
}

void CDisplayResColor3Dlg::OnBnClickedBtnColorSage3()
{
	OnBnClickedBtnColorSage(2);
}


void CDisplayResColor3Dlg::OnBnClickedCheckBold1()
{
	OnBnClickedCheckBold(0);
}

void CDisplayResColor3Dlg::OnBnClickedCheckBold2()
{
	OnBnClickedCheckBold(1);
}

void CDisplayResColor3Dlg::OnBnClickedCheckBold3()
{
	OnBnClickedCheckBold(2);
}


