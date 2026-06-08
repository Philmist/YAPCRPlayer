#pragma once


// CTestDlg ダイアログ

class CTestDlg : public CDialog
{
	DECLARE_DYNAMIC(CTestDlg)

public:
	CTestDlg(CWnd* pParent = NULL);   // 標準コンストラクター
	virtual ~CTestDlg();

// ダイアログ データ
	enum { IDD = IDD_TEST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

	DECLARE_MESSAGE_MAP()
};
