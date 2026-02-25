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

// CameraControlView.cpp : implementation of the CCameraControlView class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "CameraControl.h"
#endif

#include "CameraControlDoc.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "AppMsgs.h"
#include "CameraControlView.h"

extern "C" {
#   include <libavformat/avformat.h>
#   include <libavcodec/avcodec.h>
#   include <libavutil/imgutils.h>
#   include <libavutil/pixfmt.h>         // AVColorSpace, AVColorRange, AVPixelFormat
#   include <libswscale/swscale.h>
}



#ifndef SWS_CS_BT709
#   define SWS_CS_BT709 1
#endif
#ifndef SWS_CS_SMPTE170M
#   define SWS_CS_SMPTE170M 5
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCameraControlView

IMPLEMENT_DYNCREATE(CCameraControlView, CFormView)

BEGIN_MESSAGE_MAP(CCameraControlView, CFormView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CFormView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CCameraControlView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
    ON_WM_SIZE()
    ON_UPDATE_COMMAND_UI(ID_SINGLECAM_IP, &CCameraControlView::OnUpdateSinglecamIp)
    ON_UPDATE_COMMAND_UI(ID_SINGLECAM_CHANNEL, &CCameraControlView::OnUpdateSinglecamChannel)
    ON_COMMAND(ID_CONNECT_SINGLE_VIEW, &CCameraControlView::OnConnectSingleView)
    ON_MESSAGE(WM_NEWFRAME, &CCameraControlView::OnNewFrame)
    ON_UPDATE_COMMAND_UI(ID_SINGLECAM_AUDIO_IP, &CCameraControlView::OnUpdateSinglecamAudioIp)
    ON_UPDATE_COMMAND_UI(ID_LEFT_OVERLAY, &CCameraControlView::OnUpdateLeftOverlay)
    ON_UPDATE_COMMAND_UI(ID_RIGHT_OVERLAY, &CCameraControlView::OnUpdateRightOverlay)
    ON_UPDATE_COMMAND_UI(ID_SINGLECAM_NAME, &CCameraControlView::OnUpdateSinglecamName)
    ON_UPDATE_COMMAND_UI(ID_SINGLECAMAUDIO_IP, &CCameraControlView::OnUpdateSinglecamaudioIp)
    ON_MESSAGE(WM_APP_BEGIN_VIDEO_SHUTDOWN, &CCameraControlView::OnBeginVideoShutdown)
    ON_WM_DESTROY()
    ON_COMMAND(ID_ADD_SINGLEVIEW_NAME, &CCameraControlView::OnAddSingleviewName)
    ON_UPDATE_COMMAND_UI(ID_SINGLEVIEW_RESOLUTION, &CCameraControlView::OnUpdateSingleviewResolution)
    ON_COMMAND(ID_PAUSE_RECORDING, &CCameraControlView::OnPauseRecording)
    ON_COMMAND(ID_STOP_RECORDING, &CCameraControlView::OnStopRecording)
    ON_COMMAND(ID_SNAP_SHOT, &CCameraControlView::OnSnapShot)
    ON_UPDATE_COMMAND_UI(ID_SINGLEVIEW_COMPRESSION, &CCameraControlView::OnUpdateSingleviewCompression)
    ON_COMMAND(ID_FULLSCREEN, &CCameraControlView::OnFullscreen)
    ON_WM_LBUTTONDOWN()
    ON_WM_ERASEBKGND()
    ON_COMMAND(ID_SINGLEVIEW_RM_OVERLAYS, &CCameraControlView::OnSingleviewRmOverlays)
    ON_COMMAND(ID_SINGLEVIEW_ADD_R_OVERLAY, &CCameraControlView::OnSingleviewAddROverlay)
    ON_COMMAND(ID_SINGLEVIEW_ADD_L_OVERLAY, &CCameraControlView::OnSingleviewAddLOverlay)
    ON_UPDATE_COMMAND_UI(ID_SINGLEVIEW_PTZ_ONOFF, &CCameraControlView::OnUpdateSingleviewPtzOnoff)
    ON_COMMAND(ID_SINGLEVIEW_PTZ_ONOFF, &CCameraControlView::OnSingleviewPtzOnoff)
    ON_COMMAND(ID_RESTORE_VIEW, &CCameraControlView::OnRestoreView)
    ON_COMMAND(ID_FILL_WINDOW, &CCameraControlView::OnFillWindow)
    ON_COMMAND(ID_HIDE_RIBBON, &CCameraControlView::OnHideRibbon)
    ON_COMMAND(ID_RECORD_VID_AUDIO, &CCameraControlView::OnRecordVidAudio)
    ON_COMMAND(ID_RECORD_VIDEO, &CCameraControlView::OnRecordVideo)
    ON_UPDATE_COMMAND_UI(ID_RECORD_VIDEO, &CCameraControlView::OnUpdateRecordVideo)
    ON_UPDATE_COMMAND_UI(ID_RECORD_VID_AUDIO, &CCameraControlView::OnUpdateRecordVidAudio)
    ON_UPDATE_COMMAND_UI(ID_PAUSE_RECORDING, &CCameraControlView::OnUpdatePauseRecording)
    ON_UPDATE_COMMAND_UI(ID_STOP_RECORDING, &CCameraControlView::OnUpdateStopRecording)
    ON_UPDATE_COMMAND_UI(ID_SINGLEVIEW_RESUME, &CCameraControlView::OnUpdateSingleviewResume)
    ON_COMMAND(ID_SINGLEVIEW_RESUME, &CCameraControlView::OnSingleviewResume)
END_MESSAGE_MAP()

// CCameraControlView construction/destruction

CCameraControlView::CCameraControlView() noexcept
	: CFormView(IDD_CAMERACONTROL_FORM)
{
	// TODO: add construction code here

}

CCameraControlView::~CCameraControlView()
{
    // RESET SMART POINTERS TO GDIPLUS OBJECTS
    m_pRecFont.reset();
    m_pRedBrush.reset();
    m_pWhiteBrush.reset();
    ASSERT(!m_thr.joinable());
}

void CCameraControlView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
}

BOOL CCameraControlView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CCameraControlView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();
	ResizeParentToFit();

    std::cout << "INITIALIZING SINGLE VIEW" << std::endl;

    // GET WINDOW HANDLE
    m_hwnd = GetSafeHwnd();

    // REMOVE THE SCROLLBAR
    ShowScrollBar(SB_BOTH, FALSE);

    // GET WINDOW TITLE FROM DOC
    this->GetDocument()->m_viewName = this->GetDocument()->GetTitle();
    this->m_wndName = this->GetDocument()->m_viewName;

    // GET HANDLE TO THE MAINFRAME
    m_mainFrame = (CMainFrame*)AfxGetMainWnd();

    // PRE-ALLOCATE PIXEL BUFFER FOR EFFICIENCY AND PREVENT RE-ALLOCATION IN VIDEO THREAD
    this->PreAllocateRingBuffer();

    // DISABLE WARNINGS
    av_log_set_level(AV_LOG_QUIET);

    // INITIALIZE GDI TOOLS
    InitGdiResources();

}

