// DisplayDatetimeDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayDatetimeDlg.h"
//#include "afxdialogex.h"


// CDisplayDatetimeDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayDatetimeDlg, CDialog)

CDisplayDatetimeDlg::CDisplayDatetimeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisplayDatetimeDlg::IDD, pParent)
{

}

CDisplayDatetimeDlg::~CDisplayDatetimeDlg()
{
}

void CDisplayDatetimeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CMB_DATE, m_xcCmbDate);
	DDX_Control(pDX, IDC_CMB_TIME, m_xcCmbTime);
	DDX_Control(pDX, IDC_RADIO1, m_xcRadio[0]);
	DDX_Control(pDX, IDC_RADIO2, m_xcRadio[1]);
	DDX_Control(pDX, IDC_RADIO3, m_xcRadio[2]);
	DDX_Control(pDX, IDC_EDIT_REGEX, m_xcEditRegex);
	DDX_Control(pDX, IDC_EDIT1, m_xcEdit[0]);
	DDX_Control(pDX, IDC_EDIT2, m_xcEdit[1]);
	DDX_Control(pDX, IDC_EDIT3, m_xcEdit[2]);
	DDX_Control(pDX, IDC_EDIT4, m_xcEdit[3]);
	DDX_Control(pDX, IDC_EDIT5, m_xcEdit[4]);
	DDX_Control(pDX, IDC_EDIT6, m_xcEdit[5]);
	DDX_Control(pDX, IDC_SPIN1, m_xcSpin[0]);
	DDX_Control(pDX, IDC_SPIN2, m_xcSpin[1]);
	DDX_Control(pDX, IDC_SPIN3, m_xcSpin[2]);
	DDX_Control(pDX, IDC_SPIN4, m_xcSpin[3]);
	DDX_Control(pDX, IDC_SPIN5, m_xcSpin[4]);
	DDX_Control(pDX, IDC_SPIN6, m_xcSpin[5]);
}


BEGIN_MESSAGE_MAP(CDisplayDatetimeDlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RADIO1, &CDisplayDatetimeDlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CDisplayDatetimeDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CDisplayDatetimeDlg::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_BTN_ALL_DEFAULT, &CDisplayDatetimeDlg::OnBnClickedBtnAllDefault)
END_MESSAGE_MAP()

void CDisplayDatetimeDlg::InitControls()
{
	auto date = [](int format)->int{
		switch (format)
		{
		case sl::DATETIME_DATE_FORMAT_NONE: return 0;
		case sl::DATETIME_DATE_FORMAT_ENGLISH: return 1;
		case sl::DATETIME_DATE_FORMAT_KANJI: return 2;
		}
		return 1;
	};

	auto time = [](int format)->int {
		switch (format)
		{
		case sl::DATETIME_TIME_FORMAT_24: return 0;
		case sl::DATETIME_TIME_FORMAT_12_ENGLISH: return 1;
		case sl::DATETIME_TIME_FORMAT_12_KANJI: return 2;
		}
		return 0;
	};

	m_xcCmbDate.SetCurSel(date(datetime_.dateFormat));
	m_xcCmbTime.SetCurSel(time(datetime_.timeFormat));

	switch (datetime_.type)
	{
	case sl::DATETIME_TYPE_NONE: OnBnClickedRadio1(); break;
	case sl::DATETIME_TYPE_FILETIME: OnBnClickedRadio2(); break;
	case sl::DATETIME_TYPE_FILENAME:
	default:
		OnBnClickedRadio3();
		break;
	}

	m_xcEditRegex.SetWindowText(datetime_.exp.c_str());

	for (int i = 0; i < 6; ++i)
	{
		m_xcSpin[i].SetPos32(datetime_.sub[i]);
	}
}

void CDisplayDatetimeDlg::EnableControls(BOOL enable)
{
	m_xcEditRegex.EnableWindow(enable);

	for (int i = 0; i < 6; ++i)
	{
		m_xcEdit[i].EnableWindow(enable);
	}
}

// CDisplayDatetimeDlg メッセージ ハンドラー

BOOL CDisplayDatetimeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_xcCmbDate.InsertString(0, L"曜日なし");
	m_xcCmbDate.InsertString(1, L"曜日あり(英字)");
	m_xcCmbDate.InsertString(2, L"曜日あり(漢字)");

	m_xcCmbTime.InsertString(0, L"24時間");
	m_xcCmbTime.InsertString(1, L"12時間(英字)");
	m_xcCmbTime.InsertString(2, L"12時間(漢字)");

	for (int i = 0; i < 6; ++i)
	{
		m_xcSpin[i].SetRange32(0, 99);
	}

	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayDatetimeDlg::PreTranslateMessage(MSG* pMsg)
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


void CDisplayDatetimeDlg::OnDestroy()
{
	CDialog::OnDestroy();

	auto date = [](int format)->int {
		switch (format)
		{
		case 0: return sl::DATETIME_DATE_FORMAT_NONE;
		case 1: return sl::DATETIME_DATE_FORMAT_ENGLISH;
		case 2: return sl::DATETIME_DATE_FORMAT_KANJI;
		}
		return sl::DATETIME_DATE_FORMAT_KANJI;
	};

	auto time = [](int format)->int {
		switch (format)
		{
		case 0: return sl::DATETIME_TIME_FORMAT_24;
		case 1: return sl::DATETIME_TIME_FORMAT_12_ENGLISH;
		case 2: return sl::DATETIME_TIME_FORMAT_12_KANJI;
		}
		return sl::DATETIME_TIME_FORMAT_24;
	};

	datetime_.dateFormat = date(m_xcCmbDate.GetCurSel());
	datetime_.timeFormat = time(m_xcCmbTime.GetCurSel());

	if (m_xcRadio[0].GetCheck() == BST_CHECKED) { datetime_.type = sl::DATETIME_TYPE_NONE; }
	else if (m_xcRadio[1].GetCheck() == BST_CHECKED) { datetime_.type = sl::DATETIME_TYPE_FILETIME; }
	else { datetime_.type = sl::DATETIME_TYPE_FILENAME; }

	CString exp;
	m_xcEditRegex.GetWindowText(exp);
	datetime_.exp = exp;

	for (int i = 0; i < 6; ++i)
	{
		datetime_.sub[i] = m_xcSpin[i].GetPos32();
	}
}


void CDisplayDatetimeDlg::OnBnClickedRadio1()
{
	m_xcRadio[0].SetCheck(BST_CHECKED);
	m_xcRadio[1].SetCheck(BST_UNCHECKED);
	m_xcRadio[2].SetCheck(BST_UNCHECKED);

	EnableControls(FALSE);
}


void CDisplayDatetimeDlg::OnBnClickedRadio2()
{
	m_xcRadio[0].SetCheck(BST_UNCHECKED);
	m_xcRadio[1].SetCheck(BST_CHECKED);
	m_xcRadio[2].SetCheck(BST_UNCHECKED);

	EnableControls(FALSE);
}


void CDisplayDatetimeDlg::OnBnClickedRadio3()
{
	m_xcRadio[0].SetCheck(BST_UNCHECKED);
	m_xcRadio[1].SetCheck(BST_UNCHECKED);
	m_xcRadio[2].SetCheck(BST_CHECKED);

	EnableControls(TRUE);
}




void CDisplayDatetimeDlg::OnBnClickedBtnAllDefault()
{
	datetime_ = sl::DatetimeConfig();

	InitControls();
}
