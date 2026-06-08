// DisplayHintDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayHintDlg.h"
//#include "afxdialogex.h"


// CDisplayHintDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayHintDlg, CDialog)

CDisplayHintDlg::CDisplayHintDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisplayHintDlg::IDD, pParent)
{

}

CDisplayHintDlg::~CDisplayHintDlg()
{
}

void CDisplayHintDlg::InitControls()
{
	for (int i = 0; i < 6; ++i)
	{
		m_xcBtnColor[i].SetColor(hint_.color[i]);
		m_xcBtnColor[i].Invalidate(FALSE);
	}

	for (int i = 0; i < 1; ++i)
	{
		m_xcBtnText[i].SetColor(hint_.text[i]);
		m_xcBtnText[i].Invalidate(FALSE);
	}

	m_xcSample.SetFontIndirect(hint_.font);
	m_xcBtnFont.SetFontIndirect(hint_.font);
	m_xcBtnFont.Invalidate(FALSE);

	m_xcSpinMarginX.SetPos32(hint_.marginX);
	m_xcSpinMarginY.SetPos32(hint_.marginY);

	m_xcCheckOpaque.SetCheck(hint_.opaque.enable ? BST_CHECKED : BST_UNCHECKED);

	m_xcSpinOpaqueText.SetPos32(hint_.opaque.text);
	m_xcSpinOpaqueBack.SetPos32(hint_.opaque.back);

	BOOL enable = hint_.opaque.enable ? TRUE : FALSE;
	m_xcEditOpaqueText.EnableWindow(enable);
	m_xcSpinOpaqueText.EnableWindow(enable);

	m_xcSample.SetConfig(hint_);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayHintDlg::OnBnClickedBtnColor(int index)
{
	hint_.color[index] = m_xcBtnColor[index].GetColor();
	m_xcSample.SetColor(index, hint_.color[index]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayHintDlg::OnBnClickedBtnText(int index)
{
	hint_.text[index] = m_xcBtnText[index].GetColor();
	m_xcSample.SetText(index, hint_.text[index]);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayHintDlg::DoDataExchange(CDataExchange* pDX)
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
	DDX_Control(pDX, IDC_SPIN_MARGIN_X, m_xcSpinMarginX);
	DDX_Control(pDX, IDC_SPIN_MARGIN_Y, m_xcSpinMarginY);
	DDX_Control(pDX, IDC_CHECK_OPAQUE, m_xcCheckOpaque);
	DDX_Control(pDX, IDC_EDIT_OPAQUE_TEXT, m_xcEditOpaqueText);
	DDX_Control(pDX, IDC_SPIN_OPAQUE_TEXT, m_xcSpinOpaqueText);
	DDX_Control(pDX, IDC_EDIT_OPAQUE_BACK, m_xcEditOpaqueBack);
	DDX_Control(pDX, IDC_SPIN_OPAQUE_BACK, m_xcSpinOpaqueBack);
	DDX_Control(pDX, IDC_CMB_CUSTOM, m_xcCmbCustom);
}


BEGIN_MESSAGE_MAP(CDisplayHintDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_COLOR1, &CDisplayHintDlg::OnBnClickedBtnColor1)
	ON_BN_CLICKED(IDC_BTN_COLOR2, &CDisplayHintDlg::OnBnClickedBtnColor2)
	ON_BN_CLICKED(IDC_BTN_COLOR3, &CDisplayHintDlg::OnBnClickedBtnColor3)
	ON_BN_CLICKED(IDC_BTN_COLOR4, &CDisplayHintDlg::OnBnClickedBtnColor4)
	ON_BN_CLICKED(IDC_BTN_COLOR5, &CDisplayHintDlg::OnBnClickedBtnColor5)
	ON_BN_CLICKED(IDC_BTN_COLOR6, &CDisplayHintDlg::OnBnClickedBtnColor6)
	ON_BN_CLICKED(IDC_BTN_TEXT1, &CDisplayHintDlg::OnBnClickedBtnText1)
	ON_BN_CLICKED(IDC_BTN_FONT, &CDisplayHintDlg::OnBnClickedBtnFont)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_X, &CDisplayHintDlg::OnEnChangeEditMarginX)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_Y, &CDisplayHintDlg::OnEnChangeEditMarginY)
	ON_BN_CLICKED(IDC_CHECK_OPAQUE, &CDisplayHintDlg::OnBnClickedCheckOpaque)
	ON_EN_CHANGE(IDC_EDIT_OPAQUE_TEXT, &CDisplayHintDlg::OnEnChangeEditOpaqueText)
	ON_EN_CHANGE(IDC_EDIT_OPAQUE_BACK, &CDisplayHintDlg::OnEnChangeEditOpaqueBack)
	ON_BN_CLICKED(IDC_BTN_SAMPLE, &CDisplayHintDlg::OnBnClickedBtnSample)
	ON_BN_CLICKED(IDC_BTN_ALL_DEFAULT, &CDisplayHintDlg::OnBnClickedBtnAllDefault)
	ON_BN_CLICKED(IDC_BTN_ADD, &CDisplayHintDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, &CDisplayHintDlg::OnBnClickedBtnDel)
	ON_CBN_SELCHANGE(IDC_CMB_CUSTOM, &CDisplayHintDlg::OnCbnSelchangeCmbCustom)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDisplayHintDlg メッセージ ハンドラー


BOOL CDisplayHintDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_xcSpinMarginX.SetRange32(0, 100);
	m_xcSpinMarginY.SetRange32(0, 100);
	m_xcSpinOpaqueText.SetRange32(0, 255);
	m_xcSpinOpaqueBack.SetRange32(0, 255);

	InitControls();

	for (auto it = custom_.begin(); it != custom_.end(); ++it)
	{
		m_xcCmbCustom.AddItem(it->first.c_str(), boost::make_shared<sl::HintConfig>(it->second));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayHintDlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayHintDlg::OnBnClickedBtnColor1()
{
	OnBnClickedBtnColor(0);
}

void CDisplayHintDlg::OnBnClickedBtnColor2()
{
	OnBnClickedBtnColor(1);
}

void CDisplayHintDlg::OnBnClickedBtnColor3()
{
	OnBnClickedBtnColor(2);
}

void CDisplayHintDlg::OnBnClickedBtnColor4()
{
	OnBnClickedBtnColor(3);
}

void CDisplayHintDlg::OnBnClickedBtnColor5()
{
	OnBnClickedBtnColor(4);
}

void CDisplayHintDlg::OnBnClickedBtnColor6()
{
	OnBnClickedBtnColor(5);
}


void CDisplayHintDlg::OnBnClickedBtnText1()
{
	OnBnClickedBtnText(0);
}

void CDisplayHintDlg::OnBnClickedBtnFont()
{
	LOGFONT lf = {};
	if (m_xcBtnFont.DoModal(lf))
	{
		hint_.font = lf;
		m_xcSample.SetFontIndirect(lf);
		m_xcSample.Invalidate(FALSE);
	}
}

void CDisplayHintDlg::OnEnChangeEditMarginX()
{
	if (!IsWindow(m_xcSpinMarginX.GetSafeHwnd())) { return; }
	hint_.marginX = m_xcSpinMarginX.GetPos32();
	m_xcSample.SetMarginX(hint_.marginX);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayHintDlg::OnEnChangeEditMarginY()
{
	if (!IsWindow(m_xcSpinMarginY.GetSafeHwnd())) { return; }
	hint_.marginY = m_xcSpinMarginY.GetPos32();
	m_xcSample.SetMarginY(hint_.marginY);
	m_xcSample.Invalidate(FALSE);
}



void CDisplayHintDlg::OnBnClickedCheckOpaque()
{
	hint_.opaque.enable = m_xcCheckOpaque.GetCheck() == BST_CHECKED;
	BOOL enable = hint_.opaque.enable ? TRUE : FALSE;
	m_xcEditOpaqueText.EnableWindow(enable);
	m_xcSpinOpaqueText.EnableWindow(enable);
	m_xcSample.SetOpaqueEnable(hint_.opaque.enable);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayHintDlg::OnEnChangeEditOpaqueText()
{
	if (!IsWindow(m_xcSpinOpaqueText.GetSafeHwnd())) { return; }
	hint_.opaque.text = m_xcSpinOpaqueText.GetPos32();
	m_xcSample.SetOpaqueText(hint_.opaque.text);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayHintDlg::OnEnChangeEditOpaqueBack()
{
	if (!IsWindow(m_xcSpinOpaqueBack.GetSafeHwnd())) { return; }
	hint_.opaque.back = m_xcSpinOpaqueBack.GetPos32();
	m_xcSample.SetOpaqueBack(hint_.opaque.back);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayHintDlg::OnBnClickedBtnSample()
{
	hint_ = sl::HintConfig();

	hint_.text[0] = RGB(255, 255, 255);

	hint_.color[0] = RGB(64, 64, 64);
	hint_.color[1] = RGB(64, 64, 64);
	hint_.color[2] = RGB(96, 96, 96);
	hint_.color[3] = RGB(96, 96, 96);
	hint_.color[4] = RGB(96, 96, 96);
	hint_.color[5] = RGB(64, 64, 64);

	hint_.opaque.enable = false;
	hint_.opaque.text = 255;

	InitControls();
}



void CDisplayHintDlg::OnBnClickedBtnAllDefault()
{
	hint_ = sl::HintConfig();
	InitControls();
}



void CDisplayHintDlg::OnCbnSelchangeCmbCustom()
{
	auto data = m_xcCmbCustom.GetItem();
	if (data)
	{
		hint_ = *static_cast<sl::HintConfig*>(data.get());
		InitControls();
	}
}



void CDisplayHintDlg::OnBnClickedBtnAdd()
{
	m_xcCmbCustom.AddItem(boost::make_shared<sl::HintConfig>(hint_));
}


void CDisplayHintDlg::OnBnClickedBtnDel()
{
	m_xcCmbCustom.DelItem();
}



void CDisplayHintDlg::OnDestroy()
{
	CDialog::OnDestroy();

	custom_.clear();
	m_xcCmbCustom.Forearch(
		[this](const CString& name, const boost::shared_ptr<void>& i){
		custom_.push_back(std::make_pair(std::wstring(name), *static_cast<sl::HintConfig*>(i.get())));
	});
}

