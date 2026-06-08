#pragma once

#pragma warning(disable : 4005)
#pragma warning(disable : 4503)

#include <array>
#include <vector>
#include <string>
#include <functional>
#include <codecvt>

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>



class XmlParser {
	typedef boost::optional<boost::property_tree::wptree&> optree;

	boost::property_tree::wptree pt_;
	optree opt_;

public:
	XmlParser()
		: opt_(pt_)
	{}

	XmlParser(const optree& opt)
		: opt_(opt)
	{}

	XmlParser(const XmlParser& x)
		: opt_(x.opt_)
	{}

	XmlParser(const std::wstring& path)
		: opt_(pt_)
	{
		read(path);
	}

	XmlParser(const wchar_t* path) // 新規追加
		: opt_(pt_)
	{
		read(path);
	}

	XmlParser operator=(const XmlParser& x)
	{
		opt_ = x.opt_;
		return *this;
	}

	operator bool() const // 新規追加
	{
		return static_cast<bool>(opt_);
	}

	////////////////////////////////////////
	// Writer
	void write(const std::wstring& path)
	{
		using namespace boost::property_tree;

		std::ofstream ofs(path, std::ios::out|std::ios::binary|std::ios::trunc);
		if (ofs.is_open())
		{
			std::wstringstream wss;
			try {
				write_xml(wss, pt_, xml_writer_make_settings<std::wstring>(L'\t', 1));
			}
			catch (...) {}

			std::string str = toUTF8(wss.str());
			ofs.write(str.data(), str.size());
			ofs.close();
		}

		//-------------------------------------------------------------------------

		//std::wofstream ofs(path);
		//if (ofs.is_open())
		//{
		//	ofs.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
		//	try {
		//		write_xml(ofs, pt_, xml_writer_make_settings<std::wstring>(L'\t', 1));
		//	}
		//	catch (...) {}
		//	ofs.close();
		//}

		//-------------------------------------------------------------------------

		//FILE *file = nullptr;
		//if (!::_wfopen_s(&file, path.c_str(), L"wb"))
		//{
		//	std::wstringstream wss;
		//	try {
		//		write_xml(wss, pt_, xml_writer_make_settings<std::wstring>(L'\t', 1));
		//	}
		//	catch (...) {}

		//	std::string cnv = toUTF8(wss.str());

		//	fwrite(cnv.c_str(), sizeof(char), cnv.size(), file);
		//	
		//	::fclose(file);
		//}
	}

	// 汎用
	template<typename T>
	void put(const std::wstring& path, const T& value)
	{
		if (opt_)
		{
			opt_->put(L"<xmlattr>." + path, value);
		}
	}

	// vector
	template<typename T>
	void put(const std::wstring& path, const std::vector<T>& value)
	{
		put(path, makeCsv(value));
	}

	// array
	template<typename T, size_t SIZE>
	void put(const std::wstring& path, const std::array<T, SIZE>& value)
	{
		put(path, makeCsv(value));
	}

	// 親指定
	XmlParser put(const std::wstring& path)
	{
		if (opt_)
		{
			return opt_->put(path, L"");
		}
		return boost::none;
	}

	// 親追加
	XmlParser add(const std::wstring& path)
	{
		if (opt_)
		{
			return opt_->add(path, L"");
		}
		return boost::none;
	}

