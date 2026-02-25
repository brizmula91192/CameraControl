

#include "pch.h"
#include "framework.h"

#include "QuadViewDoc.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "AppMsgs.h"
#include "UniView.h"

/* KIND OF ANNOYING WAY TO DO THIS*/
//#ifndef SHARED_HANDLERS
//#include "CameraControl.h"
//#endif

#include "CameraControl.h"

/* FFMPEG LIBRARIES */
extern "C" {
#   include <libavformat/avformat.h>
#   include <libavcodec/avcodec.h>
#   include <libavutil/imgutils.h>
#   include <libavutil/hwcontext.h>
#   include <libavutil/pixfmt.h>         // AVColorSpace, AVColorRange, AVPixelFormat
#   include <libswscale/swscale.h>
}

/* DEFINES FOR FFMPEG */
#ifndef SWS_CS_BT709
#   define SWS_CS_BT709 1
#endif
#ifndef SWS_CS_SMPTE170M
#   define SWS_CS_SMPTE170M 5
#endif

IMPLEMENT_DYNCREATE(UniView, CFormView)

BEGIN_MESSAGE_MAP(UniView, CFormView)


    // Standard printing commands
    /*
    ON_COMMAND(ID_FILE_PRINT, &CFormView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, &CFormView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &QuadView::OnFilePrintPreview)
    ON_WM_CONTEXTMENU()
    ON_WM_RBUTTONUP()
    ON_WM_SIZE()
    ON_COMMAND(ID_CONNECT_SINGLE_VIEW, &QuadView::OnConnectSingleView)
    ON_MESSAGE(WM_NEWFRAME_QUAD, &QuadView::OnNewFrame)
    ON_MESSAGE(WM_APP_BEGIN_VIDEO_SHUTDOWN, &QuadView::OnBeginVideoShutdown)
    ON_WM_DESTROY()
    ON_UPDATE_COMMAND_UI(ID_QUAD_TL_IP, &QuadView::OnUpdateQuadTlIp)
    ON_UPDATE_COMMAND_UI(ID_QUAD_TL_CHANNEL, &QuadView::OnUpdateQuadTlChannel)
    ON_UPDATE_COMMAND_UI(ID_QUAD_TL_RESOLUTION, &QuadView::OnUpdateQuadTlResolution)
    ON_UPDATE_COMMAND_UI(ID_QUAD_TR_IP, &QuadView::OnUpdateQuadTrIp)
    ON_UPDATE_COMMAND_UI(ID_QUAD_TR_CHANNEL, &QuadView::OnUpdateQuadTrChannel)
    ON_UPDATE_COMMAND_UI(ID_QUAD_TR_RESOLUTION, &QuadView::OnUpdateQuadTrResolution)
    ON_UPDATE_COMMAND_UI(ID_QUAD_BL_IP, &QuadView::OnUpdateQuadBlIp)
    ON_UPDATE_COMMAND_UI(ID_QUAD_BL_CHANNEL, &QuadView::OnUpdateQuadBlChannel)
    ON_UPDATE_COMMAND_UI(ID_QUAD_BL_RESOLUTION, &QuadView::OnUpdateQuadBlResolution)
    ON_UPDATE_COMMAND_UI(ID_QUAD_BR_IP, &QuadView::OnUpdateQuadBrIp)
    ON_UPDATE_COMMAND_UI(ID_QUAD_BR_CHANNEL, &QuadView::OnUpdateQuadBrChannel)
    ON_UPDATE_COMMAND_UI(ID_QUAD_BR_RESOLUTION, &QuadView::OnUpdateQuadBrResolution)
    ON_COMMAND(ID_QUAD_CONNECT_TL, &QuadView::OnQuadConnectTl)
    ON_COMMAND(ID_QUAD_CONNECT_BL, &QuadView::OnQuadConnectBl)
    ON_COMMAND(ID_QUAD_CONNECT_BR, &QuadView::OnQuadConnectBr)
    ON_UPDATE_COMMAND_UI(ID_QUADVIEW_NAME, &QuadView::OnUpdateQuadviewName)
    ON_COMMAND(ID_QUADVIEW_ADDNAME, &QuadView::OnQuadviewAddname)
    ON_COMMAND(ID_START_TL_VID, &QuadView::OnStartTlVid)
    ON_COMMAND(ID_FILL_WINDOW, &QuadView::OnFillWindow)
    ON_COMMAND(ID_RESTORE_VIEW, &QuadView::OnRestoreView)

    */
END_MESSAGE_MAP()


UniView::UniView() noexcept
    : CFormView(IDD_UNI_VIEWER)
{
}

UniView::~UniView() { /*NOTHING*/ }

void UniView::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
}


BOOL UniView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    return CFormView::PreCreateWindow(cs);
}

void UniView::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();
    ResizeParentToFit();

    // GET WINDOW HANDLE
    m_hwnd = GetSafeHwnd();

    // REMOVE THE SCROLLBAR
    ShowScrollBar(SB_BOTH, FALSE);

    // GET WINDOW TITLE FROM DOC
    this->GetDocument()->m_viewName = this->GetDocument()->GetTitle();
    m_wndName = this->GetDocument()->m_viewName;

    // GET HANDLE TO THE MAINFRAME
    m_mainFrame = (CMainFrame*)AfxGetMainWnd();

    // LAYOUT THE QUADRANTS! PROBABLY DO NOT NEED HERE
    LayoutQuadrants();

    // RESERVE BUFFERS FOR PIXELS FOR CPU
    for (int i = 0; i < NUM_STREAMS; ++i) {
        const int w = m_quadrant[i].Width();
        const int h = m_quadrant[i].Height();
        // If you want absolute worst-case: use 3840x2160; otherwise use current quadrant size.
        PreallocateRingBuffersForStream(i, 3840, 2160);
    }

    // NAMES FOR DEBUGGING
    m_stream[TL].name = _T("TopLeft");
    m_stream[TR].name = _T("TopRight");
    m_stream[BL].name = _T("BottomLeft");
    m_stream[BR].name = _T("BottomRight");

    // DISABLE WARNINGS
    av_log_set_level(AV_LOG_QUIET);
}

void UniView::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);

    // RECALCULATE SIZES
    if (cx > 0 && cy > 0) {
        LayoutQuadrants(); // recompute & push sizes
    }

    // REMOVE THE SCROLLBAR
    ShowScrollBar(SB_BOTH, FALSE);

    // RESIZE AXIS CONTROL

}


void UniView::LayoutQuadrants()
{
    CRect rc; GetClientRect(&rc);
    int midX = rc.left + rc.Width() / 2;
    int midY = rc.top + rc.Height() / 2;

    m_quadrant[TL] = CRect(rc.left, rc.top, midX, midY);
    m_quadrant[TR] = CRect(midX, rc.top, rc.right, midY);
    m_quadrant[BL] = CRect(rc.left, midY, midX, rc.bottom);
    m_quadrant[BR] = CRect(midX, midY, rc.right, rc.bottom);

    // Update per-stream targets atomically
    for (size_t i = 0; i < NUM_STREAMS; ++i) {
        int w = m_quadrant[i].Width();
        int h = m_quadrant[i].Height();
        m_stream[i].targetW.store(w, std::memory_order_relaxed);
        m_stream[i].targetH.store(h, std::memory_order_relaxed);
    }
}


