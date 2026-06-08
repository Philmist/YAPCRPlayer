#include "stdafx.h"

namespace utl {

void StatusLayout::init(const StatusFormat& format, const boost::function<void(CRect&)>& callback)
{
	name.type = format.name.type;
	bitrate.type = format.bitrate.type;
	fps.type = format.fps.type;
	size.type = format.size.type;
	time.type = format.time.type;

	name.enable = format.name.enable;
	bitrate.enable = format.bitrate.enable;
	fps.enable = format.fps.enable;
	size.enable = format.size.enable;
	time.enable = format.time.enable;

	volume.enable = format.volume.enable;
	slider.enable = false; // 初期状態は隠す

	getVideoRect_ = callback;
}

void StatusLayout::draw(CDC& dc, const CRect& rc, const std::wstring& desc, const std::wstring& nametext)
{
	bool online = gl_.player.online();
	long state = State_Stopped;

	if (!online)
	{
		gl_.player.getState(state);
	}

	namePrepare(dc, nametext);
	textPrepare(dc, online, state);
	if (!online || text.width == 0)
	{
		bitratePrepare(dc, online);
		fpsPrepare(dc, online, state);
	}
	else
	{
		bitrate.width = 0;
		fps.width = 0;
	}

	sizePrepare(dc);
	timePrepare(dc, online);
	sliderPrepare(dc);
	volumePrepare(dc);

	if (desc.empty())
	{
		leftDraw(dc, rc);
		rightDraw(dc, rc);
	}
	else
	{
		rightDraw(dc, rc);

		CRect rect(rc);
		rect.right = max(rect.left, rect.right - rightWidth());
		dc.DrawText(desc.c_str(), -1, &rect, DT_NOPREFIX|DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);
	}
}

void StatusLayout::reset()
{
	bitrate.reset = true;
	fps.reset = true;
}

int StatusLayout::middleWidth()
{
	return text.width + bitrate.width + fps.width;
}

int StatusLayout::rightWidth()
{
	return size.width + time.width + slider.width + volume.width;
}

void StatusLayout::leftDraw(CDC& dc, const CRect& rc)
{
	if (gl_.value.window.status.play)
	{
		int left = rc.left;
		int right = max(rc.right - (middleWidth() + rightWidth()), rc.left);

		name.rect = rc;
		name.rect.left = left;
		name.rect.right = min(left + (name.width), right);


		left = name.rect.right;
		right = max(rc.right - (rightWidth()), rc.left);

		text.rect = rc;
		text.rect.left = left;
		text.rect.right = min(left + (text.width), right);

		left = text.rect.right;

		bitrate.rect = rc;
		bitrate.rect.left = left;
		bitrate.rect.right = min(left + (bitrate.width), right);

		left = bitrate.rect.right;

		fps.rect = rc;
		fps.rect.left = left;
		fps.rect.right = min(left + (fps.width), right);
	}
	else
	{
		int left = rc.left;
		int right = max(rc.right - (rightWidth()), rc.left);

		name.rect = rc;
		name.rect.left = left;
		name.rect.right = min(left + (name.width), right);

		left = rc.left + name.width;

		text.rect = rc;
		text.rect.left = left;
		text.rect.right = min(left + (text.width), right);

		left = rc.left + name.width + text.width;

		bitrate.rect = rc;
		bitrate.rect.left = left;
		bitrate.rect.right = min(left + (bitrate.width), right);

		left = rc.left + name.width + text.width + bitrate.width;

		fps.rect = rc;
		fps.rect.left = left;
		fps.rect.right = min(left + (fps.width), right);
	}

	//-------------------------------------------------------------

	dc.DrawText(name.text.c_str(), -1, &name.rect, DT_NOPREFIX|DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);

	CRect fix;
	fix = fps.rect;
	fix.left = min(fix.left + fps.fix, fix.right);
	dc.DrawText(fps.text.c_str(), -1, &fix, DT_NOPREFIX|DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);

	fix = bitrate.rect;
	fix.left = min(fix.left + bitrate.fix, fix.right);
	dc.DrawText(bitrate.text.c_str(), -1, &fix, DT_NOPREFIX|DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);

	dc.DrawText(text.text.c_str(), -1, &text.rect, DT_NOPREFIX|DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS);
}

void StatusLayout::rightDraw(CDC& dc, const CRect& rc)
{
	volume.rect = rc;
	volume.rect.left = max(rc.right - (volume.width), rc.left);

	slider.rect = rc;
	slider.rect.left = max(rc.right - (slider.width + volume.width), rc.left);
	slider.rect.right = volume.rect.left;

	time.rect = rc;
	time.rect.left = max(rc.right - (time.width + slider.width + volume.width), rc.left);
	time.rect.right = slider.rect.left;

	size.rect = rc;
	size.rect.left = max(rc.right - (size.width + time.width + slider.width + volume.width), rc.left);
	size.rect.right = time.rect.left;

	//-------------------------------------------------------------

	dc.DrawText(volume.text.c_str(), -1, &volume.rect, DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER);

	if (slider.enable)
	{
		// 操作性のため左右に余白を作る
		CRect rc(slider.rect);
		rc.left = min(rc.right, rc.left + SLIDER_MARGIN);
		rc.right = max(rc.left, rc.right - SLIDER_MARGIN);

		int per = gl_.value.audio.volume;
		int pos = (int)((double)rc.Width() * ((double)per / 100.0));
		utl::drawSlider(dc, rc, pos);
	}

	dc.DrawText(time.text.c_str(), -1, &time.rect, DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER);
	dc.DrawText(size.text.c_str(), -1, &size.rect, DT_NOPREFIX|DT_SINGLELINE|DT_RIGHT|DT_VCENTER);
}


void StatusLayout::namePrepare(CDC& dc, const std::wstring& nametext)
{
	std::wstring str;
	if (gl_.player.peercast())
	{// チャンネル情報
		gl_.peca.getStatus(str);
	}

	if (str.empty() && !nametext.empty())
	{// チャンネル情報が取得できないなら引数から
		str = nametext + L" ";
	}

	calcurect(dc, name, str, false);
}

void StatusLayout::textPrepare(CDC& dc, bool online, long state)
{
	std::wstring str;
	if (gl_.player.getStatusText(str))
	{
		reset();

		if (!str.empty())
		{
			str += L" ";
		}
	}
	else
	{
		if (!online)
		{
			switch (state)
			{
			case State_Running:
				str = L"再生中 ";
				break;

			case State_Paused:
				str = L"一時停止 ";
				break;

			case State_Stopped:
			default:
				break;
			}
		}
	}

	calcurect(dc, text, str, false);
}

void StatusLayout::bitratePrepare(CDC& dc, bool online)
{
	CString str;
	if (online)
	{
		int fix = 0;
		int crate = 0, rate = 0;
		if (gl_.player.getCurrentBitrate(crate))
		{
			if (gl_.player.peercast() && gl_.peca.getBitrate(rate))
			{
				switch (bitrate.type)
				{
				case 0:
				default:
					str.Format(L"%d kbps ", crate);
					break;
				case 1:
					str.Format(L"%d kbps ", rate);
					break;
				case 2:
					str.Format(L"%d/%d kbps ", crate, rate);
					break;
				case 3:
					{
						int tmp = 0;
						if (rate > 0)
						{
							tmp = utl::lround(((double)crate * 100.0) / (double)rate);
						}
						str.Format(L"%d %% ", tmp);
						break;
					}
				case 4:
					{
						int tmp = 0;
						if (rate > 0)
						{
							tmp = utl::lround(((double)crate * 100.0) / (double)rate);
						}
						str.Format(L"%d %%/%d kbps ", tmp, rate);
						break;
					}
				case 5:
					{
						int tmp = 0;
						if (rate > 0)
						{
							tmp = utl::lround(((double)crate * 100.0) / (double)rate);
						}
						str.Format(L"%d %%/%d/%d kbps ", tmp, crate, rate);
						break;
					}
				}
			}
			else
			{
				str.Format(L"%d kbps ", crate);
			}
		}
	}

	calcurect(dc, bitrate, std::wstring(str), true);
}

void StatusLayout::fpsPrepare(CDC& dc, bool online, long state)
{
	CString str;
	int per = 0;
	if (online && gl_.player.getProgress(per))
	{
		str.Format(L"バッファ中 (%d%%) ", per);

		calcurect(dc, fps, std::wstring(str), false);
		fps.reset = true;
		return;
	}
	else if (online || (gl_.value.window.status.fps && state == State_Running))
	{
		int frame = 0, cframe = 0;
		if (gl_.player.getCurrentFramerate(cframe) && gl_.player.getFramerate(frame))
		{
			switch (fps.type)
			{
			case 0:
			default:
				str.Format(L"(%d) ", utl::lround((double)cframe / 100.0));
				break;

			case 1:
				str.Format(L"(%.2f) ", (double)cframe / 100.0);
				break;

			case 2:
				str.Format(L"(%d/%d) ", utl::lround((double)cframe / 100.0), utl::lround((double)frame / 100.0));
				break;

			case 3:
				str.Format(L"(%.2f/%.2f) ", (double)cframe / 100.0, (double)frame / 100.0);
				break;
			}
		}
	}
	calcurect(dc, fps, std::wstring(str), true);
}


void StatusLayout::sizePrepare(CDC& dc)
{
	CString str;
	if (gl_.value.window.status.size)
	{
		CSize native;
		if (gl_.player.getNativeVideoSize(native))
		{
			switch (size.type)
			{
			case 0:
			default:
				{
					CRect main;
					getVideoRect_(main);
					str.Format(L"(%dx%d) ", main.Width(), main.Height());
					break;
				}

			case 1:
				{
					str.Format(L"(%dx%d) ", native.cx, native.cy);
					break;
				}

			case 2:
				{
					CRect main;
					getVideoRect_(main);
					str.Format(L"(%dx%d/%dx%d) ", main.Width(), main.Height(), native.cx, native.cy);
					break;
				}
			}
		}
		else
		{
			CRect main;
			getVideoRect_(main);
			str.Format(L"(%dx%d) ", main.Width(), main.Height());
		}
	}

	calcurect(dc, size, std::wstring(str), false);
}

void StatusLayout::timePrepare(CDC& dc, bool online)
{
	CString str;
	if (online)
	{// オンラインは現在位置のみ
		LONGLONG cur = 0;
		if (gl_.player.getCurrentPosition(cur))
		{
			str.Format(L"%s ", utl::convertTimes(cur, gl_.value.window.status.time).c_str());
		}
	}
	else
	{
		LONGLONG cur = 0, dur = 0;
		if (gl_.player.getCurrentPosition(cur) && gl_.player.getDuration(dur))
		{
			switch (time.type)
			{
			case 0:
			default:
				str.Format(L"%s / %s ", utl::convertTimes(cur, false).c_str(), utl::convertTimes(dur, false).c_str());
				break;

			case 1:
				cur = dur - cur;
				str.Format(L"-%s / %s ", utl::convertTimes(cur, false).c_str(), utl::convertTimes(dur, false).c_str());
				break;
			}
		}
	}

	calcurect(dc, time, std::wstring(str), false);
}

void StatusLayout::sliderPrepare(CDC& dc)
{
	if (slider.enable)
	{
		slider.width = 50 + (SLIDER_MARGIN * 2);
	}
	else
	{
		slider.width = 0;
	}
}

void StatusLayout::volumePrepare(CDC& dc)
{
	CString str;
	if (gl_.value.audio.mute)
	{
		str = L"---";
	}
	else
	{
		int vol = gl_.value.audio.volume;
		vol = max(vol, 0);
		vol = min(vol, 100);
		str.Format(L"%d", vol);
	}

	CRect calc;
	dc.DrawText(L"000", -1, &calc, DT_CALCRECT|DT_NOPREFIX|DT_SINGLELINE|DT_LEFT|DT_VCENTER);

	volume.text = str;
	volume.width = calc.Width();
}

void StatusLayout::calcurect(CDC& dc, Layout& layout, const std::wstring& str, bool indent)
{
	layout.text = str;

	if (layout.text.empty())
	{
		layout.width = 0;
	}
	else
	{
		CRect calc;
		dc.DrawText(str.c_str(), -1, &calc, DT_CALCRECT|DT_NOPREFIX|DT_SINGLELINE|DT_LEFT|DT_VCENTER);

		if (indent)
		{
			if (layout.reset || layout.type != layout.old)
			{// インデントリセット
				layout.width = calc.Width();
				layout.fix = 0;
				layout.old = layout.type;
				layout.reset = false;
			}
			else
			{
				int width = layout.width;
				layout.width = max(width, calc.Width());
				layout.fix = max(width - calc.Width(), 0);
			}
		}
		else
		{
			layout.width = calc.Width();
		}
	}
}

} // namespace utl