void CCameraControlView ::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);

    // TRACK WINDOW SIZE
    if (cx > 0 && cy > 0) {
        // Atomic update so WorkerProc sees it immediately
        m_targetW.store(cx, std::memory_order_relaxed);
        m_targetH.store(cy, std::memory_order_relaxed);
    }


    // REMOVE THE SCROLLBAR
    ShowScrollBar(SB_BOTH, FALSE);
}


// CCameraControlView printing


void CCameraControlView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CCameraControlView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

BOOL CCameraControlView::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}

void CCameraControlView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CCameraControlView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CCameraControlView::OnPrint(CDC* pDC, CPrintInfo* /*pInfo*/)
{
	// TODO: add customized printing code here
}

void CCameraControlView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CCameraControlView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CCameraControlView diagnostics

#ifdef _DEBUG
void CCameraControlView::AssertValid() const
{
	CFormView::AssertValid();
}

void CCameraControlView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CCameraControlDoc* CCameraControlView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCameraControlDoc)));
	return (CCameraControlDoc*)m_pDocument;
}
#endif //_DEBUG

void CCameraControlView::OnDestroy() {

    // Debug console
    std::cout << "DESTROYING SINGLE VIEW VIDEO THREAD NOW!" << std::endl;

    // SET HARDWARE WINDOW CONTEXT TO NULL
    HWND old = m_hwnd;
    m_hwnd = NULL;

    // END THE VIDEO THREAD AND ASSERT THREAD IS JOINED
    Stop();
    ASSERT(!m_thr.joinable());

    // SWITCH CONTEXT
    if (auto* pMF = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd())) {
        pMF->CheckNumWindows();
    }

    // CALL BASE CLASS ON DESTROY FOR CFORMVIEW
    CFormView::OnDestroy();
}


/* ON NEW FRAME */
LRESULT CCameraControlView::OnNewFrame(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    if (!m_run) return 0;
    Invalidate(FALSE); // schedule paint
    return 0;
}

/* PRE ALLOCATE PIXEL BUFFER HERE TO PREVENT REALLOCATION IN VIDEO THREAD*/ 
void CCameraControlView::InitBuffer() {
    // Example: Pre - allocating for 4K(3840 x 2160) at 32bpp
    const size_t maxW = 3840;
    const size_t maxH = 2160;
    const size_t bytesPerPixel = 4; // BGRA

    // reserve() allocates the memory but keeps size() at 0.
    // This prevents the heap manager from re-allocating during the loop.
    m_bgra.reserve(maxW * maxH * bytesPerPixel);
}



/* START THE VIDEO */
void CCameraControlView::StartVideo(const CString& ip, const CString& user, const CString& pass, const CString& chan, const CString& res,
    const CString& decoder_opts, const CString& frameRate) {
    Stop(); // if already running

    CString url;
    // Axis RTSP for channel 4 + H.264 (you can omit videocodec if default is OK) 1920x1080  640x480 &videocodec=h264&resolution=1920x1080
    url.Format(_T("rtsp://%s:%s@%s/axis-media/media.amp?camera=%s&fps=60"), user, pass, ip, chan);


    // Setup the thread
    std::cout << "BEGINING VIDEO THREAD RIGHT NOW!" << std::endl;
    m_run = true;
    HWND hwnd = m_hwnd;

    // Start thread
    m_thr = std::thread([this, url, hwnd] {WorkerProc(url); }); 

}


void CCameraControlView::BeginVideoShutdown()
{
    
}


/* PREALLOCATE ALL THE RING BUFFERS TO IMPROVE PERFORMANCE */
void CCameraControlView::PreAllocateRingBuffer()
{
    // RESERVE ENOUGH SPACE FOR 4K VIDEO FOR FUTURE
    const size_t maxW = 3840;
    const size_t maxH = 2160;
    const size_t bytesPerPixel = 4; // BGRA / BGR0
    const size_t bufferSize = maxW * maxH * bytesPerPixel;

    for (int i = 0; i < RING_BUFFER_SIZE; ++i)
    {
        // reserve() allocates the block of memory on the heap
        // but keeps the vector's size() at 0.
        m_ring[i].pixels.reserve(bufferSize);

        // Optional: Initialize metadata to prevent garbage values
        m_ring[i].w = 0;
        m_ring[i].h = 0;
        m_ring[i].stride = 0;
        ZeroMemory(&m_ring[i].bmi, sizeof(BITMAPINFO));
    }

    // Reset indexes
    m_writeIdx.store(0);
    m_readIdx.store(0);
}


/* BEGIN SHUTTING DOWN THE THREAD BEFORE ONDESTROY. CALLED FROM CHILDFRAME CLASS ONCLOSE */
LRESULT CCameraControlView::OnBeginVideoShutdown(WPARAM, LPARAM)
{
    // Tell the worker loop to stop as soon as possible
    m_run.store(false, std::memory_order_release);
    return 0;
}



/* STOP THE THREAD */
void CCameraControlView::Stop() {

    // End Video Thread
    m_run.store(false, std::memory_order_release);
    m_run = false;
	if (m_thr.joinable()) m_thr.join();
    
	// No need to hold the frame mutex for FFmpeg contexts, but free safely:
	if (m_sws) { sws_freeContext(m_sws); m_sws = nullptr; }
	if (m_swsView) { sws_freeContext(m_swsView); m_swsView = nullptr; }
	if (m_dec) { avcodec_free_context(&m_dec); m_dec = nullptr; }
	if (m_fmt) { avformat_close_input(&m_fmt); m_fmt = nullptr; }

	// Clear buffers under lock
	{
		std::lock_guard<std::mutex> lk(m_frameMtx);
		m_bgra.clear(); m_bgra.shrink_to_fit();
		m_viewBgra.clear(); m_viewBgra.shrink_to_fit();
		m_w = m_h = m_bgraStride = 0;
		m_viewW = m_viewH = m_viewStride = 0;
		ZeroMemory(&m_bmi, sizeof(m_bmi));
		ZeroMemory(&m_viewBmi, sizeof(m_viewBmi));
	}
}



