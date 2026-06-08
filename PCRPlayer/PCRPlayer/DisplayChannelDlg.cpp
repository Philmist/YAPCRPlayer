// DisplayChannelDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayChannelDlg.h"
//#include "afxdialogex.h"


// CDisplayChannelDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayChannelDlg, CDialog)

CDisplayChannelDlg::CDisplayChannelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisplayChannelDlg::IDD, pParent)
{

}

CDisplayChannelDlg::~CDisplayChannelDlg()
{
}

void CDisplayChannelDlg::InitControls()
{
	for (int i = 0; i < 6; ++i)
	{
		m_xcBtnColor[i].SetColor(channel_.color[i]);
		m_xcBtnColor[i].Invalidate(FALSE);
	}

	for (int i = 0; i < 1; ++i)
	{
		m_xcBtnText[i].SetColor(channel_.text[i]);
		m_xcBtnText[i].Invalidate(FALSE);
	}

	m_xcSample.SetFontIndirect(channel_.font);
	m_xcBtnFont.SetFontIndirect(channel_.font);
	m_xcBtnFont.Invalidate(FALSE);

	m_xcSpinMarginX.SetPos32(channel_.marginX);
	m_xcSpinMarginY.SetPos32(channel_.marginY);

	m_xcCheckOpaque.SetCheck(channel_.opaque.enable ? BST_CHECKED : BST_UNCHECKED);

	m_xcSpinOpaqueText.SetPos32(channel_.opaque.text);
	m_xcSpinOpaqueBack.SetPos32(channel_.opaque.back);

	BOOL enable = channel_.opaque.enable ? TRUE : FALSE;
	m_xcEditOpaqueText.EnableWindow(enable);
	m_xcSpinOpaqueText.EnableWindow(enable);

	m_xcSample.SetConfig(channel_);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayChannelDlg::OnBnClickedBtnColor(int index)
{
	channel_.color[index] = m_xcBtnColor[index].GetColor();
	m_xcSample.SetColor(index, channel_.color[index]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayChannelDlg::OnBnClickedBtnText(int index)
{
	channel_.text[index] = m_xcBtnText[index].GetColor();
	m_xcSample.SetText(index, channel_.text[index]);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayChannelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SAMPLE, m_xcSample);
	DDX_Control(pDX, IDC_BTN_COLOR1, m_xcBtnColor[0]);
	DDX_Control(pDX, IDC_BTN_COLOR2, m_xcBtnColor[1]);
	DDX_Control(pDX, IDC_BTN_COLOR3, m_xcBtnColor[2]);
	DDX_Control(pDX, IDC_BTN_COLOR4, m_xcBtnColor[3]);
	DDX_Control(pDX, IDC_BTN_COLOR5, m_xcBtnColor[4]);
	DDX_Control(pDX, IDC_BTN_COLOR6, m_xcBtnColor[5]);
	DDX_Control(pDX, IDC_BTN_TEXT1, m_xcBtnText[0]);
	DDX_Control(pDX, IDC_BTN_FONT, m_xcBtnFont);
	DDX_Control(pDX, IDC_EDIT_MARGIN_X, m_xcEditMarginX);
	DDX_Control(pDX, IDC_SPIN_MARGIN_X, m_xcSpinMarginX);
	DDX_Control(pDX, IDC_EDIT_MARGIN_Y, m_xcEditMarginY);
	DDX_Control(pDX, IDC_SPIN_MARGIN_Y, m_xcSpinMarginY);
	DDX_Control(pDX, IDC_CHECK_OPAQUE, m_xcCheckOpaque);
	DDX_Control(pDX, IDC_EDIT_OPAQUE_TEXT, m_xcEditOpaqueText);
	DDX_Control(pDX, IDC_SPIN_OPAQUE_TEXT, m_xcSpinOpaqueText);
	DDX_Control(pDX, IDC_EDIT_OPAQUE_BACK, m_xcEditOpaqueBack);
	DDX_Control(pDX, IDC_SPIN_OPAQUE_BACK, m_xcSpinOpaqueBack);
	DDX_Control(pDX, IDC_CMB_CUSTOM, m_xcCmbCustom);
}


BEGIN_MESSAGE_MAP(CDisplayChannelDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_COLOR1, &CDisplayChannelDlg::OnBnClickedBtnColor1)
	ON_BN_CLICKED(IDC_BTN_COLOR2, &CDisplayChannelDlg::OnBnClickedBtnColor2)
	ON_BN_CLICKED(IDC_BTN_COLOR3, &CDisplayChannelDlg::OnBnClickedBtnColor3)
	ON_BN_CLICKED(IDC_BTN_COLOR4, &CDisplayChannelDlg::OnBnClickedBtnColor4)
	ON_BN_CLICKED(IDC_BTN_COLOR5, &CDisplayChannelDlg::OnBnClickedBtnColor5)
	ON_BN_CLICKED(IDC_BTN_COLOR6, &CDisplayChannelDlg::OnBnClickedBtnColor6)
	ON_BN_CLICKED(IDC_BTN_TEXT1, &CDisplayChannelDlg::OnBnClickedBtnText1)
	ON_BN_CLICKED(IDC_BTN_FONT, &CDisplayChannelDlg::OnBnClickedBtnFont)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_X, &CDisplayChannelDlg::OnEnChangeEditMarginX)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_Y, &CDisplayChannelDlg::OnEnChangeEditMarginY)
	ON_BN_CLICKED(IDC_CHECK_OPAQUE, &CDisplayChannelDlg::OnBnClickedCheckOpaque)
	ON_EN_CHANGE(IDC_EDIT_OPAQUE_TEXT, &CDisplayChannelDlg::OnEnChangeEditOpaqueText)
	ON_EN_CHANGE(IDC_EDIT_OPAQUE_BACK, &CDisplayChannelDlg::OnEnChangeEditOpaqueBack)
	ON_BN_CLICKED(IDC_BTN_SAMPLE, &CDisplayChannelDlg::OnBnClickedBtnSample)
	ON_BN_CLICKED(IDC_BTN_ALL_DEFAULT, &CDisplayChannelDlg::OnBnClickedBtnAllDefault)
	ON_BN_CLICKED(IDC_BTN_ADD, &CDisplayChannelDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, &CDisplayChannelDlg::OnBnClickedBtnDel)
	ON_CBN_SELCHANGE(IDC_CMB_CUSTOM, &CDisplayChannelDlg::OnCbnSelchangeCmbCustom)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDisplayChannelDlg メッセージ ハンドラー


