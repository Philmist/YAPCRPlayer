#pragma once

#include <boost/functional.hpp>

namespace utl {

// ステータスバー書式
struct StatusFormat {
	struct Format {
		Format()
			: enable(true)
			, type(0)

		{}

		bool enable; // 書式変更
		int type; // 書式
	};

	Format name; // 名前
	Format bitrate; // ビットレート
	Format fps; // fps
	Format size; // 画面サイズ
	Format time; // 再生時間
	Format volume; // 音量
};

class StatusLayout {
public:
	enum { SLIDER_MARGIN = 8 };

	struct Layout {
		Layout()
			: width(0)
			, fix(0)
			, reset(false)
			, enable(true)
			, type(0)
			, old(0)
		{}

		std::wstring text;
		int width; // 幅
		int fix; // インデント
		bool reset; // インデントリセット
		bool enable; // 表示/非表示
		int type; // 書式
		int old; // 前の書式
		CRect rect; // 外接矩形
	};

	Layout name;
	Layout text;
	Layout bitrate;
	Layout fps;

	Layout size;
	Layout time;
	Layout slider;// enableは遅延表示のための一時使用で保存しない
	Layout volume;// enableでスライダー表示切り替え

public:
	void init(const StatusFormat& format, const boost::function<void(CRect&)>& callback);
	void draw(CDC& dc, const CRect& rc, const std::wstring& desc, const std::wstring& nametext);
	void reset(); // インデントリセット

private:
	int middleWidth();
	int rightWidth();
	
	void leftDraw(CDC& dc, const CRect& rc);
	void rightDraw(CDC& dc, const CRect& rc);
	

	void namePrepare(CDC& dc, const std::wstring& name);
	void textPrepare(CDC& dc, bool online, long state);
	void bitratePrepare(CDC& dc, bool online);
	void fpsPrepare(CDC& dc, bool online, long state);

	void sizePrepare(CDC& dc);
	void timePrepare(CDC& dc, bool online);
	void sliderPrepare(CDC& dc);
	void volumePrepare(CDC& dc);

	void calcurect(CDC& dc, Layout& layout, const std::wstring& text, bool indent);

private:
	boost::function<void(CRect&)> getVideoRect_;
};

} // namespace utl