/* ON DRAW METHOD CALLED WHEN NEW FRAME IS POSTED */
void CCameraControlView::OnDraw(CDC* pDC)
{
    // 1) Client area
    CRect rc;
    GetClientRect(&rc);

    // 2) If thread not running, clear and exit
    if (!m_run.load(std::memory_order_relaxed)) {
        pDC->FillSolidRect(rc, RGB(0, 0, 0));
        // Clear gate so next start can post again
        m_needRepaint.store(false, std::memory_order_release);
        return;
    }

    // 3) Propagate current view size to worker (for scaling)
    const int cw = rc.Width();
    const int ch = rc.Height();
    m_targetW.store(cw, std::memory_order_relaxed);
    m_targetH.store(ch, std::memory_order_relaxed);

    if (cw <= 0 || ch <= 0) {
        // Nothing to draw (minimized)
        m_needRepaint.store(false, std::memory_order_release);
        return;
    }

    // 4) Get latest published frame index with acquire fence
    size_t wIdx = m_writeIdx.load(std::memory_order_acquire);
    if (wIdx == 0) {
        // No frames yet
        m_needRepaint.store(false, std::memory_order_release);
        return;
    }

    // 5) Choose the newest frame (latest-wins)
    const size_t idxToDraw = wIdx - 1;

    // 6) Coalesce redundant paints (skip if we already drew it)
    if (m_lastDrawn == idxToDraw) {
        // We already presented this frame; clear gate and exit
        m_needRepaint.store(false, std::memory_order_release);
        return;
    }

    // 7) Fetch slot
    VideoFrame& slot = m_ring[idxToDraw % RING_BUFFER_SIZE];
    if (slot.w <= 0 || slot.h <= 0 || slot.pixels.empty()) {
        m_needRepaint.store(false, std::memory_order_release);
        return;
    }

    // 8) Blit (stretch to window).
    //    If you want aspect-preserve, compute a letterbox rect and blit into that instead.
    ::StretchDIBits(
        pDC->GetSafeHdc(),
        0, 0, cw, ch,           // destination rectangle
        0, 0, slot.w, slot.h,   // source rectangle
        slot.pixels.data(), &slot.bmi,
        DIB_RGB_COLORS, SRCCOPY
    );

    // 9) Mark as drawn and open the gate for the next post
    m_lastDrawn = idxToDraw;
    m_needRepaint.store(false, std::memory_order_release);

    // NOTE:
    // - We do NOT touch m_readIdx here; latest-wins does not need it.
    // - m_lastDrawn is per-view and only accessed by the UI thread.
}


/*
void CCameraControlView::OnDraw(CDC* pDC) {

    // GET SIZE
    CRect rc;
    GetClientRect(&rc);

    // CHECK IF THREAD ACTIVE
    if (!m_run) {
        pDC->FillSolidRect(rc, RGB(0,0,0));
    }
    
    // UPDATE SIZE
    m_targetW.store(rc.Width(), std::memory_order_relaxed);
    m_targetH.store(rc.Height(), std::memory_order_relaxed);

    // IF SCREEN IS MINIMIZED DO NO WORK AND EXIT
    if (m_targetW <= 0 || m_targetH <= 0) {
        return;
    }

    size_t wIdx = m_writeIdx.load(std::memory_order_acquire);
    if (wIdx == 0) return; // No frames yet

    // CHECK IF ALREADY DRAWN
    size_t idxToDraw = wIdx - 1;
    if (m_lastDrawn.load(std::memory_order_relaxed) == idxToDraw)
        std::cout << "ALREADY DRAWN FRAME SKIP" << std::endl;
        return; // already drew this one


    // Get the absolute latest frame
    VideoFrame& slot = m_ring[(wIdx - 1) % RING_BUFFER_SIZE];

    // Blit directly from the ring buffer slot
    ::StretchDIBits(pDC->GetSafeHdc(),
        0, 0, rc.Width(), rc.Height(),
        0, 0, slot.w, slot.h,
        slot.pixels.data(), &slot.bmi, DIB_RGB_COLORS, SRCCOPY);

    // Update read index
    // m_readIdx.store(wIdx, std::memory_order_release);
    m_lastDrawn.store(idxToDraw, std::memory_order_relaxed);

}
*/


/* HELPER METHOD FOR CONVERTING ENCODING */
static std::string ToUtf8(const CString& s) {
    CStringA a(CW2A(s, CP_UTF8));
    return std::string(a);
}

/* AV FFPMEG CALL BACK FUNCTION FOR INTERRUPT */
int CCameraControlView::AvInterrupt(void* opaque)
{
    CCameraControlView* self = static_cast<CCameraControlView*>(opaque);
    // return non-zero to interrupt
    return self && !self->m_run.load(std::memory_order_acquire);
}


