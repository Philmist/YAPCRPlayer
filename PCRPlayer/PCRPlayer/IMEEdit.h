#pragma once


// CIMEEdit

class CIMEEdit : public CSimpleEdit
{
	DECLARE_DYNAMIC(CIMEEdit)

	struct Value {
		Value() : state(FALSE) {}
		BOOL state;
	} value_;
	
public:
	CIMEEdit();
	virtual ~CIMEEdit();

	static bool& through()
	{
		static bool flag = false;
		return flag;
	}

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnSetfocus();
	afx_msg void OnEnKillfocus();
};


