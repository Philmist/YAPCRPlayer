// DetailDecoderDlg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "DetailDecoderDlg.h"
//#include "afxdialogex.h"


// CDetailDecoderDlg ダイアログ

IMPLEMENT_DYNAMIC(CDetailDecoderDlg, CDialog)

CDetailDecoderDlg::CDetailDecoderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDetailDecoderDlg::IDD, pParent)
{

}

CDetailDecoderDlg::~CDetailDecoderDlg()
{
}

void CDetailDecoderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_DXVA2, m_xcCheckDXVA2);
	DDX_Control(pDX, IDC_CMB_VIDEO, m_xcCmbVideo);
	DDX_Control(pDX, IDC_CMB_AUDIO, m_xcCmbAudio);
	DDX_Control(pDX, IDC_LIST_VIDEO, m_xcListVideo);
	DDX_Control(pDX, IDC_LIST_AUDIO, m_xcListAudio);
}


BEGIN_MESSAGE_MAP(CDetailDecoderDlg, CDialog)
	ON_BN_CLICKED(IDC_CHECK_DXVA2, &CDetailDecoderDlg::OnBnClickedCheckDxva2)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_VIDEO, &CDetailDecoderDlg::OnLvnGetdispinfoListVideo)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_AUDIO, &CDetailDecoderDlg::OnLvnGetdispinfoListAudio)
	ON_NOTIFY(NM_CLICK, IDC_LIST_VIDEO, &CDetailDecoderDlg::OnNMClickListVideo)
	ON_NOTIFY(NM_CLICK, IDC_LIST_AUDIO, &CDetailDecoderDlg::OnNMClickListAudio)
	ON_BN_CLICKED(IDC_BTN_VIDEO_ADD, &CDetailDecoderDlg::OnBnClickedBtnVideoAdd)
	ON_BN_CLICKED(IDC_BTN_AUDIO_ADD, &CDetailDecoderDlg::OnBnClickedBtnAudioAdd)
	ON_BN_CLICKED(IDC_BTN_VIDEO_DEL, &CDetailDecoderDlg::OnBnClickedBtnVideoDel)
	ON_BN_CLICKED(IDC_BTN_AUDIO_DEL, &CDetailDecoderDlg::OnBnClickedBtnAudioDel)
	ON_BN_CLICKED(IDC_BTN_VIDEO_UP, &CDetailDecoderDlg::OnBnClickedBtnVideoUp)
	ON_BN_CLICKED(IDC_BTN_AUDIO_UP, &CDetailDecoderDlg::OnBnClickedBtnAudioUp)
	ON_BN_CLICKED(IDC_BTN_VIDEO_DOWN, &CDetailDecoderDlg::OnBnClickedBtnVideoDown)
	ON_BN_CLICKED(IDC_BTN_AUDIO_DOWN, &CDetailDecoderDlg::OnBnClickedBtnAudioDown)
END_MESSAGE_MAP()


// CDetailDecoderDlg メッセージ ハンドラー


void CDetailDecoderDlg::EnumFilterMonikerForeach(REFCLSID clsidDeviceClass, DWORD dwFlags,
	const std::function<bool(const CComPtr<IMoniker>&, const FilterMoniker&)>& func)
{
	CoInitialize(NULL);
	{
		CComPtr<ICreateDevEnum> devEnum;
		devEnum.CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER);
		if (devEnum)
		{
			// カテゴリが存在しないか空の場合、戻り値は S_FALSE で ppEnumMoniker 引数は値 NULL を受け取る。
			// したがって、SUCCEEDED マクロを使う代わりに戻り値 S_OK についてテストする。

			CComPtr<IEnumMoniker> enumMoniker;
			ULONG fetched = 0;
			HRESULT hr = devEnum->CreateClassEnumerator(clsidDeviceClass, &enumMoniker, dwFlags);
			if (hr == S_OK)
			{
				CComPtr<IMoniker> moniker;
				while (enumMoniker->Next(1, &moniker, &fetched) == S_OK)
				{
					if (!moniker) { continue; }

					CComPtr<IPropertyBag> bag;
					HRESULT hr = moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&bag);

					if (SUCCEEDED(hr))
					{
						FilterMoniker fm;
						{
							VARIANT var = {};
							var.vt = VT_BSTR;
							hr = bag->Read(L"FriendlyName", &var, 0);
							if (SUCCEEDED(hr))
							{
								fm.name = var.bstrVal;
							}
							VariantClear(&var);
						}

						{
							VARIANT var = {};
							var.vt = VT_BSTR;
							hr = bag->Read(L"DevicePath", &var, 0);
							if (SUCCEEDED(hr))
							{
								fm.path = var.bstrVal;
							}
							VariantClear(&var);
						}

						if (!func(moniker, fm)) { break; }
					}

					moniker.Release();
				}
			}
		}
	}
	CoUninitialize();
}

