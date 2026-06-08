// OperationKeyDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "OperationShortcutDlg.h"
//#include "afxdialogex.h"


// COperationShortcutDlg ダイアログ

IMPLEMENT_DYNAMIC(COperationShortcutDlg, CDialog)

COperationShortcutDlg::COperationShortcutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COperationShortcutDlg::IDD, pParent)
	, capture_(false)
{

}

COperationShortcutDlg::~COperationShortcutDlg()
{
}

void COperationShortcutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_xcList);
	DDX_Control(pDX, IDC_EDIT_KEY, m_xcEditKey);
	DDX_Control(pDX, IDC_CHECK_KEY_SHIFT, m_xcCheckKeyShift);
	DDX_Control(pDX, IDC_CHECK_KEY_CTRL, m_xcCheckKeyCtrl);
	DDX_Control(pDX, IDC_CHECK_KEY_ALT, m_xcCheckKeyAlt);
	DDX_Control(pDX, IDC_BTN_KEY_CLEAR, m_xcBtnKeyClear);
	DDX_Control(pDX, IDC_EDIT_GESTURE, m_xcEditGesture);
	DDX_Control(pDX, IDC_BTN_GESTURE_CLEAR, m_xcBtnGestureClear);
	DDX_Control(pDX, IDC_SPIN_MOVEMENT, m_xcSpinMovement);
}


BEGIN_MESSAGE_MAP(COperationShortcutDlg, CDialog)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST, &COperationShortcutDlg::OnLvnGetdispinfoList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &COperationShortcutDlg::OnLvnItemchangedList)
	ON_WM_CTLCOLOR()
	ON_EN_SETFOCUS(IDC_EDIT_KEY, &COperationShortcutDlg::OnEnSetfocusEditKey)
	ON_EN_KILLFOCUS(IDC_EDIT_KEY, &COperationShortcutDlg::OnEnKillfocusEditKey)
	ON_BN_CLICKED(IDC_CHECK_KEY_SHIFT, &COperationShortcutDlg::OnBnClickedCheckKeyShift)
	ON_BN_CLICKED(IDC_CHECK_KEY_CTRL, &COperationShortcutDlg::OnBnClickedCheckKeyCtrl)
	ON_BN_CLICKED(IDC_CHECK_KEY_ALT, &COperationShortcutDlg::OnBnClickedCheckKeyAlt)
	ON_BN_CLICKED(IDC_BTN_KEY_CLEAR, &COperationShortcutDlg::OnBnClickedBtnKeyClear)
	ON_EN_SETFOCUS(IDC_EDIT_GESTURE, &COperationShortcutDlg::OnEnSetfocusEditGesture)
	ON_EN_KILLFOCUS(IDC_EDIT_GESTURE, &COperationShortcutDlg::OnEnKillfocusEditGesture)
	ON_BN_CLICKED(IDC_BTN_GESTURE_CLEAR, &COperationShortcutDlg::OnBnClickedBtnGestureClear)
	ON_BN_CLICKED(IDC_BTN_ALL_CLEAR, &COperationShortcutDlg::OnBnClickedBtnAllClear)
	ON_BN_CLICKED(IDC_BTN_ALL_DEFAULT, &COperationShortcutDlg::OnBnClickedBtnAllDefault)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_EN_CHANGE(IDC_EDIT_MOVEMENT, &COperationShortcutDlg::OnEnChangeEditMovement)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MOVEMENT, &COperationShortcutDlg::OnDeltaposSpinMovement)
END_MESSAGE_MAP()





void COperationShortcutDlg::EnableControls(BOOL enable)
{
	m_xcEditKey.EnableWindow(enable);
	m_xcCheckKeyShift.EnableWindow(enable);
	m_xcCheckKeyCtrl.EnableWindow(enable);
	m_xcCheckKeyAlt.EnableWindow(enable);
	m_xcBtnKeyClear.EnableWindow(enable);
	//--------------------------------
	m_xcEditGesture.EnableWindow(enable);
	m_xcBtnGestureClear.EnableWindow(enable);
}

