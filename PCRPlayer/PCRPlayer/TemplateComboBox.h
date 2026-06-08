#pragma once


// CTemplateComboBox


class CTemplateComboBox : public CComboBox
{
	DECLARE_DYNAMIC(CTemplateComboBox)

	std::vector<boost::shared_ptr<void> > data_;

public:
	CTemplateComboBox();
	virtual ~CTemplateComboBox();

	void AddItem(const CString& name, const boost::shared_ptr<void>& ptr);
	void AddItem(const boost::shared_ptr<void>& ptr);
	
	void DelItem();
	boost::shared_ptr<void> GetItem();

	void Forearch(boost::function<void(const CString&, const boost::shared_ptr<void>&)> fun);

protected:
	DECLARE_MESSAGE_MAP()
};


