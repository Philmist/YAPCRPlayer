#pragma once


// CCustomEdit

class CCustomEdit : public CSimpleEdit
{
	DECLARE_DYNAMIC(CCustomEdit)

public:
	CCustomEdit();
	virtual ~CCustomEdit();

protected:
	bool focus_;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
};


