#include "../stdafx.h"
#include "FLVMetaData.h"

#ifdef _DEBUG
#	define DebugText(str, ...) \
	{ \
		wchar_t c[1024]; \
		swprintf_s(c, str, __VA_ARGS__); \
		OutputDebugString(c); \
	}
#else
#    define DebugText(str, ...) // ‹óŽÀ‘•
#endif


namespace flv {

void addUI16(FLVSCRIPT& dst, unsigned short src)
{
	dst.push_back((src & 0xFF00) >> 8);
	dst.push_back(src & 0x00FF);
}

void addUI32(FLVSCRIPT& dst, unsigned int src)
{
	dst.push_back((src & 0xFF000000) >> 24);
	dst.push_back((src & 0x00FF0000) >> 16);
	dst.push_back((src & 0x0000FF00) >> 8);
	dst.push_back((src & 0x000000FF));
	
}

void addDouble(FLVSCRIPT& dst, double src)
{
	const unsigned int size = sizeof(double);
	unsigned char buffer[size];

	std::memcpy(buffer, &src, size);

	for (int i = size - 1; i >= 0; --i)
	{
		dst.push_back(buffer[i]);
	}
}

void addString(FLVSCRIPT& dst, const std::string& src)
{
	addUI16(dst, (unsigned short)src.length());
	for (std::string::const_iterator it = src.cbegin(); it != src.cend(); ++it)
	{
		dst.push_back((unsigned char)*it);
	}
}

void FLVMetaData::clear()
{
	script_.clear();
	keyframes_ = 0;
	prepared_ = false;
	reserve_ = 0;

	keylist_.clear();
	encoder_.clear();
	videoDatarate_ = 0;
	audioDatarate_ = 0;
}




bool FLVMetaData::prepare(const FLVSCRIPT& src, unsigned int keyframes)
{
	if (prepared_ || keyframes <= 1) { return false; }

	FLVSCRIPT script(src);

	if (fixMetaData(script))
	{
		script_ = script;
		keyframes_ = keyframes;
		prepared_ = true;
		reserve_ = 47 + (keyframes * 18);
		return true;
	}
	return false;
}

bool FLVMetaData::create(FLVSCRIPT& dst, const std::vector<FLVKeyframe>& keyframes)
{
	if (!prepared_) { return false; }
	if (keyframes_ == 0 || keyframes.size() == 0) { return false; }

	const unsigned int size = (unsigned int)min(keyframes_, keyframes.size());
	const unsigned int divide = keyframes_ - 1;
	const unsigned int sample = (unsigned int)keyframes.size();
	const unsigned int count = min(sample, divide);
	unsigned int result = 0;

	FLVSCRIPT src = script_;

	// delete OBJECTEND
	src.pop_back();
	src.pop_back();
	src.pop_back();

	// OBJECTPROPERTY(STRING)
	addString(src, "keyframes");
	// OBJECTPROPERTY(VALUE[OBJECT])
	src.push_back(3);
	//     OBJECTPROPERTY(STRING)
	addString(src, "filepositions");
	//     OBJECTPROPERTY(VALUE[STRICTARRAY])
	src.push_back(10);
	//         STRICTARRAY
	addUI32(src, size);

	//             VALUE[NUMBER]
	if (divide < sample)
	{
		src.push_back(0);
		addDouble(src, keyframes[0].fileposition);
	}
	for (unsigned int i = 0; i < count; ++i)
	{
		result = ((sample / divide) * (i + 1)) + min(sample % divide, i + 1) - 1;
		src.push_back(0);
		addDouble(src, keyframes[result].fileposition);
	}

	//     OBJECTPROPERTY(STRING)
	addString(src, "times");
	//     OBJECTPROPERTY(VALUE[STRICTARRAY])
	src.push_back(10);
	//         STRICTARRAY
	addUI32(src, size);

	//             VALUE[NUMBER]
	if (divide < sample)
	{
		src.push_back(0);
		addDouble(src, keyframes[0].time);
	}
	for (unsigned int i = 0; i < count; ++i)
	{
		result = ((sample / divide) * (i + 1)) + min(sample % divide, i + 1) - 1;
		src.push_back(0);
		addDouble(src, keyframes[result].time);
	}

	//     OBJECTEND
	src.push_back(0);
	src.push_back(0);
	src.push_back(9);

	// OBJECTEND
	src.push_back(0);
	src.push_back(0);
	src.push_back(9);

	dst = src;

	return true;
}


bool FLVMetaData::readMetaData(const FLVSCRIPT& src)
{
	unsigned int pos = 0, size = 0;

	// SCRIPTDATAVALUE(script name)
	unsigned char type;
	if ((size = readType(type, src, pos)) == 0) { return false; }
	if (type != 2) { return false; }
	pos += size;

	std::string name;
	if ((size = readString(name, src, pos)) == 0) { return false; }

	if (_stricmp(name.c_str(), "onMetaData")) { return false; }
	pos += size;

	//DebugText(L"SCRIPTDATAVALUE(%d, %S)\n", type, name.c_str());

	// SCRIPTDATAVALUE(script value)
	if ((size = readType(type, src, pos)) == 0) { return false; }
	pos += size;

	std::vector<FLVObjectProperty> list;
	switch(type)
	{
	case 3:
		if ((size = listObject(list, src, pos)) == 0) { return false; }
		break;
	case 8:
		if ((size = listECMAArray(list, src, pos)) == 0) { return false; }
		break;
	default:
		return false;
	}

	for (std::vector<FLVObjectProperty>::iterator it = list.begin(); it != list.end(); ++it)
	{
		if (!_stricmp(it->name.c_str(), "keyframes"))
		{
			keylist_.clear();
			listKeyframes(keylist_, it->value, 0);

			//DebugText(L"keyframes\n");
		}
		else if (!_stricmp(it->name.c_str(), "encoder"))
		{
			std::string encoder;
			if (it->value.size() > 0 && it->value[0] == 2)
			{
				readString(encoder, it->value, 1);
			}
			encoder_ = encoder;

			//DebugText(L"encoder:%S\n", encoder_.c_str());
		}
		else if (!_stricmp(it->name.c_str(), "videodatarate"))
		{
			double number = 0.0;
			if (it->value.size() > 0 && it->value[0] == 0)
			{
				readDouble(number, it->value, 1);
			}
			videoDatarate_ = static_cast<unsigned int>(number);

			//DebugText(L"videodatarate:%d\n", videoDatarate_);
		}
		else if (!_stricmp(it->name.c_str(), "audiodatarate"))
		{
			double number = 0.0;
			if (it->value.size() > 0 && it->value[0] == 0)
			{
				readDouble(number, it->value, 1);
			}
			audioDatarate_ = static_cast<unsigned int>(number);

			//DebugText(L"audiodatarate:%d\n", audioDatarate_);
		}
		else if (!_stricmp(it->name.c_str(), "framerate"))
		{
			double number = 0.0;
			if (it->value.size() > 0 && it->value[0] == 0)
			{
				readDouble(number, it->value, 1);
			}
			framerate_ = static_cast<unsigned int>(number * 100.0);

			//DebugText(L"framerate:%d\n", framerate_);
		}
	}

	return true;
}

LONGLONG FLVMetaData::getKeyframe(LONGLONG time)
{
	//DebugText(L"getKeyframe Enter keylist_.size:%d\n", keylist_.size());
	if (keylist_.size() <= 0) { return LONGLONG_MIN; }

	LONGLONG pos = 0;
	for (auto it = keylist_.begin(); it != keylist_.end(); ++it)
	{
		//DebugText(L"getKeyframe time:%lld it->time:%lld\n", time, static_cast<LONGLONG>(it->time * 10000000.0));
		if (static_cast<LONGLONG>(it->time * 10000000.0) > time)
		{
			break;
		}
		pos = static_cast<LONGLONG>(it->fileposition);
	}

	//DebugText(L"getKeyframe Exit\n");
	return pos;
}


unsigned int FLVMetaData::safeRead(void* dst, const FLVSCRIPT& src, unsigned int pos, unsigned int size)
{
	if ((src.size() - pos) < size || size == 0)
	{
		return 0;
	}
	std::memcpy(dst, src.data() + pos, size);
	return size;
}

unsigned int FLVMetaData::readSI16(short& dst, const FLVSCRIPT& src, unsigned int pos)
{
	const unsigned int size = sizeof(short);

	short i = 0;
	if (safeRead(&i, src, pos, size) == size)
	{
		dst = (i & 0x00FF) << 8 | (i & 0xFF00) >> 8;
		return size;
	}
	return 0;
}

unsigned int FLVMetaData::readUI16(unsigned short& dst, const FLVSCRIPT& src, unsigned int pos)
{
	const unsigned int size = sizeof(unsigned short);

	unsigned short i = 0;
	if (safeRead(&i, src, pos, size) == size)
	{
		dst = (i & 0x00FF) << 8 | (i & 0xFF00) >> 8;
		return size;
	}
	return 0;
}

unsigned int FLVMetaData::readUI32(unsigned int& dst, const FLVSCRIPT& src, unsigned int pos)
{
	const unsigned int size = sizeof(unsigned int);

	unsigned int i = 0;
	if (safeRead(&i, src, pos, size) == size)
	{
		dst =
			(i & 0x000000FF) << 24 |
			(i & 0x0000FF00) << 8 |
			(i & 0x00FF0000) >> 8 |
			(i & 0xFF000000) >> 24;
		return size;
	}
	return 0;
}

unsigned int FLVMetaData::readDouble(double& dst, const FLVSCRIPT& src, unsigned int pos)
{
	const unsigned int size = sizeof(double);
	unsigned char buffer[size];
	unsigned char tmp;

	if (safeRead(buffer, src, pos, size))
	{
		for (int i = 0; i < (size / 2); ++i)
		{
			tmp = buffer[i];
			buffer[i] = buffer[size - (i + 1)];
			buffer[size - (i + 1)] = tmp;
		}
		std::memcpy(&dst, buffer, size);
		return size;
	}
	return 0;
}

unsigned int FLVMetaData::readType(unsigned char& dst, const FLVSCRIPT& src, unsigned int pos)
{
	const unsigned int size = sizeof(unsigned char);

	return safeRead(&dst, src, pos, size);
}

unsigned int FLVMetaData::readValue(FLVSCRIPT& dst, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;
	unsigned char type;
	if ((size = readType(type, src, pos)) == 0) { return 0; }
	all += size;

	//std::cout << "value_type:" << (int)type << std::endl;

	switch (type) {
	case 0: // Number
		//DOUBLE
		{
			double number;
			if ((size = readDouble(number, src, pos + all)) == 0) { return 0; }
			all += size;
			//std::cout << "number:" << number << std::endl;
		}
		break;
	case 1: // Boolean
		//UI8
		{
			bool boolean;
			if ((size = readBoolean(boolean, src, pos + all)) == 0) { return 0; }
			all += size;

			//std::cout << "boolean:" << (boolean ? "true" : "false") << std::endl;
		}
		break;
	case 2: // String
		// SCRIPTDATASTRING
		{
			std::string str;
			if ((size = readString(str, src, pos + all)) == 0) { return 0; }
			all += size;
			//std::cout << "\"" << str << "\"" << std::endl;
		}
		break;
	case 3: // Object
		// SCRIPTDATAOBJECT
		{
			FLVSCRIPT obj;
			if ((size = readObject(obj, src, pos + all)) == 0) { return 0; }
			all += size;
		}
		break;

	case 4: // MovieClip (reserved, not supported)
		dst.clear();
		dst.push_back(4);
		return sizeof(unsigned char);
		break;
	case 5: // Null
		dst.clear();
		dst.push_back(5);
		return sizeof(unsigned char);
	case 6: // Undefined
		dst.clear();
		dst.push_back(6);
		return sizeof(unsigned char);

	case 7: // Reference
		// UI16
		{
			unsigned short ref;
			if ((size = readUI16(ref, src, pos + all)) == 0) { return 0; }
			all += size;
		}
		break;
	case 8: // ECMA array
		// SCRIPTDATAECMAARRAY
		{
			FLVSCRIPT ecma;
			if ((size = readECMAArray(ecma, src, pos + all)) == 0) { return 0; }
			all += size;
		}
		break;
	case 9: // Object end marker
		dst.clear();
		dst.push_back(9);
		return sizeof(unsigned char);
	case 10: // Strict array
		// SCRIPTDATASTRICTARRAY
		{
			FLVSCRIPT arr;
			if ((size = readStrictArray(arr, src, pos + all)) == 0) { return 0; }
			all += size;
		}
		break;
	case 11: // Date
		// SCRIPTDATADATE
		{
			FLVSCRIPT date;
			if ((size = readDate(date, src, pos + all)) == 0) { return 0; }
			all += size;
		}
		break;
	case 12: // Long string
		// SCRIPTDATALONGSTRING
		//scriptDataLongString();
		break;
	default:
		return 0;
	}

	dst.resize(all);
	return safeRead(dst.data(), src, pos, all);
}

unsigned int FLVMetaData::readBoolean(bool& dst, const FLVSCRIPT& src, unsigned int pos)
{
	const unsigned int size = sizeof(unsigned char);
	unsigned char boolean;

	if (safeRead(&boolean, src, pos, size))
	{
		dst = boolean != 0 ? true : false;
		return size;
	}
	return 0;
}

unsigned int FLVMetaData::readString(std::string& dst, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;

	unsigned short length = 0;
	if ((size = readUI16(length, src, pos)) == 0) { return 0; }
	if (length == 0) { return size; }
	all += size;

	std::vector<char> buffer;
	buffer.resize(length);
	if ((size = safeRead(buffer.data(), src, pos + all, length)) == 0) { return 0; }
	all += size;

	buffer.push_back('\0');
	dst = buffer.data();

	return all;
}

unsigned int FLVMetaData::readLongString(std::string& dst, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;

	unsigned int length = 0;
	if ((size = readUI32(length, src, pos)) == 0) { return 0; }
	if (length == 0) { return size; }
	all += size;

	std::vector<char> buffer;
	buffer.resize(length);
	if ((size = safeRead(buffer.data(), src, pos + all, length)) == 0) { return 0; }
	all += size;

	buffer.push_back('\0');
	dst = buffer.data();

	return all;
}

bool FLVMetaData::isEndMarker(const FLVSCRIPT& src, unsigned int pos)
{
	unsigned char marker[3];
	unsigned int size = safeRead(marker, src, pos, 3);
	if (size != 3) { return false; }
	if (marker[0] != 0 || marker[1] != 0 || marker[2] != 9) { return false; }
	return true;
}

unsigned int FLVMetaData::readECMAArray(FLVSCRIPT& dst, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;
	unsigned int length;
	if ((size = readUI32(length, src, pos)) == 0) { return 0; }
	all += size;

	FLVSCRIPT obj;
	if ((size = readObject(obj, src, pos + all)) == 0) { return 0; }
	all += size;

	dst.resize(all);
	return safeRead(dst.data(), src, pos, all);
}

unsigned int FLVMetaData::readObject(FLVSCRIPT& dst, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;
	FLVSCRIPT prop;

	while (true)
	{
		if ((size = readObjectProperty(prop, src, pos + all)) == 0) { return 0; }
		all += size;

		if (isEndMarker(src, pos + all))
		{
			dst.resize(all + 3);
			return safeRead(dst.data(), src, pos, all + 3);
		}
	}
	return 0;
}

unsigned int FLVMetaData::readObjectProperty(FLVSCRIPT& dst, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;

	std::string name;
	if ((size = readString(name, src, pos)) == 0) { return 0; }
	all += size;

	FLVSCRIPT value;
	if ((size = readValue(value, src, pos + all)) == 0) { return 0; }
	all += size;

	dst.resize(all);
	return safeRead(dst.data(), src, pos, all);
}

unsigned int FLVMetaData::readStrictArray(FLVSCRIPT& dst, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;

	unsigned int length;
	if ((size = readUI32(length, src, pos)) == 0) { return 0; }
	all += size;

	FLVSCRIPT value;
	for (unsigned int i = 0; i < length; ++i)
	{
		if ((size = readValue(value, src, pos + all)) == 0) { return 0; }
		all += size;
	}

	dst.resize(all);
	return safeRead(dst.data(), src, pos, all);
}

unsigned int FLVMetaData::readDate(FLVSCRIPT& dst, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;

	double time;
	if ((size = readDouble(time, src, pos)) == 0) { return 0; }
	all += size;

	short offset;
	if ((size = readSI16(offset, src, pos + all)) == 0) { return 0; }
	all += size;

	dst.resize(all);
	return safeRead(dst.data(), src, pos, all);
}



unsigned int FLVMetaData::writeUI32(void *dst, unsigned int src)
{
	const unsigned int size = sizeof(unsigned int);

	unsigned int i =
		(src & 0x000000FF) << 24 |
		(src & 0x0000FF00) << 8 |
		(src & 0x00FF0000) >> 8 |
		(src & 0xFF000000) >> 24;

	std::memcpy(dst, &i, size);

	return size;
}

// SCRIPTDATAECMAARRAY
unsigned int FLVMetaData::listECMAArray(std::vector<FLVObjectProperty>& list, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;
	unsigned int length;

	// UI32 ECMAArrayLength
	if ((size = readUI32(length, src, pos)) == 0) { return 0; }
	all += size;

	// SCRIPTDATAOBJECT
	if ((size = listObject(list, src, pos + all)) == 0) { return 0; }
	all += size;

	return all;
}

// SCRIPTDATAOBJECT
unsigned int FLVMetaData::listObject(std::vector<FLVObjectProperty>& list, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;

	// SCRIPTDATAOBJECTPROPERTY [ ]
	while (true)
	{
		if ((size = listObjectProperty(list, src, pos + all)) == 0) { return 0; }
		all += size;

		// SCRIPTDATAOBJECTEND
		if (isEndMarker(src, pos + all))
		{
			return all + 3;
		}
	}
	return 0;
}

// SCRIPTDATAOBJECTPROPERTY
unsigned int FLVMetaData::listObjectProperty(std::vector<FLVObjectProperty>& list, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;

	// SCRIPTDATASTRING
	std::string name;
	if ((size = readString(name, src, pos)) == 0) { return 0; }
	all += size;

	// SCRIPTDATAVALUE
	FLVSCRIPT value;
	if ((size = readValue(value, src, pos + all)) == 0) { return 0; }
	all += size;

	list.push_back(FLVObjectProperty(name, value));

	return all;
}

unsigned int FLVMetaData::listStrictArray(std::vector<FLVSCRIPT>& list, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;
	
	// UI32 StrictArrayLength
	unsigned int length;
	if ((size = readUI32(length, src, pos)) == 0) { return 0; }
	all += size;

	// SCRIPTDATAVALUE []
	for (unsigned int i = 0; i < length; ++i)
	{
		FLVSCRIPT value;
		if ((size = readValue(value, src, pos + all)) == 0) { return 0; }
		all += size;
		list.push_back(std::move(value));
	}
	return all;
}

unsigned int FLVMetaData::listStrictArray(std::vector<double>& list, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;
	
	// UI32 StrictArrayLength
	unsigned int length;
	if ((size = readUI32(length, src, pos)) == 0) { return 0; }
	all += size;

	// SCRIPTDATAVALUE []
	for (unsigned int i = 0; i < length; ++i)
	{
		FLVSCRIPT value;
		if ((size = readValue(value, src, pos + all)) == 0) { return 0; }
		all += size;

		if (value.size() > 0 && value[0] == 0)
		{
			double number = 0.0;
			readDouble(number, value, 1);
			list.push_back(number);
		}
	}
	return all;
}


unsigned int FLVMetaData::listKeyframes(std::vector<FLVKeyframe>& list, const FLVSCRIPT& src, unsigned int pos)
{
	unsigned int all = 0, size = 0;
	unsigned char type;

	if ((size = readType(type, src, pos)) == 0) { return 0; }
	all += size;

	std::vector<FLVObjectProperty> obj;
	switch(type)
	{
	case 3:
		if ((size = listObject(obj, src, pos + all)) == 0) { return 0; }
		all += size;
		break;
	case 8:
		if ((size = listECMAArray(obj, src, pos + all)) == 0) { return 0; }
		all += size;
		break;
	default:
		return 0;
	}

	std::vector<double> fp, tm;
	for (auto it = obj.begin(); it != obj.end(); ++it)
	{
		if (!_stricmp(it->name.c_str(), "filepositions"))
		{
			if (it->value.size() > 0 && it->value[0] == 10)
			{
				listStrictArray(fp, it->value, 1);
			}

			//DebugText(L"filepositions\n");
		}
		else if (!_stricmp(it->name.c_str(), "times"))
		{
			if (it->value.size() > 0 && it->value[0] == 10)
			{
				listStrictArray(tm, it->value, 1);
			}

			//DebugText(L"times\n");
		}
	}

	list.clear();
	for (auto itF = fp.begin(), itT = tm.begin(); itF != fp.end() && itT != tm.end(); ++itF, ++itT)
	{
		list.push_back(FLVKeyframe(*itF, *itT));
		//DebugText(L"FLVKeyframe(%.3f, %.3f)\n", *itF, *itT);
	}

	return all;
}

bool FLVMetaData::fixMetaData(FLVSCRIPT& src)
{
	unsigned int pos = 0, size = 0;

	// SCRIPTDATAVALUE(script name)
	unsigned char type;
	if ((size = readType(type, src, pos)) == 0) { return false; }
	if (type != 2) { return false; }
	pos += size;

	std::string name;
	if ((size = readString(name, src, pos)) == 0) { return false; }

	if (_stricmp(name.c_str(), "onMetaData")) { return false; }
	pos += size;

	// SCRIPTDATAVALUE(script value)
	if ((size = readType(type, src, pos)) == 0) { return false; }
	pos += size;

	std::vector<FLVObjectProperty> list;
	switch(type)
	{
	case 3:
		if ((size = listObject(list, src, pos)) == 0) { return false; }
		break;
	case 8:
		if ((size = listECMAArray(list, src, pos)) == 0) { return false; }
		break;
	default:
		return false;
	}

	// find "keyframes" element
	bool existKeyframes = false;
	for (std::vector<FLVObjectProperty>::iterator it = list.begin(); it != list.end(); ++it)
	{
		if (!_stricmp(it->name.c_str(), "keyframes"))
		{
			existKeyframes = true;

			keylist_.clear();
			listKeyframes(keylist_, it->value, 0);
		}
		else if (!_stricmp(it->name.c_str(), "encoder"))
		{
			std::string encoder;
			if (it->value.size() > 0 && it->value[0] == 2)
			{
				readString(encoder, it->value, 1);
			}
			encoder_ = encoder;
		}
		else if (!_stricmp(it->name.c_str(), "videodatarate"))
		{
			double number = 0.0;
			if (it->value.size() > 0 && it->value[0] == 0)
			{
				readDouble(number, it->value, 1);
			}
			videoDatarate_ = static_cast<unsigned int>(number);
		}
		else if (!_stricmp(it->name.c_str(), "audiodatarate"))
		{
			double number = 0.0;
			if (it->value.size() > 0 && it->value[0] == 0)
			{
				readDouble(number, it->value, 1);
			}
			audioDatarate_ = static_cast<unsigned int>(number);
		}
		else if (!_stricmp(it->name.c_str(), "framerate"))
		{
			double number = 0.0;
			if (it->value.size() > 0 && it->value[0] == 0)
			{
				readDouble(number, it->value, 1);
			}
			framerate_ = static_cast<unsigned int>(number * 100.0);

			//DebugText(L"framerate:%d\n", framerate_);
		}
	}

	if (existKeyframes) { return false; }

	// fix size
	if (src.size() > (pos + size))
	{
		FLVSCRIPT flip;
		flip.assign(src.begin(), src.begin() + (pos + size));
		std::swap(src, flip);
	}

	// set array length
	if (type == 8)
	{
		unsigned int length = (unsigned int)list.size() + 1;
		writeUI32(src.data() + pos, length);
	}

	return true;
}

} // namespace flv

