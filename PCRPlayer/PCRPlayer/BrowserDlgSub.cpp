#include "stdafx.h"
#include "PCRPlayer.h"
#include "BrowserDlg.h"

bool CBrowserDlg::getStatus(std::wstring& text)
{
	int per = this_.dat.size() == 0 ? 100 : (((int)this_.pos * 100) / (int)this_.dat.size());
	CString str;
	str.Format(L"ì«Ç›çûÇ›íÜ %d%%", per);
	text = str.GetString();
	return per < 100 && per > 0;
}


bool CBrowserDlg::createRes(int size)
{
	if (this_.dat.size() <= this_.pos)
	{
		return false;
	}

	update_();

	CComPtr<IHTMLDocument2> doc2;
	if (!SUCCEEDED(GetDHtmlDocument(&doc2)) || !doc2) { return false; }

	std::wstring noname;
	gl_.bbs.getNoName(noname);
	bbs::BBSSkin skin(gl_.value.detail.skin.thread, noname);

	auto resid = [](const std::wstring &html, const bbs::ResInfo& info) {
		return L"<div id=\"res:" + info.number + L"\">" + html + L"</div>";
	};

	auto resskin = [&](const bbs::BBSSkin &skin, const bbs::ResInfo& info)->std::wstring {
		return resid(skin.res(
			gl_.value.detail.board.newly && info.latest ? gl_.html.newRes : gl_.html.res,
			info), info) + L"\r\n";
	};


	std::wstring html;
	if (this_.first)
	{
		this_.first = false;
		if (this_.pos != 0)
		{
			auto it = this_.dat.begin();
			if (it != this_.dat.end())
			{
				html += resskin(skin, *it);
			}
		}
	}

	int count = 0;
	for (auto it = this_.dat.begin() + this_.pos; it != this_.dat.end(); ++it)
	{
		html += resskin(skin, *it);

		++this_.pos;
		++count;
		if (size > 0 && count >= size) { break; }
	}

	if (!html.empty())
	{
		doc2->write(utl::SafeArray(html));
	}


	if (this_.dat.size() <= this_.pos)
	{
		utl::refreshColor(doc2, this_.dat, gl_.value.detail.skin.thread);


		if (this_.init)
		{
			if (this_.reset)
			{
				RestoreScroll();
			}
			else if (gl_.value.detail.scroll.position.init)
			{
				ScrollToBottom();
			}
		}
		else
		{
			if (gl_.value.detail.scroll.position.latest)
			{
				RestoreScroll();
			}
		}

		//::RedrawWindow(GetSafeBrowserHwnd(), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);

		DWORD time = GetTickCount() - this_.time;
		gl_.addLog(L"time:%u", time);

		update_();
		return false;
	}
	update_();
	return true;
}