void CDetailDecoderDlg::EnumFilterMoniker(std::vector<FilterMoniker>& data)
{
	data.clear();

	EnumFilterMonikerForeach(CLSID_LegacyAmFilterCategory, CDEF_DEVMON_CMGR_DEVICE|CDEF_DEVMON_DMO|CDEF_DEVMON_FILTER,
		[&](const CComPtr<IMoniker>& moniker, const FilterMoniker& fm)->bool{
			data.push_back(fm);
			return true;
	});
}

bool CDetailDecoderDlg::FilterMonikerToGUID(const FilterMoniker& moniker, GUID* guid)
{
	bool res = false;
	EnumFilterMonikerForeach(CLSID_LegacyAmFilterCategory, CDEF_DEVMON_CMGR_DEVICE|CDEF_DEVMON_DMO|CDEF_DEVMON_FILTER,
		[&](const CComPtr<IMoniker>& moni, const FilterMoniker& fm)->bool{
			if (moniker.name == fm.name && moniker.path == fm.path)
			{
				CComPtr<IBaseFilter> filter;
				HRESULT hr = moni->BindToObject(0, 0, IID_IBaseFilter, (void**)&filter);
				if (SUCCEEDED(hr) && filter)
				{
					if (SUCCEEDED(filter->GetClassID(guid)))
					{
						res = true;
						return false;
					}
				}
			}
			return true;
	});
	return res;
}


