#pragma once

namespace utl {

class ITemplateList
{
public:
	ITemplateList() {}
	virtual ~ITemplateList() {}

	virtual int size() = 0;
	virtual bool empty() = 0;
	virtual void add() = 0;
	virtual bool copy(int index) = 0;
	virtual bool del(int index) = 0;
	virtual bool up(int index) = 0;
	virtual bool down(int index) = 0;
	virtual bool check(int index) = 0;
	virtual bool toggle(int index) = 0;
	virtual std::wstring text(int index, int sub) = 0;
};

template <typename T>
class TemplateList : public ITemplateList
{
protected:
	T& list_;

public:
	typedef typename T conteiner_type;
	typedef typename T::value_type value_type;

	TemplateList(T& list) : list_(list) {}
	virtual ~TemplateList() {}

	virtual int size()
	{
		return static_cast<int>(list_.size());
	}

	virtual bool empty()
	{
		return list_.empty();
	}

	virtual void add()
	{
		list_.push_back(value_type());
	}

	virtual bool copy(int index)
	{
		if (index < 0 || index >= size()) { return false; }

		try {
			list_.push_back(list_.at(index));
			return true;
		}
		catch (...) {}
		return false;
	}

	virtual bool del(int index)
	{
		if (index < 0 || index >= size()) { return false; }

		return list_.erase(list_.begin() + index) != list_.end();
	}

	virtual bool up(int index)
	{
		if (index < 0 || index >= size()) { return false; }

		try {
			std::swap(list_.at(index), list_.at(index - 1));
			return true;
		}
		catch (...) {}
		return false;
	}

	virtual bool down(int index)
	{
		if (index < 0 || index >= size()) { return false; }
		// ÉäÉXÉgÇÃàÍî‘â∫Ç»ÇÃÇ≈â∫Ç…ÇÕà⁄ìÆÇ≈Ç´Ç»Ç¢
		if (index >= size() - 1) { return false; }

		try {
			std::swap(list_.at(index), list_.at(index + 1));
			return true;
		}
		catch (...) {}
		return false;
	}

	virtual bool check(int index)
	{
		if (index < 0 || index >= size()) { return false; }
		try {
			return checkImpl(list_.at(index));
		}
		catch (...) {}
		return false;
	}

	virtual bool toggle(int index)
	{
		if (index < 0 || index >= size()) { return false; }
		try {
			return toggleImpl(list_.at(index));
		}
		catch (...) {}
		return false;
	}

	virtual std::wstring text(int index, int sub)
	{
		if (index < 0 || index >= size()) { return false; }
		try {
			return textImpl(list_.at(index), sub);
		}
		catch (...) {}
		return false;
	}

public:
	virtual void insert(const T& list, size_t limit)
	{
		list_.insert(list_.end(), list.begin(), list.end());

		if (limit > 0 && list_.size() > limit)
		{
			size_t pos = list_.size() - limit;
			list_.erase(list_.begin(), list_.begin() + pos);
		}
	}

	typename T::iterator begin()
	{
		return list_.begin();
	}

	typename T::iterator end()
	{
		return list_.end();
	}

	bool get(int index, value_type& value)
	{
		if (list_.empty()) { return false; }
		if (index < 0 || index >= size()) { return false; }
		try {
			value = list_.at(index);
			return true;
		}
		catch (...) {}
		return false;
	}

	bool get(int index, const boost::function<void(value_type&)>& func)
	{
		if (list_.empty()) { return false; }
		if (index < 0 || index >= size()) { return false; }
		try {
			func(list_.at(index));
			return true;
		}
		catch (...) {}
		return false;
	}

	void foreach(const boost::function<void(value_type&)>& func)
	{
		for (auto it = list_.begin(); it != list_.end(); ++it)
		{
			func(*it);
		}
	}

	void set(const conteiner_type& list)
	{
		list_ = list;
	}

protected:
	virtual bool checkImpl(value_type& value) { return false; };
	virtual bool toggleImpl(value_type& value) { return false; };
	virtual std::wstring textImpl(value_type& value, int sub) { return std::wstring(); };
};

//------------------------------------------------------------------------------------

class ReplaceElementList : public utl::TemplateList<std::vector<utl::ReplaceElement>>
{
public:
	ReplaceElementList(conteiner_type& list)
		: utl::TemplateList<conteiner_type>(list)
	{}
	virtual ~ReplaceElementList() {}

protected:
	virtual bool checkImpl(value_type& value)
	{
		return value.enable;
	};

	virtual bool toggleImpl(value_type& value)
	{
		value.enable = value.enable ? false : true;
		return value.enable;
	};

	virtual std::wstring textImpl(value_type& value, int sub)
	{
		switch (sub)
		{
		case 1: return value.desc;
		case 2: return value.regex;
		case 3: return value.format;
		case 4: return value.icase ? L"Åõ" : L"Å~";
		}
		return std::wstring();
	}
};

class WindowZoomList : public utl::TemplateList<std::vector<sl::WindowZoom>>
{
public:
	WindowZoomList(conteiner_type& list)
		: utl::TemplateList<conteiner_type>(list)
	{}
	virtual ~WindowZoomList() {}

protected:
	virtual bool checkImpl(value_type& value)
	{
		return value.enable;
	};

	virtual bool toggleImpl(value_type& value)
	{
		value.enable = value.enable ? false : true;
		return value.enable;
	};

	virtual std::wstring textImpl(value_type& value, int sub)
	{
		switch (sub)
		{
		case 1:
			{
				CString str;
				str.Format(L"%d%%", value.zoom);
				return std::wstring(str);
			}
		}
		return std::wstring();
	}
};

class WindowSizeList : public utl::TemplateList<std::vector<sl::WindowSize>>
{
public:
	WindowSizeList(conteiner_type& list)
		: utl::TemplateList<conteiner_type>(list)
	{}
	virtual ~WindowSizeList() {}

protected:
	virtual bool checkImpl(value_type& value)
	{
		return value.enable;
	};

	virtual bool toggleImpl(value_type& value)
	{
		value.enable = value.enable ? false : true;
		return value.enable;
	};

	virtual std::wstring textImpl(value_type& value, int sub)
	{
		switch (sub)
		{
		case 1:
		{
			CString str;
			str.Format(L"%dx%d", value.width, value.height);
			return std::wstring(str);
		}
		}
		return std::wstring();
	}
};

class WindowAspectList : public utl::TemplateList<std::vector<sl::WindowAspect>>
{
public:
	WindowAspectList(conteiner_type& list)
		: utl::TemplateList<conteiner_type>(list)
	{}
	virtual ~WindowAspectList() {}

protected:
	virtual std::wstring textImpl(value_type& value, int sub)
	{
		switch (sub)
		{
		case 0:
		{
			CString str;
			str.Format(L"%d:%d", value.ax, value.ay);
			return std::wstring(str);
		}
		}
		return std::wstring();
	}
};


} // namespace utl