
/* INCLUDES */
#include "pch.h"
#include "PTZView.h"
#include "CameraControl.h"

// PTZView.cpp : implementation file
//



// PTZView

IMPLEMENT_DYNCREATE(PTZView, CFormView)

BEGIN_MESSAGE_MAP(PTZView, CFormView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CFormView::OnFilePrintPreview)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_CONNET_DEVICE, &PTZView::OnSelchangeConnetDevice)
	ON_CBN_DROPDOWN(IDC_CONNET_DEVICE, &PTZView::OnDropdownConnetDevice)
	ON_UPDATE_COMMAND_UI(ID_CONTEXT2, &PTZView::OnUpdateContext2)
	ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

PTZView::PTZView() noexcept
	: CFormView(IDD_PTZ_CTRL)
{

}

PTZView::~PTZView()
{
}

void PTZView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONNET_DEVICE, m_viewDropdown);
	DDX_Control(pDX, IDC_PTZ_CTRL_TILTDWN, m_tiltDown);
	DDX_Control(pDX, IDC_PTZ_CTRL_PANLEFT, m_panLeft);
	DDX_Control(pDX, IDC_PTZ_CTRL_PANRIGHT, m_panRight);
	DDX_Control(pDX, IDC_PTZ_CTRL_TILTUP, m_tiltUP);
	DDX_Control(pDX, IDC_PTZ_CTRL_ZOOMIN, m_zoomIN);
	DDX_Control(pDX, IDC_PTZ_CTRL_ZOOMOUT, m_zoomOut);
	DDX_Control(pDX, IDC_PTZ_CTRL_FOCUSNEAR, m_focusNear);
	DDX_Control(pDX, IDC_PTZ_CTRL_FOCUSFAR, m_focusFar);
	DDX_Control(pDX, IDC_PTZ_CTRL_FOCUSAUTO, m_focusAuto);
	DDX_Control(pDX, IDC_PTZ_CTRL_IRISOPEN, m_irisOpen);
	DDX_Control(pDX, IDC_PTZ_CTRL_IRISCLOSE, m_irisClose);
	DDX_Control(pDX, IDC_PTZ_CTRL_IRISAUTO, m_irisAuto);
}

BOOL PTZView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void PTZView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	ResizeParentToFit();

	// DRAW THE BUTTONS
	this->DrawButtons();

	// REMOVE THE SCROLLBAR
	ShowScrollBar(SB_BOTH, FALSE);

	// SET INITIAL SIZE
	CFrameWnd* pParentFrame = GetParentFrame();
	if (pParentFrame != nullptr)
	{
		pParentFrame->MoveWindow(0, 0, 285, 675, TRUE);
		pParentFrame->RecalcLayout();
	}
}

void PTZView::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);

	// REMOVE THE SCROLLBAR
	ShowScrollBar(SB_BOTH, FALSE);
}


