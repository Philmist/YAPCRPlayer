#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#include "CustomListCtrl.h"

// CDetailDecoderDlg ダイアログ

class CDetailDecoderDlg : public CDialog
{
	DECLARE_DYNAMIC(CDetailDecoderDlg)

public:
	CDetailDecoderDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CDetailDecoderDlg();

// ダイアログ データ
	enum { IDD = IDD_DETAIL_DECODER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	struct FilterMoniker {
		std::wstring name;
		std::wstring path;
	};

	std::vector<FilterMoniker> data_;

	void EnumFilterMonikerForeach(REFCLSID clsidDeviceClass, DWORD dwFlags,
		const std::function<bool(const CComPtr<IMoniker>&, const FilterMoniker&)>& func);

	void EnumFilterMoniker(std::vector<FilterMoniker>& data);

	bool FilterMonikerToGUID(const FilterMoniker& moniker, GUID* guid);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnBnClickedCheckDxva2();

	void DispInfo(NMHDR *pNMHDR, LRESULT *pResult, std::vector<sl::DecoderInfo>& list);
	afx_msg void OnLvnGetdispinfoListVideo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetdispinfoListAudio(NMHDR *pNMHDR, LRESULT *pResult);

	void ClickList(NMHDR *pNMHDR, LRESULT *pResult, std::vector<sl::DecoderInfo>& list, CListCtrl& xcList);
	afx_msg void OnNMClickListVideo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListAudio(NMHDR *pNMHDR, LRESULT *pResult);

	void BtnAdd(std::vector<sl::DecoderInfo>& list, CComboBox& xcCmb, CListCtrl& xcList);
	afx_msg void OnBnClickedBtnVideoAdd();
	afx_msg void OnBnClickedBtnAudioAdd();

	void BtnDel(std::vector<sl::DecoderInfo>& list, CListCtrl& xcList);
	afx_msg void OnBnClickedBtnVideoDel();
	afx_msg void OnBnClickedBtnAudioDel();

	void BtnUp(std::vector<sl::DecoderInfo>& list, CListCtrl& xcList);
	afx_msg void OnBnClickedBtnVideoUp();
	afx_msg void OnBnClickedBtnAudioUp();

	void BtnDown(std::vector<sl::DecoderInfo>& list, CListCtrl& xcList);
	afx_msg void OnBnClickedBtnVideoDown();
	afx_msg void OnBnClickedBtnAudioDown();

	dsp::DecoderControl decoder_;
	std::vector<sl::DecoderInfo> video_;
	std::vector<sl::DecoderInfo> audio_;

	CButton m_xcCheckDXVA2;
	CComboBox m_xcCmbVideo;
	CComboBox m_xcCmbAudio;
	CCustomListCtrl m_xcListVideo;
	CCustomListCtrl m_xcListAudio;
};
