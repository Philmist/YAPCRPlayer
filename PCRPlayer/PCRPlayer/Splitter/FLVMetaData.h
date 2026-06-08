#pragma once

#include <vector>
#include <string>

namespace flv {

typedef std::vector<unsigned char> FLVSCRIPT;

/*
keyframeData metaData size
*/

struct FLVKeyframe {
	double fileposition;
	double time;

	FLVKeyframe(double fileposition, double time)
		: fileposition(fileposition)
		, time(time)
	{}
};

struct FLVObjectProperty {
	std::string name;
	FLVSCRIPT value;

	FLVObjectProperty(const std::string& name, const FLVSCRIPT& value)
		: name(name)
		, value(value)
	{}
};

class FLVMetaData {
	FLVSCRIPT script_;
	unsigned int keyframes_;
	bool prepared_;
	unsigned int reserve_;

	std::vector<FLVKeyframe> keylist_;
	std::string encoder_;
	unsigned int videoDatarate_;
	unsigned int audioDatarate_;
	unsigned int framerate_;

public:
	FLVMetaData()
		: script_()
		, keyframes_(0)
		, prepared_(false)
		, reserve_(0)
		, videoDatarate_(0)
		, audioDatarate_(0)
		, framerate_(0)
	{}



	bool prepare(const FLVSCRIPT& src, unsigned int keyframeSize);
	bool prepared() { return prepared_; }
	void clear();

	unsigned int reserve() { return reserve_; }

	bool create(FLVSCRIPT& dst, const std::vector<FLVKeyframe>& keyframes);

	bool readMetaData(const FLVSCRIPT& src);
	LONGLONG getKeyframe(LONGLONG time);
	std::string getEncoder() { return encoder_; }
	unsigned int getVideoDatarate() { return videoDatarate_; }
	unsigned int getAudioDatarate() { return audioDatarate_; }
	unsigned int getDatarate() { return videoDatarate_ + audioDatarate_; }
	unsigned int getFramerate() { return framerate_; }

private:
	unsigned int safeRead(void* dst, const FLVSCRIPT& src, unsigned int pos, unsigned int size);
	unsigned int readSI16(short& dst, const FLVSCRIPT& src, unsigned int pos);
	unsigned int readUI16(unsigned short& dst, const FLVSCRIPT& src, unsigned int pos);
	unsigned int readUI32(unsigned int& dst, const FLVSCRIPT& src, unsigned int pos);
	unsigned int readDouble(double& dst, const FLVSCRIPT& src, unsigned int pos);
	unsigned int readType(unsigned char& dst, const FLVSCRIPT& src, unsigned int pos);
	unsigned int readValue(FLVSCRIPT& dst, const FLVSCRIPT& src, unsigned int pos);
	unsigned int readBoolean(bool& dst, const FLVSCRIPT& src, unsigned int pos);
	unsigned int readString(std::string& dst, const FLVSCRIPT& src, unsigned int pos);
	unsigned int readLongString(std::string& dst, const FLVSCRIPT& src, unsigned int pos);
	bool isEndMarker(const FLVSCRIPT& src, unsigned int pos);
	unsigned int readECMAArray(FLVSCRIPT& dst, const FLVSCRIPT& src, unsigned int pos);
	unsigned int readObject(FLVSCRIPT& dst, const FLVSCRIPT& src, unsigned int pos);
	unsigned int readObjectProperty(FLVSCRIPT& dst, const FLVSCRIPT& src, unsigned int pos);
	unsigned int readReference(unsigned short& dst, const FLVSCRIPT& src, unsigned int pos);
	unsigned int readStrictArray(FLVSCRIPT& dst, const FLVSCRIPT& src, unsigned int pos);
	unsigned int readDate(FLVSCRIPT& dst, const FLVSCRIPT& src, unsigned int pos);
	

	unsigned int writeUI32(void *dst, unsigned int src);
	unsigned int listECMAArray(std::vector<FLVObjectProperty>& list, const FLVSCRIPT& src, unsigned int pos);
	unsigned int listObject(std::vector<FLVObjectProperty>& list, const FLVSCRIPT& src, unsigned int pos);
	unsigned int listObjectProperty(std::vector<FLVObjectProperty>& list, const FLVSCRIPT& src, unsigned int pos);
	unsigned int listStrictArray(std::vector<FLVSCRIPT>& list, const FLVSCRIPT& src, unsigned int pos);
	unsigned int listStrictArray(std::vector<double>& list, const FLVSCRIPT& src, unsigned int pos);
	unsigned int listKeyframes(std::vector<FLVKeyframe>& list, const FLVSCRIPT& src, unsigned int pos);

	bool fixMetaData(FLVSCRIPT& src);
};

} // namespace flv

