// DisplaySeekDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplaySeekDlg.h"
//#include "afxdialogex.h"


// CDisplaySeekDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplaySeekDlg, CDialog)

CDisplaySeekDlg::CDisplaySeekDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisplaySeekDlg::IDD, pParent)
{

}

CDisplaySeekDlg::~CDisplaySeekDlg()
{
}

void CDisplaySeekDlg::InitControls()
{
	for (int i = 0; i < 10; ++i)
	{
		m_xcBtnColor[i].SetColor(seek_.color[i]);
		m_xcBtnColor[i].Invalidate(FALSE);
	}

	for (int i = 0; i < 2; ++i)
	{
		m_xcBtnText[i].SetColor(seek_.text[i]);
		m_xcBtnText[i].Invalidate(FALSE);
	}

	m_xcSample.SetFontIndirect(seek_.font);
	m_xcBtnFont.SetFontIndirect(seek_.font);
	m_xcBtnFont.Invalidate(FALSE);

	m_xcSpinMarginX.SetPos32(seek_.marginX);
	m_xcSpinMarginY.SetPos32(seek_.marginY);
	m_xcSpinKnob.SetPos32(seek_.knob);

	m_xcSample.SetConfig(seek_);
	m_xcSample.Invalidate(FALSE);
}

void CDisplaySeekDlg::OnBnClickedBtnColor(int index)
{
	seek_.color[index] = m_xcBtnColor[index].GetColor();
	m_xcSample.SetColor(index, seek_.color[index]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplaySeekDlg::OnBnClickedBtnText(int index)
{
	seek_.text[index] = m_xcBtnText[index].GetColor();
	m_xcSample.SetText(index, seek_.text[index]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplaySeekDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SAMPLE, m_xcSample);
	DDX_Control(pDX, IDC_BTN_COLOR1, m_xcBtnColor[0]);
	DDX_Control(pDX, IDC_BTN_COLOR2, m_xcBtnColor[1]);
	DDX_Control(pDX, IDC_BTN_COLOR3, m_xcBtnColor[2]);
	DDX_Control(pDX, IDC_BTN_COLOR4, m_xcBtnColor[3]);
	DDX_Control(pDX, IDC_BTN_COLOR5, m_xcBtnColor[4]);
	DDX_Control(pDX, IDC_BTN_COLOR6, m_xcBtnColor[5]);
	DDX_Control(pDX, IDC_BTN_COLOR7, m_xcBtnColor[6]);
	DDX_Control(pDX, IDC_BTN_COLOR8, m_xcBtnColor[7]);
	DDX_Control(pDX, IDC_BTN_COLOR9, m_xcBtnColor[8]);
	DDX_Control(pDX, IDC_BTN_COLOR10, m_xcBtnColor[9]);
	DDX_Control(pDX, IDC_BTN_TEXT1, m_xcBtnText[0]);
	DDX_Control(pDX, IDC_BTN_TEXT2, m_xcBtnText[1]);
	DDX_Control(pDX, IDC_BTN_FONT, m_xcBtnFont);
	DDX_Control(pDX, IDC_EDIT_MARGIN_X, m_xcEditMarginX);
	DDX_Control(pDX, IDC_SPIN_MARGIN_X, m_xcSpinMarginX);
	DDX_Control(pDX, IDC_EDIT_MARGIN_Y, m_xcEditMarginY);
	DDX_Control(pDX, IDC_SPIN_MARGIN_Y, m_xcSpinMarginY);
	DDX_Control(pDX, IDC_EDIT_KNOB, m_xcEditKnob);
	DDX_Control(pDX, IDC_SPIN_KNOB, m_xcSpinKnob);
	DDX_Control(pDX, IDC_CMB_CUSTOM, m_xcCmbCustom);
}


BEGIN_MESSAGE_MAP(CDisplaySeekDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_COLOR1, &CDisplaySeekDlg::OnBnClickedBtnColor1)
	ON_BN_CLICKED(IDC_BTN_COLOR2, &CDisplaySeekDlg::OnBnClickedBtnColor2)
	ON_BN_CLICKED(IDC_BTN_COLOR3, &CDisplaySeekDlg::OnBnClickedBtnColor3)
	ON_BN_CLICKED(IDC_BTN_COLOR4, &CDisplaySeekDlg::OnBnClickedBtnColor4)
	ON_BN_CLICKED(IDC_BTN_COLOR5, &CDisplaySeekDlg::OnBnClickedBtnColor5)
	ON_BN_CLICKED(IDC_BTN_COLOR6, &CDisplaySeekDlg::OnBnClickedBtnColor6)
	ON_BN_CLICKED(IDC_BTN_COLOR7, &CDisplaySeekDlg::OnBnClickedBtnColor7)
	ON_BN_CLICKED(IDC_BTN_COLOR8, &CDisplaySeekDlg::OnBnClickedBtnColor8)
	ON_BN_CLICKED(IDC_BTN_COLOR9, &CDisplaySeekDlg::OnBnClickedBtnColor9)
	ON_BN_CLICKED(IDC_BTN_COLOR10, &CDisplaySeekDlg::OnBnClickedBtnColor10)
	ON_BN_CLICKED(IDC_BTN_TEXT1, &CDisplaySeekDlg::OnBnClickedBtnText1)
	ON_BN_CLICKED(IDC_BTN_TEXT2, &CDisplaySeekDlg::OnBnClickedBtnText2)
	ON_BN_CLICKED(IDC_BTN_FONT, &CDisplaySeekDlg::OnBnClickedBtnFont)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_X, &CDisplaySeekDlg::OnEnChangeEditMarginX)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_Y, &CDisplaySeekDlg::OnEnChangeEditMarginY)
	ON_EN_CHANGE(IDC_EDIT_KNOB, &CDisplaySeekDlg::OnEnChangeEditKnob)
	ON_BN_CLICKED(IDC_BTN_SAMPLE, &CDisplaySeekDlg::OnBnClickedBtnSample)
	ON_BN_CLICKED(IDC_BTN_ALL_DEFAULT, &CDisplaySeekDlg::OnBnClickedBtnAllDefault)
	ON_BN_CLICKED(IDC_BTN_ADD, &CDisplaySeekDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, &CDisplaySeekDlg::OnBnClickedBtnDel)
	ON_CBN_SELCHANGE(IDC_CMB_CUSTOM, &CDisplaySeekDlg::OnCbnSelchangeCmbCustom)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDisplaySeekDlg メッセージ ハンドラー


