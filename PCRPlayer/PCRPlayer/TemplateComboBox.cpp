// TemplateComboBox.cpp : 実装ファイル
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "TemplateComboBox.h"


// CTemplateComboBox

IMPLEMENT_DYNAMIC(CTemplateComboBox, CComboBox)

CTemplateComboBox::CTemplateComboBox()
{
}

CTemplateComboBox::~CTemplateComboBox()
{
}

void CTemplateComboBox::AddItem(const CString& name, const boost::shared_ptr<void>& ptr)
{
	AddString(name);
	data_.push_back(ptr);
}

void CTemplateComboBox::AddItem(const boost::shared_ptr<void>& ptr)
{
	CString text;
	GetWindowText(text);

	// 同一の名前があれば追加しない
	int i = FindStringExact(0, text);
	if (i == CB_ERR)
	{
		if (!text.IsEmpty())
		{
			AddString(text);
			data_.push_back(ptr);
			SetCurSel((int)data_.size() - 1);
			return;
		}
	}

	for (int i = 0; i < 100; i++)
	{
		text.Format(L"カスタム%d", i+1);
		if (FindStringExact(0, text) == CB_ERR)
		{
			AddString(text);
			SetWindowText(text);
			data_.push_back(ptr);
			SetCurSel((int)data_.size() - 1);
			break;
		}
	}

	return;
}

void CTemplateComboBox::DelItem()
{
	int i = GetCurSel();
	if (i >= 0 && i < (int)data_.size())
	{
		DeleteString(i);
		data_.erase(data_.begin() + i);
		SetCurSel(i);
	}
}

boost::shared_ptr<void> CTemplateComboBox::GetItem()
{
	int i = GetCurSel();
	if (i >= 0 && i < (int)data_.size())
	{
		try {
			return data_.at(i);
		} catch(...) {}
	}
	return boost::shared_ptr<void>();
}

void CTemplateComboBox::Forearch(boost::function<void(const CString&, const boost::shared_ptr<void>&)> fun)
{
	int i = 0;
	for (auto it = data_.begin(); it != data_.end(); ++it)
	{
		CString name;
		GetLBText(i, name);
		fun(name, *it);
		i++;
	}
}

BEGIN_MESSAGE_MAP(CTemplateComboBox, CComboBox)
END_MESSAGE_MAP()



// CTemplateComboBox メッセージ ハンドラー


