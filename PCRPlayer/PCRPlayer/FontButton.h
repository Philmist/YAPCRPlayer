#pragma once


// CFontButton

class CFontButton : public CButton
{
	DECLARE_DYNAMIC(CFontButton)

	CFont font_;

public:
	CFontButton();
	virtual ~CFontButton();

	void SetFontIndirect(const LOGFONT& lf);
	BOOL GetLogFont(LOGFONT& lf);

	BOOL DoModal(LOGFONT& lfont);

protected:
	DECLARE_MESSAGE_MAP()

	virtual void PreSubclassWindow();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	
public:
	//afx_msg BOOL OnBnClicked();
};


