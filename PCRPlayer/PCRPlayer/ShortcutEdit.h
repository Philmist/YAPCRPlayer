#pragma once


// CShortcutEdit

class CShortcutEdit : public CSimpleEdit
{
	DECLARE_DYNAMIC(CShortcutEdit)

public:
	CShortcutEdit();
	virtual ~CShortcutEdit();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


