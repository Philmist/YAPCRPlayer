// DisplayResColor1Dlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayResColor1Dlg.h"
#include "afxdialogex.h"


// CDisplayResColor1Dlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayResColor1Dlg, CDialog)

//CDisplayResColor1Dlg::CDisplayResColor1Dlg(CWnd* pParent /*=NULL*/)
//	: CDialog(CDisplayResColor1Dlg::IDD, pParent)
//{
//
//}

CDisplayResColor1Dlg::CDisplayResColor1Dlg(sl::ResConfig& res, CResSampleView& sample)
	: CDialog(CDisplayResColor1Dlg::IDD, NULL)
	, res_(res)
	, m_xcSample(sample)
{

}

CDisplayResColor1Dlg::~CDisplayResColor1Dlg()
{
}

void CDisplayResColor1Dlg::InitControls()
{
	for (int i = 0; i < 4; ++i)
	{
		m_xcBtnColorText[i].SetColor(res_.color.text[i]);
		m_xcBtnColorText[i].Invalidate(FALSE);

		m_xcBtnColorLink[i].SetColor(res_.color.link[i]);
		m_xcBtnColorLink[i].Invalidate(FALSE);

		m_xcBtnColorSage[i].SetColor(res_.color.sage[i]);
		m_xcBtnColorSage[i].Invalidate(FALSE);

		m_xcCheckBold[i].SetCheck(res_.color.bold[i] ? BST_CHECKED : BST_UNCHECKED);
	}
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorText(int index)
{
	res_.color.text[index] = m_xcBtnColorText[index].GetColor();
	m_xcSample.SetColorText(index, res_.color.text[index]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorLink(int index)
{
	res_.color.link[index] = m_xcBtnColorLink[index].GetColor();
	m_xcSample.SetColorLink(index, res_.color.link[index]);
	m_xcSample.Invalidate(FALSE);
}
void CDisplayResColor1Dlg::OnBnClickedBtnColorSage(int index)
{
	res_.color.sage[index] = m_xcBtnColorSage[index].GetColor();
	m_xcSample.SetColorSage(index, res_.color.sage[index]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResColor1Dlg::OnBnClickedCheckBold(int index)
{
	res_.color.bold[index] = m_xcCheckBold[index].GetCheck() == BST_CHECKED ? TRUE : FALSE;
	m_xcSample.SetColorBold(index, res_.color.bold[index]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResColor1Dlg::DoDataExchange(CDataExchange* pDX)
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


BEGIN_MESSAGE_MAP(CDisplayResColor1Dlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_COLOR_TEXT1, &CDisplayResColor1Dlg::OnBnClickedBtnColorText1)
	ON_BN_CLICKED(IDC_BTN_COLOR_TEXT2, &CDisplayResColor1Dlg::OnBnClickedBtnColorText2)
	ON_BN_CLICKED(IDC_BTN_COLOR_TEXT3, &CDisplayResColor1Dlg::OnBnClickedBtnColorText3)
	ON_BN_CLICKED(IDC_BTN_COLOR_TEXT4, &CDisplayResColor1Dlg::OnBnClickedBtnColorText4)
	ON_BN_CLICKED(IDC_BTN_COLOR_LINK1, &CDisplayResColor1Dlg::OnBnClickedBtnColorLink1)
	ON_BN_CLICKED(IDC_BTN_COLOR_LINK2, &CDisplayResColor1Dlg::OnBnClickedBtnColorLink2)
	ON_BN_CLICKED(IDC_BTN_COLOR_LINK3, &CDisplayResColor1Dlg::OnBnClickedBtnColorLink3)
	ON_BN_CLICKED(IDC_BTN_COLOR_LINK4, &CDisplayResColor1Dlg::OnBnClickedBtnColorLink4)
	ON_BN_CLICKED(IDC_BTN_COLOR_SAGE1, &CDisplayResColor1Dlg::OnBnClickedBtnColorSage1)
	ON_BN_CLICKED(IDC_BTN_COLOR_SAGE2, &CDisplayResColor1Dlg::OnBnClickedBtnColorSage2)
	ON_BN_CLICKED(IDC_BTN_COLOR_SAGE3, &CDisplayResColor1Dlg::OnBnClickedBtnColorSage3)
	ON_BN_CLICKED(IDC_BTN_COLOR_SAGE4, &CDisplayResColor1Dlg::OnBnClickedBtnColorSage4)
	ON_BN_CLICKED(IDC_CHECK_BOLD1, &CDisplayResColor1Dlg::OnBnClickedCheckBold1)
	ON_BN_CLICKED(IDC_CHECK_BOLD2, &CDisplayResColor1Dlg::OnBnClickedCheckBold2)
	ON_BN_CLICKED(IDC_CHECK_BOLD3, &CDisplayResColor1Dlg::OnBnClickedCheckBold3)
	ON_BN_CLICKED(IDC_CHECK_BOLD4, &CDisplayResColor1Dlg::OnBnClickedCheckBold4)
END_MESSAGE_MAP()


// CDisplayResColor1Dlg メッセージ ハンドラー


BOOL CDisplayResColor1Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayResColor1Dlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayResColor1Dlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorText1()
{
	OnBnClickedBtnColorText(0);
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorText2()
{
	OnBnClickedBtnColorText(1);
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorText3()
{
	OnBnClickedBtnColorText(2);
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorText4()
{
	OnBnClickedBtnColorText(3);
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorLink1()
{
	OnBnClickedBtnColorLink(0);
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorLink2()
{
	OnBnClickedBtnColorLink(1);
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorLink3()
{
	OnBnClickedBtnColorLink(2);
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorLink4()
{
	OnBnClickedBtnColorLink(3);
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorSage1()
{
	OnBnClickedBtnColorSage(0);
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorSage2()
{
	OnBnClickedBtnColorSage(1);
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorSage3()
{
	OnBnClickedBtnColorSage(2);
}

void CDisplayResColor1Dlg::OnBnClickedBtnColorSage4()
{
	OnBnClickedBtnColorSage(3);
}


void CDisplayResColor1Dlg::OnBnClickedCheckBold1()
{
	OnBnClickedCheckBold(0);
}

void CDisplayResColor1Dlg::OnBnClickedCheckBold2()
{
	OnBnClickedCheckBold(1);
}

void CDisplayResColor1Dlg::OnBnClickedCheckBold3()
{
	OnBnClickedCheckBold(2);
}

void CDisplayResColor1Dlg::OnBnClickedCheckBold4()
{
	OnBnClickedCheckBold(3);
}

