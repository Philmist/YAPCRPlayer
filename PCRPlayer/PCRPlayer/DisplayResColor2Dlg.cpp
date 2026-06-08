// DisplayResColor2Dlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayResColor2Dlg.h"
#include "afxdialogex.h"

#define TOP_INDEX 4

// CDisplayResColor2Dlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayResColor2Dlg, CDialog)

//CDisplayResColor2Dlg::CDisplayResColor2Dlg(CWnd* pParent /*=NULL*/)
//	: CDialog(CDisplayResColor2Dlg::IDD, pParent)
//{
//
//}

CDisplayResColor2Dlg::CDisplayResColor2Dlg(sl::ResConfig& res, CResSampleView& sample)
	: CDialog(CDisplayResColor2Dlg::IDD, NULL)
	, res_(res)
	, m_xcSample(sample)
{

}

CDisplayResColor2Dlg::~CDisplayResColor2Dlg()
{
}

void CDisplayResColor2Dlg::InitControls()
{
	for (int i = 0; i < 4; ++i)
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

void CDisplayResColor2Dlg::OnBnClickedBtnColorText(int index)
{
	int i = index + TOP_INDEX;
	res_.color.text[i] = m_xcBtnColorText[index].GetColor();
	m_xcSample.SetColorText(i, res_.color.text[i]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResColor2Dlg::OnBnClickedBtnColorLink(int index)
{
	int i = index + TOP_INDEX;
	res_.color.link[i] = m_xcBtnColorLink[index].GetColor();
	m_xcSample.SetColorLink(i, res_.color.link[i]);
	m_xcSample.Invalidate(FALSE);
}
void CDisplayResColor2Dlg::OnBnClickedBtnColorSage(int index)
{
	int i = index + TOP_INDEX;
	res_.color.sage[i] = m_xcBtnColorSage[index].GetColor();
	m_xcSample.SetColorSage(i, res_.color.sage[i]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResColor2Dlg::OnBnClickedCheckBold(int index)
{
	int i = index + TOP_INDEX;
	res_.color.bold[i] = m_xcCheckBold[index].GetCheck() == BST_CHECKED ? TRUE : FALSE;
	m_xcSample.SetColorBold(i, res_.color.bold[i]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResColor2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_COLOR_TEXT1, m_xcBtnColorText[0]);
	DDX_Control(pDX, IDC_BTN_COLOR_TEXT2, m_xcBtnColorText[1]);
	DDX_Control(pDX, IDC_BTN_COLOR_TEXT3, m_xcBtnColorText[2]);
	DDX_Control(pDX, IDC_BTN_COLOR_TEXT4, m_xcBtnColorText[3]);
	DDX_Control(pDX, IDC_BTN_COLOR_LINK1, m_xcBtnColorLink[0]);
	DDX_Control(pDX, IDC_BTN_COLOR_LINK2, m_xcBtnColorLink[1]);
	DDX_Control(pDX, IDC_BTN_COLOR_LINK3, m_xcBtnColorLink[2]);
	DDX_Control(pDX, IDC_BTN_COLOR_LINK4, m_xcBtnColorLink[3]);
	DDX_Control(pDX, IDC_BTN_COLOR_SAGE1, m_xcBtnColorSage[0]);
	DDX_Control(pDX, IDC_BTN_COLOR_SAGE2, m_xcBtnColorSage[1]);
	DDX_Control(pDX, IDC_BTN_COLOR_SAGE3, m_xcBtnColorSage[2]);
	DDX_Control(pDX, IDC_BTN_COLOR_SAGE4, m_xcBtnColorSage[3]);
	DDX_Control(pDX, IDC_CHECK_BOLD1, m_xcCheckBold[0]);
	DDX_Control(pDX, IDC_CHECK_BOLD2, m_xcCheckBold[1]);
	DDX_Control(pDX, IDC_CHECK_BOLD3, m_xcCheckBold[2]);
	DDX_Control(pDX, IDC_CHECK_BOLD4, m_xcCheckBold[3]);
}


BEGIN_MESSAGE_MAP(CDisplayResColor2Dlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_COLOR_TEXT1, &CDisplayResColor2Dlg::OnBnClickedBtnColorText1)
	ON_BN_CLICKED(IDC_BTN_COLOR_TEXT2, &CDisplayResColor2Dlg::OnBnClickedBtnColorText2)
	ON_BN_CLICKED(IDC_BTN_COLOR_TEXT3, &CDisplayResColor2Dlg::OnBnClickedBtnColorText3)
	ON_BN_CLICKED(IDC_BTN_COLOR_TEXT4, &CDisplayResColor2Dlg::OnBnClickedBtnColorText4)
	ON_BN_CLICKED(IDC_BTN_COLOR_LINK1, &CDisplayResColor2Dlg::OnBnClickedBtnColorLink1)
	ON_BN_CLICKED(IDC_BTN_COLOR_LINK2, &CDisplayResColor2Dlg::OnBnClickedBtnColorLink2)
	ON_BN_CLICKED(IDC_BTN_COLOR_LINK3, &CDisplayResColor2Dlg::OnBnClickedBtnColorLink3)
	ON_BN_CLICKED(IDC_BTN_COLOR_LINK4, &CDisplayResColor2Dlg::OnBnClickedBtnColorLink4)
	ON_BN_CLICKED(IDC_BTN_COLOR_SAGE1, &CDisplayResColor2Dlg::OnBnClickedBtnColorSage1)
	ON_BN_CLICKED(IDC_BTN_COLOR_SAGE2, &CDisplayResColor2Dlg::OnBnClickedBtnColorSage2)
	ON_BN_CLICKED(IDC_BTN_COLOR_SAGE3, &CDisplayResColor2Dlg::OnBnClickedBtnColorSage3)
	ON_BN_CLICKED(IDC_BTN_COLOR_SAGE4, &CDisplayResColor2Dlg::OnBnClickedBtnColorSage4)
	ON_BN_CLICKED(IDC_CHECK_BOLD1, &CDisplayResColor2Dlg::OnBnClickedCheckBold1)
	ON_BN_CLICKED(IDC_CHECK_BOLD2, &CDisplayResColor2Dlg::OnBnClickedCheckBold2)
	ON_BN_CLICKED(IDC_CHECK_BOLD3, &CDisplayResColor2Dlg::OnBnClickedCheckBold3)
	ON_BN_CLICKED(IDC_CHECK_BOLD4, &CDisplayResColor2Dlg::OnBnClickedCheckBold4)
END_MESSAGE_MAP()


// CDisplayResColor2Dlg メッセージ ハンドラー


BOOL CDisplayResColor2Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayResColor2Dlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayResColor2Dlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
}

void CDisplayResColor2Dlg::OnBnClickedBtnColorText1()
{
	OnBnClickedBtnColorText(0);
}

void CDisplayResColor2Dlg::OnBnClickedBtnColorText2()
{
	OnBnClickedBtnColorText(1);
}

void CDisplayResColor2Dlg::OnBnClickedBtnColorText3()
{
	OnBnClickedBtnColorText(2);
}

void CDisplayResColor2Dlg::OnBnClickedBtnColorText4()
{
	OnBnClickedBtnColorText(3);
}

void CDisplayResColor2Dlg::OnBnClickedBtnColorLink1()
{
	OnBnClickedBtnColorLink(0);
}

void CDisplayResColor2Dlg::OnBnClickedBtnColorLink2()
{
	OnBnClickedBtnColorLink(1);
}

void CDisplayResColor2Dlg::OnBnClickedBtnColorLink3()
{
	OnBnClickedBtnColorLink(2);
}

void CDisplayResColor2Dlg::OnBnClickedBtnColorLink4()
{
	OnBnClickedBtnColorLink(3);
}

void CDisplayResColor2Dlg::OnBnClickedBtnColorSage1()
{
	OnBnClickedBtnColorSage(0);
}

void CDisplayResColor2Dlg::OnBnClickedBtnColorSage2()
{
	OnBnClickedBtnColorSage(1);
}

void CDisplayResColor2Dlg::OnBnClickedBtnColorSage3()
{
	OnBnClickedBtnColorSage(2);
}

void CDisplayResColor2Dlg::OnBnClickedBtnColorSage4()
{
	OnBnClickedBtnColorSage(3);
}


void CDisplayResColor2Dlg::OnBnClickedCheckBold1()
{
	OnBnClickedCheckBold(0);
}

void CDisplayResColor2Dlg::OnBnClickedCheckBold2()
{
	OnBnClickedCheckBold(1);
}

void CDisplayResColor2Dlg::OnBnClickedCheckBold3()
{
	OnBnClickedCheckBold(2);
}

void CDisplayResColor2Dlg::OnBnClickedCheckBold4()
{
	OnBnClickedCheckBold(3);
}

