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

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "pch.h"
#include "framework.h"
#include "CameraControl.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	ON_UPDATE_COMMAND_UI(ID_OPEN_CAMERA, &CMainFrame::OnUpdateOpenCamera)
	ON_UPDATE_COMMAND_UI(ID_OPEN_QUAD_VIEW, &CMainFrame::OnUpdateOpenQuadView)
	ON_UPDATE_COMMAND_UI(ID_OPEN_SINGLE_VIEW, &CMainFrame::OnUpdateOpenSingleView)
	ON_COMMAND(ID_OPEN_SINGLE_VIEW, &CMainFrame::OnOpenSingleView)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_MDISETMENU, &CMainFrame::OnMdiSetMenu)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_CLOSE_ALL, &CMainFrame::OnCloseAll)
	ON_COMMAND(ID_TILE, &CMainFrame::OnTile)
	ON_COMMAND(ID_CASCADE, &CMainFrame::OnCascade)
	ON_COMMAND(ID_OPEN_QUAD, &CMainFrame::OnOpenQuad)
	ON_COMMAND(ID_OPEN_PLAYBACK, &CMainFrame::OnOpenPlayback)
	ON_COMMAND(ID_PTZ_OPEN, &CMainFrame::OnPtzOpen)
	ON_COMMAND(ID_HIDE_RIBBON, &CMainFrame::OnHideRibbon)
	ON_COMMAND(ID_FULLSCREEN_VIEW, &CMainFrame::OnFullscreenView)
END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame() noexcept : m_wndRibbonBar(FALSE)
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2003);
}

CMainFrame::~CMainFrame()
{
	
	

}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndRibbonBar.SetWindows7Look(TRUE);
	CString strTitlePane1;
	CString strTitlePane2;
	bNameValid = strTitlePane1.LoadString(IDS_STATUS_PANE1);
	ASSERT(bNameValid);
	bNameValid = strTitlePane2.LoadString(IDS_STATUS_PANE2);
	ASSERT(bNameValid);
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, strTitlePane1, TRUE), strTitlePane1);
	m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, strTitlePane2, TRUE), strTitlePane2);
	m_wndStatusBar.ShowWindow(SW_HIDE);
	m_wndRibbonBar.GetQuickAccessToolbar()->RemoveAll();
	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	// set the visual manager and style based on persisted value
	//OnApplicationLook(theApp.m_nAppLook);
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Enable enhanced windows management dialog
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);
	CMDITabInfo tabParams;
	EnableMDITabbedGroups(FALSE, tabParams);

	// GET THE SLIDER CONTROL IN PLAYBACK RIBBON CATEGORY AND RESIZE
	CMFCRibbonSlider* pSlider = DYNAMIC_DOWNCAST(CMFCRibbonSlider, m_wndRibbonBar.FindByID(ID_PLAYBACKSLIDER));
	if (pSlider != nullptr) {
		((CMyRibbonSlider*)pSlider)->SetWidth(750);
		m_wndRibbonBar.ForceRecalcLayout();
	}

	// DISABLE ALL MENU BARS
	// After the frame window is created:
	::SetMenu(m_hWnd, NULL);         // Detach the classic Win32 menu
	m_hMenuDefault = NULL;           // <- important for MDI frames
	::DrawMenuBar(m_hWnd);
	//if (m_wndMenuBar.GetSafeHwnd())   m_wndMenuBar.ShowPane(FALSE, FALSE, FALSE);

	// SET COMBO BOXES TO READ ONLY?
	//CMFCRibbonComboBox* pEdit = DYNAMIC_DOWNCAST(CMFCRibbonComboBox, this->m_wndRibbonBar.FindByID(ID_SINGLECAM_CHANNEL));



	// RETURN HERE!
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style |= WS_OVERLAPPEDWINDOW;

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(TRUE);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
	}

	RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}



void CMainFrame::OnUpdateOpenCamera(CCmdUI* pCmdUI)
{
	// Enable Button
	pCmdUI->Enable();
}


void CMainFrame::OnUpdateOpenQuadView(CCmdUI* pCmdUI)
{
	// Enable Button
	pCmdUI->Enable();
}


void CMainFrame::OnUpdateOpenSingleView(CCmdUI* pCmdUI)
{
	// Enable Button
	pCmdUI->Enable();
}


/* OPEN SINGLE CAMERA VIEW BUTTON */
void CMainFrame::OnOpenSingleView()
{
	// console output
	std::cout << "OPEN SINGLE VIEW BUTTON PRESSED" << std::endl;

	CCameraControlApp* pApp = (CCameraControlApp*)AfxGetApp();
	if (!pApp->m_singleView)
	{
		AfxMessageBox(_T("camView template is null."));
		return;
	}
	if (!pApp->m_singleView->OpenDocumentFile(nullptr))
	{
		AfxMessageBox(_T("Failed to open document/view template."));
	}
}













