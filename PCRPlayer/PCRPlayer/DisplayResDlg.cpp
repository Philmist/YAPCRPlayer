// DisplayTooltipDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayResDlg.h"
//#include "afxdialogex.h"


// CDisplayResDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayResDlg, CDialog)

CDisplayResDlg::CDisplayResDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisplayResDlg::IDD, pParent)
	, select_(0)
	, dlg_(res_, m_xcSample)
{

}

CDisplayResDlg::~CDisplayResDlg()
{
}

void CDisplayResDlg::InitControls()
{
	for (auto it = vec_.begin(); it != vec_.end(); ++it)
	{
		it->init();
	}

	m_xcSample.SetConfig(res_);
	m_xcSample.Invalidate(FALSE);
}



void CDisplayResDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SAMPLE, m_xcSample);
	DDX_Control(pDX, IDC_CMB_CUSTOM, m_xcCmbCustom);
	DDX_Control(pDX, IDC_TAB, m_xcTab);
}


BEGIN_MESSAGE_MAP(CDisplayResDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_SAMPLE, &CDisplayResDlg::OnBnClickedBtnSample)
	ON_BN_CLICKED(IDC_BTN_ALL_DEFAULT, &CDisplayResDlg::OnBnClickedBtnAllDefault)
	ON_BN_CLICKED(IDC_BTN_ADD, &CDisplayResDlg::OnBnClickedBtnAdd)
	ON_BN_CLICKED(IDC_BTN_DEL, &CDisplayResDlg::OnBnClickedBtnDel)
	ON_CBN_SELCHANGE(IDC_CMB_CUSTOM, &CDisplayResDlg::OnCbnSelchangeCmbCustom)
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CDisplayResDlg::OnTcnSelchangeTab)
END_MESSAGE_MAP()


// CDisplayResDlg メッセージ ハンドラー


