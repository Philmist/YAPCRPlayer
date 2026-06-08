// DisplayBBSDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayBBSDlg.h"
//#include "afxdialogex.h"


// CDisplayBBSDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayBBSDlg, CDialog)

CDisplayBBSDlg::CDisplayBBSDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisplayBBSDlg::IDD, pParent)
{

}

CDisplayBBSDlg::~CDisplayBBSDlg()
{
}

void CDisplayBBSDlg::InitControls()
{
	for (int i = 0; i < 4; ++i)
	{
		m_xcBtnColor[i].SetColor(bbs_.color[i]);
		m_xcBtnColor[i].Invalidate(FALSE);
	}

	for (int i = 0; i < 1; ++i)
	{
		m_xcBtnText[i].SetColor(bbs_.text[i]);
		m_xcBtnText[i].Invalidate(FALSE);
	}

	m_xcSample.SetFontIndirect(bbs_.font);
	m_xcBtnFont.SetFontIndirect(bbs_.font);
	m_xcBtnFont.Invalidate(FALSE);

	m_xcSpinMarginX.SetPos32(bbs_.marginX);
	m_xcSpinMarginY.SetPos32(bbs_.marginY);


	m_xcCheckTitleBold.SetCheck(bbs_.title.bold ? BST_CHECKED : BST_UNCHECKED);

	m_xcBtnTitleText[0].SetColor(bbs_.title.text[0]);
	m_xcBtnTitleText[0].Invalidate(FALSE);

	m_xcBtnEditFont.SetFontIndirect(bbs_.edit.font);
	m_xcBtnEditFont.Invalidate(FALSE);

	m_xcSample.SetConfig(bbs_);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayBBSDlg::OnBnClickedBtnColor(int index)
{
	bbs_.color[index] = m_xcBtnColor[index].GetColor();
	m_xcSample.SetColor(index, bbs_.color[index]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayBBSDlg::OnBnClickedBtnText(int index)
{
	bbs_.text[index] = m_xcBtnText[index].GetColor();
	m_xcSample.SetText(index, bbs_.text[index]);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayBBSDlg::OnBnClickedBtnTitleText(int index)
{
	bbs_.title.text[index] = m_xcBtnTitleText[index].GetColor();
	m_xcSample.SetTitleText(index, bbs_.title.text[index]);
	m_xcSample.Invalidate(FALSE);
}



void CDisplayBBSDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SAMPLE, m_xcSample);
	DDX_Control(pDX, IDC_BTN_COLOR1, m_xcBtnColor[0]);
	DDX_Control(pDX, IDC_BTN_COLOR2, m_xcBtnColor[1]);
	DDX_Control(pDX, IDC_BTN_COLOR3, m_xcBtnColor[2]);
	DDX_Control(pDX, IDC_BTN_COLOR4, m_xcBtnColor[3]);
	DDX_Control(pDX, IDC_BTN_TEXT1, m_xcBtnText[0]);
	DDX_Control(pDX, IDC_BTN_FONT, m_xcBtnFont);
	DDX_Control(pDX, IDC_SPIN_MARGIN_X, m_xcSpinMarginX);
	DDX_Control(pDX, IDC_SPIN_MARGIN_Y, m_xcSpinMarginY);
	DDX_Control(pDX, IDC_CHECK_TITLE_BOLD, m_xcCheckTitleBold);
	DDX_Control(pDX, IDC_BTN_TITLE_TEXT1, m_xcBtnTitleText[0]);
	DDX_Control(pDX, IDC_BTN_EDIT_FONT, m_xcBtnEditFont);
	DDX_Control(pDX, IDC_CMB_CUSTOM, m_xcCmbCustom);
}


BEGIN_MESSAGE_MAP(CDisplayBBSDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_COLOR1, &CDisplayBBSDlg::OnBnClickedBtnColor1)
	ON_BN_CLICKED(IDC_BTN_COLOR2, &CDisplayBBSDlg::OnBnClickedBtnColor2)
	ON_BN_CLICKED(IDC_BTN_COLOR3, &CDisplayBBSDlg::OnBnClickedBtnColor3)
	ON_BN_CLICKED(IDC_BTN_COLOR4, &CDisplayBBSDlg::OnBnClickedBtnColor4)
	ON_BN_CLICKED(IDC_BTN_TEXT1, &CDisplayBBSDlg::OnBnClickedBtnText1)
	ON_BN_CLICKED(IDC_BTN_FONT, &CDisplayBBSDlg::OnBnClickedBtnFont)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_X, &CDisplayBBSDlg::OnEnChangeEditMarginX)
	ON_EN_CHANGE(IDC_EDIT_MARGIN_Y, &CDisplayBBSDlg::OnEnChangeEditMarginY)
	ON_BN_CLICKED(IDC_CHECK_TITLE_BOLD, &CDisplayBBSDlg::OnBnClickedCheckTitleBold)
	ON_BN_CLICKED(IDC_BTN_TITLE_TEXT1, &CDisplayBBSDlg::OnBnClickedBtnTitleText1)
	ON_BN_CLICKED(IDC_BTN_EDIT_FONT, &CDisplayBBSDlg::OnBnClickedBtnEditFont)
	ON_BN_CLICKED(IDC_BTN_SAMPLE, &CDisplayBBSDlg::OnBnClickedBtnSample)
	ON_BN_CLICKED(IDC_BTN_ALL_DEFAULT, &CDisplayBBSDlg::OnBnClickedBtnAllDefault)
	ON_BN_CLICKED(IDC_BTN_ADD, &CDisplayBBSDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, &CDisplayBBSDlg::OnBnClickedBtnDel)
	ON_CBN_SELCHANGE(IDC_CMB_CUSTOM, &CDisplayBBSDlg::OnCbnSelchangeCmbCustom)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDisplayBBSDlg メッセージ ハンドラー


BOOL CDisplayBBSDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_xcSpinMarginX.SetRange32(0, 100);
	m_xcSpinMarginY.SetRange32(0, 100);

	InitControls();

	for (auto it = custom_.begin(); it != custom_.end(); ++it)
	{
		m_xcCmbCustom.AddItem(it->first.c_str(), boost::make_shared<sl::BBSConfig>(it->second));
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayBBSDlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayBBSDlg::OnBnClickedBtnColor1()
{
	OnBnClickedBtnColor(0);
}

void CDisplayBBSDlg::OnBnClickedBtnColor2()
{
	OnBnClickedBtnColor(1);
}

void CDisplayBBSDlg::OnBnClickedBtnColor3()
{
	OnBnClickedBtnColor(2);
}

void CDisplayBBSDlg::OnBnClickedBtnColor4()
{
	OnBnClickedBtnColor(3);
}


void CDisplayBBSDlg::OnBnClickedBtnText1()
{
	OnBnClickedBtnText(0);
}


void CDisplayBBSDlg::OnBnClickedBtnFont()
{
	LOGFONT lf = {};
	if (m_xcBtnFont.DoModal(lf))
	{
		bbs_.font = lf;
		m_xcSample.SetFontIndirect(lf);
		m_xcSample.Invalidate(FALSE);
	}

	/*
	LOGFONT lf = {};
	m_xcBtnFont.GetLogFont(lf);
	bbs_.font = lf;
	m_xcSample.SetFontIndirect(lf);
	m_xcSample.Invalidate(FALSE);
	*/
}



void CDisplayBBSDlg::OnEnChangeEditMarginX()
{
	if (!IsWindow(m_xcSpinMarginX.GetSafeHwnd())) { return; }
	bbs_.marginX = m_xcSpinMarginX.GetPos32();
	m_xcSample.SetMarginX(bbs_.marginX);
	m_xcSample.Invalidate(FALSE);
}


void CDisplayBBSDlg::OnEnChangeEditMarginY()
{
	if (!IsWindow(m_xcSpinMarginY.GetSafeHwnd())) { return; }
	bbs_.marginY = m_xcSpinMarginY.GetPos32();
	m_xcSample.SetMarginY(bbs_.marginY);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayBBSDlg::OnBnClickedBtnTitleText1()
{
	OnBnClickedBtnTitleText(0);
}

void CDisplayBBSDlg::OnBnClickedCheckTitleBold()
{
	bbs_.title.bold = m_xcCheckTitleBold.GetCheck() == BST_CHECKED;
	m_xcSample.SetTitleBold(bbs_.title.bold);
	m_xcSample.Invalidate(FALSE);
}

void CDisplayBBSDlg::OnBnClickedBtnEditFont()
{
	LOGFONT lf = {};
	if (m_xcBtnEditFont.DoModal(lf))
	{
		bbs_.edit.font = lf;
	}
}


void CDisplayBBSDlg::OnBnClickedBtnSample()
{
	bbs_ = sl::BBSConfig();

	bbs_.text[0] = RGB(255, 255, 255);

	bbs_.color[0] = RGB(0, 0, 0);
	bbs_.color[1] = RGB(0, 0, 0);
	bbs_.color[2] = RGB(24, 24, 24);
	bbs_.color[3] = RGB(24, 24, 24);
	
	InitControls();
}


void CDisplayBBSDlg::OnBnClickedBtnAllDefault()
{
	bbs_ = sl::BBSConfig();
	InitControls();
}



void CDisplayBBSDlg::OnCbnSelchangeCmbCustom()
{
	auto data = m_xcCmbCustom.GetItem();
	if (data)
	{
		bbs_ = *static_cast<sl::BBSConfig*>(data.get());
		InitControls();
	}
}

void CDisplayBBSDlg::OnBnClickedBtnAdd()
{
	m_xcCmbCustom.AddItem(boost::make_shared<sl::BBSConfig>(bbs_));
}


void CDisplayBBSDlg::OnBnClickedBtnDel()
{
	m_xcCmbCustom.DelItem();
}


void CDisplayBBSDlg::OnDestroy()
{
	CDialog::OnDestroy();

	custom_.clear();
	m_xcCmbCustom.Forearch(
		[this](const CString& name, const boost::shared_ptr<void>& i){
		custom_.push_back(std::make_pair(std::wstring(name), *static_cast<sl::BBSConfig*>(i.get())));
	});
}