	////////////////////////////////////////
	// Reader
	void read(const std::wstring& path)
	{
		using namespace boost::property_tree;

		std::ifstream ifs(path, std::ios::in|std::ios::binary);
		if (ifs.is_open())
		{
			ifs.seekg (0, ifs.end);
			size_t length = (size_t)ifs.tellg();
			ifs.seekg (0, ifs.beg);

			if (length > 0)
			{
				std::vector<char> dat;
				dat.resize(length);
				ifs.read(dat.data(), dat.size());
				ifs.close();

				std::wstring wstr = toUTF16(std::string(dat.data(), dat.size()));

				if (!wstr.empty())
				{
					std::wstringstream wss;
					wss.write(wstr.data(), wstr.size());

					try {
						read_xml(wss, pt_);
					}
					catch (...) {}
				}
			}
		}

		//-------------------------------------------------------------------------

		//std::wifstream ifs(path);
		//if (ifs.is_open())
		//{
		//	ifs.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
		//	try {
		//		read_xml(ifs, pt_);
		//	}
		//	catch (...) {}
		//	ifs.close();
		//}

		//-------------------------------------------------------------------------

		//FILE *file = nullptr;
		//if (!::_wfopen_s(&file, path.c_str(), L"rb"))
		//{

		//	std::string cnv = toUTF8(wss.str());

		//	fread(cnv.c_str(), sizeof(char), cnv.size(), file);

		//	std::wstringstream wss;
		//	try {
		//		read_xml(wss, pt_);
		//	}
		//	catch (...) {}


		//	::fclose(file);
		//}
	}

	// 汎用例外
	template<typename T>
	bool getx(const std::wstring& path, T& value) const
	{
		if (opt_)
		{
			auto v = opt_->get_optional<T>(L"<xmlattr>." + path);
			if (v)
			{
				value = *v;
				return true;
			}
		}
		throw std::exception();
		return false;
	}

	// vector例外
	template<typename T>
	bool getx(const std::wstring& path, std::vector<T>& value) const
	{
		if (opt_)
		{
			auto v = opt_->get_optional<std::wstring>(L"<xmlattr>." + path);
			if (v)
			{
				readCsv(value, *v);
				return true;
			}
		}
		throw std::exception();
		return false;
	}

	// array例外
	template<typename T, size_t SIZE>
	bool getx(const std::wstring& path, std::array<T, SIZE>& value) const
	{
		if (opt_)
		{
			auto v = opt_->get_optional<std::wstring>(L"<xmlattr>." + path);
			if (v)
			{
				readCsv(value, *v);
				return true;
			}
		}
		throw std::exception();
		return false;
	}

	// 汎用
	template<typename T>
	bool get(const std::wstring& path, T& value) const
	{
		if (opt_)
		{
			auto v = opt_->get_optional<T>(L"<xmlattr>." + path);
			if (v)
			{
				value = *v;
				return true;
			}
		}
		return false;
	}

	template<typename T>
	bool get(const std::wstring& path, T& value, const T& def) const
	{
		if (get(path, value)) { return true; }
		value = def;
		return false;
	}

	// vector
	template<typename T>
	bool get(const std::wstring& path, std::vector<T>& value) const
	{
		if (opt_)
		{
			auto v = opt_->get_optional<std::wstring>(L"<xmlattr>." + path);
			if (v)
			{
				readCsv(value, *v);
				return true;
			}
		}
		return false;
	}

	template<typename T>
	bool get(const std::wstring& path, std::vector<T>& value, const wchar_t* def) const
	{
		if (get(path, value)) { return true; }
		readCsv(value, def);
		return false;
	}


	// array
	template<typename T, size_t SIZE>
	bool get(const std::wstring& path, std::array<T, SIZE>& value) const
	{
		if (opt_)
		{
			auto v = opt_->get_optional<std::wstring>(L"<xmlattr>." + path);
			if (v)
			{
				readCsv(value, *v);
				return true;
			}
		}
		return false;
	}

	// array
	template<typename T, size_t SIZE>
	bool get(const std::wstring& path, std::array<T, SIZE>& value, const wchar_t* def) const
	{
		if (get(path, value)) { return true; }
		readCsv(value, def);
		return false;
	}

	// array
	template<typename T, size_t SIZE>
	bool get(const std::wstring& path, std::array<T, SIZE>& value, const std::array<T, SIZE>& def) const
	{
		if (get(path, value)) { return true; }
		value = def;
		return false;
	}

	// 文字列用
	bool get(const std::wstring& path, std::wstring& value, const wchar_t* def) const
	{
		return get(path, value, std::wstring(def));
	}

