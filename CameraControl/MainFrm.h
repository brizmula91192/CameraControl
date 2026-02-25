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

// MainFrm.h : interface of the CMainFrame class
//

#pragma once

class CMyRibbonSlider : public CMFCRibbonSlider {
public:
	void SetWidth(int nWidth) { m_nWidth = nWidth; }
};



/*
CMAINFRAME CLASS FOR HANDLING ALL FRAME AND RIBBON ACTIONS.
*/
class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)

/* CONSTRUCTOR */
public:
	CMainFrame() noexcept;

/* PUBLIC MEMBERS */
public:
	BOOL m_toolBarState = TRUE;

private:
	BOOL  m_bFullScreen = FALSE;
	DWORD m_dwStyleSaved = 0;
	DWORD m_dwExStyleSaved = 0;
	WINDOWPLACEMENT m_wpPrev = { sizeof(WINDOWPLACEMENT) };
	afx_msg LRESULT OnMdiSetMenu(WPARAM wParam, LPARAM lParam);


/* --------------PUBLIC METHODS-------------- */
public:
	/* DESTRUCTOR */
	virtual ~CMainFrame();

	/* VARIABLE FOR THE RIBBON CONTROL */
	CMFCRibbonBar     m_wndRibbonBar;

	/* PRE CREATE WINDOW. OVERRIDE FROM CFORMVIEW CLASS. CALLED BEFORE ONCREATE */
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	/* Check if any windows are still open. If not, switch to home tab */
	void CheckNumWindows();

	/* FULLSCREEN ENABLE/DISABLE */
	void ToggleFullScreen();

	/* HELPER FUNCTION FOR HIDING TOOLBAR */
	void HideToolbar();
	void ShowToolbar();

	/* SWITCHING CATEGORIES IN THE RIBBON FIND BY NAME THEN SET AS ACTIVE */
	CMFCRibbonCategory* FindCategoryByName(LPCTSTR name) const;
	BOOL ActivateCategoryByName(LPCTSTR name);

	/* --------------AFX MESSAGES-------------- */
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct); // ON CREATE. CALLED WHEN THE WINDOW IS CREATED
	afx_msg void OnWindowManager(); // WINDOW MANAGER OPEN
	afx_msg void OnApplicationLook(UINT id); // THIS BUTTON HAS BEEN DELETED? WAS FOR SELECTING STYLE. CAN PROBABLY DELETE FUNCTION.
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI); // OPEN A SINGLE VIEW CAMERA
	/* ENABLE THE OPEN BUTTONS */
	afx_msg void OnUpdateOpenCamera(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOpenQuadView(CCmdUI* pCmdUI);
	/* ON COMMAND FOR OPEN BUTTONS */
	afx_msg void OnUpdateOpenSingleView(CCmdUI* pCmdUI);
	afx_msg void OnOpenSingleView();
	afx_msg void OnOpenQuad();
	/* CLOSE ALL BUTTON PRESSED */
	afx_msg void OnCloseAll();
	/* TILE BUTTON PRESSED */
	afx_msg void OnTile();
	/* CASCADE BUTTON PRESSED */
	afx_msg void OnCascade();
	/* FOR OPENING THE CONTEXT MENU */
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/* --------------PROTECTED-------------- */
protected:  // control bar embedded members
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages m_PanelImages;
	CMFCRibbonStatusBar  m_wndStatusBar;
	virtual void OnClose();
	CMFCRibbonSlider* m_playbackSlider;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnOpenPlayback();
	afx_msg void OnPtzOpen();
	afx_msg void OnHideRibbon();
	afx_msg void OnFullscreenView();
};


