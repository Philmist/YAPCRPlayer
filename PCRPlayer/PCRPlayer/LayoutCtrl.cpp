// SplitterWndCtrl.cpp : 実装ファイル
//

#include "stdafx.h"
#include "LayoutCtrl.h"


// CLayoutCtrl

IMPLEMENT_DYNAMIC(CLayoutCtrl, CWnd)

CLayoutCtrl::CLayoutCtrl()
{
	RegisterWindowClass();
}

CLayoutCtrl::~CLayoutCtrl()
{
	this_.main.clear();
	this_.sub.clear();
}

BOOL CLayoutCtrl::RegisterWindowClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (::GetClassInfo(hInst, LAYOUTCTRL_CLASSNAME, &wndcls))
	{
		return TRUE;
	}

	wndcls.style          = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc    = ::DefWindowProc;
	wndcls.cbClsExtra     = wndcls.cbWndExtra = 0;
	wndcls.hInstance      = hInst;
	wndcls.hIcon          = NULL;
	wndcls.hCursor        = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.hbrBackground  = (HBRUSH) (COLOR_3DFACE + 1);
	wndcls.lpszMenuName   = NULL;
	wndcls.lpszClassName  = LAYOUTCTRL_CLASSNAME;

	if (AfxRegisterClass(&wndcls))
	{
		return TRUE;
	}

	AfxThrowResourceException();
	return FALSE;
}

BOOL CLayoutCtrl::Create(CWnd* pParentWnd)
{
	// 子ウィンドウにトラッカーを描画するので
	// 以下のクリップスタイルは使わない
	// WS_CLIPSIBLINGS | WS_CLIPCHILDREN
	return CWnd::Create(
		LAYOUTCTRL_CLASSNAME,
		L"",
		WS_CHILD | WS_VISIBLE,
		CRect(),
		pParentWnd,
		0);
}

void CLayoutCtrl::ResizeWindow()
{
	if (::IsWindow(GetSafeHwnd()))
	{
		CRect rc;
		GetParent()->GetClientRect(rc);
		MoveWindow(rc);
	}
}

void CLayoutCtrl::RecalcLayout()
{
	if (IsVisibleMain() && IsVisibleSub())
	{
		CRect client;
		GetClientRect(client);
		CRect main(client), sub(client);
		CRect rc;
		GetSplitterRect(rc);

		if (this_.side == SIDE_TOP || this_.side == SIDE_BOTTOM)
		{
			if (this_.side == SIDE_TOP)
			{
				sub.bottom = rc.top;
				main.top = rc.bottom;
			}
			else
			{
				sub.top = rc.bottom;
				main.bottom = rc.top;
			}
		}
		else
		{
			if (this_.side == SIDE_LEFT)
			{
				sub.right = rc.left;
				main.left = rc.right;
			}
			else
			{
				sub.left = rc.right;
				main.right = rc.left;
			}
		}

		this_.main.wnd->MoveWindow(main);
		this_.sub.wnd->MoveWindow(sub);
	}
	else if (IsVisibleMain())
	{
		CRect client;
		GetClientRect(client);
		this_.main.wnd->MoveWindow(client);
	}
	else if (IsVisibleSub())
	{
		CRect client;
		GetClientRect(client);
		this_.sub.wnd->MoveWindow(client);
	}
}

// 内部で生成して親ウィンドウを割り当てる
BOOL CLayoutCtrl::CreateMain(CDialog* wnd, UINT id)
{
	if (!wnd) { return FALSE; }
	this_.main.clear();
	this_.main.wnd = wnd;
	wnd->Create(id, this);
	wnd->ShowWindow(SW_SHOWNA);
	this_.main.visible = TRUE;
	this_.main.layout = FALSE;
	return TRUE;
}

BOOL CLayoutCtrl::CreateMain(CLayoutCtrl* wnd)
{
	if (!wnd) { return FALSE; }
	this_.main.clear();
	this_.main.wnd = wnd;
	wnd->Create(this);
	this_.main.visible = TRUE;
	this_.main.layout = TRUE;
	return TRUE;
}

