#pragma once


// CLayoutCtrl

#define LAYOUTCTRL_CLASSNAME    _T("CLayoutCtrl")  // Window class name

class CLayoutCtrl : public CWnd
{
	DECLARE_DYNAMIC(CLayoutCtrl)

public:
	CLayoutCtrl();
	virtual ~CLayoutCtrl();

	BOOL Create(CWnd* pParentWnd);
	void ResizeWindow();
	void RecalcLayout();

	BOOL CreateMain(CDialog* wnd, UINT id);
	BOOL CreateMain(CLayoutCtrl* wnd);
	BOOL CreateSub(CDialog* wnd, UINT id);
	BOOL CreateSub(CLayoutCtrl* wnd);

	BOOL AttachMain(CDialog* wnd);
	BOOL AttachMain(CLayoutCtrl* wnd);
	BOOL AttachSub(CDialog* wnd);
	BOOL AttachSub(CLayoutCtrl* wnd);

	CWnd* DetachMain();
	CWnd* DetachSub();

	CWnd* GetMain();
	CWnd* GetSub();
	HWND GetSafeMainHwnd();
	HWND GetSafeSubHwnd();
	BOOL IsMain();
	BOOL IsSub();
	BOOL IsVisibleMain();
	BOOL IsVisibleSub();
	BOOL IsLayoutMain();
	BOOL IsLayoutSub();
	void SetSpaceMain(int space);
	int GetSpaceMain();
	void SetSpaceSub(int space);
	int GetSpaceSub();
	void ShowMain(BOOL show);
	void ShowSub(BOOL show);
	void Swap();

	void SetIdealX(int ideal);
	void SetIdealY(int ideal);
	int GetIdealX();
	int GetIdealY();
	void SetSplit(int split);
	int GetSplit();
	void SetSide(int side);
	int GetSide();
	void SetReverse(BOOL reverse);
	BOOL GetReverse();
	void SetEdge(BOOL edge);
	BOOL GetEdge();
	void SetFace(COLORREF color);
	COLORREF GetFace();
	void SetBorder(COLORREF color);
	COLORREF GetBorder();
	void SetInvert(BOOL invert);
	BOOL GetInvert();

	void GetSplitterRect(LPRECT prc);

	BOOL RedrawAll();
	BOOL RedrawMain();
	BOOL RedrawSub();
	BOOL GetMainOtherSize(CSize& size);
	BOOL GetSubOtherSize(CSize& size);

	enum SIDE {
		SIDE_TOP = 0,
		SIDE_LEFT,
		SIDE_RIGHT,
		SIDE_BOTTOM,
	};

protected:
	BOOL RegisterWindowClass();
	void InvertTracker(const CRect& rect);
	BOOL Reverse(BOOL reverse, BOOL value);

	struct This {
		This()
			: idealX(300)
			, idealY(100)
			, split(6)
			, side(SIDE_BOTTOM)
			, reverse(FALSE)
			, edge(FALSE)
			, face(RGB(240, 240, 240))
			, border(RGB(208, 208, 208))
			, invert(FALSE)
			, capture(FALSE)
		{}

		struct Window {
			Window()
				: wnd(NULL)
				, visible(FALSE)
				, layout(FALSE)
				, space(0)
			{}

			void clear()
			{
				if (wnd)
				{
					if (::IsWindow(wnd->GetSafeHwnd()))
					{
						wnd->DestroyWindow();
					}
					delete wnd;
					wnd = NULL;
					visible = FALSE;
					layout = FALSE;
					space = 0;
				}
			}
			CWnd *wnd;
			BOOL visible;
			BOOL layout;
			int space;
		};

		Window main;
		Window sub;
		int idealX;
		int idealY;
		int split;
		int side;
		BOOL reverse;
		BOOL edge;
		COLORREF face;
		COLORREF border;
		BOOL invert;
		CRect tracker;
		BOOL capture;
	} this_;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
};