BOOL CDetailDecoderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// DXVA2
	m_xcCheckDXVA2.SetCheck(decoder_.dxva2 ? BST_CHECKED : BST_UNCHECKED);

	// コンボボックス
	data_.clear();
	EnumFilterMoniker(data_);

	std::stable_sort(data_.begin(), data_.end(),
		[](const FilterMoniker& l, const FilterMoniker& r)->bool{
			return StrCmpLogicalW(l.name.c_str(), r.name.c_str()) < 0;
	});

	for (auto it = data_.begin(); it != data_.end(); ++it)
	{
		m_xcCmbVideo.AddString(it->name.c_str());
		m_xcCmbAudio.AddString(it->name.c_str());
	}

	m_xcCmbVideo.SetCurSel(0);
	m_xcCmbAudio.SetCurSel(0);

	CRect rc(18, 0, 170, 0);
	MapDialogRect(&rc);

	// リストコントロール
	m_xcListVideo.SetExtendedStyle(m_xcListVideo.GetExtendedStyle()|
		LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_INFOTIP|LVS_EX_DOUBLEBUFFER|LVS_EX_CHECKBOXES);
	m_xcListVideo.SetCallbackMask(LVIS_STATEIMAGEMASK);

	m_xcListVideo.InsertColumn(0, L"", LVCFMT_CENTER, rc.left);
	m_xcListVideo.InsertColumn(1, L"", LVCFMT_LEFT, rc.right);

	m_xcListVideo.SetItemCountEx(static_cast<int>(video_.size()), LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);


	m_xcListAudio.SetExtendedStyle(m_xcListAudio.GetExtendedStyle()|
		LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_INFOTIP|LVS_EX_DOUBLEBUFFER|LVS_EX_CHECKBOXES);
	m_xcListAudio.SetCallbackMask(LVIS_STATEIMAGEMASK);

	m_xcListAudio.InsertColumn(0, L"", LVCFMT_CENTER, rc.left);
	m_xcListAudio.InsertColumn(1, L"", LVCFMT_LEFT, rc.right);

	m_xcListAudio.SetItemCountEx(static_cast<int>(audio_.size()), LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


BOOL CDetailDecoderDlg::PreTranslateMessage(MSG* pMsg)
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



void CDetailDecoderDlg::OnBnClickedCheckDxva2()
{
	decoder_.dxva2 = m_xcCheckDXVA2.GetCheck() == BST_CHECKED;
	
	m_xcCheckDXVA2.SetCheck(decoder_.dxva2 ? BST_CHECKED : BST_UNCHECKED);
}


void CDetailDecoderDlg::DispInfo(NMHDR *pNMHDR, LRESULT *pResult, std::vector<sl::DecoderInfo>& list)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if (!pDispInfo) { return; }
	int i = pDispInfo->item.iItem;

	if (pDispInfo->item.mask & LVIF_STATE)
	{
		try {
			pDispInfo->item.state = LVIS_STATEIMAGEMASK;

			if (list.at(i).enable) {
				pDispInfo->item.stateMask = INDEXTOSTATEIMAGEMASK(2);
			}
			else {
				pDispInfo->item.stateMask = INDEXTOSTATEIMAGEMASK(1);
			}
		} catch (std::exception const&) {}
	}

	if (pDispInfo->item.mask & LVIF_TEXT)
	{ 
		CString csLine;
		switch (pDispInfo->item.iSubItem)
		{
		case 1:
			{
				try {
					sl::DecoderInfo info = list.at(i);
					csLine = info.name.c_str();
				} catch (std::exception&) {}
				break;
			}
		}
		wcsncpy_s(
			pDispInfo->item.pszText,
			pDispInfo->item.cchTextMax,
			csLine, _TRUNCATE);
	}

	if (pResult) { *pResult = 0; }
}

void CDetailDecoderDlg::OnLvnGetdispinfoListVideo(NMHDR *pNMHDR, LRESULT *pResult)
{
	DispInfo(pNMHDR, pResult, video_);
}


void CDetailDecoderDlg::OnLvnGetdispinfoListAudio(NMHDR *pNMHDR, LRESULT *pResult)
{
	DispInfo(pNMHDR, pResult, audio_);
}


void CDetailDecoderDlg::ClickList(NMHDR *pNMHDR, LRESULT *pResult, std::vector<sl::DecoderInfo>& list, CListCtrl& xcList)
{
	CPoint pt;
	::GetCursorPos(&pt);
	xcList.ScreenToClient(&pt);

	LVHITTESTINFO lvHit;
	lvHit.pt = pt;
	int i = xcList.SubItemHitTest(&lvHit);

	if (lvHit.iSubItem == 0 && lvHit.flags & LVHT_ONITEMSTATEICON)
	{
		try {
			bool enable = list.at(i).enable;

			for (auto it = list.begin(); it != list.end(); ++it)
			{
				it->enable = false;
			}

			list.at(i).enable = enable ? false : true;
		} catch (std::exception const&) {}

		xcList.RedrawWindow();
	}

	if (pResult) { *pResult = 0; }
}

void CDetailDecoderDlg::OnNMClickListVideo(NMHDR *pNMHDR, LRESULT *pResult)
{
	ClickList(pNMHDR, pResult, video_, m_xcListVideo);
}


void CDetailDecoderDlg::OnNMClickListAudio(NMHDR *pNMHDR, LRESULT *pResult)
{
	ClickList(pNMHDR, pResult, audio_, m_xcListAudio);
}