BOOL CDisplaySeekDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_xcSpinMarginX.SetRange32(0, 100);
	m_xcSpinMarginY.SetRange32(0, 100);
	m_xcSpinKnob.SetRange32(0, 100);

	InitControls();

	for (auto it = custom_.begin(); it != custom_.end(); ++it)
	{
		m_xcCmbCustom.AddItem(it->first.c_str(), boost::make_shared<sl::SeekConfig>(it->second));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplaySeekDlg::PreTranslateMessage(MSG* pMsg)
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


void CDisplaySeekDlg::OnBnClickedBtnColor1()
{
	OnBnClickedBtnColor(0);
}


void CDisplaySeekDlg::OnBnClickedBtnColor2()
{
	OnBnClickedBtnColor(1);
}


void CDisplaySeekDlg::OnBnClickedBtnColor3()
{
	OnBnClickedBtnColor(2);
}


void CDisplaySeekDlg::OnBnClickedBtnColor4()
{
	OnBnClickedBtnColor(3);
}


void CDisplaySeekDlg::OnBnClickedBtnColor5()
{
	OnBnClickedBtnColor(4);
}


void CDisplaySeekDlg::OnBnClickedBtnColor6()
{
	OnBnClickedBtnColor(5);
}


void CDisplaySeekDlg::OnBnClickedBtnColor7()
{
	OnBnClickedBtnColor(6);
}


void CDisplaySeekDlg::OnBnClickedBtnColor8()
{
	OnBnClickedBtnColor(7);
}

void CDisplaySeekDlg::OnBnClickedBtnColor9()
{
	OnBnClickedBtnColor(8);
}

void CDisplaySeekDlg::OnBnClickedBtnColor10()
{
	OnBnClickedBtnColor(9);
}


void CDisplaySeekDlg::OnBnClickedBtnText1()
{
	OnBnClickedBtnText(0);
}


void CDisplaySeekDlg::OnBnClickedBtnText2()
{
	OnBnClickedBtnText(1);
}


void CDisplaySeekDlg::OnBnClickedBtnFont()
{
	LOGFONT lf = {};
	if (m_xcBtnFont.DoModal(lf))
	{
		seek_.font = lf;
		m_xcSample.SetFontIndirect(lf);
		m_xcSample.Invalidate(FALSE);
	}
}


void CDisplaySeekDlg::OnEnChangeEditMarginX()
{
	if (!IsWindow(m_xcSpinMarginX.GetSafeHwnd())) { return; }
	seek_.marginX = m_xcSpinMarginX.GetPos32();
	m_xcSample.SetMarginX(seek_.marginX);
	m_xcSample.Invalidate(FALSE);
}


void CDisplaySeekDlg::OnEnChangeEditMarginY()
{
	if (!IsWindow(m_xcSpinMarginY.GetSafeHwnd())) { return; }
	seek_.marginY = m_xcSpinMarginY.GetPos32();
	m_xcSample.SetMarginY(seek_.marginY);
	m_xcSample.Invalidate(FALSE);
}

void CDisplaySeekDlg::OnEnChangeEditKnob()
{
	if (!IsWindow(m_xcSpinKnob.GetSafeHwnd())) { return; }
	seek_.knob = m_xcSpinKnob.GetPos32();
	m_xcSample.SetKnob(seek_.knob);
	m_xcSample.Invalidate(FALSE);
}



void CDisplaySeekDlg::OnBnClickedBtnSample()
{
	seek_ = sl::SeekConfig();

	seek_.text[0] = RGB(255, 255, 255);
	seek_.text[1] = RGB(180, 180, 180);

	seek_.color[0] = RGB(64, 64, 64);
	seek_.color[1] = RGB(64, 64, 64);
	seek_.color[2] = RGB(0, 0, 0);
	seek_.color[3] = RGB(0, 0, 0);
	seek_.color[4] = RGB(180, 180, 180);
	seek_.color[5] = RGB(180, 180, 180);
	seek_.color[6] = RGB(96, 96, 96);
	seek_.color[7] = RGB(32, 32, 32);
	seek_.color[8] = RGB(32, 32, 32);
	seek_.color[9] = RGB(32, 32, 32);

	InitControls();
}



void CDisplaySeekDlg::OnBnClickedBtnAllDefault()
{
	seek_ = sl::SeekConfig();
	InitControls();
}


void CDisplaySeekDlg::OnCbnSelchangeCmbCustom()
{
	auto data = m_xcCmbCustom.GetItem();
	if (data)
	{
		seek_ = *static_cast<sl::SeekConfig*>(data.get());
		InitControls();
	}
}

void CDisplaySeekDlg::OnBnClickedBtnAdd()
{
	m_xcCmbCustom.AddItem(boost::make_shared<sl::SeekConfig>(seek_));
}


void CDisplaySeekDlg::OnBnClickedBtnDel()
{
	m_xcCmbCustom.DelItem();
}



void CDisplaySeekDlg::OnDestroy()
{
	CDialog::OnDestroy();

	custom_.clear();
	m_xcCmbCustom.Forearch(
		[this](const CString& name, const boost::shared_ptr<void>& i){
		custom_.push_back(std::make_pair(std::wstring(name), *static_cast<sl::SeekConfig*>(i.get())));
	});
}