void COperationShortcutDlg::SetControls(int index)
{
	if (shortcut_.data.empty() || index < 0)
	{
		EnableControls(FALSE);
		//--------------------------------
		m_xcEditKey.SetWindowText(L"");
		m_xcCheckKeyShift.SetCheck(BST_UNCHECKED);
		m_xcCheckKeyCtrl.SetCheck(BST_UNCHECKED);
		m_xcCheckKeyAlt.SetCheck(BST_UNCHECKED);
		//--------------------------------
		m_xcEditGesture.SetWindowText(L"");
	}
	else if (index < static_cast<int>(shortcut_.data.size()))
	{
		try {
			utl::Shortcut item = shortcut_.data.at(index);

			EnableControls(TRUE);
			//--------------------------------
			SetEditKeyText(item.key, item.mod);
			DWORD mod = item.mod;
			m_xcCheckKeyShift.SetCheck((mod & utl::MODIFY_CODE_SHIFT) ? BST_CHECKED : BST_UNCHECKED);
			m_xcCheckKeyCtrl.SetCheck((mod & utl::MODIFY_CODE_CTRL) ? BST_CHECKED : BST_UNCHECKED);
			m_xcCheckKeyAlt.SetCheck((mod & utl::MODIFY_CODE_ALT) ? BST_CHECKED : BST_UNCHECKED);
			//--------------------------------
			SetEditGestureText(item.gesture);
		}
		catch (std::exception const&) {}
	}
}

void COperationShortcutDlg::GetCheckBox(int index)
{
	try {
		DWORD shift = m_xcCheckKeyShift.GetCheck() == BST_CHECKED ? utl::MODIFY_CODE_SHIFT : utl::MODIFY_CODE_NONE;
		DWORD ctrl = m_xcCheckKeyCtrl.GetCheck() == BST_CHECKED ? utl::MODIFY_CODE_CTRL : utl::MODIFY_CODE_NONE;
		DWORD alt = m_xcCheckKeyAlt.GetCheck() == BST_CHECKED ? utl::MODIFY_CODE_ALT : utl::MODIFY_CODE_NONE;

		DWORD key = shortcut_.data.at(index).key;
		DWORD mod = shift | ctrl | alt;

		shortcut_.reset(key, mod);

		shortcut_.data.at(index).mod = mod;

		SetEditKeyText(key, mod);

		m_xcList.RedrawItems(index, index);
	}
	catch (std::exception const&) {}
}

void COperationShortcutDlg::SetEditKeyText(DWORD key, DWORD mod)
{
	if (key != 0 && mod != 0)
	{
		std::wstring modstr = utl::getModText(mod);
		std::wstring keystr = utl::getKeyText(key);
		m_xcEditKey.SetWindowText((modstr + L" + " + keystr).c_str());
	}
	else
	{
		m_xcEditKey.SetWindowText(utl::getKeyText(key).c_str());
	}
}

void COperationShortcutDlg::SetEditGestureText(const std::vector<int>& gesture)
{
	std::wstring gesturestr = utl::getGestureText(gesture);
	m_xcEditGesture.SetWindowText(gesturestr.c_str());
}

// COperationShortcutDlg メッセージ ハンドラー