BOOL CLayoutCtrl::CreateSub(CDialog* wnd, UINT id)
{
	if (!wnd) { return FALSE; }
	this_.sub.clear();
	this_.sub.wnd = wnd;
	wnd->Create(id, this);
	wnd->ShowWindow(SW_SHOWNA);
	this_.sub.visible = TRUE;
	this_.sub.layout = FALSE;
	return TRUE;
}

BOOL CLayoutCtrl::CreateSub(CLayoutCtrl* wnd)
{
	if (!wnd) { return FALSE; }
	this_.sub.clear();
	this_.sub.wnd = wnd;
	wnd->Create(this);
	this_.sub.visible = TRUE;
	this_.sub.layout = TRUE;
	return TRUE;
}

// 外部で生成して親ウィンドウを割り当てる
BOOL CLayoutCtrl::AttachMain(CDialog* wnd)
{
	if (!wnd) { return FALSE; }
	wnd->SetParent(this);
	wnd->ShowWindow(SW_SHOWNA);
	this_.main.wnd = wnd;
	this_.main.visible = TRUE;
	this_.main.layout = FALSE;
	return TRUE;
}

BOOL CLayoutCtrl::AttachMain(CLayoutCtrl* wnd)
{
	if (!wnd) { return FALSE; }
	wnd->SetParent(this);
	wnd->ShowWindow(SW_SHOWNA);
	this_.main.wnd = wnd;
	this_.main.visible = TRUE;
	this_.main.layout = TRUE;
	return TRUE;
}

BOOL CLayoutCtrl::AttachSub(CDialog* wnd)
{
	if (!wnd) { return FALSE; }
	wnd->SetParent(this);
	wnd->ShowWindow(SW_SHOWNA);
	this_.sub.wnd = wnd;
	this_.sub.visible = TRUE;
	this_.sub.layout = FALSE;
	return TRUE;
}

BOOL CLayoutCtrl::AttachSub(CLayoutCtrl* wnd)
{
	if (!wnd) { return FALSE; }
	wnd->SetParent(this);
	wnd->ShowWindow(SW_SHOWNA);
	this_.sub.wnd = wnd;
	this_.sub.visible = TRUE;
	this_.sub.layout = TRUE;
	return TRUE;
}

CWnd* CLayoutCtrl::DetachMain()
{
	CWnd* wnd = this_.main.wnd;
	this_.main.wnd = NULL;
	this_.main.visible = FALSE;
	this_.main.layout = FALSE;
	return wnd;
}

CWnd* CLayoutCtrl::DetachSub()
{
	CWnd* wnd = this_.sub.wnd;
	this_.sub.wnd = NULL;
	this_.sub.visible = FALSE;
	this_.sub.layout = FALSE;
	return wnd;
}

CWnd* CLayoutCtrl::GetMain()
{
	return this_.main.wnd;
}

CWnd* CLayoutCtrl::GetSub()
{
	return this_.sub.wnd;
}

HWND CLayoutCtrl::GetSafeMainHwnd()
{
	return this_.main.wnd ? this_.main.wnd->GetSafeHwnd() : NULL;
}

HWND CLayoutCtrl::GetSafeSubHwnd()
{
	return this_.sub.wnd ? this_.sub.wnd->GetSafeHwnd() : NULL;
}

BOOL CLayoutCtrl::IsMain()
{
	return this_.main.wnd && ::IsWindow(this_.main.wnd->GetSafeHwnd());
}

BOOL CLayoutCtrl::IsSub()
{
	return this_.sub.wnd && ::IsWindow(this_.sub.wnd->GetSafeHwnd());
}

BOOL CLayoutCtrl::IsVisibleMain()
{
	if (IsMain())
	{
		return this_.main.visible;
	}
	return FALSE;
}

BOOL CLayoutCtrl::IsVisibleSub()
{
	if (IsSub())
	{
		return this_.sub.visible;
	}
	return FALSE;
}

BOOL CLayoutCtrl::IsLayoutMain()
{
	if (IsMain())
	{
		return this_.main.layout;
	}
	return FALSE;
}
BOOL CLayoutCtrl::IsLayoutSub()
{
	if (IsSub())
	{
		return this_.sub.layout;
	}
	return FALSE;
}

void CLayoutCtrl::SetSpaceMain(int space)
{
	this_.main.space = space;
}

int CLayoutCtrl::GetSpaceMain()
{
	return this_.main.space;
}