/* THIS IS THE MAIN VIDEO WORKER THREAD LOOP */
void CCameraControlView::WorkerProc(CString curl)
{
    // LOCALS
    char err_buffer[AV_ERROR_MAX_STRING_SIZE];
    int ret = 0;

    // REFORMAT THE URL TO UTF8 ENCODING FOR FFMPEG LIBRARY
    std::string url = ToUtf8(curl);

    // SETUP THE CALLBACK FUNCTION AND ALLOCATE M_FMT
    std::cout << "FFMPEG: ALLOCATE FORMAT CONTEXT" << std::endl;
    m_fmt = avformat_alloc_context();
    if (!m_fmt) {
        m_run = false;
        std::cout << Utils::RED << "FFMPEG: FAILED TO ALLOCATE MEMORY FOR FORMAT. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        return;
    }

    std::cout << "FFMPEG: SETTING UP INTERRUPTS" << std::endl;
    m_fmt->interrupt_callback.callback = &CCameraControlView::AvInterrupt;
    m_fmt->interrupt_callback.opaque = this;


    // CREATE OPTIONS 
    std::cout << "FFMPEG: CREATE OPTIONS FOR AV: RTSP/TCP/TIMEOUT=5 SECONDS" << std::endl;
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "stimeout", "2000000", 0); // 5s (microseconds)
    // Optional lower latency (uncomment if desired):
    // av_dict_set(&opts, "fflags", "nobuffer", 0);
    // av_dict_set(&opts, "max_delay", "0", 0);


    // OPENS THE VIDEO STREAM! GET FORMAT FROM SOURCE!
    std::cout << "FFMPEG: OPEN THE INPUT NOW!" << std::endl;
    ret = avformat_open_input(&m_fmt, url.c_str(), nullptr, &opts);
    if (ret < 0) {
        av_dict_free(&opts);
        m_run = false;
        av_make_error_string(err_buffer, AV_ERROR_MAX_STRING_SIZE, ret);
        std::cout << Utils::RED << "FFMPEG: COULD NOT OPEN STREAM FOR " << m_wndName << Utils::RESET << std::endl;
        std::cout << Utils::RED << "FFMPEG: ERROR CODE: " << ret << ": " << err_buffer << Utils::RESET << std::endl;
        std::cout << Utils::RED << "FFMPEG: ENDING VIDEO THREAD FOR: " << m_wndName << Utils::RESET << std::endl;
        memset(err_buffer, 0, sizeof(err_buffer));
        CString msg = _T("Could not open RTSP stream over TCP for: ");
        msg += m_wndName + _T("\n\n Check physical network, IP Address, channel #, and credentials!\n\nNote: Ensure user and password on axis device is user=root password=pass. Use web browser.");
        AfxMessageBox(msg);
        return;
    }
    av_dict_free(&opts);


    // GET THE STREAM INFORMATION
    std::cout << "FFMPEG: FIND THE STREAM INFO FROM CAMERA/ENCODER" << std::endl;
    ret = avformat_find_stream_info(m_fmt, nullptr);
    if (ret < 0) {
        av_make_error_string(err_buffer, AV_ERROR_MAX_STRING_SIZE, ret);
        std::cout << Utils::RED << "FFMPEG: ERROR CODE: " << ret << ": " << err_buffer << Utils::RESET << std::endl;
        std::cout << Utils::RED << "FFMPEG: FAILED TO FIND STREAM INFO. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        m_run = false;
        return;
    }

    // FIND THE BEST STREAM POSSIBLE
    std::cout << "FFMPEG: BASED ON STREAM INFO GET THE BEST STREAM NOW" << std::endl;
    m_vindex = av_find_best_stream(m_fmt, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (m_vindex < 0) {
        av_make_error_string(err_buffer, AV_ERROR_MAX_STRING_SIZE, m_vindex);
        std::cout << Utils::RED << "FFMPEG: ERROR CODE: " << m_vindex << ": " << err_buffer << Utils::RESET << std::endl;
        memset(err_buffer, 0, sizeof(err_buffer));
        std::cout << Utils::RED << "FFMPEG: COULD NOT FIND BEST STREAM. VIDEO INDEX IS NEGATIVE " <<
            "EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        m_run = false;
        return;
    }

    // GET THE CODEC PARAMETERS
    std::cout << "FFMPEG: FIND DECODER WITH THE RIGHT PARAMETERS THAT MATCH THE STREAM" << std::endl;
    const AVCodecParameters* par = m_fmt->streams[m_vindex]->codecpar;
    const AVCodec* dec = avcodec_find_decoder(par->codec_id);
    if (!dec) {
        std::cout << Utils::RED << "FFMPEG: COULD NOT FIND VALID DECODER WITH MATCHING CODEX ID. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        m_run = false;
        return;
    }

    // ALLOCATE THE CODEC AND SET IT TO THE MEMBER OF THIS CLASS
    std::cout << "FFMPEG: ALLOCATE THE CODEC CONTEXT WITH THE APPROPRIATE DECODER THAT MATCHES THE STREAM" << std::endl;
    m_dec = avcodec_alloc_context3(dec);
    if (!m_dec) {
        std::cout << Utils::RED << "FFMPEG: COULD NOT ALLOCATE CONTEXT. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        m_run = false;
        return;
    }

    // SET CODEC PARAMETERS
    std::cout << "FFMPEG: APPLY THE PARAMETERS TO THE CODEC AND DO IT RIGHT" << std::endl;
    ret = avcodec_parameters_to_context(m_dec, par);
    if (ret < 0) {
        av_make_error_string(err_buffer, AV_ERROR_MAX_STRING_SIZE, ret);
        std::cout << Utils::RED << "FFMPEG: ERROR CODE: " << ret << ": " << err_buffer << Utils::RESET << std::endl;
        memset(err_buffer, 0, sizeof(err_buffer));
        std::cout << Utils::RED << "FFMPEG: COULD NOT ADD CODEC PARAMETERS TO CONTEXT. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        m_run = false;
        return;
    }

    // OPEN THE CODEC WITH THE SET PARAMETERS
    std::cout << "FFMPEG: NOW OPEN THE CODEC WITH THE CORRECT PARAMETERS FOR THE STREAM" << std::endl;
    ret = avcodec_open2(m_dec, dec, nullptr);
    if (ret < 0) {
        av_make_error_string(err_buffer, AV_ERROR_MAX_STRING_SIZE, ret);
        std::cout << Utils::RED << "FFMPEG: ERROR CODE: " << ret << ": " << err_buffer << Utils::RESET << std::endl;
        memset(err_buffer, 0, sizeof(err_buffer));
        std::cout << Utils::RED << "FFMPEG: COULD NOT OPEN CODEC. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        m_run = false;
        return;
    }

    // DECODE A SINGLE FRAME AND ALLOCATE IT WITH DEFAULT VALUES
    std::cout << "FFMPEG: DECODE A SINGLE FRAME WITH THE OPENED CODEC AND ALLOCATE IT" << std::endl;
    AVFrame* f = av_frame_alloc();
    if (!f) {
        std::cout << Utils::RED << "FFMPEG: COULD NOT ALLOCATE FRAME. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        m_run = false;
        return;
    }

    // Helper: (re)create sws and set colorspace/range details
    auto create_or_update_sws = [this](int srcW, int srcH, AVPixelFormat srcFmt, const AVFrame* srcFrame) -> bool
    {
        // Use cached context to avoid repeated free/alloc
        int flags = SWS_BILINEAR | SWS_FULL_CHR_H_INT | SWS_FULL_CHR_H_INP;
        m_sws = sws_getCachedContext(
            m_sws,
            srcW, srcH, srcFmt,
            srcW, srcH, AV_PIX_FMT_BGRA,
            flags, nullptr, nullptr, nullptr
        );

        // Check if cache context is valid
        if (!m_sws) {
            std::cout << Utils::RED << "FFMPEG: CACHED SCALING CONTEXT IS INVALID? HOW THE F DID THIS HAPPEN? EXIT VIDEO THREAD NOW!" 
                << Utils::RESET << std::endl;
            return false;
        }

        // Decide colorspace: prefer the frame's signal; otherwise use resolution heuristic
        int cs = SWS_CS_DEFAULT;
        if (srcFrame && srcFrame->colorspace != AVCOL_SPC_UNSPECIFIED) {
            switch (srcFrame->colorspace) {
            case AVCOL_SPC_BT709:      cs = SWS_CS_BT709;      break;
            case AVCOL_SPC_BT470BG:    // same as SMPTE170M/BT.601
            case AVCOL_SPC_SMPTE170M:  cs = SWS_CS_SMPTE170M;  break;
            default:                   cs = (srcW >= 1280 ? SWS_CS_BT709 : SWS_CS_SMPTE170M); break;
            }
        }
        else {
            cs = (srcW >= 1280 ? SWS_CS_BT709 : SWS_CS_SMPTE170M);
        }

        // GET COLOR SPACE COEFFICIENTS. DEFAULTS TO DEFAULT VALUE WHEN FAIL?
        const int* inv_table = sws_getCoefficients(cs); // src YUV->RGB coeffs
        const int* fwd_table = inv_table;               // dst RGB coeffs

        // Source range: most RTSP cameras are limited (MPEG). Respect signalled value if present.
        int srcRange = (srcFrame && srcFrame->color_range == AVCOL_RANGE_JPEG) ? 1 : 0;
        int dstRange = 1; // BGRA is full-range

        // Keep default brightness/contrast/saturation
        int brightness = 0, contrast = 1 << 16, saturation = 1 << 16;

        // SET THE COLOR SPACE DETAILS AFTER SETTING UP
        char err_buffer[AV_ERROR_MAX_STRING_SIZE];
        int ret = sws_setColorspaceDetails(m_sws, inv_table, srcRange, fwd_table, dstRange,
            brightness, contrast, saturation);
        if (ret < 0) {
            av_make_error_string(err_buffer, AV_ERROR_MAX_STRING_SIZE, ret);
            std::cout << Utils::RED << "FFMPEG: ERROR CODE: " << ret << ": " << err_buffer << Utils::RESET << std::endl;
            memset(err_buffer, 0, sizeof(err_buffer));
            std::cout << Utils::RED << "FFMPEG: COULD NOT SET COLOR SPACE DETAILS. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        }
        return true;
    };

    // Helper: prepare GDI-friendly DIB and buffer (DWORD-aligned stride)
    auto prepare_output_buffers = [this](int srcW, int srcH)
    {
        // DWORD-aligned stride for 32bpp: round up to multiple of 4 bytes
        int stride = ((srcW * 32 + 31) / 32) * 4; // equivalent to ((srcW * 4 + 3) & ~3)

        std::lock_guard<std::mutex> lk(m_frameMtx);
        m_w = srcW;
        m_h = srcH;
        m_bgraStride = stride;

        ZeroMemory(&m_bmi, sizeof(m_bmi));
        m_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        m_bmi.bmiHeader.biWidth = m_w;
        m_bmi.bmiHeader.biHeight = -m_h; // top-down DIB
        m_bmi.bmiHeader.biPlanes = 1;
        m_bmi.bmiHeader.biBitCount = 32;
        m_bmi.bmiHeader.biCompression = BI_RGB;
        m_bmi.bmiHeader.biSizeImage = m_bgraStride * m_h; // helpful for some GDI paths

        m_bgra.resize(m_bgraStride * m_h);
    };


    // ALLOCATE A VIDEO PACKET
    std::cout << "FFMPEG: ALLOCATE AN ENTIRE PACKET (VIDEO PACKETS CONTAIN 1 FRAME BUT DO IT ANYWAYS!) CANT HURT" << std::endl;
    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        std::cout << Utils::RED << "FFMPEG: COULD NOT ALLOCATE THE VIDEO PACKET. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        av_frame_free(&f);
        m_run = false;
        return;
    }

    int curW = 0, curH = 0;
    AVPixelFormat curFmt = AV_PIX_FMT_NONE;

    // BEGIN WHILE LOOP
    std::cout << "FFMPEG: VIDEO RENDERING BEGIN NOW! THIS IS IT!" << std::endl;
    while (m_run) {
        if (av_read_frame(m_fmt, pkt) < 0) { Sleep(10); continue; }
        if (pkt->stream_index == m_vindex) {
            if (avcodec_send_packet(m_dec, pkt) == 0) {
                while (m_run) {
                    if (avcodec_receive_frame(m_dec, f) < 0) break;

                    // 1. Get current UI dimensions
                    int tW = m_targetW.load(std::memory_order_relaxed);
                    int tH = m_targetH.load(std::memory_order_relaxed);
                    if (tW <= 0 || tH <= 0) continue;

                    // GET THE NEXT SLOT IN THE RING BUFFER
                    size_t wIdx = m_writeIdx.load(std::memory_order_relaxed);
                    VideoFrame& slot = m_ring[wIdx % RING_BUFFER_SIZE];

                    // 3. (Re)initialize final view scaler if window size changed
                    // RESCALE THE VIDEO FRAME IF THE WINDOW SIZE IS DIFFERENT FROM LAST
                    m_swsView = sws_getCachedContext(m_swsView,
                        f->width, f->height, (AVPixelFormat)f->format,
                        tW, tH, AV_PIX_FMT_BGRA, // Final Output Format
                        SWS_BICUBIC, nullptr, nullptr, nullptr);

                    // LOAD THE VIDEO FRAME INTO A SLOT
                    int stride = ((tW * 4 + 3) & ~3);
                    slot.pixels.resize(stride * tH);
                    slot.stride = stride;
                    slot.w = tW; slot.h = tH;

                    // Setup BMI for this specific frame size
                    slot.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                    slot.bmi.bmiHeader.biWidth = tW;
                    slot.bmi.bmiHeader.biHeight = -tH; // Top-down
                    slot.bmi.bmiHeader.biPlanes = 1;
                    slot.bmi.bmiHeader.biBitCount = 32;
                    slot.bmi.bmiHeader.biCompression = BI_RGB;

                    // SCALE TO WINDOW SIZE
                    uint8_t* dstData[4] = { slot.pixels.data(), nullptr, nullptr, nullptr };
                    int      dstLines[4] = { stride, 0, 0, 0 };
                    sws_scale(m_swsView, f->data, f->linesize, 0, f->height, dstData, dstLines);

                    // CHECK IF RECORDING
                    if (m_isRecording) {
                        DrawRecOverlay(slot);
                    }

                    // CHECK IF PAUSED
                    if (m_isPaused) {
                        DrawPauseOverlay(slot);
                    }

                    // PUSH TO THE CONSUMER
                    m_writeIdx.store(wIdx + 1, std::memory_order_release);
                    bool was = m_needRepaint.exchange(true, std::memory_order_acq_rel);
                    if (!was && m_hwnd) {
                        ::PostMessage(m_hwnd, WM_NEWFRAME, 0, 0);
                    }
                    //
                    //::PostMessage(m_hwnd, WM_NEWFRAME, 0, 0);
                }
            }
        }
        av_packet_unref(pkt);
    }

    // FLUSH DECODER FRAMES TO OBTAIN TAIL FRAMES WHEN STOPPING
    av_packet_free(&pkt);
    av_frame_free(&f);
    // NOTE: m_sws, m_dec, m_fmt are released in Stop()
}


