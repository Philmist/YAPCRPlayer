// DisplayResLatestDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayResLatestDlg.h"
#include "afxdialogex.h"


// CDisplayResLatestDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayResLatestDlg, CDialog)

//CDisplayResLatestDlg::CDisplayResLatestDlg(CWnd* pParent /*=NULL*/)
//	: CDialog(CDisplayResLatestDlg::IDD, pParent)
//{
//
//}

CDisplayResLatestDlg::CDisplayResLatestDlg(sl::ResConfig& res, CResSampleView& sample)
	: CDialog(CDisplayResLatestDlg::IDD, NULL)
	, res_(res)
	, m_xcSample(sample)
{

}

CDisplayResLatestDlg::~CDisplayResLatestDlg()
{
}

void CDisplayResLatestDlg::InitControls()
{
	for (int i = 0; i < 6; ++i)
	{
		m_xcBtnColor[i].SetColor(res_.box.latest[i]);
		m_xcBtnColor[i].Invalidate(FALSE);
	}
}

void CDisplayResLatestDlg::OnBnClickedBtnColor(int index)
{
	res_.box.latest[index] = m_xcBtnColor[index].GetColor();
	m_xcSample.SetBoxLatest(index, res_.box.latest[index]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayResLatestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_COLOR1, m_xcBtnColor[0]);
	DDX_Control(pDX, IDC_BTN_COLOR2, m_xcBtnColor[1]);
	DDX_Control(pDX, IDC_BTN_COLOR3, m_xcBtnColor[2]);
	DDX_Control(pDX, IDC_BTN_COLOR4, m_xcBtnColor[3]);
	DDX_Control(pDX, IDC_BTN_COLOR5, m_xcBtnColor[4]);
	DDX_Control(pDX, IDC_BTN_COLOR6, m_xcBtnColor[5]);
}


BEGIN_MESSAGE_MAP(CDisplayResLatestDlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_COLOR1, &CDisplayResLatestDlg::OnBnClickedBtnColor1)
	ON_BN_CLICKED(IDC_BTN_COLOR2, &CDisplayResLatestDlg::OnBnClickedBtnColor2)
	ON_BN_CLICKED(IDC_BTN_COLOR3, &CDisplayResLatestDlg::OnBnClickedBtnColor3)
	ON_BN_CLICKED(IDC_BTN_COLOR4, &CDisplayResLatestDlg::OnBnClickedBtnColor4)
	ON_BN_CLICKED(IDC_BTN_COLOR5, &CDisplayResLatestDlg::OnBnClickedBtnColor5)
	ON_BN_CLICKED(IDC_BTN_COLOR6, &CDisplayResLatestDlg::OnBnClickedBtnColor6)
END_MESSAGE_MAP()


// CDisplayResLatestDlg メッセージ ハンドラー


BOOL CDisplayResLatestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayResLatestDlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayResLatestDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
}


void CDisplayResLatestDlg::OnBnClickedBtnColor1()
{
	OnBnClickedBtnColor(0);
}


void CDisplayResLatestDlg::OnBnClickedBtnColor2()
{
	OnBnClickedBtnColor(1);
}


void CDisplayResLatestDlg::OnBnClickedBtnColor3()
{
	OnBnClickedBtnColor(2);
}


void CDisplayResLatestDlg::OnBnClickedBtnColor4()
{
	OnBnClickedBtnColor(3);
}


void CDisplayResLatestDlg::OnBnClickedBtnColor5()
{
	OnBnClickedBtnColor(4);
}


void CDisplayResLatestDlg::OnBnClickedBtnColor6()
{
	OnBnClickedBtnColor(5);
}
