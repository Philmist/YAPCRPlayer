#include "stdafx.h"

#pragma warning(disable : 4503)

// グローバルデータ本体
gl::GlobalValues gl_;
sl::SerializeValues sl_;

namespace gl {

Callback::Callback()
	: dat(
		[](const bbs::BBSMultiCallback<bool>::CallbackType& callback){
			gl_.bbs.setDatCallback(callback);
		},
		[](CWnd* wnd, bool scroll){
			if (wnd && ::IsWindow(wnd->GetSafeHwnd()))
			{
				if (scroll)
				{
					wnd->PostMessage(WM_COMMAND, IDM_UPDATE_SCROLL_END);
				}
				else
				{
					wnd->PostMessage(WM_COMMAND, IDM_UPDATE_RES_END);
				}
			}
		})
	, extract(
		[](const bbs::BBSCallback<void>::CallbackType& callback){
			gl_.bbs.setExtractCallback(callback);
		},
		[](CWnd* wnd){
			if (wnd && ::IsWindow(wnd->GetSafeHwnd()))
			{
				wnd->PostMessage(WM_COMMAND, IDM_UPDATE_EXTRACT_END);
			}
		})
{}


void GlobalValues::getAspect(SIZE& aspect)
{
	aspect.cx = 0;
	aspect.cy = 0;
	if  (value.window.aspect == sl::WINDOW_ASPECT_DEFAULT)
	{
		player.getNativeVideoSize(aspect);
	}
	else if (value.window.aspect == sl::WINDOW_ASPECT_NONE)
	{
		aspect.cx = 0;
		aspect.cy = 0;
	}
	else
	{
		int i = value.window.aspect - sl::WINDOW_ASPECT_CUSTOM;

		try {
			sl::WindowAspect x = value.display.window.aspect.at(i);
			aspect.cx = x.ax;
			aspect.cy = x.ay;
		} catch(...) {}
	}
}

void GlobalValues::createChannelInfo()
{
	peca.createInfo(value.display.information);
	peca.createStatus(
		value.display.information.status,
		value.window.status.channel.name,
		value.window.status.channel.type,
		value.window.status.channel.genre,
		value.window.status.channel.comment,
		value.window.status.channel.playing,
		value.window.status.channel.relay);
	peca.createPanel(value.display.information.panel);
}

}
