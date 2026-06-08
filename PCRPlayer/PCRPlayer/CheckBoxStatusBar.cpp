// CheckBoxStatusBar.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "CheckBoxStatusBar.h"


// CCheckBoxStatusBar

IMPLEMENT_DYNAMIC(CCheckBoxStatusBar, CStatusBar)

CCheckBoxStatusBar::CCheckBoxStatusBar()
{

}

CCheckBoxStatusBar::~CCheckBoxStatusBar()
{
}


BEGIN_MESSAGE_MAP(CCheckBoxStatusBar, CStatusBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()



// CCheckBoxStatusBar メッセージ ハンドラー




int CCheckBoxStatusBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CStatusBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	//CRect rc;
	//check_.Create(caption_.c_str(), WS_CHILD|WS_VISIBLE|BS_CHECKBOX, rc, this, id_);//

	//SetFont(GetParent()->GetFont());
	//check_.SetFont(GetParent()->GetFont());//

	CRect rc;
	for (auto it = items_.begin(); it != items_.end(); ++it)
	{
		boost::shared_ptr<CButton> btn(new CButton());
		if (btn)
		{
			btn->Create(it->caption.c_str(), WS_CHILD|WS_VISIBLE|BS_CHECKBOX, rc, this, it->id);
			btn->SetFont(GetParent()->GetFont());

			btns_.insert(std::make_pair(it->id, btn));
		}
	}

	return 0;
}


void CCheckBoxStatusBar::OnSize(UINT nType, int cx, int cy)
{
	CStatusBar::OnSize(nType, cx, cy);

	CRect rc;
	GetItemRect(0, &rc);

	int left = rc.left + 8;
	for (auto it = btns_.begin(); it != btns_.end(); ++it)
	{
		if (it->second && ::IsWindow(it->second->GetSafeHwnd()))
		{
			CSize size;
			it->second->GetIdealSize(&size);

			int width = left + size.cx + 8 > rc.right ? (rc.right - left) - 8 : size.cx ;
			int y = rc.top + ((rc.Height() - size.cy) / 2);
			it->second->SetWindowPos(NULL, left, y, width, size.cy, SWP_NOZORDER|SWP_NOACTIVATE);

			left += size.cx + 8;
		}
	}
}

void CCheckBoxStatusBar::AddCheckBox(const std::wstring& caption, UINT id)
{
	Item item = { caption, id };
	items_.push_back(item);
}

bool CCheckBoxStatusBar::GetCheck(UINT id, int& nCheck) const
{
	auto it = btns_.find(id);
	if (it != btns_.end())
	{
		nCheck = it->second->GetCheck();
		return true;
	}
	return false;
}

bool CCheckBoxStatusBar::SetCheck(UINT id, int nCheck)
{
	auto it = btns_.find(id);
	if (it != btns_.end())
	{
		it->second->SetCheck(nCheck);
		return true;
	}
	return false;
}