BOOL CDisplayChannelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_xcSpinMarginX.SetRange32(0, 100);
	m_xcSpinMarginY.SetRange32(0, 100);

	m_xcSpinOpaqueText.SetRange32(0, 255);
	m_xcSpinOpaqueBack.SetRange32(0, 255);

	InitControls();

	for (auto it = custom_.begin(); it != custom_.end(); ++it)
	{
		m_xcCmbCustom.AddItem(it->first.c_str(), boost::make_shared<sl::ChannelConfig>(it->second));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayChannelDlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayChannelDlg::OnBnClickedBtnColor1()
{
	OnBnClickedBtnColor(0);
}

void CDisplayChannelDlg::OnBnClickedBtnColor2()
{
	OnBnClickedBtnColor(1);
}

void CDisplayChannelDlg::OnBnClickedBtnColor3()
{
	OnBnClickedBtnColor(2);
}

void CDisplayChannelDlg::OnBnClickedBtnColor4()
{
	OnBnClickedBtnColor(3);
}

void CDisplayChannelDlg::OnBnClickedBtnColor5()
{
	OnBnClickedBtnColor(4);
}

void CDisplayChannelDlg::OnBnClickedBtnColor6()
{
	OnBnClickedBtnColor(5);
}


void CDisplayChannelDlg::OnBnClickedBtnText1()
{
	OnBnClickedBtnText(0);
}

void CDisplayChannelDlg::OnBnClickedBtnFont()
{
	LOGFONT lf = {};
	if (m_xcBtnFont.DoModal(lf))
	{
		channel_.font = lf;
		m_xcSample.SetFontIndirect(lf);
		m_xcSample.Invalidate(FALSE);
	}
}

void CDisplayChannelDlg::OnEnChangeEditMarginX()
{
	if (!IsWindow(m_xcSpinMarginX.GetSafeHwnd())) { return; }
	channel_.marginX = m_xcSpinMarginX.GetPos32();
	m_xcSample.SetMarginX(channel_.marginX);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayChannelDlg::OnEnChangeEditMarginY()
{
	if (!IsWindow(m_xcSpinMarginY.GetSafeHwnd())) { return; }
	channel_.marginY = m_xcSpinMarginY.GetPos32();
	m_xcSample.SetMarginY(channel_.marginY);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayChannelDlg::OnBnClickedCheckOpaque()
{
	channel_.opaque.enable = m_xcCheckOpaque.GetCheck() == BST_CHECKED;
	BOOL enable = channel_.opaque.enable ? TRUE : FALSE;
	m_xcEditOpaqueText.EnableWindow(enable);
	m_xcSpinOpaqueText.EnableWindow(enable);
	m_xcSample.SetOpaqueEnable(channel_.opaque.enable);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayChannelDlg::OnEnChangeEditOpaqueText()
{
	if (!IsWindow(m_xcSpinOpaqueText.GetSafeHwnd())) { return; }
	channel_.opaque.text = m_xcSpinOpaqueText.GetPos32();
	m_xcSample.SetOpaqueText(channel_.opaque.text);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayChannelDlg::OnEnChangeEditOpaqueBack()
{
	if (!IsWindow(m_xcSpinOpaqueBack.GetSafeHwnd())) { return; }
	channel_.opaque.back = m_xcSpinOpaqueBack.GetPos32();
	m_xcSample.SetOpaqueBack(channel_.opaque.back);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayChannelDlg::OnBnClickedBtnSample()
{
	channel_ = sl::ChannelConfig();

	channel_.text[0] = RGB(255, 255, 255);

	channel_.color[0] = RGB(64, 64, 64);
	channel_.color[1] = RGB(64, 64, 64);
	channel_.color[2] = RGB(96, 96, 96);
	channel_.color[3] = RGB(96, 96, 96);
	channel_.color[4] = RGB(96, 96, 96);
	channel_.color[5] = RGB(64, 64, 64);

	channel_.opaque.enable = false;
	channel_.opaque.text = 255;

	InitControls();
}



void CDisplayChannelDlg::OnBnClickedBtnAllDefault()
{
	channel_ = sl::ChannelConfig();
	InitControls();
}



void CDisplayChannelDlg::OnCbnSelchangeCmbCustom()
{
	auto data = m_xcCmbCustom.GetItem();
	if (data)
	{
		channel_ = *static_cast<sl::ChannelConfig*>(data.get());
		InitControls();
	}
}



void CDisplayChannelDlg::OnBnClickedBtnAdd()
{
	m_xcCmbCustom.AddItem(boost::make_shared<sl::ChannelConfig>(channel_));
}


void CDisplayChannelDlg::OnBnClickedBtnDel()
{
	m_xcCmbCustom.DelItem();
}



void CDisplayChannelDlg::OnDestroy()
{
	CDialog::OnDestroy();

	custom_.clear();
	m_xcCmbCustom.Forearch(
		[this](const CString& name, const boost::shared_ptr<void>& i){
		custom_.push_back(std::make_pair(std::wstring(name), *static_cast<sl::ChannelConfig*>(i.get())));
	});
}



