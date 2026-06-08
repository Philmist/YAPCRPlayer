#pragma once

#include "CustomListCtrl.h"
#include "TemplateList.h"

// CTemplateListCtrl

class CTemplateListCtrl : public CCustomListCtrl
{
	DECLARE_DYNAMIC(CTemplateListCtrl)

public:
	CTemplateListCtrl(utl::ITemplateList& list, bool check = false);
	virtual ~CTemplateListCtrl();

	void Init();
	void Apply();
	BOOL IsSelectLast();
	void SelectLast();
	int GetIndex();
	void AddItem();
	void CopyItem();
	void DelItem();
	void UpItem();
	void DownItem();

protected:
	utl::ITemplateList& list_;
	bool check_;

	DECLARE_MESSAGE_MAP()
public:
	virtual void PreSubclassWindow();
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
};