BOOL CDisplayResDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitControls();

	for (auto it = custom_.begin(); it != custom_.end(); ++it)
	{
		m_xcCmbCustom.AddItem(it->first.c_str(), boost::make_shared<sl::ResConfig>(it->second));
	}

	auto bind = [this](const std::wstring& text, CDialog* dlg, int idd, int index, const boost::function<void()>& init) {
		m_xcTab.InsertItem(index, text.c_str());

		dlg->Create(idd, this);
		dlg->SetParent(&m_xcTab);
		Item item = { dlg, idd, index, init };
		vec_.push_back(item);
	};

	bind(L"書式1", &dlg_.format1, CDisplayResFormat1Dlg::IDD, 0, [this](){ dlg_.format1.InitControls(); });
	bind(L"書式2", &dlg_.format2, CDisplayResFormat2Dlg::IDD, 1, [this](){ dlg_.format2.InitControls(); });
	bind(L"ID", &dlg_.id, CDisplayResIDDlg::IDD, 2, [this](){ dlg_.id.InitControls(); });
	bind(L"配色1", &dlg_.color1, CDisplayResColor1Dlg::IDD, 3, [this](){ dlg_.color1.InitControls(); });
	bind(L"配色2", &dlg_.color2, CDisplayResColor2Dlg::IDD, 4, [this](){ dlg_.color2.InitControls(); });
	bind(L"配色3", &dlg_.color3, CDisplayResColor3Dlg::IDD, 5, [this](){ dlg_.color3.InitControls(); });
	bind(L"レス", &dlg_.box, CDisplayResBoxDlg::IDD, 6, [this](){ dlg_.box.InitControls(); });
	bind(L"新着レス", &dlg_.latest, CDisplayResLatestDlg::IDD, 7, [this](){ dlg_.latest.InitControls(); });
	bind(L"背景", &dlg_.back, CDisplayResBackDlg::IDD, 8, [this](){ dlg_.back.InitControls(); });

	m_xcTab.GetWindowRect(&rect_);
	m_xcTab.AdjustRect(FALSE, &rect_);
	m_xcTab.ScreenToClient(&rect_);

	for (auto it = vec_.begin(); it != vec_.end(); ++it)
	{ 
		it->dlg->MoveWindow(rect_);
	}

	m_xcTab.SetCurSel(select_);
	OnTcnSelchangeTab(0, 0);
	m_xcTab.ModifyStyle(0, WS_CLIPCHILDREN);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayResDlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayResDlg::OnBnClickedBtnSample()
{
	int type = res_.format.type;
	res_ = sl::ResConfig();
	res_.format.type = type;

	res_.back.opaque.enable = true;
	res_.back.opaque.text = 255;
	res_.back.opaque.back = 192;

	res_.color.text[0] = RGB(255, 255, 255);
	res_.color.text[1] = RGB(128, 255, 128);
	res_.color.text[2] = RGB(128, 255, 128);
	res_.color.text[3] = RGB(255, 255, 255);
	res_.color.text[4] = RGB(255, 255, 255);
	res_.color.text[5] = RGB(255, 255, 255);
	res_.color.text[6] = RGB(255, 255, 255);
	res_.color.text[7] = RGB(128, 255, 128);
	res_.color.text[8] = RGB(255, 255, 255);
	res_.color.text[9] = RGB(255, 255, 255);
	res_.color.text[10] = RGB(255, 255, 255);

	res_.color.link[0] = RGB(255, 255, 255);
	res_.color.link[1] = RGB(128, 255, 255);
	res_.color.link[2] = RGB(128, 255, 255);
	res_.color.link[3] = RGB(255, 255, 255);
	res_.color.link[4] = RGB(255, 255, 255);
	res_.color.link[5] = RGB(255, 255, 255);
	res_.color.link[6] = RGB(255, 255, 255);
	res_.color.link[7] = RGB(128, 255, 255);
	res_.color.link[8] = RGB(255, 255, 255);
	res_.color.link[9] = RGB(255, 255, 255);
	res_.color.link[10] = RGB(128, 128, 255);

	res_.color.sage[0] = RGB(255, 255, 255);
	res_.color.sage[1] = RGB(255, 128, 255);
	res_.color.sage[2] = RGB(255, 128, 255);
	res_.color.sage[3] = RGB(255, 255, 255);
	res_.color.sage[4] = RGB(255, 255, 255);
	res_.color.sage[5] = RGB(255, 255, 255);
	res_.color.sage[6] = RGB(255, 255, 255);
	res_.color.sage[7] = RGB(255, 128, 255);
	res_.color.sage[8] = RGB(255, 255, 255);
	res_.color.sage[9] = RGB(255, 255, 255);
	res_.color.sage[10] = RGB(255, 128, 128);

	res_.box.color[0] = RGB(32, 32, 32);
	res_.box.color[1] = RGB(32, 32, 32);
	res_.box.color[2] = RGB(32, 32, 32);
	res_.box.color[3] = RGB(32, 32, 32);
	res_.box.color[4] = RGB(32, 32, 32);
	res_.box.color[5] = RGB(32, 32, 32);

	res_.box.latest[0] = RGB(32, 32, 80);
	res_.box.latest[1] = RGB(32, 32, 80);
	res_.box.latest[2] = RGB(32, 32, 80);
	res_.box.latest[3] = RGB(32, 32, 80);
	res_.box.latest[4] = RGB(32, 32, 80);
	res_.box.latest[5] = RGB(32, 32, 80);

	res_.back.color[0] = RGB(80, 80, 80);
	res_.back.color[1] = RGB(80, 80, 80);
	res_.back.color[2] = RGB(192, 192, 192);
	res_.back.color[3] = RGB(192, 192, 192);
	res_.back.color[4] = RGB(192, 192, 192);
	res_.back.color[5] = RGB(192, 192, 192);

	InitControls();
}



void CDisplayResDlg::OnBnClickedBtnAllDefault()
{

	int type = res_.format.type;
	res_ = sl::ResConfig();
	res_.format.type = type;

	InitControls();
}



void CDisplayResDlg::OnCbnSelchangeCmbCustom()
{
	auto data = m_xcCmbCustom.GetItem();
	if (data)
	{
		res_ = *static_cast<sl::ResConfig*>(data.get());
		InitControls();
	}
}



void CDisplayResDlg::OnBnClickedBtnAdd()
{
	m_xcCmbCustom.AddItem(boost::make_shared<sl::ResConfig>(res_));
}


void CDisplayResDlg::OnBnClickedBtnDel()
{
	m_xcCmbCustom.DelItem();
}



void CDisplayResDlg::OnDestroy()
{
	CDialog::OnDestroy();

	custom_.clear();
	m_xcCmbCustom.Forearch(
		[this](const CString& name, const boost::shared_ptr<void>& i){
		custom_.push_back(std::make_pair(std::wstring(name), *static_cast<sl::ResConfig*>(i.get())));
	});
}



void CDisplayResDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	int sel = m_xcTab.GetCurSel();
	if (sel < 0) { if (pResult) { *pResult = 0; } }

	for (auto it = vec_.begin(); it != vec_.end(); ++it)
	{
		if (it->dlg)
		{
			if (it->index == sel)
			{
				if (::IsWindow(it->dlg->GetSafeHwnd()))
				{
					it->dlg->ShowWindow(SW_SHOW);
				}
				select_ = sel;
			}
			else
			{
				if (::IsWindow(it->dlg->GetSafeHwnd()))
				{
					it->dlg->ShowWindow(SW_HIDE);
				}
			}
		}
	}

	if (pResult) { *pResult = 0; }
}
