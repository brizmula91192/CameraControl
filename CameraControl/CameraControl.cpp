// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface
// (the "Fluent UI") and is provided only as referential material to supplement the
// Microsoft Foundation Classes Reference and related electronic documentation
// included with the MFC C++ library software.
// License terms to copy, use or distribute the Fluent UI are available separately.
// To learn more about our Fluent UI licensing program, please visit
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// CameraControl.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "CameraControl.h"
#include "MainFrm.h"


#include "ChildFrm.h"
#include "CameraControlDoc.h"
#include "CameraControlView.h"
#include "QuadViewDoc.h"
#include "QuadView.h"
#include "PTZDoc.h"
#include "PTZView.h"
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")


#include <Uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEBUG_CONSOLE

// CCameraControlApp

BEGIN_MESSAGE_MAP(CCameraControlApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CCameraControlApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CCameraControlApp construction

CCameraControlApp::CCameraControlApp() noexcept
{

	m_nAppLook = 0;
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("CameraControl.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CCameraControlApp object

CCameraControlApp theApp;


// CCameraControlApp initialization

BOOL CCameraControlApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// Remove all themes for classic look
	SetThemeAppProperties(0); // Pass 0 to disable all theming properties

	// AfxInitRichEdit2() is required to use RichEdit control
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	//SetRegistryKey(_T("Camera Control"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	m_singleView = new CMultiDocTemplate(IDR_CameraControlTYPE,
		RUNTIME_CLASS(CCameraControlDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CCameraControlView));
	if (!m_singleView)
		return FALSE;
	AddDocTemplate(m_singleView);
	m_singleView->m_hMenuShared = NULL;

	m_quadView = new CMultiDocTemplate(IDR_QUADVIEW_TYPE,
		RUNTIME_CLASS(QuadViewDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(QuadView));
	if (!m_quadView)
		return FALSE;
	AddDocTemplate(m_quadView);

	m_ptzView = new CMultiDocTemplate(IDR_PTZ_TYPE,
		RUNTIME_CLASS(PTZDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(PTZView));
	if (!m_ptzView)
		return FALSE;
	AddDocTemplate(m_ptzView);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;


	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Do not auto create first view
	cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;  // <— add this line
	if (!ProcessShellCommand(cmdInfo)) {
		return FALSE;
	}

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The main window has been initialized, so show and update it
	// The main window has been initialized, so show and update it
	// m_pMainWnd->SetWindowText(_T("Camera Control"));

	// START UP THE GDI!
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);

#ifdef DEBUG_CONSOLE
	this->CreateConsole();
	this->MakeConsoleCornersSquare();
	Utils::EnableVirtualTerminal();
#endif
	pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
	pMainFrame->UpdateWindow();

	

	return TRUE;
}

int CCameraControlApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	// Free GDI allocated memory
	Gdiplus::GdiplusShutdown(m_gdiplusToken);


	// Free the console
#ifdef DEBUG_CONSOLE
	FreeConsole();
#endif

	return CWinAppEx::ExitInstance();
}

// CCameraControlApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CCameraControlApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CCameraControlApp customization load/save methods

void CCameraControlApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CCameraControlApp::LoadCustomState()
{
}

void CCameraControlApp::SaveCustomState()
{
}

// Console
void CCameraControlApp::CreateConsole() {

	// 1) Try to attach to the parent console (e.g., if launched from cmd)
	if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
		// If no parent console, create a new one
		AllocConsole();
	}

	// 2) Reopen C file handles to the console
	//    Use "CONOUT$" for stdout/stderr and "CONIN$" for stdin

	// STDOUT
	FILE* fp_out;
	freopen_s(&fp_out, "CONOUT$", "w", stdout);

	// STDERR
	FILE* fp_err;
	freopen_s(&fp_err, "CONOUT$", "w", stderr);

	// STDIN (optional, if you need input)
	FILE* fp_in;
	freopen_s(&fp_in, "CONIN$", "r", stdin);

	// 3) Sync C++ iostreams with C stdio
	std::ios::sync_with_stdio(true);

	// Optional: clear error flags
	std::wcout.clear();
	std::cout.clear();
	std::wcerr.clear();
	std::cerr.clear();

	// Optional: make unbuffered for immediate flush behavior
	setvbuf(stdout, nullptr, _IONBF, 0);
	setvbuf(stderr, nullptr, _IONBF, 0);


	std::cout << "WELCOME TO THE PROGRAM" << std::endl;

}



void CCameraControlApp::MakeConsoleCornersSquare()
{
	HWND hwnd = GetConsoleWindow();
	if (!hwnd) return;

	// DWMWA_WINDOW_CORNER_PREFERENCE = 33 on recent SDKs
	const DWORD DWMWA_WINDOW_CORNER_PREFERENCE = 33;
	enum DWM_WINDOW_CORNER_PREFERENCE {
		DWMWCP_DEFAULT = 0,
		DWMWCP_DONOTROUND = 1,  // <- square corners
		DWMWCP_ROUND = 2,
		DWMWCP_ROUNDSMALL = 3
	};

	DWM_WINDOW_CORNER_PREFERENCE pref = DWMWCP_DONOTROUND;
	DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE,
		&pref, sizeof(pref));
}


std::vector<CString> CCameraControlApp::ListChildNames_ByViewClass()
{
	std::vector<CString> m_camViews;
	CRuntimeClass* pWantedViewClass = RUNTIME_CLASS(CCameraControlView);

	//CWinApp* pApp = AfxGetApp();
	//if (!pApp) return;

	// Iterate templates
	POSITION tplPos = this->GetFirstDocTemplatePosition();
	while (tplPos)
	{
		CDocTemplate* pTpl = this->GetNextDocTemplate(tplPos);
		POSITION docPos = pTpl->GetFirstDocPosition();

		// Iterate documents
		while (docPos)
		{
			CDocument* pDoc = pTpl->GetNextDoc(docPos);

			POSITION viewPos = pDoc->GetFirstViewPosition();
			// Iterate views in this doc
			while (viewPos)
			{
				CView* pView = pDoc->GetNextView(viewPos);
				if (!pView) continue;

				// Match the specific type
				if (pView->IsKindOf(pWantedViewClass))
				{
					// Preferred "name" = document title (drives child caption/tab/Window menu)
					CString name = pDoc->GetTitle();
					m_camViews.push_back(name);
				}
			}
		}
	}

	// return here
	return m_camViews;
}

/* FOR GETTING THE CMAINFRAME FROM CHILD VIEW */

// CCameraControlApp message handlers