/* RIBBON HANDLING */
CString CCameraControlView::GetRibbonEdit(UINT nID) {

    // Ensure the ribbon bar object exists
    if (!m_mainFrame->m_wndRibbonBar.GetSafeHwnd())
    {
        return _T("ERROR"); // Or some default error value
    }

    // Find the element by its command ID (replace ID_RIBBON_SLIDER with your actual ID)
    CMFCRibbonBaseElement* pElement = m_mainFrame->m_wndRibbonBar.FindByID(nID);

    if (pElement != nullptr)
    {
        // Cast the base element pointer to a CMFCRibbonSlider pointer
        CMFCRibbonEdit* pEdit = DYNAMIC_DOWNCAST(CMFCRibbonEdit, pElement);

        if (pEdit != nullptr)
        {
            // Retrieve and return the current position of the slider
            return pEdit->GetEditText();
        }
    }

    // COULD NOT FIND RIBBON ELEMENT
    std::cout << Utils::RED << "SINGLEVIEW::GETRIBBONEDIT: COULD NOT FIND RIBBON ELEMENT WITH ID " << nID << Utils::RESET << std::endl;
    return _T("ERROR"); // Slider element not found or not a CMFCRibbonSlider
}

/* ON ACTIVATE WINDOW. THIS FUNCTION NEEDS TO BE FAST! */
void CCameraControlView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{

    // ALWAYS ACTIVATE THE WINDOW FIRST THEN SWITCH CONTEXT

    // ACTIVATE THE VIEW
    CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
    if (!bActivate) {
        std::cout << Utils::RED << "SINGLE VIEW NOT ACTIVE?" << Utils::RESET << std::endl;
        return;
    }

    // SWITCH CONTEXT
    if (auto* pMF = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd())) {
        std::cout << "SINGLE VIEW ACTIVATE" << std::endl;
        pMF->ActivateCategoryByName(_T("Single View"));
    }
        

    if (bActivate)
    {
        // Get pointer to MainFrame
        CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
        if (pMainFrame)
        {
            CString strData;
            // Find and update the ribbon control
            CMFCRibbonEdit* pRibbon_videoIP = DYNAMIC_DOWNCAST(
                CMFCRibbonEdit,
                pMainFrame->m_wndRibbonBar.FindByID(ID_SINGLECAM_IP));

            if (pRibbon_videoIP)
            {
                strData = this->GetDocument()->m_videoIP;
                pRibbon_videoIP->SetEditText(strData);
            }

            // Find and update the ribbon control
            CMFCRibbonEdit* pRibbon_audioIP = DYNAMIC_DOWNCAST(
                CMFCRibbonEdit,
                pMainFrame->m_wndRibbonBar.FindByID(ID_SINGLECAMAUDIO_IP));

            if (pRibbon_audioIP)
            {
                strData = this->GetDocument()->m_audioIP;
                pRibbon_audioIP->SetEditText(strData);
            }

            // Find and update the ribbon control
            CMFCRibbonEdit* pRibbon_leftOverlay = DYNAMIC_DOWNCAST(
                CMFCRibbonEdit,
                pMainFrame->m_wndRibbonBar.FindByID(ID_LEFT_OVERLAY));

            if (pRibbon_leftOverlay)
            {
                strData = this->GetDocument()->m_leftOverlay;
                pRibbon_leftOverlay->SetEditText(strData);
            }

            // Find and update the ribbon control
            CMFCRibbonEdit* pRibbon_rightOverlay = DYNAMIC_DOWNCAST(
                CMFCRibbonEdit,
                pMainFrame->m_wndRibbonBar.FindByID(ID_RIGHT_OVERLAY));

            if (pRibbon_rightOverlay)
            {
                strData = this->GetDocument()->m_rightOverlay;
                pRibbon_rightOverlay->SetEditText(strData);
            }

            // Find and update the ribbon control
            CMFCRibbonComboBox* pRibbon_videoChannel = DYNAMIC_DOWNCAST(
                CMFCRibbonComboBox,
                pMainFrame->m_wndRibbonBar.FindByID(ID_SINGLECAM_CHANNEL));

            if (pRibbon_videoChannel)
            {
                strData = this->GetDocument()->m_videoChannel;
                pRibbon_videoChannel->SetEditText(strData);
            }

            // Find and update the ribbon control
            CMFCRibbonComboBox* pRibbon_resolution = DYNAMIC_DOWNCAST(
                CMFCRibbonComboBox,
                pMainFrame->m_wndRibbonBar.FindByID(ID_SINGLEVIEW_RESOLUTION));

            if (pRibbon_resolution)
            {
                strData = this->GetDocument()->m_resolution;
                pRibbon_resolution->SetEditText(strData);
            }

            // Find and update the ribbon control
            CMFCRibbonEdit* pRibbon_viewName = DYNAMIC_DOWNCAST(
                CMFCRibbonEdit,
                pMainFrame->m_wndRibbonBar.FindByID(ID_SINGLECAM_NAME));

            if (pRibbon_viewName)
            {
                strData = this->GetDocument()->m_viewName;
                pRibbon_viewName->SetEditText(strData);
            }
        }
    }
}