void CLayoutCtrl::SetSpaceSub(int space)
{
	this_.sub.space = space;
}

int CLayoutCtrl::GetSpaceSub()
{
	return this_.sub.space;
}

void CLayoutCtrl::ShowMain(BOOL show)
{
	if (IsMain())
	{
		this_.main.wnd->ShowWindow(show ? SW_SHOWNA : SW_HIDE);
		this_.main.visible = show;
	}
}

void CLayoutCtrl::ShowSub(BOOL show)
{
	if (IsSub())
	{
		this_.sub.wnd->ShowWindow(show ? SW_SHOWNA : SW_HIDE);
		this_.sub.visible = show;
	}
}

void CLayoutCtrl::Swap()
{
	This::Window tmp = this_.main;
	this_.main = this_.sub;
	this_.sub = tmp;
}


void CLayoutCtrl::SetIdealX(int ideal)
{
	this_.idealX = ideal;
}

void CLayoutCtrl::SetIdealY(int ideal)
{
	this_.idealY = ideal;
}

int CLayoutCtrl::GetIdealX()
{
	return this_.idealX;
}

int CLayoutCtrl::GetIdealY()
{
	return this_.idealY;
}

void CLayoutCtrl::SetSplit(int split)
{
	this_.split = split;
}

int CLayoutCtrl::GetSplit()
{
	return this_.split;
}

void CLayoutCtrl::SetSide(int side)
{
	this_.side = side;
}

int CLayoutCtrl::GetSide()
{
	return this_.side;
}

void CLayoutCtrl::SetReverse(BOOL reverse)
{
	this_.reverse = reverse;
}

BOOL CLayoutCtrl::GetReverse()
{
	return this_.reverse;
}

void CLayoutCtrl::SetEdge(BOOL edge)
{
	this_.edge = edge;
}

BOOL CLayoutCtrl::GetEdge()
{
	return this_.edge;
}

void CLayoutCtrl::SetFace(COLORREF color)
{
	this_.face = color;
}

COLORREF CLayoutCtrl::GetFace()
{
	return this_.face;
}

void CLayoutCtrl::SetBorder(COLORREF color)
{
	this_.border = color;
}

COLORREF CLayoutCtrl::GetBorder()
{
	return this_.border;
}

void CLayoutCtrl::SetInvert(BOOL invert)
{
	this_.invert = invert;
}

BOOL CLayoutCtrl::GetInvert()
{
	return this_.invert;
}

void CLayoutCtrl::GetSplitterRect(LPRECT prc)
{
	int spaceTL = 0, spaceRB = 0;
	if (this_.side == SIDE_LEFT || this_.side == SIDE_TOP)
	{
		spaceTL = this_.sub.space;
		spaceRB = this_.main.space;
	}
	else
	{
		spaceTL = this_.main.space;
		spaceRB = this_.sub.space;
	}

	CRect rc;
	GetClientRect(rc);
	CRect src(rc);
	if (this_.side == SIDE_TOP || this_.side == SIDE_BOTTOM)
	{
		BOOL side = Reverse(this_.reverse, this_.side == SIDE_TOP);
		if ((this_.idealY + this_.split) > rc.Height())
		{
			if (side)
			{
				src.top = max(rc.bottom - this_.split, rc.top);
				src.bottom = rc.bottom;
			}
			else
			{
				src.bottom = min(rc.top + this_.split, rc.bottom);
				src.top = rc.top;
			}
		}
		else
		{
			if (side)
			{
				src.top = rc.top + this_.idealY;
				src.bottom = src.top + this_.split;
			}
			else
			{
				src.bottom = rc.bottom - this_.idealY;
				src.top = src.bottom - this_.split;
			}
		}

		if (src.top < rc.top + spaceTL)
		{
			src.top = rc.top + spaceTL;
			src.bottom = src.top + this_.split;
		}
		if (src.bottom > rc.bottom - spaceRB)
		{
			src.bottom = rc.bottom - spaceRB;
			src.top = src.bottom - this_.split;
		}
	}
	else
	{
		BOOL side = Reverse(this_.reverse, this_.side == SIDE_LEFT);
		if ((this_.idealX + this_.split) > rc.Width())
		{
			if (side)
			{
				src.left = max(rc.right - this_.split, rc.left);
				src.right = rc.right;
			}
			else
			{
				src.right = min(rc.left + this_.split, rc.right);
				src.left = rc.left;
			}
		}
		else
		{
			if (side)
			{
				src.left = rc.left + this_.idealX;
				src.right = src.left + this_.split;
			}
			else
			{
				src.right = rc.right - this_.idealX;
				src.left = src.right - this_.split;
			}
		}

		if (src.left < rc.left + spaceTL)
		{
			src.left = rc.left + spaceTL;
			src.right = src.left + this_.split;
		}
		if (src.right > rc.right - spaceRB)
		{
			src.right = rc.right - spaceRB;
			src.left = src.right - this_.split;
		}
	}
	*prc = src;
}

