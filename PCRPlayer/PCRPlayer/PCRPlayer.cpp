
// PCRPlayer.cpp : アプリケーションのクラス動作を定義します。
//

#include "stdafx.h"
#include "PCRPlayer.h"
#include "MainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainApp

BEGIN_MESSAGE_MAP(CMainApp, CWinApp)
	//ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMainApp コンストラクション

CMainApp::CMainApp()
{
	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
}


// 唯一の CMainApp オブジェクトです。

CMainApp theApp;


// CMainApp 初期化

BOOL CMainApp::InitInstance()
{
	// アプリケーション マニフェストが visual スタイルを有効にするために、
	// ComCtl32.dll Version 6 以降の使用を指定する場合は、
	// Windows XP に InitCommonControlsEx() が必要です。さもなければ、ウィンドウ作成はすべて失敗します。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// アプリケーションで使用するすべてのコモン コントロール クラスを含めるには、
	// これを設定します。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// ActiveX
	AfxEnableControlContainer();

	// ダイアログにシェル ツリー ビューまたはシェル リスト ビュー コントロールが
	// 含まれている場合にシェル マネージャーを作成します。
	CShellManager *pShellManager = new CShellManager;

	// MFC コントロールでテーマを有効にするために、"Windows ネイティブ" のビジュアル マネージャーをアクティブ化
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 標準初期化
	// これらの機能を使わずに最終的な実行可能ファイルの
	// サイズを縮小したい場合は、以下から不要な初期化
	// ルーチンを削除してください。
	// 設定が格納されているレジストリ キーを変更します。
	// TODO: 会社名または組織名などの適切な文字列に
	// この文字列を変更してください。
	//SetRegistryKey(_T("アプリケーション ウィザードで生成されたローカル アプリケーション"));


	// ロケール設定
	::setlocale(LC_ALL, "");

	// =======================================================
	// ダイアログベースにウィンドウクラス名を設定
	WNDCLASS wc;
	// Get the info for this class.
	// #32770 is the default class name for dialogs boxes.
	::GetClassInfo(AfxGetInstanceHandle(), L"#32770", &wc);

	// Change the name of the class.
	wc.lpszClassName = L"PCRPlayer";

	// Register this class so that MFC can use it.
	AfxRegisterClass(&wc);	
	// =======================================================

	// DLL初期化
	utl::DWMLoader::Init();
	utl::CommandLine::init();
	d3dutl::D3DLoader::Init();
	utl::GdiplusInit gdipini;
	Mlang::Init();

	// カレントパス取得
	utl::getModuleFolder(gl_.current);

	CMainDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: ダイアログが <OK> で消された時のコードを
		//  記述してください。
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: ダイアログが <キャンセル> で消された時のコードを
		//  記述してください。
	}

	// 上で作成されたシェル マネージャーを削除します。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ダイアログは閉じられました。アプリケーションのメッセージ ポンプを開始しないで
	//  アプリケーションを終了するために FALSE を返してください。
	return FALSE;
}

