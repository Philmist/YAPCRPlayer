// DisplayWindowDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayWindowDlg.h"
//#include "afxdialogex.h"

#define TOTAL_COUNT (30) // 総数

// CDisplayWindowDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayWindowDlg, CDialog)

CDisplayWindowDlg::CDisplayWindowDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisplayWindowDlg::IDD, pParent)
	, listZoom_(zoom_)
	, m_xcListZoom(listZoom_, true)
	, listSize_(size_)
	, m_xcListSize(listSize_, true)
{

}

CDisplayWindowDlg::~CDisplayWindowDlg()
{
}

void CDisplayWindowDlg::InitZoomControls()
{
	int index = m_xcListZoom.GetIndex();
	if (!listZoom_.get(index, [this](const sl::WindowZoom& value) {
		EnableZoomControls(TRUE);
		//--------------------------------
		m_xcSpinZoom.SetPos32(value.zoom);
	}))
	{
		EnableZoomControls(FALSE);
		//--------------------------------
		m_xcSpinZoom.SetPos32(100);
	}
}

void CDisplayWindowDlg::InitSizeControls()
{
	int index = m_xcListSize.GetIndex();
	if (!listSize_.get(index, [this](const sl::WindowSize& value) {
		EnableSizeControls(TRUE);
		//--------------------------------
		m_xcSpinSize1.SetPos32(value.width);
		m_xcSpinSize2.SetPos32(value.height);
	}))
	{
		EnableSizeControls(FALSE);
		//--------------------------------
		m_xcSpinSize1.SetPos32(800);
		m_xcSpinSize2.SetPos32(600);
	}
}

void CDisplayWindowDlg::EnableZoomControls(BOOL enable)
{
	m_xcEditZoom.EnableWindow(enable);
	m_xcSpinZoom.EnableWindow(enable);
}

void CDisplayWindowDlg::EnableSizeControls(BOOL enable)
{
	m_xcEditSize1.EnableWindow(enable);
	m_xcSpinSize1.EnableWindow(enable);
	m_xcEditSize2.EnableWindow(enable);
	m_xcSpinSize2.EnableWindow(enable);
}

void CDisplayWindowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ZOOM, m_xcListZoom);
	DDX_Control(pDX, IDC_BTN_ZOOM_ADD, m_xcBtnZoomAdd);
	DDX_Control(pDX, IDC_BTN_ZOOM_DEL, m_xcBtnZoomDel);
	DDX_Control(pDX, IDC_BTN_ZOOM_UP, m_xcBtnZoomUp);
	DDX_Control(pDX, IDC_BTN_ZOOM_DOWN, m_xcBtnZoomDown);
	DDX_Control(pDX, IDC_EDIT_ZOOM, m_xcEditZoom);
	DDX_Control(pDX, IDC_SPIN_ZOOM, m_xcSpinZoom);
	DDX_Control(pDX, IDC_LIST_SIZE, m_xcListSize);
	DDX_Control(pDX, IDC_BTN_SIZE_ADD, m_xcBtnSizeAdd);
	DDX_Control(pDX, IDC_BTN_SIZE_DEL, m_xcBtnSizeDel);
	DDX_Control(pDX, IDC_BTN_SIZE_UP, m_xcBtnSizeUp);
	DDX_Control(pDX, IDC_BTN_SIZE_DOWN, m_xcBtnSizeDown);
	DDX_Control(pDX, IDC_EDIT_SIZE1, m_xcEditSize1);
	DDX_Control(pDX, IDC_SPIN_SIZE1, m_xcSpinSize1);
	DDX_Control(pDX, IDC_EDIT_SIZE2, m_xcEditSize2);
	DDX_Control(pDX, IDC_SPIN_SIZE2, m_xcSpinSize2);
}