/* ENABLE SINGLE VIEW CONTROLS */
void CCameraControlView::OnUpdateSinglecamIp(CCmdUI* pCmdUI)
{
    // Enable it
    pCmdUI->Enable();

    // Store Value
    this->GetDocument()->m_videoIP = GetRibbonEdit(ID_SINGLECAM_IP);
}
void CCameraControlView::OnUpdateSinglecamChannel(CCmdUI* pCmdUI)
{
    // Enable it
    pCmdUI->Enable();

    // Store Value
    this->GetDocument()->m_videoChannel = GetRibbonEdit(ID_SINGLECAM_CHANNEL);
}

void CCameraControlView::OnUpdateSinglecamaudioIp(CCmdUI* pCmdUI)
{
    // Enable it
    pCmdUI->Enable();

    // Store Value
    this->GetDocument()->m_audioIP = GetRibbonEdit(ID_SINGLECAMAUDIO_IP);
}
void CCameraControlView::OnUpdateSinglecamAudioIp(CCmdUI* pCmdUI)
{
    // Enable it
    pCmdUI->Enable();
}
void CCameraControlView::OnUpdateLeftOverlay(CCmdUI* pCmdUI)
{
    // Enable it
    pCmdUI->Enable();

    // Store Value
    this->GetDocument()->m_leftOverlay = GetRibbonEdit(ID_LEFT_OVERLAY);
}
void CCameraControlView::OnUpdateRightOverlay(CCmdUI* pCmdUI)
{
    // Enable it
    pCmdUI->Enable();

    // Store Value
    this->GetDocument()->m_rightOverlay = GetRibbonEdit(ID_RIGHT_OVERLAY);
}
void CCameraControlView::OnUpdateSinglecamName(CCmdUI* pCmdUI)
{
    // Enable it
    pCmdUI->Enable();

    // Store Value
    this->GetDocument()->m_viewName = GetRibbonEdit(ID_SINGLECAM_NAME);
}
void CCameraControlView::OnUpdateSingleviewResolution(CCmdUI* pCmdUI)
{
    // Enable it
    pCmdUI->Enable();

    // Store Value
    this->GetDocument()->m_resolution = GetRibbonEdit(ID_SINGLEVIEW_RESOLUTION);
}
void CCameraControlView::OnUpdateSingleviewCompression(CCmdUI* pCmdUI)
{
    // Enable it
    pCmdUI->Enable();
}

