#pragma once


// CCustomListCtrl

class CCustomListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CCustomListCtrl)

public:
	CCustomListCtrl();
	virtual ~CCustomListCtrl();

	void SizeHelper();
	void SetStretchColumn(int stretch) { stretch_ = stretch; }
	void SetStripe(bool stripe) { stripe_ = stripe; }
	void SetHeaderText(int index, const std::wstring& text);

protected:
	virtual ULONG GetGestureStatus(__in CPoint /*ptTouch*/) { return 0; }

protected:
	bool drag_;
	int stretch_;
	bool stripe_;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);

	BOOL SetItemCountEx(_In_ int iCount, _In_ DWORD dwFlags = LVSICF_NOINVALIDATEALL);

private:
	void ShowVScrollBar();
};


