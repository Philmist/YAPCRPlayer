#pragma once


// CSimpleEdit

class CSimpleEdit : public CEdit
{
	DECLARE_DYNAMIC(CSimpleEdit)

public:
	CSimpleEdit();
	virtual ~CSimpleEdit();

protected:
	// プレス アンド ホールド ジェスチャを有効にする
	virtual ULONG GetGestureStatus(__in CPoint /*ptTouch*/) { return 0; }

protected:
	DECLARE_MESSAGE_MAP()
};