void CDetailDecoderDlg::BtnAdd(std::vector<sl::DecoderInfo>& list, CComboBox& xcCmb, CListCtrl& xcList)
{
	int sel = xcCmb.GetCurSel();

	FilterMoniker moniker;
	try {
		moniker = data_.at(sel);
	} catch(...) {}

	GUID id = {};
	if (!FilterMonikerToGUID(moniker, &id))
	{
		MessageBox(L"このフィルタは使用できません", L"PCRPlayer", MB_OK|MB_ICONWARNING);
		return;
	}

	sl::DecoderInfo info;
	info.name = moniker.name;
	info.id = id;
	info.enable = true;

	bool exist = false;
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		it->enable = false;
		if (IsEqualGUID(it->id, info.id))
		{
			it->enable = true;
			exist = true;
		}
	}
	if (!exist) { list.push_back(info); }

	int i = static_cast<int>(list.size());
	xcList.SetItemCountEx(i, LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);
	xcList.SetItemState(i - 1, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	xcList.EnsureVisible(i - 1, FALSE);
	xcList.RedrawWindow();
}

void CDetailDecoderDlg::OnBnClickedBtnVideoAdd()
{
	BtnAdd(video_, m_xcCmbVideo, m_xcListVideo);
}


void CDetailDecoderDlg::OnBnClickedBtnAudioAdd()
{
	BtnAdd(audio_, m_xcCmbAudio, m_xcListAudio);
}

void CDetailDecoderDlg::BtnDel(std::vector<sl::DecoderInfo>& list, CListCtrl& xcList)
{
	int index = xcList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (index < 0 || index >= (int)list.size()) { return; }

	try {
		auto it = list.begin() + index;
		list.erase(it);
		xcList.SetItemCountEx((int)list.size(), LVSICF_NOINVALIDATEALL|LVSICF_NOSCROLL);

		if ((int)list.size() > index)
		{
			xcList.SetItemState(index, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			xcList.EnsureVisible(index, FALSE);
		}
		xcList.RedrawWindow();

	} catch (...) {}
}


void CDetailDecoderDlg::OnBnClickedBtnVideoDel()
{
	BtnDel(video_, m_xcListVideo);
}


void CDetailDecoderDlg::OnBnClickedBtnAudioDel()
{
	BtnDel(audio_, m_xcListAudio);
}


void CDetailDecoderDlg::BtnUp(std::vector<sl::DecoderInfo>& list, CListCtrl& xcList)
{
	int index = xcList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (index < 0 || index >= (int)list.size()) { return; }
	if (index == 0) { return; } // 最上にあるので動かせない

	try {
		auto tmp = list.at(index - 1);
		list.at(index - 1) = list.at(index);
		list.at(index) = tmp;

		xcList.SetItemState(index - 1, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		xcList.EnsureVisible(index - 1, FALSE);
		xcList.RedrawWindow();
	} catch (...) {}
}

void CDetailDecoderDlg::OnBnClickedBtnVideoUp()
{
	BtnUp(video_, m_xcListVideo);
}


void CDetailDecoderDlg::OnBnClickedBtnAudioUp()
{
	BtnUp(audio_, m_xcListAudio);
}

void CDetailDecoderDlg::BtnDown(std::vector<sl::DecoderInfo>& list, CListCtrl& xcList)
{
	int index = xcList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);
	if (index < 0 || index >= (int)list.size()) { return; }
	if (index + 1 == (int)list.size()) { return; } // 最下にあるので動かせない

	try {
		auto tmp = list.at(index + 1);
		list.at(index + 1) = list.at(index);
		list.at(index) = tmp;

		xcList.SetItemState(index + 1, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		xcList.EnsureVisible(index + 1, FALSE);
		xcList.RedrawWindow();
	} catch (std::exception const&) {}
}

void CDetailDecoderDlg::OnBnClickedBtnVideoDown()
{
	BtnDown(video_, m_xcListVideo);
}


void CDetailDecoderDlg::OnBnClickedBtnAudioDown()
{
	BtnDown(audio_, m_xcListAudio);
}

