// DisplayAspectDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DisplayAspectDlg.h"
//#include "afxdialogex.h"

#define TOTAL_COUNT (30) // 総数

// CDisplayAspectDlg ダイアログ

IMPLEMENT_DYNAMIC(CDisplayAspectDlg, CDialog)

CDisplayAspectDlg::CDisplayAspectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDisplayAspectDlg::IDD, pParent)
	, orderAspect_(false)
	, listAspect_(aspect_)
	, m_xcListAspect(listAspect_, false)
{

}


void CDisplayAspectDlg::InitAspectControls()
{
	int index = m_xcListAspect.GetIndex();
	if (!listAspect_.get(index, [this](const sl::WindowAspect& value) {
		EnableAspectControls(TRUE);
		//--------------------------------
		m_xcSpinAspect1.SetPos32(value.ax);
		m_xcSpinAspect2.SetPos32(value.ay);
	}))
	{
		EnableAspectControls(FALSE);
		//--------------------------------
		m_xcSpinAspect1.SetPos32(4);
		m_xcSpinAspect2.SetPos32(3);
	}
}


void CDisplayAspectDlg::EnableAspectControls(BOOL enable)
{
	m_xcEditAspect1.EnableWindow(enable);
	m_xcSpinAspect1.EnableWindow(enable);
	m_xcEditAspect2.EnableWindow(enable);
	m_xcSpinAspect2.EnableWindow(enable);
}

CDisplayAspectDlg::~CDisplayAspectDlg()
{
}

void CDisplayAspectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ASPECT, m_xcListAspect);
	DDX_Control(pDX, IDC_BTN_ASPECT_UP, m_xcBtnAspectUp);
	DDX_Control(pDX, IDC_BTN_ASPECT_DOWN, m_xcBtnAspectDown);
	DDX_Control(pDX, IDC_EDIT_ASPECT1, m_xcEditAspect1);
	DDX_Control(pDX, IDC_SPIN_ASPECT1, m_xcSpinAspect1);
	DDX_Control(pDX, IDC_EDIT_ASPECT2, m_xcEditAspect2);
	DDX_Control(pDX, IDC_SPIN_ASPECT2, m_xcSpinAspect2);
}


BEGIN_MESSAGE_MAP(CDisplayAspectDlg, CDialog)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ASPECT, &CDisplayAspectDlg::OnLvnItemchangedListAspect)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_ASPECT, &CDisplayAspectDlg::OnLvnColumnclickListAspect)
	ON_EN_CHANGE(IDC_EDIT_ASPECT1, &CDisplayAspectDlg::OnEnChangeEditAspect1)
	ON_EN_CHANGE(IDC_EDIT_ASPECT2, &CDisplayAspectDlg::OnEnChangeEditAspect2)
	ON_BN_CLICKED(IDC_BTN_ASPECT_UP, &CDisplayAspectDlg::OnBnClickedBtnAspectUp)
	ON_BN_CLICKED(IDC_BTN_ASPECT_DOWN, &CDisplayAspectDlg::OnBnClickedBtnAspectDown)
	ON_BN_CLICKED(IDC_BTN_ASPECT_ALL_DEFAULT, &CDisplayAspectDlg::OnBnClickedBtnAspectAllDefault)
END_MESSAGE_MAP()


// CDisplayAspectDlg メッセージ ハンドラー
BOOL CDisplayAspectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();


	CRect rc(0, 0, 90, 0);
	MapDialogRect(&rc);
	m_xcListAspect.InsertColumn(0, L"アスペクト比", LVCFMT_LEFT, rc.right);

	m_xcSpinAspect1.SetRange32(1, INT_MAX);
	m_xcSpinAspect2.SetRange32(1, INT_MAX);

	InitAspectControls();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDisplayAspectDlg::PreTranslateMessage(MSG* pMsg)
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

void CDisplayAspectDlg::OnLvnItemchangedListAspect(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	InitAspectControls();

	if (pResult) { *pResult = 0; }
}


void CDisplayAspectDlg::OnLvnColumnclickListAspect(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (!pNMLV) { return; }

	switch(pNMLV->iSubItem)
	{
	case 0:
		std::stable_sort(aspect_.begin(), aspect_.end(),
			[this](const sl::WindowAspect& l, const sl::WindowAspect& r)->bool{
				return orderAspect_ ? l.ax * l.ay > r.ax * r.ay : l.ax * l.ay < r.ax * r.ay;
		});
		m_xcListAspect.RedrawWindow();
		orderAspect_ = orderAspect_ ? false : true;
		break;

	default:
		break;
	}

	InitAspectControls();

	if (pResult) { *pResult = 0; }
}




void CDisplayAspectDlg::OnEnChangeEditAspect1()
{
	if (!IsWindow(m_xcListAspect.GetSafeHwnd())) { return; }

	int index = m_xcListAspect.GetIndex();
	if (listAspect_.get(index, [this](sl::WindowAspect& value) {
		value.ax = m_xcSpinAspect1.GetPos32();
	}))
	{
		m_xcListAspect.Update(index);
	}
}


void CDisplayAspectDlg::OnEnChangeEditAspect2()
{
	if (!IsWindow(m_xcListAspect.GetSafeHwnd())) { return; }

	int index = m_xcListAspect.GetIndex();
	if (listAspect_.get(index, [this](sl::WindowAspect& value) {
		value.ay = m_xcSpinAspect2.GetPos32();
	}))
	{
		m_xcListAspect.Update(index);
	}
}


void CDisplayAspectDlg::OnBnClickedBtnAspectUp()
{
	m_xcListAspect.UpItem();
	InitAspectControls();
}


void CDisplayAspectDlg::OnBnClickedBtnAspectDown()
{
	m_xcListAspect.DownItem();
	InitAspectControls();
}




void CDisplayAspectDlg::OnBnClickedBtnAspectAllDefault()
{
	sl::WindowConfig win;
	aspect_ = win.aspect;

	m_xcListAspect.Init();
	m_xcListAspect.RedrawWindow();
	InitAspectControls();
}
