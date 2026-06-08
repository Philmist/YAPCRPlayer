#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog は Windows CE ではサポートされていません。"
#endif 

#include <functional>

#include <strsafe.h>
#include <exdispid.h> // DISPID_BEFORENAVIGATE2


class CCustomControlSite :
	public CBrowserControlSite
{
public:
	CCustomControlSite(COleControlContainer* pCtrlCont, CDHtmlDialog *pHandler);
	virtual ~CCustomControlSite();

protected:
	DECLARE_INTERFACE_MAP();
	// Implement IServiceProvider
	BEGIN_INTERFACE_PART(ServiceProvider, IServiceProvider)
		STDMETHOD(QueryService)(REFGUID,REFIID,void**);
	END_INTERFACE_PART(ServiceProvider)

	// Implement IServiceProvider
	BEGIN_INTERFACE_PART(ProtectFocus, IProtectFocus)
		STDMETHOD(AllowFocusChange)(BOOL *);
	END_INTERFACE_PART(ProtectFocus)
};



// CCustomDHtmlDialog ダイアログ

class CCustomDHtmlDialog : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CCustomDHtmlDialog)

public:
	CCustomDHtmlDialog();   // 標準コンストラクター
	CCustomDHtmlDialog(UINT nIDTemplate, CWnd* pParent = NULL);
	virtual ~CCustomDHtmlDialog();

	virtual BOOL CreateControlSite(COleControlContainer* pContainer, 
		COleControlSite** ppSite, UINT /* nID */, REFCLSID /* clsid */);

// ダイアログ データ
//#ifdef AFX_DESIGN_TIME
//	enum { IDD = IDD_CUSTOMDHTML/*, IDH = IDR_HTML_CUSTOMDHTML*/ };
//#endif

protected:
	struct Custom {
		Custom()
			: navigate(false)
			, scroll(false)
			, scrollTop(0)
			, scrollHeight(0)
			, clientHeight(0)
		{}
		bool navigate;
		bool scroll;
		long scrollTop;
		long scrollHeight;
		long clientHeight;
	} custom_;

public:
	typedef std::unordered_map<std::wstring, int> BookmarkMap;

	HWND GetSafeBrowserHwnd();

	BOOL StoreScroll();
	BOOL RestoreScroll();
	BOOL ScrollToTop();
	BOOL ScrollToBottom();
	BOOL ScrollToBottomBy(long y);

	BOOL StringToIStream(const std::wstring& str, IStream** out);
	BOOL LoadFromString(const std::wstring& html);

	BOOL GetDocumentElement(CComPtr<IHTMLElement>& elem);
	BOOL GetDocumentElement(CComPtr<IHTMLElement2>& elem2);
	BOOL GetBodyElement(CComPtr<IHTMLElement>& elem);
	BOOL GetBodyElement(CComPtr<IHTMLElement2>& elem2);

	BOOL RemoveClassName(const std::wstring& search);

	double GetZoomFactor();
	BOOL GetBoundingClientRect(const CComPtr<IHTMLElement>& element, CRect& rc);
	BOOL GetBoundingClientZoomRect(const CComPtr<IHTMLElement>& element, CRect& rc);

	BOOL GetScrollParam(long& scrollTop, long& scrollHeight, long& clientHeight);

	BOOL CreateTxtRange(CComPtr<IHTMLTxtRange>& range);
	BOOL GetSelectedTxtRange(CComPtr<IHTMLTxtRange>& range);

	int HighLight(int partial,
		CComPtr<IMarkupPointer>& begin, CComPtr<IMarkupPointer>& end,
		const std::wstring& search, bool enable, const std::wstring& attr);
	
	BOOL CreateBookmarks(const std::wstring& search, std::vector<std::wstring>& bookmarks);
	BOOL CreateBookmarkMap(const std::wstring& search, BookmarkMap& map);

	BOOL SearchParent(const std::wstring& search, const boost::function<void(const CComPtr<IHTMLElement>&)>& callback);
	BOOL GetParentID(const CComPtr<IHTMLElement>& element, const std::wstring& search, std::wstring& id);

	BOOL SelectAll();
	BOOL CopyText(const std::wstring& text);
	BOOL CopySelectedText();
	BOOL CopyAnchorHref(const CComQIPtr<IHTMLAnchorElement>& anchor);
	BOOL GetAnchorHref(const CComQIPtr<IHTMLAnchorElement>& anchor, std::wstring& href);
	BOOL OpenWebSearch();

	BOOL IsEnableCommand(ULONG id);

	BOOL IsTextArea(const CPoint& screen);

	BOOL GetSource(std::wstring& source);

	void ChangeFocus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual STDMETHODIMP GetOptionKeyPath(LPOLESTR *pchKey, DWORD dw);

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
	DECLARE_EVENTSINK_MAP() // 追加
public:
	void OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT* URL,VARIANT* Flags, VARIANT* TargetFrameName, VARIANT* PostData,VARIANT* Headers, BOOL* Cancel);
};
