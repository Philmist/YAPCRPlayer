#pragma once

#include <unordered_map>
#include <boost/shared_ptr.hpp>

// CCheckBoxStatusBar

class CCheckBoxStatusBar : public CStatusBar
{
	DECLARE_DYNAMIC(CCheckBoxStatusBar)

public:
	CCheckBoxStatusBar();
	virtual ~CCheckBoxStatusBar();

public:
	struct Item {
		std::wstring caption;
		UINT id;
	};

protected:
	std::vector<Item> items_;
	std::unordered_map<UINT, boost::shared_ptr<CButton>> btns_;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	void AddCheckBox(const std::wstring& caption, UINT id);

	bool GetCheck(UINT id, int& nCheck) const;
	bool SetCheck(UINT id, int nCheck);
};


