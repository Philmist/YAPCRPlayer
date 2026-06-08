
// PCRPlayer.h : PROJECT_NAME アプリケーションのメイン ヘッダー ファイルです。
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH に対してこのファイルをインクルードする前に 'stdafx.h' をインクルードしてください"
#endif

#include "resource.h"		// メイン シンボル

#include "MainDlg.h"

// CMainApp:
// このクラスの実装については、PCRPlayer.cpp を参照してください。
//

class CMainApp : public CWinApp
{
public:
	CMainApp();

// オーバーライド
public:
	virtual BOOL InitInstance();

// 実装

	DECLARE_MESSAGE_MAP()
};

extern CMainApp theApp;