BOOL COperationShortcutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// エディットコントロール背景色
	colorKey_ = RGB(255, 255, 255);
	brushKey_.DeleteObject(); // brushKey_はダイアログを閉じても解放されない
	brushKey_.CreateSolidBrush(colorKey_);

	colorGesture_ = RGB(255, 255, 255);
	brushGesture_.DeleteObject(); // brushKey_はダイアログを閉じても解放されない
	brushGesture_.CreateSolidBrush(colorKey_);

    m_xcSpinMovement.SetRange32(0, 1000);
	m_xcSpinMovement.SetPos32(shortcut_.movement);

	// リストコントロール
	m_xcList.SetExtendedStyle(m_xcList.GetExtendedStyle()|
		LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_INFOTIP|LVS_EX_DOUBLEBUFFER);

	CRect rc(120, 0, 70, 0);
	MapDialogRect(&rc);

	m_xcList.InsertColumn(0, L"コマンド", LVCFMT_LEFT, rc.left);
	m_xcList.InsertColumn(1, L"キー", LVCFMT_CENTER, rc.right);
	m_xcList.InsertColumn(2, L"マウスジェスチャー", LVCFMT_CENTER, rc.right);

	m_xcList.SetItemCountEx(static_cast<int>(shortcut_.data.size()), LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

	// 非選択状態にする
	SetControls(-1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL COperationShortcutDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN ||
		pMsg->message == WM_SYSKEYDOWN)
	{
		CWnd *wnd = GetFocus(); // フォーカスがエディットボックスにある
		if (wnd && m_xcEditKey == *wnd)
		{
			if (pMsg->wParam == VK_SHIFT ||
				pMsg->wParam == VK_CONTROL ||
				pMsg->wParam == VK_MENU) // ALT
			{
				return TRUE;
			}
			else
			{
				// インデックス
				int index = m_xcList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
				if (index < 0) { return TRUE; }

				// キー解析
				DWORD shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? utl::MODIFY_CODE_SHIFT : 0;
				DWORD ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) ? utl::MODIFY_CODE_CTRL : 0;
				DWORD alt = (GetAsyncKeyState(VK_MENU) & 0x8000) ? utl::MODIFY_CODE_ALT : 0;
				DWORD mod = shift | ctrl | alt;
				DWORD key = static_cast<DWORD>(pMsg->wParam);

				try {
					// 重複キークリア
					shortcut_.reset(key, mod);

					// キー書き換え
					shortcut_.data.at(index).key = key;
					shortcut_.data.at(index).mod = mod;

					// エディットテキスト変更
					if (mod != 0 && key != 0)
					{
						std::wstring modstr = utl::getModText(mod);
						std::wstring keystr = utl::getKeyText(key);
						m_xcEditKey.SetWindowText((modstr + L" + " + keystr).c_str());
					}
					else
					{
						m_xcEditKey.SetWindowText(utl::getKeyText(key).c_str());
					}
					
					// チェックボックス変更
					m_xcCheckKeyShift.SetCheck(shift ? BST_CHECKED : BST_UNCHECKED);
					m_xcCheckKeyCtrl.SetCheck(ctrl ? BST_CHECKED : BST_UNCHECKED);
					m_xcCheckKeyAlt.SetCheck(alt ? BST_CHECKED : BST_UNCHECKED);

					m_xcList.RedrawWindow();

					return TRUE;
				}
				catch (std::exception const&) {}
			}
		}
	}

	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			return FALSE;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void COperationShortcutDlg::OnLvnGetdispinfoList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if (!pDispInfo) { return; }

	utl::Shortcut item;

	try {
		item = shortcut_.data.at(pDispInfo->item.iItem);
	}
	catch (std::exception const&) {
		if (pResult) { *pResult = 0; }
		return;
	}

	if (pDispInfo->item.mask & LVIF_TEXT)
	{ 
		CString tmp;
		switch (pDispInfo->item.iSubItem)
		{
		case 0:
			{
				tmp = utl::getIDText(item.id).c_str();
				break;
			}
		case 1:
			{
				if (item.mod != 0 && item.key != 0)
				{
					std::wstring mod = utl::getModText(item.mod);
					std::wstring key = utl::getKeyText(item.key);
					tmp = (mod + L" + " + key).c_str();
				}
				else if (item.key != 0)
				{
					tmp = utl::getKeyText(item.key).c_str();
				}
				break;
			}
		case 2:
			{
				tmp = utl::getGestureText(item.gesture).c_str();
				break;
			}
		}

		wcsncpy_s(
			pDispInfo->item.pszText,
			pDispInfo->item.cchTextMax,
			tmp, _TRUNCATE);
	}

	if (pResult) { *pResult = 0; }
}


void COperationShortcutDlg::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	SetControls(m_xcList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED));

	if (pResult) { *pResult = 0; }
}


HBRUSH COperationShortcutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	switch(nCtlColor)
	{	
	case CTLCOLOR_EDIT:
		// エディットボックス
		if (GetDlgItem(pWnd->GetDlgCtrlID())->GetSafeHwnd() == m_xcEditKey.GetSafeHwnd())
		{
			pDC->SetBkMode(OPAQUE);
			pDC->SetBkColor(colorKey_);
			pDC->SetTextColor(RGB(0, 0, 0));
			return reinterpret_cast<HBRUSH>(brushKey_.GetSafeHandle());
		}
		else if (GetDlgItem(pWnd->GetDlgCtrlID())->GetSafeHwnd() == m_xcEditGesture.GetSafeHwnd())
		{
			pDC->SetBkMode(OPAQUE);
			pDC->SetBkColor(colorGesture_);
			pDC->SetTextColor(RGB(0, 0, 0));
			return reinterpret_cast<HBRUSH>(brushGesture_.GetSafeHandle());
		}
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	default:
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	return hbr;
}


void COperationShortcutDlg::OnEnSetfocusEditKey()
{
	::DestroyCaret();

	colorKey_ = RGB(0xFF, 0xFF, 0x55);
	brushKey_.DeleteObject();
	brushKey_.CreateSolidBrush(colorKey_);
	m_xcEditKey.SetWindowText(L"設定するキーを押してください");
}


void COperationShortcutDlg::OnEnKillfocusEditKey()
{
	colorKey_ = RGB(255, 255, 255);
	brushKey_.DeleteObject();
	brushKey_.CreateSolidBrush(colorKey_);

	try {
		int index = m_xcList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
		DWORD key = shortcut_.data.at(index).key;
		DWORD mod = shortcut_.data.at(index).mod;

		if (key != 0 && mod != 0)
		{
			std::wstring modstr = utl::getModText(mod);
			std::wstring keystr = utl::getKeyText(key);
			m_xcEditKey.SetWindowText((modstr + L" + " + keystr).c_str());
		}
		else
		{
			m_xcEditKey.SetWindowText(utl::getKeyText(key).c_str());
		}
	}
	catch(std::exception const&) {}
}