BOOL CLayoutCtrl::RedrawAll()
{
	BOOL res = FALSE;
	if (::IsWindow(GetSafeHwnd()))
	{
		Invalidate();
		if (RedrawWindow())
		{
			res = TRUE;
		}
	}

	if (RedrawMain())
	{
		res = TRUE;
	}

	if (RedrawSub())
	{
		res = TRUE;
	}
	return res;
}


BOOL CLayoutCtrl::RedrawMain()
{
	if (IsMain())
	{
		this_.main.wnd->Invalidate();
		return this_.main.wnd->RedrawWindow();
	}
	return FALSE;
}

BOOL CLayoutCtrl::RedrawSub()
{
	if (IsSub())
	{
		this_.sub.wnd->Invalidate();
		return this_.sub.wnd->RedrawWindow();
	}
	return FALSE;
}

BOOL CLayoutCtrl::GetMainOtherSize(CSize& size)
{
	if (IsWindow(this->GetSafeHwnd()) && IsMain() && IsSub())
	{
		CRect parent, child;
		GetWindowRect(parent);
		this_.main.wnd->GetWindowRect(child);

		int side = GetSide();
		if (side == SIDE_TOP || side == SIDE_BOTTOM)
		{
			size.cy = parent.Height() - child.Height();
		}
		else
		{
			size.cx = parent.Width() - child.Width();
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CLayoutCtrl::GetSubOtherSize(CSize& size)
{
	if (IsWindow(this->GetSafeHwnd()) && IsMain() && IsSub())
	{
		CRect parent, child;
		GetWindowRect(parent);
		this_.sub.wnd->GetWindowRect(child);

		int side = GetSide();
		if (side == SIDE_TOP || side == SIDE_BOTTOM)
		{
			size.cy = parent.Height() - child.Height();
		}
		else
		{
			size.cx = parent.Width() - child.Width();
		}
		return TRUE;
	}
	return FALSE;
}

void CLayoutCtrl::InvertTracker(const CRect& rect)
{
	ASSERT_VALID(this);
	ASSERT(!rect.IsRectEmpty());
	ASSERT((GetStyle() & WS_CLIPCHILDREN) == 0);

	// pat-blt without clip children on
	CDC* pDC = GetDC();
	// invert the brush pattern (looks just like frame window sizing)
	CBrush* pBrush = CDC::GetHalftoneBrush();
	HBRUSH hOldBrush = NULL;
	if (pBrush != NULL)
		hOldBrush = (HBRUSH)SelectObject(pDC->m_hDC, pBrush->m_hObject);
	pDC->PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATINVERT);
	if (hOldBrush != NULL)
		SelectObject(pDC->m_hDC, hOldBrush);
	ReleaseDC(pDC);
}

BOOL CLayoutCtrl::Reverse(BOOL reverse, BOOL value)
{
	if (reverse)
	{
		return value ? FALSE : TRUE;
	}
	return value;
}


BEGIN_MESSAGE_MAP(CLayoutCtrl, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()



// CLayoutCtrl メッセージ ハンドラー




void CLayoutCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: ここにメッセージ ハンドラー コードを追加します。
					   // 描画メッセージで CWnd::OnPaint() を呼び出さないでください。

	if (IsVisibleMain() && IsVisibleSub())
	{
		CRect rc;
		GetSplitterRect(rc);

		if (this_.edge)
		{
			dc.FillSolidRect(rc, GetSysColor(COLOR_3DFACE));

			if (this_.side == SIDE_TOP || this_.side == SIDE_BOTTOM)
			{
				dc.DrawEdge(rc, BDR_SUNKENOUTER|BDR_RAISEDINNER, BF_TOP | BF_BOTTOM);
			}
			else
			{
				dc.DrawEdge(rc, BDR_SUNKENOUTER|BDR_RAISEDINNER, BF_LEFT | BF_RIGHT);
			}
		}
		else
		{
			dc.FillSolidRect(rc, this_.face);

			if (this_.side == SIDE_TOP || this_.side == SIDE_BOTTOM)
			{
				{
					CPen pen(PS_SOLID, 1, this_.border);
					CPen* oldPen = dc.SelectObject(&pen);
					// 下
					dc.MoveTo(rc.left, rc.bottom - 1);
					dc.LineTo(rc.right, rc.bottom - 1);

					dc.SelectObject(oldPen);
				}

				{
					CPen pen(PS_SOLID, 1, this_.border);
					CPen* oldPen = dc.SelectObject(&pen);
					// 上
					dc.MoveTo(rc.left, rc.top);
					dc.LineTo(rc.right, rc.top);

					dc.SelectObject(oldPen);
				}
			}
			else
			{
				{
					CPen pen(PS_SOLID, 1, this_.border);
					CPen* oldPen = dc.SelectObject(&pen);
					// 右
					dc.MoveTo(rc.right - 1, rc.top);
					dc.LineTo(rc.right - 1, rc.bottom);

					dc.SelectObject(oldPen);
				}

				{
					CPen pen(PS_SOLID, 1, this_.border);
					CPen* oldPen = dc.SelectObject(&pen);
					// 左
					dc.MoveTo(rc.left, rc.top);
					dc.LineTo(rc.left, rc.bottom);

					dc.SelectObject(oldPen);
				}
			}
		}

		////dc.FillSolidRect(rc, GetSysColor(COLOR_3DFACE));

		//if (this_.edge)
		//{
		//	if (this_.side == SIDE_TOP || this_.side == SIDE_BOTTOM)
		//	{
		//		dc.DrawEdge(rc, BDR_RAISEDOUTER|BDR_SUNKENINNER, BF_TOP | BF_BOTTOM);
		//	}
		//	else
		//	{
		//		dc.DrawEdge(rc, BDR_RAISEDOUTER|BDR_SUNKENINNER, BF_LEFT | BF_RIGHT);
		//	}
		//}

		////if (this_.edge)
		////{
		////	if (this_.side == SIDE_TOP || this_.side == SIDE_BOTTOM)
		////	{
		////		dc.DrawEdge(rc, BDR_SUNKENOUTER|BDR_RAISEDINNER, BF_TOP | BF_BOTTOM);
		////	}
		////	else
		////	{
		////		dc.DrawEdge(rc, BDR_SUNKENOUTER|BDR_RAISEDINNER, BF_LEFT | BF_RIGHT);
		////	}
		////}

		//if (this_.edge)
		//{
		//	if (this_.side == SIDE_TOP || this_.side == SIDE_BOTTOM)
		//	{
		//		dc.FillSolidRect(rc, GetSysColor(COLOR_3DSHADOW));
		//		rc.DeflateRect(0, 1);
		//		dc.FillSolidRect(rc, GetSysColor(COLOR_3DFACE));
		//	}
		//	else
		//	{
		//		dc.FillSolidRect(rc, GetSysColor(COLOR_3DSHADOW));
		//		rc.DeflateRect(1, 0);
		//		dc.FillSolidRect(rc, GetSysColor(COLOR_3DFACE));
		//	}
		//}
		//else
		//{
		//	dc.FillSolidRect(rc, GetSysColor(COLOR_3DFACE));
		//}

		//if (this_.edge)
		//{
		//	if (this_.side == SIDE_TOP || this_.side == SIDE_BOTTOM)
		//	{
		//		dc.DrawEdge(rc, BDR_RAISEDOUTER|BDR_RAISEDINNER, BF_TOP | BF_BOTTOM);
		//	}
		//	else
		//	{
		//		dc.DrawEdge(rc, BDR_RAISEDOUTER|BDR_RAISEDINNER, BF_LEFT | BF_RIGHT);
		//	}
		//}
	}
}


BOOL CLayoutCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
	//return CWnd::OnEraseBkgnd(pDC);
}


void CLayoutCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	RecalcLayout();
	RedrawWindow();
}


void CLayoutCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnLButtonDown(nFlags, point);

	CWnd::OnLButtonDblClk(nFlags, point);
}

void CLayoutCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!this_.capture)
	{
		if (IsVisibleMain() && IsVisibleSub())
		{
			CRect rc;
			GetSplitterRect(rc);
			if (rc.PtInRect(point))
			{
				this_.tracker = rc;
				this_.capture = TRUE;
				SetCapture();
				GetSplitterRect(this_.tracker);
				if (this_.invert) { InvertTracker(this_.tracker); }

				if (this_.side == SIDE_TOP || this_.side == SIDE_BOTTOM)
				{
					//SetCursor(LoadCursor(NULL, IDC_SIZENS));
					SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_VSPLITBAR));
				}
				else
				{
					//SetCursor(LoadCursor(NULL, IDC_SIZEWE));
					SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_HSPLITBAR));
				}
			}
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}


void CLayoutCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (this_.capture)
	{
		this_.capture = FALSE;
		ReleaseCapture();
		if (this_.invert) { InvertTracker(this_.tracker); }
		RecalcLayout();
		RedrawWindow();
	}
	CWnd::OnLButtonUp(nFlags, point);
}


void CLayoutCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	int spaceTL = 0, spaceRB = 0;
	if (this_.side == SIDE_LEFT || this_.side == SIDE_TOP)
	{
		spaceTL = this_.sub.space;
		spaceRB = this_.main.space;
	}
	else
	{
		spaceTL = this_.main.space;
		spaceRB = this_.sub.space;
	}

	if (this_.capture)
	{
		CRect rc;
		GetClientRect(rc);
		int half = (this_.split) / 2;
		if (this_.side == SIDE_TOP || this_.side == SIDE_BOTTOM)
		{
			if (point.y < (rc.top + spaceTL) + half)
			{
				point.y = (rc.top + spaceTL) + half;
			}
			else if (point.y > (rc.bottom - spaceRB) - half)
			{
				point.y = (rc.bottom - spaceRB) - half;
			}

			if (this_.invert) { InvertTracker(this_.tracker); }
			this_.tracker.OffsetRect(0, point.y - this_.tracker.top - half);
			if (this_.invert) { InvertTracker(this_.tracker); }
			

			BOOL side = Reverse(this_.reverse, this_.side == SIDE_TOP);
			if (side)
			{
				this_.idealY = (point.y - rc.top) - half;
			}
			else
			{
				this_.idealY = (rc.bottom - point.y) - half;
			}
		}
		else
		{
			if (point.x < (rc.left + spaceTL) + half)
			{
				point.x = (rc.left + spaceTL) + half;
			}
			else if (point.x > (rc.right - spaceRB) - half)
			{
				point.x = (rc.right - spaceRB) - half;
			}

			if (this_.invert) { InvertTracker(this_.tracker); }
			this_.tracker.OffsetRect(point.x - this_.tracker.left - half, 0);
			if (this_.invert) { InvertTracker(this_.tracker); }

			BOOL side = Reverse(this_.reverse, this_.side == SIDE_LEFT);
			if (side)
			{
				this_.idealX = (point.x - rc.left) - half;
			}
			else
			{
				this_.idealX = (rc.right - point.x) - half;
			}
		}

		if (!this_.invert)
		{
			RecalcLayout();
			RedrawWindow();
		}
	}
	else if (IsVisibleMain() && IsVisibleSub())
	{
		CRect rc;
		GetSplitterRect(rc);
		if (rc.PtInRect(point))
		{
			if (this_.side == SIDE_TOP || this_.side == SIDE_BOTTOM)
			{
				//SetCursor(LoadCursor(NULL, IDC_SIZENS));
				SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_VSPLITBAR));
			}
			else
			{
				//SetCursor(LoadCursor(NULL, IDC_SIZEWE));
				SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_HSPLITBAR));
			}
		}
	}

	CWnd::OnMouseMove(nFlags, point);
}



void CLayoutCtrl::OnDestroy()
{
	CWnd::OnDestroy();

	this_.main.clear();
	this_.sub.clear();
}