/* ON CLOSE METHOD OVERRIDE*/
void CMainFrame::OnClose()
{

	// Popup window to check
	if (AfxMessageBox(_T("Are you sure you want to close?"), MB_YESNO) == IDNO)
		return;

	// Free GDI allocated memory
	//GdiplusShutdown(m_gdiplusToken);

	// Call base class OnClose
	CMDIFrameWnd::OnClose();
}

/* CLOSE ALL WINDOWS */
void CMainFrame::OnCloseAll()
{
	// Check first!
	if (AfxMessageBox(_T("Are you sure you want to close all the windows?"), MB_YESNO) == IDNO)
		return;

	CMDIChildWnd* pChild = NULL;
	while ((pChild = MDIGetActive()) != NULL)
	{
		pChild->SendMessage(WM_CLOSE);
	}

	// Set ribbon control back to home!
	this->ActivateCategoryByName(_T("Home"));
	
}

/* TILE ALL WINDOWS */
void CMainFrame::OnTile()
{
	MDITile();
}

/* CASCADE ALL WINDOWS */
void CMainFrame::OnCascade()
{
	MDICascade();
}


void CMainFrame::OnOpenQuad()
{
	// console output
	std::cout << "OPEN QUAD VIEW BUTTON PRESSED" << std::endl;

	CCameraControlApp* pApp = (CCameraControlApp*)AfxGetApp();
	if (!pApp->m_quadView)
	{
		AfxMessageBox(_T("Quad View template is null."));
		return;
	}
	if (!pApp->m_quadView->OpenDocumentFile(nullptr))
	{
		AfxMessageBox(_T("Failed to open document/view template."));
	}
}

/* FOR SWITCHING CATEGORIES DEPENDING ON VIEW. FIND CATEGORY BY NAME. */
CMFCRibbonCategory* CMainFrame::FindCategoryByName(LPCTSTR name) const
{
	if (name == nullptr || *name == 0)
		return nullptr;

	const int count = m_wndRibbonBar.GetCategoryCount();
	for (int i = 0; i < count; ++i)
	{
		CMFCRibbonCategory* pCat = m_wndRibbonBar.GetCategory(i);
		if (!pCat) continue;

		// Compare the visible tab text (case-insensitive)
		if (pCat->GetName() != nullptr && _tcsicmp(pCat->GetName(), name) == 0)
			return pCat;
	}
	return nullptr;
}

/* FOR SWITCHING CATEGORIES DEPENDING ON VIEW. ACTIVATE THE CATEGORY FROM THE NAME */
BOOL CMainFrame::ActivateCategoryByName(LPCTSTR name)
{
	// CHECK IF RIBBON IS MINIMIZED AND EXIT IF IT IS
	if (m_wndRibbonBar.GetHideFlags() & AFX_RIBBONBAR_HIDE_ELEMENTS)
	{
		std::cout << "RIBBON MINIMIZED STATE! DO NOT SWITCH!" << std::endl;
		return FALSE;
	}

	// FIND CATEGORY
	if (CMFCRibbonCategory* pCat = FindCategoryByName(name))
	{
		m_wndRibbonBar.SetActiveCategory(pCat, TRUE); // switch to that tab
		return TRUE;
	}

	// DID NOT MATCH NAME
	return FALSE;
}
void CMainFrame::CheckNumWindows() {
	CMDIFrameWnd* pMDIFrame = (CMDIFrameWnd*)AfxGetMainWnd(); // Get a pointer to the main frame window
	if (pMDIFrame != nullptr)
	{
		// MDIGetActive returns the active CMDIChildWnd or NULL if no MDI child windows are open
		CMDIChildWnd* pChild = (CMDIChildWnd*)pMDIFrame->MDIGetActive();

		if (pChild == nullptr)
		{
			// NO MDI WINDOWS ARE OPEN
			std::cout << "NO CHILD WINDOWS. SWITCH CONTEXT TO HOME TAB" << std::endl;
			this->ActivateCategoryByName(_T("Home"));

		}
		else
		{
			// CHILD WINDOWS ARE OPEN
			std::cout << "NO CHILD WINDOWS. SWITCH CONTEXT TO HOME TAB" << std::endl;
			
		}
	}
}

/* OPEN PLAYBACK WINDOW */
void CMainFrame::OnOpenPlayback()
{
	// TODO: Add your command handler code here
}




void CMainFrame::OnPtzOpen()
{
	// console output
	std::cout << "OPEN PTZ VIEW BUTTON PRESSED" << std::endl;

	CCameraControlApp* pApp = (CCameraControlApp*)AfxGetApp();
	if (!pApp->m_ptzView)
	{
		AfxMessageBox(_T("PTZ View template is null."));
		return;
	}
	if (!pApp->m_ptzView->OpenDocumentFile(nullptr))
	{
		AfxMessageBox(_T("Failed to open document/view template."));
	}
}