void COperationShortcutDlg::OnBnClickedCheckKeyShift()
{
	GetCheckBox(m_xcList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED));
}


void COperationShortcutDlg::OnBnClickedCheckKeyCtrl()
{
	GetCheckBox(m_xcList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED));
}


void COperationShortcutDlg::OnBnClickedCheckKeyAlt()
{
	GetCheckBox(m_xcList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED));
}


void COperationShortcutDlg::OnBnClickedBtnKeyClear()
{
	int index = m_xcList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

	try {
		shortcut_.data.at(index).key = 0;
		shortcut_.data.at(index).mod = 0;
		m_xcEditKey.SetWindowText(utl::getKeyText(0).c_str());
		m_xcCheckKeyShift.SetCheck(BST_UNCHECKED);
		m_xcCheckKeyCtrl.SetCheck(BST_UNCHECKED);
		m_xcCheckKeyAlt.SetCheck(BST_UNCHECKED);
		m_xcList.RedrawItems(index, index);
	}
	catch (std::exception const&) {}
}

//---------------------------------------------------

void COperationShortcutDlg::OnEnSetfocusEditGesture()
{
	::DestroyCaret();

	colorGesture_ = RGB(0xFF, 0xFF, 0x55);
	brushGesture_.DeleteObject();
	brushGesture_.CreateSolidBrush(colorGesture_);
	m_xcEditGesture.SetWindowText(L"右ドラッグでジェスチャー入力");
}


void COperationShortcutDlg::OnEnKillfocusEditGesture()
{
	colorGesture_ = RGB(255, 255, 255);
	brushGesture_.DeleteObject();
	brushGesture_.CreateSolidBrush(colorGesture_);

	try {
		std::vector<int> gesture = shortcut_.data.at(m_xcList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED)).gesture;
		m_xcEditGesture.SetWindowText(utl::getGestureText(gesture).c_str());
	}
	catch(std::exception const&) {}
}


void COperationShortcutDlg::OnBnClickedBtnGestureClear()
{
	int index = m_xcList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

	try {
		shortcut_.data.at(index).gesture.clear();
		m_xcEditGesture.SetWindowText(L"");
		m_xcList.RedrawItems(index, index);
	}
	catch (std::exception const&) {}
}


//---------------------------------------------------

void COperationShortcutDlg::OnBnClickedBtnAllClear()
{
	for (auto it = shortcut_.data.begin(); it != shortcut_.data.end(); ++it)
	{
		it->key = 0;
		it->mod = 0;
		it->gesture.clear();
	}

	m_xcList.RedrawWindow();
	// 非選択状態にする
	SetControls(-1);
}


void COperationShortcutDlg::OnBnClickedBtnAllDefault()
{
	shortcut_ = utl::ShortcutManager();
	m_xcList.RedrawWindow();
	// 非選択状態にする
	SetControls(-1);

	m_xcSpinMovement.SetPos32(shortcut_.movement);
}




void COperationShortcutDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	CWnd *wnd = GetFocus(); // フォーカスがエディットボックスにある
	if (wnd && m_xcEditGesture == *wnd)
	{
		gesture_.start(shortcut_.movement, point);

		capture_ = true;
		SetCapture();
	}

	CDialog::OnRButtonDown(nFlags, point);
}


void COperationShortcutDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (capture_)
	{
		capture_ = false;
		ReleaseCapture();

		// インデックス
		int index = m_xcList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
		if (index < 0) { return; }

		try {
			std::vector<int> tmp;
			gesture_.get(tmp);

			// 重複キークリア
			shortcut_.reset(tmp);

			// キー書き換え
			shortcut_.data.at(index).gesture = tmp;

			// エディットテキスト変更
			m_xcEditGesture.SetWindowText(utl::getGestureText(tmp).c_str());

			m_xcList.RedrawWindow();

			return;
		}
		catch (std::exception const&) {}
	}

	CDialog::OnRButtonUp(nFlags, point);
}


void COperationShortcutDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (capture_)
	{
		gesture_.move(point);

		std::vector<int> tmp;
		gesture_.get(tmp);
		m_xcEditGesture.SetWindowText(utl::getGestureText(tmp).c_str());
	}

	CDialog::OnMouseMove(nFlags, point);
}



void COperationShortcutDlg::OnEnChangeEditMovement()
{
	if (::IsWindow(m_xcSpinMovement.GetSafeHwnd()))
	{
		shortcut_.movement = m_xcSpinMovement.GetPos32();
	}
}


void COperationShortcutDlg::OnDeltaposSpinMovement(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	shortcut_.movement = m_xcSpinMovement.GetPos32();

	if (pResult) { *pResult = 0; }
}