// Custom painting and drawing
void PTZView::DrawButtons() {

	// Get basic log font!
	CFont* pBase = m_tiltDown.GetSafeHwnd() ? m_tiltDown.GetFont() : nullptr;
	if (pBase == nullptr) pBase = GetFont();

	LOGFONT lf{};
	if (pBase)
		pBase->GetLogFont(&lf);
	else
		::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0);

	lf.lfWeight = FW_BOLD;          // make it bold (or adjust lf for other styles)
	lf.lfQuality = CLEARTYPE_QUALITY; // optional: nicer text

	if (m_boldFont.GetSafeHandle())
		m_boldFont.DeleteObject();

	VERIFY(m_boldFont.CreateFontIndirect(&lf));


	// iterate through the buttons!
	for (int i = 0; i < (sizeof(m_buttons) / sizeof(m_buttons[0])); ++i) {


		// Sanity checks
		CMFCButton* pBtn = m_buttons[i];
		if (!pBtn) continue;

		if (!::IsWindow(pBtn->GetSafeHwnd()))
			continue;                      // button not created yet

		// disable themeing
		m_buttons[i]->EnableWindowsTheming(FALSE);

		// Set colors
		m_buttons[i]->SetFaceColor(RGB(30, 88, 168)); // Set background to navy blue
		m_buttons[i]->SetTextColor(RGB(255, 255, 255)); // Set text to White

		// Set the font
		m_buttons[i]->SetFont(&m_boldFont, TRUE);

	}

	// no flicker redraw
	RedrawWindow(nullptr, nullptr,
		RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_NOERASE);

	/*
	m_tiltDown.EnableWindowsTheming(FALSE); // Important: disable theming
	m_tiltUP.EnableWindowsTheming(FALSE); // Important: disable theming
	m_panLeft.EnableWindowsTheming(FALSE);
	m_panRight.EnableWindowsTheming(FALSE);


	m_tiltDown.SetFaceColor(RGB(30, 88, 168)); // Set background to navy blue
	m_tiltDown.SetTextColor(RGB(255, 255, 255)); // Set text to White

	m_tiltUP.SetFaceColor(RGB(30, 88, 168)); // Set background to navy blue
	m_tiltUP.SetTextColor(RGB(255, 255, 255)); // Set text to White

	m_panRight.SetFaceColor(RGB(30, 88, 168)); // Set background to navy blue
	m_panRight.SetTextColor(RGB(255, 255, 255)); // Set text to White

	m_panLeft.SetFaceColor(RGB(30, 88, 168)); // Set background to navy blue
	m_panLeft.SetTextColor(RGB(255, 255, 255)); // Set text to White


	// Setup fonts for buttons here
	CFont* pBase = m_tiltDown.GetSafeHwnd() ? m_tiltDown.GetFont() : nullptr;
	if (pBase == nullptr) pBase = GetFont();

	LOGFONT lf{};
	if (pBase)
		pBase->GetLogFont(&lf);
	else
		::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0);

	lf.lfWeight = FW_BOLD;          // make it bold (or adjust lf for other styles)
	lf.lfQuality = CLEARTYPE_QUALITY; // optional: nicer text

	if (m_boldFont.GetSafeHandle())
		m_boldFont.DeleteObject();

	VERIFY(m_boldFont.CreateFontIndirect(&lf));

	// Apply to all your buttons
	m_tiltDown.SetFont(&m_boldFont, TRUE);
	m_tiltUP.SetFont(&m_boldFont, TRUE);
	m_panLeft.SetFont(&m_boldFont, TRUE);
	m_panRight.SetFont(&m_boldFont, TRUE);

	*/

}

// PTZView diagnostics


// CAxisMFCView printing

BOOL PTZView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void PTZView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void PTZView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void PTZView::OnPrint(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	// TODO: add customized printing code here
}
#ifdef _DEBUG
#ifndef _WIN32_WCE
void PTZView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG


// PTZView message handlers



void PTZView::OnDestroy()
{
	CFormView::OnDestroy();
	if (m_boldFont.GetSafeHandle())
		m_boldFont.DeleteObject();
}



void PTZView::OnSelchangeConnetDevice()
{
	// TODO: Add your control notification handler code here
	CString strSelectedText;
	m_viewDropdown.GetWindowText(strSelectedText);

}


void PTZView::OnDropdownConnetDevice()
{
	// TODO: Add your control notification handler code here

	// Clear the combo box
	m_viewDropdown.ResetContent();

	// Get the app
	CCameraControlApp* pApp = static_cast<CCameraControlApp*>(AfxGetApp());
	if (!pApp) return;

	// Get list of names
	m_camViews = pApp->ListChildNames_ByViewClass();

	for (int i = 0; i < m_camViews.size(); ++i) {
		m_viewDropdown.AddString(m_camViews[i]);
	}

}


void PTZView::ListChildNames_ByViewClass(CRuntimeClass* pWantedViewClass)
{
	std::vector<CString> m_camViews;

	CWinApp* pApp = AfxGetApp();
	if (!pApp) return;

	// Iterate templates
	POSITION tplPos = pApp->GetFirstDocTemplatePosition();
	while (tplPos)
	{
		CDocTemplate* pTpl = pApp->GetNextDocTemplate(tplPos);
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
}


void PTZView::OnUpdateContext2(CCmdUI* pCmdUI)
{
	pCmdUI->Enable();
}


void PTZView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);

}