/* ON CONNECT SINGLE VIEW BUTTON PRESSED */
void CCameraControlView::OnConnectSingleView()
{
    // Locals
    //CString username = _T("root");
    //CString password = _T("pass");
    std::cout << "START VIDEO: " << this->GetDocument()->m_videoIP.GetString() << std::endl;
    this->GetDocument()->m_videoIP = GetRibbonEdit(ID_SINGLECAM_IP);
    this->GetDocument()->m_videoChannel = GetRibbonEdit(ID_SINGLECAM_CHANNEL);
    this->GetDocument()->m_resolution = GetRibbonEdit(ID_SINGLEVIEW_RESOLUTION);
    this->GetDocument()->m_videoChannel = GetRibbonEdit(ID_SINGLECAM_CHANNEL);
    //const char* _num = this->GetDocument()->m_videoChannel[strlen(this->GetDocument()->m_videoChannel) - 1];
    StartVideo(this->GetDocument()->m_videoIP, _T("root"), _T("pass"), this->GetDocument()->m_videoChannel, NULL, NULL, NULL);
}

/* ADDING A NAME TO THE VIEW */
void CCameraControlView::OnAddSingleviewName()
{
    // SET WINDOW NAME
    std::cout << "SETTING THE WINDOW NAME NOW" << std::endl;
    if (auto* pChild = DYNAMIC_DOWNCAST(CChildFrame, GetParentFrame()))
    {
        // SET CUSTOM TITLE
        std::cout << "SETTING THE WINDOW NAME TO: " << this->GetDocument()->m_viewName << std::endl;
        pChild->SetCustomTitle(this->GetDocument()->m_viewName);
        this->GetDocument()->SetTitle(this->GetDocument()->m_viewName);
        m_wndName = this->GetDocument()->m_viewName; // THIS IS SO VIDEO THREAD DOES NOT TOUCH DOCUMENT! BIG NO NO
    }
    else {

        // ERROR SETTING DOCUMENT NAME!
        std::cout << Utils::RED << "ERROR: SETTING THE WINDOW NAME FAILED TO GET PARENT FRAME AND DOWNCAST" << Utils::RESET << std::endl;
    }
}



void CCameraControlView::OnFullscreen()
{
    // 1. Get the Main Frame and hide its menu
    CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
    pMainFrame->SetMenu(NULL);

    // 2. Maximize the MDI Child
    CChildFrame* pChild = (CChildFrame*)GetParentFrame();
    if (pChild) {
        // Optional: Store the shared menu before clearing it to restore later
        // pChild->m_hMenuShared = NULL; 
        //pChild->MDIMaximize();
    }

    // 3. Force the frame to recalculate its layout to fill the empty menu space
    pMainFrame->RecalcLayout();

}

/* FOR HANDLING MOUSE WHEEL UP OR DOWN EVENT */
BOOL CCameraControlView::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_MOUSEWHEEL)
    {
        // Call your OnMouseWheel handler logic here
        // You might need to extract the relevant parameters
        CPoint point(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
        ScreenToClient(&point); // Convert to client coordinates

        // Extract zDelta from wParam (high-order word)
        short zDelta = HIWORD(pMsg->wParam);

        // Implement your mouse wheel logic
        // ...
        if (zDelta > 0)
        {
            // Mouse wheel scrolled UP (forward)
            // Add your custom logic here (e.g., scroll up a view, zoom in)
            // Example: Scroll the view up by some amount
            // ScrollLines(SystemInformation.MouseWheelScrollLines); // Example logic
            std::cout << "MOUSE WHEEL UP" << std::endl;
        }
        else if (zDelta < 0)
        {
            // Mouse wheel scrolled DOWN (backward)
            // Add your custom logic here (e.g., scroll down a view, zoom out)
            // Example: Scroll the view down by some amount
            std::cout << "MOUSE WHEEL DOWN" << std::endl;
        }

        return TRUE; // Message handled
    }
    return CFormView::PreTranslateMessage(pMsg);
}


/* MOUSE DOWN EVENT */
void CCameraControlView::OnLButtonDown(UINT nFlags, CPoint point)
{
    std::cout << "SINGLE VIEW LEFT MOUSE DOWN" << std::endl;

    CFormView::OnLButtonDown(nFlags, point);
}


void CCameraControlView::OnSingleviewRmOverlays()
{
    // TODO: Add your command handler code here
}


void CCameraControlView::OnSingleviewAddROverlay()
{
    // TODO: Add your command handler code here
}


void CCameraControlView::OnSingleviewAddLOverlay()
{
    // TODO: Add your command handler code here
}




void CCameraControlView::OnUpdateSingleviewPtzOnoff(CCmdUI* pCmdUI)
{
    // Enable Checkbox
    pCmdUI->Enable();

    // SET CHECKBOX HERE BASED ON STATE
    if (m_ptzONOFF) {
        pCmdUI->SetCheck(TRUE);
    }
    else {
        pCmdUI->SetCheck(FALSE);
    }
    
}

/* FOR HANDLING CHECKBOX PRESSED EVENT */
void CCameraControlView::OnSingleviewPtzOnoff()
{
    // REVERSE IT WHEN PRESSED
    m_ptzONOFF = !m_ptzONOFF;
}