BEGIN_MESSAGE_MAP(CDisplayWindowDlg, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ZOOM, &CDisplayWindowDlg::OnLvnItemchangedListZoom)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_SIZE, &CDisplayWindowDlg::OnLvnItemchangedListSize)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_ZOOM, &CDisplayWindowDlg::OnLvnColumnclickListZoom)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_SIZE, &CDisplayWindowDlg::OnLvnColumnclickListSize)
	ON_EN_CHANGE(IDC_EDIT_ZOOM, &CDisplayWindowDlg::OnEnChangeEditZoom)
	ON_EN_CHANGE(IDC_EDIT_SIZE1, &CDisplayWindowDlg::OnEnChangeEditSize1)
	ON_EN_CHANGE(IDC_EDIT_SIZE2, &CDisplayWindowDlg::OnEnChangeEditSize2)
	ON_BN_CLICKED(IDC_BTN_ZOOM_ADD, &CDisplayWindowDlg::OnBnClickedBtnZoomAdd)
	ON_BN_CLICKED(IDC_BTN_SIZE_ADD, &CDisplayWindowDlg::OnBnClickedBtnSizeAdd)
	ON_BN_CLICKED(IDC_BTN_ZOOM_DEL, &CDisplayWindowDlg::OnBnClickedBtnZoomDel)
	ON_BN_CLICKED(IDC_BTN_SIZE_DEL, &CDisplayWindowDlg::OnBnClickedBtnSizeDel)
	ON_BN_CLICKED(IDC_BTN_ZOOM_UP, &CDisplayWindowDlg::OnBnClickedBtnZoomUp)
	ON_BN_CLICKED(IDC_BTN_SIZE_UP, &CDisplayWindowDlg::OnBnClickedBtnSizeUp)
	ON_BN_CLICKED(IDC_BTN_ZOOM_DOWN, &CDisplayWindowDlg::OnBnClickedBtnZoomDown)
	ON_BN_CLICKED(IDC_BTN_SIZE_DOWN, &CDisplayWindowDlg::OnBnClickedBtnSizeDown)
	ON_BN_CLICKED(IDC_BTN_ZOOM_ALL_DEFAULT, &CDisplayWindowDlg::OnBnClickedBtnZoomAllDefault)
	ON_BN_CLICKED(IDC_BTN_SIZE_ALL_DEFAULT, &CDisplayWindowDlg::OnBnClickedBtnSizeAllDefault)
END_MESSAGE_MAP()


// CDisplayWindowDlg メッセージ ハンドラー
BOOL CDisplayWindowDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rc(18, 0, 70, 0);
	MapDialogRect(&rc);

	m_xcListZoom.InsertColumn(0, L"", LVCFMT_CENTER, rc.left);
	m_xcListZoom.InsertColumn(1, L"倍率", LVCFMT_LEFT, rc.right);

	m_xcListSize.InsertColumn(0, L"", LVCFMT_CENTER, rc.left);
	m_xcListSize.InsertColumn(1, L"サイズ", LVCFMT_LEFT, rc.right);

	m_xcSpinZoom.SetRange32(10, 1000);

	m_xcSpinSize1.SetRange32(16, 10000);
	m_xcSpinSize2.SetRange32(16, 10000);

	InitZoomControls();
	InitSizeControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayWindowDlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayWindowDlg::OnLvnItemchangedListZoom(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	InitZoomControls();

	if (pResult) { *pResult = 0; }
}


void CDisplayWindowDlg::OnLvnItemchangedListSize(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	InitSizeControls();

	if (pResult) { *pResult = 0; }
}

void CDisplayWindowDlg::OnLvnColumnclickListZoom(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (!pNMLV) { return; }

	switch(pNMLV->iSubItem)
	{
	case 0:
		std::stable_sort(zoom_.begin(), zoom_.end(),
			[this](const sl::WindowZoom& l, const sl::WindowZoom& r)->bool{
				if (!orderZoom1_ && l.enable && !r.enable) { return true; }
				if (orderZoom1_ && !l.enable && r.enable) { return true; }
				return false;
		});
		m_xcListZoom.RedrawWindow();
		orderZoom1_ = orderZoom1_ ? false : true;
		break;

	case 1:
		std::stable_sort(zoom_.begin(), zoom_.end(),
			[this](const sl::WindowZoom& l, const sl::WindowZoom& r)->bool{
				return orderZoom2_ ? l.zoom > r.zoom : l.zoom < r.zoom;
		});
		m_xcListZoom.RedrawWindow();
		orderZoom2_ = orderZoom2_ ? false : true;
		break;

	default:
		break;
	}

	InitZoomControls();

	if (pResult) { *pResult = 0; }
}


