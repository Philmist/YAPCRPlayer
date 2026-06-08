// DisplayStatusDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayStatusDlg.h"
//#include "afxdialogex.h"


// CDisplayStatusDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayStatusDlg, CDialog)

CDisplayStatusDlg::CDisplayStatusDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisplayStatusDlg::IDD, pParent)
{

}

CDisplayStatusDlg::~CDisplayStatusDlg()
{
}

void CDisplayStatusDlg::InitControls()
{
	for (int i = 0; i < 4; ++i)
	{
		m_xcBtnColor[i].SetColor(status_.color[i]);
		m_xcBtnColor[i].Invalidate(FALSE);
	}

	for (int i = 0; i < 1; ++i)
	{
		m_xcBtnText[i].SetColor(status_.text[i]);
		m_xcBtnText[i].Invalidate(FALSE);
	}

	m_xcSample.SetFontIndirect(status_.font);
	m_xcBtnFont.SetFontIndirect(status_.font);
	m_xcBtnFont.Invalidate(FALSE);


	m_xcSpinMarginX.SetPos32(status_.marginX);
	m_xcSpinMarginY.SetPos32(status_.marginY);

	m_xcSample.SetConfig(status_);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayStatusDlg::OnBnClickedBtnColor(int index)
{
	status_.color[index] = m_xcBtnColor[index].GetColor();
	m_xcSample.SetColor(index, status_.color[index]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayStatusDlg::OnBnClickedBtnText(int index)
{
	status_.text[index] = m_xcBtnText[index].GetColor();
	m_xcSample.SetText(index, status_.text[index]);
	m_xcSample.Invalidate(FALSE);
}



void CDisplayStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SAMPLE, m_xcSample);
	DDX_Control(pDX, IDC_BTN_COLOR1, m_xcBtnColor[0]);
	DDX_Control(pDX, IDC_BTN_COLOR2, m_xcBtnColor[1]);
	DDX_Control(pDX, IDC_BTN_COLOR3, m_xcBtnColor[2]);
	DDX_Control(pDX, IDC_BTN_COLOR4, m_xcBtnColor[3]);
	DDX_Control(pDX, IDC_BTN_TEXT1, m_xcBtnText[0]);
	DDX_Control(pDX, IDC_BTN_FONT, m_xcBtnFont);
	DDX_Control(pDX, IDC_EDIT_MARGIN_X, m_xcEditMarginX);
	DDX_Control(pDX, IDC_SPIN_MARGIN_X, m_xcSpinMarginX);
	DDX_Control(pDX, IDC_EDIT_MARGIN_Y, m_xcEditMarginY);
	DDX_Control(pDX, IDC_SPIN_MARGIN_Y, m_xcSpinMarginY);
	DDX_Control(pDX, IDC_CMB_CUSTOM, m_xcCmbCustom);
}


BEGIN_MESSAGE_MAP(CDisplayStatusDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_COLOR1, &CDisplayStatusDlg::OnBnClickedBtnColor1)
	ON_BN_CLICKED(IDC_BTN_COLOR2, &CDisplayStatusDlg::OnBnClickedBtnColor2)
	ON_BN_CLICKED(IDC_BTN_COLOR3, &CDisplayStatusDlg::OnBnClickedBtnColor3)
	ON_BN_CLICKED(IDC_BTN_COLOR4, &CDisplayStatusDlg::OnBnClickedBtnColor4)
	ON_BN_CLICKED(IDC_BTN_TEXT1, &CDisplayStatusDlg::OnBnClickedBtnText1)
	ON_BN_CLICKED(IDC_BTN_FONT, &CDisplayStatusDlg::OnBnClickedBtnFont)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_X, &CDisplayStatusDlg::OnEnChangeEditMarginX)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_Y, &CDisplayStatusDlg::OnEnChangeEditMarginY)
	ON_BN_CLICKED(IDC_BTN_SAMPLE, &CDisplayStatusDlg::OnBnClickedBtnSample)
	ON_BN_CLICKED(IDC_BTN_ALL_DEFAULT, &CDisplayStatusDlg::OnBnClickedBtnAllDefault)
	ON_BN_CLICKED(IDC_BTN_ADD, &CDisplayStatusDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, &CDisplayStatusDlg::OnBnClickedBtnDel)
	ON_CBN_SELCHANGE(IDC_CMB_CUSTOM, &CDisplayStatusDlg::OnCbnSelchangeCmbCustom)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDisplayStatusDlg メッセージ ハンドラー


