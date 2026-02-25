#pragma once

#include "PTZDoc.h"
#include <vector>


// PTZView form view

class PTZView : public CFormView
{
public:
	PTZView() noexcept;
	DECLARE_DYNCREATE(PTZView)

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PTZ_CTRL };
#endif

	// Attributes
public:
	PTZDoc* GetDocument() { return (PTZDoc*)m_pDocument; }

	// Operations
public:
	//afx_msg void OnSize(UINT nType, int cx, int cy);

	// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	afx_msg void OnDestroy();
	void ListChildNames_ByViewClass(CRuntimeClass* pWantedViewClass);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) override;
	void DrawButtons();

	// Implementation
public:
	virtual ~PTZView();
#ifdef _DEBUG
	//virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	// for keeping the bold font
	CFont m_boldFont;
	// tilt down button
	CMFCButton m_tiltDown;
	// button for panning left
	CMFCButton m_panLeft;
	// button for panning right
	CMFCButton m_panRight;
	// button for tilting up
	CMFCButton m_tiltUP;
	// button for zooming in
	CMFCButton m_zoomIN;
	// button for zooming out
	CMFCButton m_zoomOut;
	// button for focusing near
	CMFCButton m_focusNear;
	// button for far focus
	CMFCButton m_focusFar;
	// button for auto focus
	CMFCButton m_focusAuto;
	// button for iris open
	CMFCButton m_irisOpen;
	// button for iris close
	CMFCButton m_irisClose;
	// button for auto iris
	CMFCButton m_irisAuto;

	/* FUNCTION FOR SETTING THE COLOR OF THE PTZ CAMERAS */
	CMFCButton* m_buttons[12] = { &m_tiltDown, &m_tiltUP, &m_panLeft, &m_panRight, &m_zoomIN, &m_zoomOut, &m_focusNear, &m_focusFar,
	&m_focusAuto, &m_irisOpen, &m_irisClose, &m_irisAuto };

	std::vector<CString> m_camViews;
	CString m_ipAddress;
	CString m_videoChannel = _T("1");
	CString m_userName = _T("root");
	CString m_passName = _T("pass");

	afx_msg void OnSelchangeConnetDevice();
	afx_msg void OnDropdownConnetDevice();
	// drop down box for displaying the active cam views
	CComboBox m_viewDropdown;
	afx_msg void OnUpdateContext2(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};



