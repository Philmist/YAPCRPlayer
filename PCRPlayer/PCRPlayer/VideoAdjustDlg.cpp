// VideoAdjustDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "VideoAdjustDlg.h"
#include "afxdialogex.h"


// CVideoAdjustDlg ダイアログ

IMPLEMENT_DYNAMIC(CVideoAdjustDlg, CDialog)

CVideoAdjustDlg::CVideoAdjustDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoAdjustDlg::IDD, pParent)
	, nominal_(dsp::NOMINAL_RANGE_0_255)
	, resizer_(d3dutl::RESIZER_DEFAULT)
{

}

CVideoAdjustDlg::~CVideoAdjustDlg()
{
}


void CVideoAdjustDlg::ApplyValue(int index, int value)
{
	// スクロールバー
	m_xcScrollbar[index].SetScrollPos(value);

	// エディットボックス
	CString str;
	str.Format(L"%.2f", (float)value / 100.0f);
	m_xcEdit[index].SetWindowText(str);
}


void CVideoAdjustDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCROLLBAR1, m_xcScrollbar[0]);
	DDX_Control(pDX, IDC_SCROLLBAR2, m_xcScrollbar[1]);
	DDX_Control(pDX, IDC_SCROLLBAR3, m_xcScrollbar[2]);
	DDX_Control(pDX, IDC_SCROLLBAR4, m_xcScrollbar[3]);
	DDX_Control(pDX, IDC_EDIT1, m_xcEdit[0]);
	DDX_Control(pDX, IDC_EDIT2, m_xcEdit[1]);
	DDX_Control(pDX, IDC_EDIT3, m_xcEdit[2]);
	DDX_Control(pDX, IDC_EDIT4, m_xcEdit[3]);
	DDX_Control(pDX, IDC_BUTTON1, m_xcButton[0]);
	DDX_Control(pDX, IDC_BUTTON2, m_xcButton[1]);
	DDX_Control(pDX, IDC_BUTTON3, m_xcButton[2]);
	DDX_Control(pDX, IDC_BUTTON4, m_xcButton[3]);
	DDX_Control(pDX, IDC_CMB_NOMINAL, m_xcCmbNominal);
	DDX_Control(pDX, IDC_CMB_RESIZER, m_xcCmbResizer);
}


BEGIN_MESSAGE_MAP(CVideoAdjustDlg, CDialog)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON1, &CVideoAdjustDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CVideoAdjustDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CVideoAdjustDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CVideoAdjustDlg::OnBnClickedButton4)
	ON_CBN_SELCHANGE(IDC_CMB_NOMINAL, &CVideoAdjustDlg::OnCbnSelchangeCmbNominal)
	ON_CBN_SELCHANGE(IDC_CMB_RESIZER, &CVideoAdjustDlg::OnCbnSelchangeCmbResizer)
	ON_BN_CLICKED(IDC_BTN_SAVE, &CVideoAdjustDlg::OnBnClickedBtnSave)
END_MESSAGE_MAP()


// CVideoAdjustDlg メッセージ ハンドラー