/* HELPER FUNCTION FOR HIDING THE TOOLBAR. CALLED FROM VIEW CLASS */
void CMainFrame::HideToolbar()
{
	std::cout << "HIDING THE TOOLBAR" << std::endl;
	this->m_wndRibbonBar.ShowWindow(SW_HIDE);
	this->m_toolBarState = !this->m_toolBarState;
	RecalcLayout();

}
void CMainFrame::ShowToolbar()
{
	std::cout << "HIDING THE TOOLBAR" << std::endl;
	this->m_wndRibbonBar.ShowWindow(SW_SHOW);
	this->m_toolBarState = !this->m_toolBarState;
	RecalcLayout();

}
/* DISPLAY CONTEXT MENU */
void CMainFrame::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	/* DISPLAY THE CONTEXT MENU */
	CContextMenuManager* pManager = theApp.GetContextMenuManager();
	if (pManager) {
		theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
	}
	else {
		AfxMessageBox(_T("Context menu manager failed to start!\n\n This is not good. Trying to continue..."));
		std::cout << Utils::RED << "CMAINFRAME::ONCONTEXTMENU -> FAILED TO START CONTEXT MENU MANAGER!" << Utils::RESET << std::endl;
	}
}

/* EVENTS FOR HANDLING HIDING AND UNHIDING THE RIBBON */
void CMainFrame::OnHideRibbon()
{
	if (!m_toolBarState) {
		this->ShowToolbar();
	}
	else {
		this->HideToolbar();
	}
}

/* TOGGLE FULLSCREEN MODE */
void CMainFrame::ToggleFullScreen()
{
	if (!m_bFullScreen)
	{
		// --- ENTER FULLSCREEN ---

		// 1) Save placement and styles
		m_wpPrev.length = sizeof(WINDOWPLACEMENT);
		GetWindowPlacement(&m_wpPrev);

		m_dwStyleSaved = (DWORD)::GetWindowLongPtr(m_hWnd, GWL_STYLE);
		m_dwExStyleSaved = (DWORD)::GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);

		// 2) Optionally hide frame UI (menu/toolbar/status) – adjust to your app
		if (CWnd* pStatus = GetDlgItem(AFX_IDW_STATUS_BAR)) pStatus->ShowWindow(SW_HIDE);
		if (CWnd* pTool = GetDlgItem(AFX_IDW_TOOLBAR))    pTool->ShowWindow(SW_HIDE);

		// 3) Switch to borderless popup
		DWORD dwStyleNew = (m_dwStyleSaved & ~WS_OVERLAPPEDWINDOW) | WS_POPUP;
		::SetWindowLongPtr(m_hWnd, GWL_STYLE, dwStyleNew);

		// (optional) strip some extended styles for a truly borderless frame
		DWORD dwExNew = m_dwExStyleSaved & ~(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME);
		::SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, dwExNew);

		// 4) Get the monitor rect that contains our window (not just primary screen)
		HMONITOR hMon = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		::GetMonitorInfo(hMon, &mi);
		const RECT& r = mi.rcMonitor; // Full monitor area (covers taskbar)

		// 5) Apply placement + force non-client recomputation
		SetWindowPos(nullptr, r.left, r.top,
			r.right - r.left, r.bottom - r.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER |
			SWP_FRAMECHANGED | SWP_SHOWWINDOW);

		m_bFullScreen = TRUE;
	}
	else
	{
		// --- EXIT FULLSCREEN ---

		// 1) Restore styles exactly
		::SetWindowLongPtr(m_hWnd, GWL_STYLE, m_dwStyleSaved);
		::SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, m_dwExStyleSaved);

		// 2) Force non-client recomputation before restoring placement
		SetWindowPos(nullptr, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		// 3) Restore placement (position, size, show state)
		m_wpPrev.length = sizeof(WINDOWPLACEMENT);
		SetWindowPlacement(&m_wpPrev);

		// 4) Restore UI
		//if (CWnd* pStatus = GetDlgItem(AFX_IDW_STATUS_BAR)) pStatus->ShowWindow(SW_SHOWNOACTIVATE);
		//if (CWnd* pTool = GetDlgItem(AFX_IDW_TOOLBAR))    pTool->ShowWindow(SW_SHOWNOACTIVATE);

		m_bFullScreen = FALSE;
	}

	// Clean repaint
	RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
}
void CMainFrame::OnFullscreenView()
{
	//m_bFullScreen = !m_bFullScreen;
	ToggleFullScreen();
}



LRESULT CMainFrame::OnMdiSetMenu(WPARAM, LPARAM)
{
	// Keep the frame menu-less no matter what child is active
	::SetMenu(m_hWnd, NULL);
	::DrawMenuBar(m_hWnd);
	return 0; // tell MDI we handled it; no merging
}