	// 親指定
	XmlParser get(const std::wstring& path) const
	{
		if (opt_)
		{
			return opt_->get_child_optional(path);
		}
		return boost::none;
	}


	// 走査
	void foreach(const std::wstring& path, const std::function<void(const XmlParser&)>& func) const
	{
		if (opt_)
		{
			for (auto it = opt_->begin(); it != opt_->end(); ++it)
			{
				if (it->first == path)
				{
					func(XmlParser(it->second));
				}
			}
		}
	}

	void search(const std::wstring& path, const std::function<bool(const XmlParser&)>& func) const
	{
		if (opt_)
		{
			for (auto it = opt_->begin(); it != opt_->end(); ++it)
			{
				if (it->first == path)
				{
					if (func(XmlParser(it->second))) { break; }
				}
			}
		}
	}


private:

	// コンテナをCSVに変換
	template<typename T>
	std::wstring makeCsv(const T& v)
	{
		std::wstringstream ss;
		for (auto it = v.begin(); it != v.end(); ++it)
		{
			if (it != v.begin()) { ss << L","; }
			ss << *it;
		}
		return ss.str();
	}

	// vector
	template<typename T>
	std::wstring makeCsv(const std::vector<T>& v)
	{
		return makeCsv<std::vector<T> >(v);
	}

	// array
	template<typename T, size_t SIZE>
	std::wstring makeCsv(const std::array<T, SIZE>& v)
	{
		return makeCsv<std::array<T, SIZE> >(v);
	}

	// csvをvectorに読み込み
	template<typename T>
	void readCsv(std::vector<T>& v, const std::wstring &str) const
	{
		typedef boost::char_separator<wchar_t> separator;
		typedef boost::tokenizer<separator, std::wstring::const_iterator, std::wstring> tokenizer;

		separator sep(L",", L"", boost::keep_empty_tokens);
		tokenizer tokens(str, sep);

		for (auto it = tokens.begin(); it != tokens.end(); ++it)
		{
			try {
				v.push_back(boost::lexical_cast<T>(it->c_str()));
			} catch(...) {
				v.push_back(T());
			}
		}
	}

	// csvをarrayに読み込み
	template<typename T, size_t SIZE>
	void readCsv(std::array<T, SIZE>& v, const std::wstring &str) const
	{
		typedef boost::char_separator<wchar_t> separator;
		typedef boost::tokenizer<separator, std::wstring::const_iterator, std::wstring> tokenizer;

		separator sep(L",", L"", boost::keep_empty_tokens);
		tokenizer tokens(str, sep);

		int i = 0;
		for (auto it = tokens.begin(); it != tokens.end(); ++it)
		{
			if (i == v.size()) { break; }
			try {
				v[i] = boost::lexical_cast<T>(it->c_str());
			} catch (...) {
				v[i] = T();
			}
			++i;
		}
	}

	std::string toUTF8(const std::wstring& src)
	{
		const int size = ::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, NULL, 0, NULL, NULL);

		if (size < 0) { return std::string(); }

		std::vector<char> buff(size);

		int result = ::WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, buff.data(), size, NULL, NULL);

		if (result < 0) { return std::string(); }

		return buff.data();
	}

	std::wstring toUTF16(const std::string& src)
	{
		const int size = ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, NULL, 0);

		if (size < 0) { return std::wstring(); }

		std::vector<wchar_t> buff(size);

		int result = ::MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, buff.data(), size);

		if (result < 0) { return std::wstring(); }

		return buff.data();
	}
};



// xmlの読み書き
/*
void save()
{
XmlParser xml;
auto root = xml.put(L"root");
auto p = root.put(L"window.size");
p.put(L"x", 640);
p.put(L"y", 480);
xml.write(L"test.xml");
}

void load()
{
int x, y;
XmlParser xml(L"test.xml");
auto root = xml.get(L"root");
auto p = root.get(L"window.size");
p.get(L"x", x, 640);
p.get(L"y", y, 480);
}
*/

