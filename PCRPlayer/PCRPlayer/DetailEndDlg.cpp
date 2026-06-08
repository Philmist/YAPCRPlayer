// DetailEndDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailEndDlg.h"
//#include "afxdialogex.h"


// CDetailEndDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailEndDlg, CDialog)

CDetailEndDlg::CDetailEndDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDetailEndDlg::IDD, pParent)
{

}

CDetailEndDlg::~CDetailEndDlg()
{
}

void CDetailEndDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_POS, m_xcCheckPos);
	DDX_Control(pDX, IDC_CHECK_SIZE, m_xcCheckSize);
	DDX_Control(pDX, IDC_CHECK_ASPECT, m_xcCheckAspect);
	DDX_Control(pDX, IDC_CHECK_VOLUME, m_xcCheckVolume);
	DDX_Control(pDX, IDC_CHECK_BALANCE, m_xcCheckBalance);
	DDX_Control(pDX, IDC_CHECK_MUTE, m_xcCheckMute);
}

void CDetailEndDlg::InitControls()
{
	m_xcCheckPos.SetCheck(end_.pos ? BST_CHECKED : BST_UNCHECKED);
	m_xcCheckSize.SetCheck(end_.size ? BST_CHECKED : BST_UNCHECKED);
	m_xcCheckAspect.SetCheck(end_.aspect ? BST_CHECKED : BST_UNCHECKED);
	m_xcCheckVolume.SetCheck(end_.volume ? BST_CHECKED : BST_UNCHECKED);
	m_xcCheckBalance.SetCheck(end_.balance ? BST_CHECKED : BST_UNCHECKED);
	m_xcCheckMute.SetCheck(end_.mute ? BST_CHECKED : BST_UNCHECKED);
}


BEGIN_MESSAGE_MAP(CDetailEndDlg, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_WINDOW_CURRENT, &CDetailEndDlg::OnBnClickedBtnWindowCurrent)
	ON_BN_CLICKED(IDC_BTN_WINDOW_INIT, &CDetailEndDlg::OnBnClickedBtnWindowInit)
	ON_BN_CLICKED(IDC_BTN_AUDIO_CURRENT, &CDetailEndDlg::OnBnClickedBtnAudioCurrent)
	ON_BN_CLICKED(IDC_BTN_AUDIO_INIT, &CDetailEndDlg::OnBnClickedBtnAudioInit)
	ON_BN_CLICKED(IDC_BTN_ALL_DEFAULT, &CDetailEndDlg::OnBnClickedBtnAllDefault)
END_MESSAGE_MAP()


// CDetailEndDlg メッセージ ハンドラー


BOOL CDetailEndDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDetailEndDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ここに特定なコードを追加するか、もしくは基本クラスを呼び出してください。

	return CDialog::PreTranslateMessage(pMsg);
}


void CDetailEndDlg::OnDestroy()
{
	CDialog::OnDestroy();

	end_.pos = m_xcCheckPos.GetCheck() == BST_CHECKED;
	end_.size = m_xcCheckSize.GetCheck() == BST_CHECKED;
	end_.aspect = m_xcCheckAspect.GetCheck() == BST_CHECKED;
	end_.volume = m_xcCheckVolume.GetCheck() == BST_CHECKED;
	end_.balance = m_xcCheckBalance.GetCheck() == BST_CHECKED;
	end_.mute = m_xcCheckMute.GetCheck() == BST_CHECKED;
}

void CDetailEndDlg::OnBnClickedBtnWindowCurrent()
{
	reinterpret_cast<CMainDlg*>(AfxGetMainWnd())->StoreVideoWindowRect(gl_.value.placement);

	sl_.placement = gl_.value.placement;
	sl_.window.aspect = gl_.value.window.aspect;
	sl_.save(gl_.current);
}

void CDetailEndDlg::OnBnClickedBtnWindowInit()
{
	sl::WindowControl window;

	sl_.placement = utl::defWindowRect(480, 360);
	sl_.window.aspect = window.aspect;
	sl_.save(gl_.current);
}


void CDetailEndDlg::OnBnClickedBtnAudioCurrent()
{
	sl_.audio = gl_.value.audio;
	sl_.save(gl_.current);
}

void CDetailEndDlg::OnBnClickedBtnAudioInit()
{
	dsp::AudioControl audio;
	sl_.audio = audio;
	sl_.save(gl_.current);
}


void CDetailEndDlg::OnBnClickedBtnAllDefault()
{
	end_ = sl::EndConfig();
	InitControls();
}