BOOL CDisplayStatusDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_xcSpinMarginX.SetRange32(0, 100);
	m_xcSpinMarginY.SetRange32(0, 100);

	InitControls();

	for (auto it = custom_.begin(); it != custom_.end(); ++it)
	{
		m_xcCmbCustom.AddItem(it->first.c_str(), boost::make_shared<sl::StatusConfig>(it->second));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayStatusDlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayStatusDlg::OnBnClickedBtnColor1()
{
	OnBnClickedBtnColor(0);
}

void CDisplayStatusDlg::OnBnClickedBtnColor2()
{
	OnBnClickedBtnColor(1);
}

void CDisplayStatusDlg::OnBnClickedBtnColor3()
{
	OnBnClickedBtnColor(2);
}

void CDisplayStatusDlg::OnBnClickedBtnColor4()
{
	OnBnClickedBtnColor(3);
}



void CDisplayStatusDlg::OnBnClickedBtnText1()
{
	OnBnClickedBtnText(0);
}


void CDisplayStatusDlg::OnBnClickedBtnFont()
{
	LOGFONT lf = {};
	if (m_xcBtnFont.DoModal(lf))
	{
		status_.font = lf;
		m_xcSample.SetFontIndirect(lf);
		m_xcSample.Invalidate(FALSE);
	}

	/*
	LOGFONT lf = {};
	m_xcBtnFont.GetLogFont(lf);
	status_.font = lf;
	m_xcSample.SetFontIndirect(lf);
	m_xcSample.Invalidate(FALSE);
	*/
}



void CDisplayStatusDlg::OnEnChangeEditMarginX()
{
	if (!IsWindow(m_xcSpinMarginX.GetSafeHwnd())) { return; }
	status_.marginX = m_xcSpinMarginX.GetPos32();
	m_xcSample.SetMarginX(status_.marginX);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayStatusDlg::OnEnChangeEditMarginY()
{
	if (!IsWindow(m_xcSpinMarginY.GetSafeHwnd())) { return; }
	status_.marginY = m_xcSpinMarginY.GetPos32();
	m_xcSample.SetMarginY(status_.marginY);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayStatusDlg::OnBnClickedBtnSample()
{
	status_ = sl::StatusConfig();

	status_.text[0] = RGB(255, 255, 255);

	status_.color[0] = RGB(0, 0, 0);
	status_.color[1] = RGB(0, 0, 0);
	status_.color[2] = RGB(0, 0, 0);
	status_.color[3] = RGB(0, 0, 0);

	InitControls();
}


void CDisplayStatusDlg::OnBnClickedBtnAllDefault()
{
	status_ = sl::StatusConfig();
	InitControls();
}



void CDisplayStatusDlg::OnCbnSelchangeCmbCustom()
{
	auto data = m_xcCmbCustom.GetItem();
	if (data)
	{
		status_ = *static_cast<sl::StatusConfig*>(data.get());
		InitControls();
	}
}

void CDisplayStatusDlg::OnBnClickedBtnAdd()
{
	m_xcCmbCustom.AddItem(boost::make_shared<sl::StatusConfig>(status_));
}


void CDisplayStatusDlg::OnBnClickedBtnDel()
{
	m_xcCmbCustom.DelItem();
}


void CDisplayStatusDlg::OnDestroy()
{
	CDialog::OnDestroy();

	custom_.clear();
	m_xcCmbCustom.Forearch(
		[this](const CString& name, const boost::shared_ptr<void>& i){
		custom_.push_back(std::make_pair(std::wstring(name), *static_cast<sl::StatusConfig*>(i.get())));
	});
}