BOOL CVideoAdjustDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (gl_.player.getProcAmpRange(group_))
	{
		if (!value_.init)
		{
			value_.brightness =		group_.brightness.def;
			value_.contrast =		group_.contrast.def;
			value_.hue =			group_.hue.def;
			value_.saturation =		group_.saturation.def;

			value_.init = true;
		}

		// Brightness
		m_xcScrollbar[0].SetScrollRange(group_.brightness.min, group_.brightness.max);
		ApplyValue(0, value_.brightness);

		// Contrast
		m_xcScrollbar[1].SetScrollRange(group_.contrast.min, group_.contrast.max);
		ApplyValue(1, value_.contrast);

		// Hue
		m_xcScrollbar[2].SetScrollRange(group_.hue.min, group_.hue.max);
		ApplyValue(2, value_.hue);

		// Saturation
		m_xcScrollbar[3].SetScrollRange(group_.saturation.min, group_.saturation.max);
		ApplyValue(3, value_.saturation);
	}
	else
	{
		for (int i = 0; i < 4; ++i)
		{
			m_xcScrollbar[i].EnableWindow(FALSE);
			m_xcEdit[i].EnableWindow(FALSE);
			m_xcEdit[i].SetWindowText(L"");
			m_xcButton[i].EnableWindow(FALSE);
		}
	}

	//---------------------------------------------------------------------

	m_xcCmbNominal.InsertString(0, L"0-255 (デフォルト)");
	m_xcCmbNominal.InsertString(1, L"16-235 (ワイド)");
	//m_xcCmbNominal.InsertString(2, L"48-208");
	//m_xcCmbNominal.InsertString(3, L"64-127");

	if (nominal_ < 0 || nominal_ >= dsp::NOMINAL_RANGE_COUNT)
	{
		nominal_ = 0;
	}

	m_xcCmbNominal.SetCurSel(nominal_);

	//---------------------------------------------------------------------

	m_xcCmbResizer.AddString(L"デフォルト");
	m_xcCmbResizer.AddString(L"Nearest neighbor");
	m_xcCmbResizer.AddString(L"Bilinear");
	m_xcCmbResizer.AddString(L"SmootherStep");
	m_xcCmbResizer.AddString(L"Bicubic");
	m_xcCmbResizer.AddString(L"Bicubic (シャープネス弱)");
	m_xcCmbResizer.AddString(L"Bicubic (シャープネス強)");
	m_xcCmbResizer.AddString(L"B-spline");
	m_xcCmbResizer.AddString(L"Mitchell-Netravali spline");
	m_xcCmbResizer.AddString(L"Catmull-Rom spline");
	m_xcCmbResizer.AddString(L"Lanczos2");
	m_xcCmbResizer.AddString(L"Lanczos3");

	switch (resizer_)
	{
	case d3dutl::RESIZER_DEFAULT:				m_xcCmbResizer.SetCurSel(0); break;
	case d3dutl::RESIZER_SHADER_NEAREST:		m_xcCmbResizer.SetCurSel(1); break;
	case d3dutl::RESIZER_SHADER_BILINEAR:		m_xcCmbResizer.SetCurSel(2); break;
	case d3dutl::RESIZER_SHADER_SMOOTHERSTEP:	m_xcCmbResizer.SetCurSel(3); break;
	case d3dutl::RESIZER_SHADER_BICUBIC06:		m_xcCmbResizer.SetCurSel(4); break;
	case d3dutl::RESIZER_SHADER_BICUBIC08:		m_xcCmbResizer.SetCurSel(5); break;
	case d3dutl::RESIZER_SHADER_BICUBIC10:		m_xcCmbResizer.SetCurSel(6); break;
	case d3dutl::RESIZER_SHADER_BSPLINE4:		m_xcCmbResizer.SetCurSel(7); break;
	case d3dutl::RESIZER_SHADER_MITCHELL4:		m_xcCmbResizer.SetCurSel(8); break;
	case d3dutl::RESIZER_SHADER_CATMULL4:		m_xcCmbResizer.SetCurSel(9); break;
	case d3dutl::RESIZER_SHADER_2PASS_LANCZOS2:	m_xcCmbResizer.SetCurSel(10); break;
	case d3dutl::RESIZER_SHADER_2PASS_LANCZOS3:	m_xcCmbResizer.SetCurSel(11); break;
	default: m_xcCmbResizer.SetCurSel(0); break;
	}

	if (d3dutl::GetD3X9Dll() == NULL)
	{
		m_xcCmbResizer.EnableWindow(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CVideoAdjustDlg::PreTranslateMessage(MSG* pMsg)
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


void CVideoAdjustDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int next = pScrollBar->GetScrollPos();

	dsp::ProcAmpRange* range = NULL;
	dsp::ProcAmpRange* ranges[4];
	ranges[0] = &(group_.brightness);
	ranges[1] = &(group_.contrast);
	ranges[2] = &(group_.hue);
	ranges[3] = &(group_.saturation);

	int* value[4];
	value[0] = &(value_.brightness);
	value[1] = &(value_.contrast);
	value[2] = &(value_.hue);
	value[3] = &(value_.saturation);

	for (int i = 0; i < 4; ++i)
	{
		if (*pScrollBar != m_xcScrollbar[i]) { continue; }

		range = ranges[i];
		switch (nSBCode)
		{
		// つまみをドラッグ
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			if (range->step > 0)
			{
				next = (signed int(nPos) / range->step) * range->step;
			}
			else
			{
				next = signed int(nPos);
			}
			break;

		// 左ボタンをクリック
		case SB_LINELEFT:
		case SB_PAGELEFT:
			next = max(range->min, next - range->step);
			break;

		// 右ボタンをクリック
		case SB_LINERIGHT:
		case SB_PAGERIGHT:
			next = min(range->max, next + range->step);
			break;

		default:
			break;
		}

		ApplyValue(i, next);


		*value[i] = next;
		gl_.player.setProcAmp(value_);
		return;
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CVideoAdjustDlg::OnBnClickedButton1()
{
	value_.brightness = group_.brightness.def;
	ApplyValue(0, value_.brightness);

	gl_.player.setProcAmp(value_);
}


void CVideoAdjustDlg::OnBnClickedButton2()
{
	value_.contrast = group_.contrast.def;
	ApplyValue(1, value_.contrast);

	gl_.player.setProcAmp(value_);
}


void CVideoAdjustDlg::OnBnClickedButton3()
{
	value_.hue = group_.hue.def;
	ApplyValue(2, value_.hue);

	gl_.player.setProcAmp(value_);
}


void CVideoAdjustDlg::OnBnClickedButton4()
{
	value_.saturation = group_.saturation.def;
	ApplyValue(3, value_.saturation);

	gl_.player.setProcAmp(value_);
}


void CVideoAdjustDlg::OnCbnSelchangeCmbNominal()
{
	nominal_ = m_xcCmbNominal.GetCurSel();
	gl_.player.setNominalRange(nominal_);
}


void CVideoAdjustDlg::OnCbnSelchangeCmbResizer()
{
	switch (m_xcCmbResizer.GetCurSel())
	{
	case 0:		resizer_ = d3dutl::RESIZER_DEFAULT; break;
	case 1:		resizer_ = d3dutl::RESIZER_SHADER_NEAREST; break;
	case 2:		resizer_ = d3dutl::RESIZER_SHADER_BILINEAR; break;
	case 3:		resizer_ = d3dutl::RESIZER_SHADER_SMOOTHERSTEP; break;
	case 4:		resizer_ = d3dutl::RESIZER_SHADER_BICUBIC06; break;
	case 5:		resizer_ = d3dutl::RESIZER_SHADER_BICUBIC08; break;
	case 6:		resizer_ = d3dutl::RESIZER_SHADER_BICUBIC10; break;
	case 7:		resizer_ = d3dutl::RESIZER_SHADER_BSPLINE4; break;
	case 8:		resizer_ = d3dutl::RESIZER_SHADER_MITCHELL4; break;
	case 9:		resizer_ = d3dutl::RESIZER_SHADER_CATMULL4; break;
	case 10:	resizer_ = d3dutl::RESIZER_SHADER_2PASS_LANCZOS2; break;
	case 11:	resizer_ = d3dutl::RESIZER_SHADER_2PASS_LANCZOS3; break;
	default: return;
	}

	gl_.player.setResizer(resizer_);
}


void CVideoAdjustDlg::OnBnClickedBtnSave()
{
	gl_.value.video.procamp = value_;
	gl_.value.video.nominal = nominal_;
	gl_.value.video.resizer = resizer_;

	sl_.video = gl_.value.video;
	sl_.save(gl_.current);
}
