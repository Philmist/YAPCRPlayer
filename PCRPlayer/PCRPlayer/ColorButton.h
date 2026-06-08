#pragma once


// CColorButton

class CColorButton : public CButton
{
	DECLARE_DYNAMIC(CColorButton)

	COLORREF color_;

public:
	CColorButton();
	virtual ~CColorButton();

	void SetColor(COLORREF color) { color_ = color; }
	COLORREF GetColor() { return color_; }

protected:
	DECLARE_MESSAGE_MAP()

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
public:
	afx_msg BOOL OnBnClicked();
};