void CCameraControlView::OnRestoreView()
{
    std::cout << "RESTORE CHILD WINDOW" << std::endl;
    // Cast it to your specific child frame class
    CFrameWnd* pFrame = GetParentFrame();
    CChildFrame* pChildFrame = dynamic_cast<CChildFrame*>(pFrame);
    pChildFrame->RestoreFrame();
}


/* EVENT FOR FILLING ENTIRE SCREEN WITH VIEW */
void CCameraControlView::OnFillWindow()
{
    std::cout << "FILL ENTIRE WINDOW" << std::endl;
    try {
        CFrameWnd* pFrame = GetParentFrame();
        CChildFrame* pChildFrame = dynamic_cast<CChildFrame*>(pFrame);
        pChildFrame->FillWindow();
    }
    catch(const std::runtime_error& e){
        std::cerr << "CCAMERACONTROLVIEW::ONFILLWINDOW::::EXCEPTION! " << e.what() << std::endl;
    }
}

/* EVENTS FOR HIDDING THE RIBBON CONTROL */
void CCameraControlView::OnHideRibbon()
{
    if (!this->m_mainFrame->m_toolBarState) {
        this->m_mainFrame->ShowToolbar();
    }
    else {
        this->m_mainFrame->HideToolbar();
    }
}


/* INITIALIZE GDI DRAWING */
void CCameraControlView::InitGdiResources() {
    std::cout << "INITIALIZING GDIPLUS TOOLS! THE BEST TOOLS!" << std::endl;
    if (!m_pRecFont) {
        m_pRecFont = std::make_unique<Gdiplus::Font>(L"Arial", 14, Gdiplus::FontStyleBold);
        m_pRedBrush = std::make_unique<Gdiplus::SolidBrush>(Gdiplus::Color(255, 255, 0, 0));
        m_pWhiteBrush = std::make_unique<Gdiplus::SolidBrush>(Gdiplus::Color(255, 255, 255, 255));
    }
}


/* ADDING REC OVERLAY WHEN RECORDING */
void CCameraControlView::DrawRecOverlay(VideoFrame& slot) {
    using namespace Gdiplus;

    Bitmap bitmap(slot.w, slot.h, slot.stride, PixelFormat32bppRGB, slot.pixels.data());
    Graphics g(&bitmap); // Use stack-based Graphics object to auto-clean up

    g.SetSmoothingMode(SmoothingModeAntiAlias);
    g.SetTextRenderingHint(TextRenderingHintAntiAlias);

    int padding = 20;
    int circleSize = 16;
    int x = slot.w - 100;
    int y = padding;

    g.FillEllipse(m_pRedBrush.get(), x, y, circleSize, circleSize);
    g.DrawString(L"REC", -1, m_pRecFont.get(),
        PointF((float)x + circleSize + 5.0f, (float)y - 2.0f),
        m_pWhiteBrush.get());
}
void CCameraControlView::DrawPauseOverlay(VideoFrame& slot) {
    using namespace Gdiplus;
    //InitGdiOverlayResources(); // Ensure white brush/font exist

    Bitmap bitmap(slot.w, slot.h, slot.stride, PixelFormat32bppRGB, slot.pixels.data());
    Graphics g(&bitmap);

    g.SetSmoothingMode(SmoothingModeAntiAlias);

    int padding = 20;
    int barWidth = 6;
    int barHeight = 18;
    int spacing = 5; // Space between the two bars

    // Position it in the upper right, same as the REC circle
    int x = slot.w - 120;
    int y = padding;

    // Draw the two vertical bars of the pause symbol
    // Left Bar
    g.FillRectangle(m_pWhiteBrush.get(), x, y, barWidth, barHeight);
    // Right Bar
    g.FillRectangle(m_pWhiteBrush.get(), x + barWidth + spacing, y, barWidth, barHeight);

    // Optional: Draw "PAUSED" text next to it
    g.SetTextRenderingHint(TextRenderingHintAntiAlias);
    g.DrawString(L"PAUSE", -1, m_pRecFont.get(),
        PointF((float)x + (barWidth * 2) + spacing + 8.0f, (float)y - 2.0f),
        m_pWhiteBrush.get());
}



/* RECORDING VIDEO AND AUDIO BUTTON PRESSED */
void CCameraControlView::OnRecordVidAudio()
{
    // DO NOTHING IF VIDEO NOT STARTED
    if (!m_run.load()) {
        return;
    }
    m_isRecording = TRUE;
}
void CCameraControlView::OnUpdateRecordVidAudio(CCmdUI* pCmdUI)
{
    if (m_isRecording || m_isPaused) {
        pCmdUI->Enable(FALSE);
    }
    else {
        pCmdUI->Enable(TRUE);
    }
}


/* RECORDING VIDEO ONLY BUTTON PRESSED */
void CCameraControlView::OnRecordVideo()
{
    // DO NOTHING IF VIDEO NOT STARTED
    if (!m_run.load()) {
        return;
    }
    m_isRecording = TRUE;
    m_isPaused = FALSE;
}
void CCameraControlView::OnUpdateRecordVideo(CCmdUI* pCmdUI)
{
    if (m_isRecording || m_isPaused) {
        pCmdUI->Enable(FALSE);
    }
    else {
        pCmdUI->Enable(TRUE);
    }
}

/* ON PUASE RECORDING BUTTON PRESSED */
void CCameraControlView::OnPauseRecording()
{
    m_isRecording = FALSE;
    m_isPaused = TRUE;
}
void CCameraControlView::OnUpdatePauseRecording(CCmdUI* pCmdUI)
{
    if (m_isRecording && !m_isPaused) {
        pCmdUI->Enable(TRUE);
    }
    else {
        pCmdUI->Enable(FALSE);
    }
}

/* STOP RECORDING */
void CCameraControlView::OnStopRecording()
{
    
    m_isRecording = FALSE;
    m_isPaused = FALSE;
}
void CCameraControlView::OnUpdateStopRecording(CCmdUI* pCmdUI)
{
    if (m_isRecording || m_isPaused) {
        pCmdUI->Enable(TRUE);
    }
    else {
        pCmdUI->Enable(FALSE);
    }
}


void CCameraControlView::OnSnapShot()
{
    // DO NOTHING IF VIDEO NOT STARTED
    if (!m_run.load()) {
        return;
    }
}

/* RESUME BUTTON PRESSED */
void CCameraControlView::OnUpdateSingleviewResume(CCmdUI* pCmdUI)
{
    if (m_isPaused) {
        pCmdUI->Enable(TRUE);
    }
    else {
        pCmdUI->Enable(FALSE);
    }
}
void CCameraControlView::OnSingleviewResume()
{
    m_isPaused = FALSE;
    m_isRecording = TRUE;
}