void CDisplayWindowDlg::OnLvnColumnclickListSize(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (!pNMLV) { return; }

	switch(pNMLV->iSubItem)
	{
	case 0:
		std::stable_sort(size_.begin(), size_.end(),
			[this](const sl::WindowSize& l, const sl::WindowSize& r)->bool{
				if (!orderSize1_ && l.enable && !r.enable) { return true; }
				if (orderSize1_ && !l.enable && r.enable) { return true; }
				return false;
		});
		m_xcListSize.RedrawWindow();
		orderSize1_ = orderSize1_ ? false : true;
		break;

	case 1:
		std::stable_sort(size_.begin(), size_.end(),
			[this](const sl::WindowSize& l, const sl::WindowSize& r)->bool{
				return orderSize2_ ? l.width * l.height > r.width * r.height : l.width * l.height < r.width * r.height;
		});
		m_xcListSize.RedrawWindow();
		orderSize2_ = orderSize2_ ? false : true;
		break;

	default:
		break;
	}

	InitSizeControls();

	if (pResult) { *pResult = 0; }
}



void CDisplayWindowDlg::OnEnChangeEditZoom()
{
	if (!IsWindow(m_xcListZoom.GetSafeHwnd())) { return; }

	int index = m_xcListZoom.GetIndex();
	if (listZoom_.get(index, [this](sl::WindowZoom& value) {
		value.zoom = m_xcSpinZoom.GetPos32();
	}))
	{
		m_xcListZoom.Update(index);
	}
}


void CDisplayWindowDlg::OnEnChangeEditSize1()
{
	if (!IsWindow(m_xcListSize.GetSafeHwnd())) { return; }

	int index = m_xcListSize.GetIndex();
	if (listSize_.get(index, [this](sl::WindowSize& value) {
		value.width = m_xcSpinSize1.GetPos32();
	}))
	{
		m_xcListSize.Update(index);
	}
}


void CDisplayWindowDlg::OnEnChangeEditSize2()
{
	if (!IsWindow(m_xcListSize.GetSafeHwnd())) { return; }

	int index = m_xcListSize.GetIndex();
	if (listSize_.get(index, [this](sl::WindowSize& value) {
		value.height = m_xcSpinSize2.GetPos32();
	}))
	{
		m_xcListSize.Update(index);
	}
}


void CDisplayWindowDlg::OnBnClickedBtnZoomAdd()
{
	m_xcListZoom.AddItem();
	InitZoomControls();
}


void CDisplayWindowDlg::OnBnClickedBtnSizeAdd()
{
	m_xcListSize.AddItem();
	InitSizeControls();
}


void CDisplayWindowDlg::OnBnClickedBtnZoomDel()
{
	m_xcListZoom.DelItem();
	InitZoomControls();
}


void CDisplayWindowDlg::OnBnClickedBtnSizeDel()
{
	m_xcListSize.DelItem();
	InitSizeControls();
}


void CDisplayWindowDlg::OnBnClickedBtnZoomUp()
{
	m_xcListZoom.UpItem();
	InitZoomControls();
}


void CDisplayWindowDlg::OnBnClickedBtnSizeUp()
{
	m_xcListSize.UpItem();
	InitSizeControls();
}


void CDisplayWindowDlg::OnBnClickedBtnZoomDown()
{
	m_xcListZoom.DownItem();
	InitZoomControls();
}


void CDisplayWindowDlg::OnBnClickedBtnSizeDown()
{
	m_xcListSize.DownItem();
	InitSizeControls();
}




void CDisplayWindowDlg::OnBnClickedBtnZoomAllDefault()
{
	sl::WindowConfig win;
	zoom_ = win.zoom;

	m_xcListZoom.Init();
	m_xcListZoom.RedrawWindow();
	InitZoomControls();
}


void CDisplayWindowDlg::OnBnClickedBtnSizeAllDefault()
{
	sl::WindowConfig win;
	size_ = win.size;

	m_xcListSize.Init();
	m_xcListSize.RedrawWindow();
	InitSizeControls();
}
