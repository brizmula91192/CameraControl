




/* HEADERS */
#include "pch.h"
#include "framework.h"

#include "QuadViewDoc.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "AppMsgs.h"
#include "QuadView.h"

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



IMPLEMENT_DYNCREATE(QuadView, CFormView)

BEGIN_MESSAGE_MAP(QuadView, CFormView)
    // Standard printing commands
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
END_MESSAGE_MAP()

// QuadView construction/destruction

QuadView::QuadView() noexcept
    : CFormView(IDD_QUADVIEW_DLG)
{
    // TODO: add construction code here

}

QuadView::~QuadView()
{
}

void QuadView::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
}

BOOL QuadView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    return CFormView::PreCreateWindow(cs);
}

void QuadView::OnInitialUpdate()
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
    LayoutQuadrants(); // NEW

    // RESERVE BUFFERS FOR PIXELS
    for (int i = 0; i < NUM_STREAMS; ++i) {
        const int w = m_quadrant[i].Width();
        const int h = m_quadrant[i].Height();
        // If you want absolute worst-case: use 3840x2160; otherwise use current quadrant size.
        PreallocateRingBuffersForStream(i, w, h);
    }


    // NAMES FOR DEBUGGING
    m_stream[TL].name = _T("TopLeft");
    m_stream[TR].name = _T("TopRight");
    m_stream[BL].name = _T("BottomLeft");
    m_stream[BR].name = _T("BottomRight");

    // DISABLE WARNINGS
    av_log_set_level(AV_LOG_QUIET);
}

void QuadView::OnSize(UINT nType, int cx, int cy)
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


void QuadView::LayoutQuadrants()
{
    CRect rc; GetClientRect(&rc);
    int midX = rc.left + rc.Width() / 2;
    int midY = rc.top + rc.Height() / 2;

    m_quadrant[TL] = CRect(rc.left, rc.top, midX, midY);
    m_quadrant[TR] = CRect(midX, rc.top, rc.right, midY);
    m_quadrant[BL] = CRect(rc.left, midY, midX, rc.bottom);
    m_quadrant[BR] = CRect(midX, midY, rc.right, rc.bottom);

    // Update per-stream targets atomically
    for (int i = 0; i < NUM_STREAMS; ++i) {
        int w = m_quadrant[i].Width();
        int h = m_quadrant[i].Height();
        m_stream[i].targetW.store(w, std::memory_order_relaxed);
        m_stream[i].targetH.store(h, std::memory_order_relaxed);
    }
}

void QuadView::StartStream(int idx, const CString& ip, const CString& chan)
{
    auto& s = m_stream[idx];
    StopStream(idx);

    CString url;
    url.Format(_T("rtsp://%s:%s@%s/axis-media/media.amp?camera=%s&fps=30"), _T("root"), _T("pass"), ip, chan);
    std::cout << "URL RTSP LINK IS: " << url << " AT INDEX: " << idx << std::endl;
    s.run.store(true, std::memory_order_release);
    HWND hwnd = m_hwnd;
    s.thr = std::thread([this, idx, url] { WorkerProc(idx, url); });
}

void QuadView::StopStream(int idx)
{
    auto& s = m_stream[idx];
    s.run.store(false, std::memory_order_release);
    if (s.thr.joinable()) s.thr.join();

    if (s.swsView) { sws_freeContext(s.swsView);      s.swsView = nullptr; }
    if (s.swsFmtToBgra) { sws_freeContext(s.swsFmtToBgra); s.swsFmtToBgra = nullptr; }
    if (s.dec) { avcodec_free_context(&s.dec);    s.dec = nullptr; }
    if (s.fmt) { avformat_close_input(&s.fmt);    s.fmt = nullptr; }

    // Clear buffers
    for (auto& slot : s.ring) {
        slot.pixels.clear(); slot.pixels.shrink_to_fit();
        ZeroMemory(&slot.bmi, sizeof(slot.bmi));
        slot.w = slot.h = slot.stride = 0;
    }
    s.writeIdx.store(0); s.readIdx.store(0);
}


void QuadView::StopAll()
{
    for (int i = 0; i < NUM_STREAMS; ++i) StopStream(i);
}




// QuadView printing


void QuadView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
    AFXPrintPreview(this);
#endif
}

BOOL QuadView::OnPreparePrinting(CPrintInfo* pInfo)
{
    // default preparation
    return DoPreparePrinting(pInfo);
}

void QuadView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add extra initialization before printing
}

void QuadView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add cleanup after printing
}

void QuadView::OnPrint(CDC* pDC, CPrintInfo* /*pInfo*/)
{
    // TODO: add customized printing code here
}

void QuadView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
    ClientToScreen(&point);
    OnContextMenu(this, point);
}

void QuadView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
    theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// QuadView diagnostics
#ifdef _DEBUG
void QuadView::AssertValid() const
{
    CFormView::AssertValid();
}

void QuadView::Dump(CDumpContext& dc) const
{
    CFormView::Dump(dc);
}
#endif //_DEBUG

QuadViewDoc* QuadView::GetDocument()
{
    return reinterpret_cast<QuadViewDoc*>(m_pDocument);
}

/* ON DESTROY METHOD PART OF CFORMVIEW CLASS. CALLED AFTER CHILDWND CALLS ON CLOSE */
void QuadView::OnDestroy() {

    // Debug console
    std::cout << "DESTROYING VIDEO THREAD NOW!" << std::endl;

    // SET HARDWARE WINDOW CONTEXT TO NULL
    HWND old = m_hwnd;
    m_hwnd = NULL;      // tells worker that window is gone

    // END THE VIDEO THREAD AND ASSERT THREAD IS JOINED
    StopAll(); // instead of Stop()
    //ASSERT(!m_thr.joinable());
    
    // SWITCH CONTEXT
    if (auto* pMF = DYNAMIC_DOWNCAST(CMainFrame, AfxGetMainWnd())) {
        pMF->CheckNumWindows();
    }

    // CALL BASE CLASS ON DESTROY FOR CFORMVIEW
    CFormView::OnDestroy();
}


/* PREALLOCATE BUFFERS FOR THE PIXELS IN THE RING BUFFER */
void QuadView::PreallocateRingBuffersForStream(int idx, int maxW, int maxH)
{
    auto& s = m_stream[idx];
    const size_t bytesPerPixel = 4; // BGRA
    const int maxStride = ((maxW * 4 + 3) & ~3);
    const size_t bufferSize = size_t(maxStride) * size_t(maxH);

    for (auto& slot : s.ring) {
        slot.pixels.reserve(bufferSize);      // capacity only; no size yet
        ZeroMemory(&slot.bmi, sizeof(slot.bmi));
        slot.stride = 0; slot.w = slot.h = 0;
    }
}



/* ON NEW FRAME */
LRESULT QuadView::OnNewFrame(WPARAM wParam, LPARAM lParam)
{

    int idx = (int)wParam;
    if (idx < 0 || idx >= NUM_STREAMS) return 0;

    // Record which stream asked for paint; this guides OnDraw.
    m_lastPaintStream.store(idx, std::memory_order_release);

    // Invalidate only the quadrant for that stream (avoid coalescing/all-stream draws)
    InvalidateRect(&m_quadrant[idx], FALSE);
    return 0;

}

/* BEGIN SHUTTING DOWN THE THREAD BEFORE ONDESTROY. CALLED FROM CHILDFRAME CLASS ONCLOSE */
void QuadView::BeginVideoShutdown()
{
    // Tell the worker loop to stop as soon as possible
}

/* STOP VIDEO THREAD AS SOON AS POSSIBLE */
LRESULT QuadView::OnBeginVideoShutdown(WPARAM, LPARAM)
{
    // BEGIN THE SHUTDOWN. SET ALL RUNS TO FALSE!
    for (int i = 0; i < NUM_STREAMS; ++i) {
        auto& s = m_stream[i];
        s.run.store(false, std::memory_order_release);
    }

    return 0;
}


/* PER QUADRANT DRAW */
void QuadView::OnDraw(CDC* pDC)
{

    CRect rcClient; GetClientRect(&rcClient);

    // If nothing is running, clear
    bool anyRun = false;
    for (int i = 0; i < NUM_STREAMS; ++i) anyRun |= m_stream[i].run.load();
    if (!anyRun) {
        pDC->FillSolidRect(rcClient, RGB(0, 0, 0));
        return;
    }

    // Which stream requested this paint?
    int idx = m_lastPaintStream.load(std::memory_order_acquire);
    if (idx < 0 || idx >= NUM_STREAMS) return;

    auto& s = m_stream[idx];
    CRect dst = m_quadrant[idx];

    size_t wIdx = s.writeIdx.load(std::memory_order_acquire);
    if (wIdx == 0) {
        // No frames yet—fill its quadrant with black
        pDC->FillSolidRect(dst, RGB(0, 0, 0));
        return;
    }

    // Latest available frame in the ring
    VideoFrame& slot = s.ring[(wIdx - 1) % StreamCtx::RING_BUFFER_SIZE];

    // Paint only this quadrant
    ::StretchDIBits(
        pDC->GetSafeHdc(),
        dst.left, dst.top, dst.Width(), dst.Height(),  // destination (quadrant)
        0, 0, slot.w, slot.h,                          // source
        slot.pixels.data(), &slot.bmi, DIB_RGB_COLORS, SRCCOPY
    );

    // Mark read index advanced (optional for stats)
    s.readIdx.store(wIdx, std::memory_order_release);
}

/* HELPER METHOD FOR CONVERTING ENCODING */
static std::string ToUtf8(const CString& s) {
    CStringA a(CW2A(s, CP_UTF8));
    return std::string(a);
}

/* AV FFPMEG CALL BACK FUNCTION FOR INTERRUPT */
int QuadView::AvInterrupt(void* opaque)
{
    auto* s = static_cast<StreamCtx*>(opaque);
    // return non-zero to interrupt I/O
    return (s && !s->run.load(std::memory_order_acquire)) ? 1 : 0;
}


enum AVPixelFormat QuadView::GetHwFormat(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts) {
    for (const enum AVPixelFormat* p = pix_fmts; *p != -1; p++) {
        if (*p == AV_PIX_FMT_D3D11) return *p; // Or AV_PIX_FMT_CUDA for NVIDIA
    }
    return AV_PIX_FMT_NONE;
}


/* THIS IS THE MAIN WORKER THREAD THAT DECODES THE VIDEO FRAMES */
void QuadView::WorkerProc(int idx, CString curl)
{
    // LOCALS
    auto& s = m_stream[idx];
    int ret;
    char err_buffer[AV_ERROR_MAX_STRING_SIZE]{};
    std::string url = ToUtf8(curl);

    std::cout << "FFMPEG: ALLOCATE FORMAT CONTEXT" << std::endl;
    s.fmt = avformat_alloc_context();
    if (!s.fmt) { s.run = false; return; }

    // SETUP THE INTERRUPTS
#ifdef VERBOSE 
    std::cout << "FFMPEG: SETTING UP INTERRUPTS" << std::endl; 
#endif
    s.fmt->interrupt_callback.callback = &QuadView::AvInterrupt;
    s.fmt->interrupt_callback.opaque = &s;

    // SETUP THE OPTIONS
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "stimeout", "2000000", 0); // etc...


    // OPEN THE INPUT
    ret = avformat_open_input(&s.fmt, url.c_str(), nullptr, &opts);
    if (ret < 0) {
        std::cout << "COULD NOT OPEN INPUT " << std::endl;
        av_make_error_string(err_buffer, AV_ERROR_MAX_STRING_SIZE, ret);
        std::cout << Utils::RED << "FFMPEG: COULD NOT OPEN STREAM FOR " << m_wndName << Utils::RESET << std::endl;
        std::cout << Utils::RED << "FFMPEG: ERROR CODE: " << ret << ": " << err_buffer << Utils::RESET << std::endl;
        std::cout << Utils::RED << "FFMPEG: ENDING VIDEO THREAD FOR: " << m_wndName << Utils::RESET << std::endl;
        memset(err_buffer, 0, sizeof(err_buffer));
        av_dict_free(&opts); s.run = false; return;
    }
    av_dict_free(&opts);


    // FIND STREAM INFO
    ret = avformat_find_stream_info(s.fmt, nullptr);
    if (ret < 0) 
    {
        av_make_error_string(err_buffer, AV_ERROR_MAX_STRING_SIZE, ret);
        std::cout << Utils::RED << "FFMPEG: ERROR CODE: " << ret << ": " << err_buffer << Utils::RESET << std::endl;
        std::cout << Utils::RED << "FFMPEG: FAILED TO FIND STREAM INFO. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        s.run = false; 
        return; 
    }


    // FIND THE BEST STREAM TO USE
    s.vindex = av_find_best_stream(s.fmt, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (s.vindex < 0) 
    {
        av_make_error_string(err_buffer, AV_ERROR_MAX_STRING_SIZE, s.vindex);
        std::cout << Utils::RED << "FFMPEG: ERROR CODE: " << s.vindex << ": " << err_buffer << Utils::RESET << std::endl;
        memset(err_buffer, 0, sizeof(err_buffer));
        std::cout << Utils::RED << "FFMPEG: COULD NOT FIND BEST STREAM. VIDEO INDEX IS NEGATIVE " <<
            "EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        s.run = false; 
        return; 
    }

    // GET THE CODEC PARAMETERS
    const AVCodecParameters* par = s.fmt->streams[s.vindex]->codecpar;
    const AVCodec* dec = avcodec_find_decoder(par->codec_id);
    if (!dec) 
    {
        std::cout << Utils::RED << "FFMPEG: COULD NOT FIND VALID DECODER WITH MATCHING CODEX ID. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        s.run = false; 
        return; 
    }

    // ALLOCATE THE CODEC AND SET IT TO THE MEMBER OF THIS CLASS
    s.dec = avcodec_alloc_context3(dec);
    if (!s.dec) 
    {
        std::cout << Utils::RED << "FFMPEG: COULD NOT ALLOCATE CONTEXT. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        s.run = false; 
        return; 
    }

    // SET CODEC PARAMETERS BASED ON FORMAT
    ret = avcodec_parameters_to_context(s.dec, par);
    if (ret < 0) 
    {
        av_make_error_string(err_buffer, AV_ERROR_MAX_STRING_SIZE, ret);
        std::cout << Utils::RED << "FFMPEG: ERROR CODE: " << ret << ": " << err_buffer << Utils::RESET << std::endl;
        memset(err_buffer, 0, sizeof(err_buffer));
        std::cout << Utils::RED << "FFMPEG: COULD NOT ADD CODEC PARAMETERS TO CONTEXT. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        s.run = false; 
        return; 
    }

    // SETUP DECODER HW ACCELERATION
    ret = av_hwdevice_ctx_create(&m_hwDeviceCtx, AV_HWDEVICE_TYPE_D3D11VA, NULL, NULL, 0);
    if (ret == 0) {
        s.dec->hw_device_ctx = av_buffer_ref(m_hwDeviceCtx);
        s.dec->get_format = GetHwFormat;
    }
    else {
        av_make_error_string(err_buffer, AV_ERROR_MAX_STRING_SIZE, ret);
        std::cout << Utils::RED << "FFMPEG: ERROR CODE: " << ret << ": " << err_buffer << Utils::RESET << std::endl;
        memset(err_buffer, 0, sizeof(err_buffer));
        std::cout << Utils::RED << "FFMPEG: FAILED TO SETUP HW ACCELERATION FOR DECODE! REVERT TO CPU DECODE" << Utils::RESET << std::endl;
    }
    


    // OPEN THE CODEC NOW WITH THE PARAMETERS
    if (avcodec_open2(s.dec, dec, nullptr) < 0) 
    { 
        av_make_error_string(err_buffer, AV_ERROR_MAX_STRING_SIZE, ret);
        std::cout << Utils::RED << "FFMPEG: ERROR CODE: " << ret << ": " << err_buffer << Utils::RESET << std::endl;
        memset(err_buffer, 0, sizeof(err_buffer));
        std::cout << Utils::RED << "FFMPEG: COULD NOT OPEN CODEC. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        s.run = false; 
        return; 
    }

    // ALLOCATE RAM FRAME
    AVFrame* f = av_frame_alloc();
    if (!f) 
    {
        std::cout << Utils::RED << "FFMPEG: COULD NOT ALLOCATE FRAME. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        s.run = false; 
        return; 
    }

    // ALLOCATE GPU FRAME
    AVFrame* gpuf = av_frame_alloc();
    if (!gpuf)
    {
        std::cout << Utils::RED << "FFMPEG: COULD NOT ALLOCATE FRAME. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        s.run = false;
        return;
    }

    // ALLOCATE PACKET
    AVPacket* pkt = av_packet_alloc();
    if (!pkt) 
    { 
        std::cout << Utils::RED << "FFMPEG: COULD NOT ALLOCATE THE VIDEO PACKET. EXIT VIDEO THREAD!" << Utils::RESET << std::endl;
        av_frame_free(&f); 
        s.run = false; 
        return; 
    }

    std::cout << "MADE IT TO WHILE LOOP" << std::endl;
    while (s.run) {
        if (av_read_frame(s.fmt, pkt) < 0) { Sleep(10); continue; }
        if (pkt->stream_index != s.vindex) { av_packet_unref(pkt); continue; }

        // SEND PACK TO DECODER
        if (avcodec_send_packet(s.dec, pkt) == 0) {
            while (s.run) {

                // GET FRAME FROM DECODER
                //if (avcodec_receive_frame(s.dec, f) < 0) break;
                if (avcodec_receive_frame(s.dec, gpuf) < 0) break;

                // TRANSFER FROM GPU VRAM TO RAM
                av_hwframe_transfer_data(f, gpuf, 0);

                // (1) Ensure source->BGRA converter
                s.swsFmtToBgra = sws_getCachedContext(
                    s.swsFmtToBgra,
                    f->width, f->height, (AVPixelFormat)f->format,
                    f->width, f->height, AV_PIX_FMT_BGRA,
                    SWS_BILINEAR, nullptr, nullptr, nullptr);

                // (2) Make/resize a temporary BGRA buffer if you want two-step scaling,
                //     or scale source->quadrant directly. We’ll go direct source->quadrant.

                int tW = s.targetW.load(std::memory_order_relaxed);
                int tH = s.targetH.load(std::memory_order_relaxed);
                if (tW <= 0 || tH <= 0) continue;

                // (3) Claim slot in ring
                size_t wIdx = s.writeIdx.load(std::memory_order_relaxed);
                auto& slot = s.ring[wIdx % StreamCtx::RING_BUFFER_SIZE];

                // Destination stride and buffer
                int dstStride = ((tW * 4 + 3) & ~3);
                slot.pixels.resize(dstStride * tH);
                slot.stride = dstStride; slot.w = tW; slot.h = tH;

                ZeroMemory(&slot.bmi, sizeof(BITMAPINFO));
                slot.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                slot.bmi.bmiHeader.biWidth = tW;
                slot.bmi.bmiHeader.biHeight = -tH; // top-down
                slot.bmi.bmiHeader.biPlanes = 1;
                slot.bmi.bmiHeader.biBitCount = 32;
                slot.bmi.bmiHeader.biCompression = BI_RGB;

                // (4) Source->quadrant BGRA scaling
                s.swsView = sws_getCachedContext(
                    s.swsView,
                    f->width, f->height, (AVPixelFormat)f->format,
                    tW, tH, AV_PIX_FMT_BGRA,
                    SWS_BICUBIC, nullptr, nullptr, nullptr);

                uint8_t* dstData[4] = { slot.pixels.data(), nullptr, nullptr, nullptr };
                int      dstLine[4] = { dstStride, 0, 0, 0 };
                sws_scale(s.swsView, f->data, f->linesize, 0, f->height, dstData, dstLine);

                // (5) Publish & notify
                s.writeIdx.store(wIdx + 1, std::memory_order_release);
                ::PostMessage(m_hwnd, WM_NEWFRAME_QUAD, (WPARAM)idx, 0);
            }
        }
        av_packet_unref(pkt);
    }

    av_packet_free(&pkt);
    av_frame_free(&f);
    // contexts freed in StopStream(idx)
}



/* RIBBON HANDLING. GET THE TEXT IN A RIBBON EDIT */
CString QuadView::GetRibbonEdit(UINT nID) {

    // CHECK IF RIBBON EXISTS
    if (!this->m_mainFrame->m_wndRibbonBar.GetSafeHwnd())
    {
        return _T("ERROR"); // Or some default error value
    }

    // GET ELEMENT BY ID
    CMFCRibbonBaseElement* pElement = this->m_mainFrame->m_wndRibbonBar.FindByID(nID);

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

    // Could not find valid ribbon element

    return _T("ERROR"); // Slider element not found or not a CMFCRibbonSlider
}

/* QUICK INLINE HELPER METHOD FOR UPDATING RIBBON CONTROL */
inline void QuadView::SetRibbon(CMFCRibbonEdit* pRibbon, const CString &val, const UINT ID) {
    pRibbon = DYNAMIC_DOWNCAST(
        CMFCRibbonEdit,
        this->m_mainFrame->m_wndRibbonBar.FindByID(ID));
    if (pRibbon)
    {
        pRibbon->SetEditText(val);
    }
}

/* WHEN WINDOW IS SELECTED */
void QuadView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{

    // ALWAYS ACTIVATE WINDOW FIRST
    CFormView::OnActivateView(bActivate, pActivateView, pDeactiveView);
    if (!bActivate) {
        std::cout << Utils::RED << "QUAD VIEW NOT ACTIVE?" << Utils::RESET << std::endl;
        return;
    }

    // SWITCH CONTEXT
    if (this->m_mainFrame) {
        std::cout << "QUAD VIEW ACTIVATE" << std::endl;
        this->m_mainFrame->ActivateCategoryByName(_T("Quad View"));
    }

    if (bActivate)
    {
        // Get pointer to MainFrame
        CMFCRibbonEdit* pRibbon = nullptr;{};
        if (this->m_mainFrame)
        {
            SetRibbon(pRibbon, this->GetDocument()->m_TL_Video.m_videoIP, ID_QUAD_TL_IP);
            SetRibbon(pRibbon, this->GetDocument()->m_TR_Video.m_videoIP, ID_QUAD_TR_IP);
            SetRibbon(pRibbon, this->GetDocument()->m_BL_Video.m_videoIP, ID_QUAD_BL_IP);
            SetRibbon(pRibbon, this->GetDocument()->m_BR_Video.m_videoIP, ID_QUAD_BR_IP);

            SetRibbon(pRibbon, this->GetDocument()->m_TL_Video.m_videoChannel, ID_QUAD_TL_CHANNEL);
            SetRibbon(pRibbon, this->GetDocument()->m_TR_Video.m_videoChannel, ID_QUAD_TR_CHANNEL);
            SetRibbon(pRibbon, this->GetDocument()->m_BL_Video.m_videoChannel, ID_QUAD_BL_CHANNEL);
            SetRibbon(pRibbon, this->GetDocument()->m_BR_Video.m_videoChannel, ID_QUAD_BR_CHANNEL);

            SetRibbon(pRibbon, this->GetDocument()->m_TL_Video.m_resolution, ID_QUAD_TL_RESOLUTION);
            SetRibbon(pRibbon, this->GetDocument()->m_TR_Video.m_resolution, ID_QUAD_TR_RESOLUTION);
            SetRibbon(pRibbon, this->GetDocument()->m_BL_Video.m_resolution, ID_QUAD_BL_RESOLUTION);
            SetRibbon(pRibbon, this->GetDocument()->m_BR_Video.m_resolution, ID_QUAD_BR_RESOLUTION);

            SetRibbon(pRibbon, this->GetDocument()->m_viewName, ID_QUADVIEW_NAME);
        }
    }
}



void QuadView::OnConnectSingleView()
{
    // Locals
    //CString username = _T("root");
    //CString password = _T("pass");
    //std::cout << "START VIDEO: " << this->GetDocument()->m_videoIP.GetString() << std::endl;
    //this->GetDocument()->m_videoIP = GetRibbonEdit(ID_SINGLECAM_IP);
    //StartVideo(this->GetDocument()->m_videoIP, _T("root"), _T("pass"));

}


void QuadView::OnUpdateQuadTlIp(CCmdUI* pCmdUI)
{
    // Enable
    pCmdUI->Enable();

    // Update
    this->GetDocument()->m_TL_Video.m_videoIP = GetRibbonEdit(ID_QUAD_TL_IP);
}
void QuadView::OnUpdateQuadTlChannel(CCmdUI* pCmdUI)
{
    // Enable
    pCmdUI->Enable();

    // Update
    this->GetDocument()->m_TL_Video.m_videoChannel = GetRibbonEdit(ID_QUAD_TL_CHANNEL);
}
void QuadView::OnUpdateQuadTlResolution(CCmdUI* pCmdUI)
{
    // Enable
    pCmdUI->Enable();

    // Update
    this->GetDocument()->m_TL_Video.m_resolution = GetRibbonEdit(ID_QUAD_TL_RESOLUTION);
}
void QuadView::OnUpdateQuadTrIp(CCmdUI* pCmdUI)
{
    // Enable
    pCmdUI->Enable();

    // Update
    this->GetDocument()->m_TR_Video.m_videoIP = GetRibbonEdit(ID_QUAD_TR_IP);
}
void QuadView::OnUpdateQuadTrChannel(CCmdUI* pCmdUI)
{
    // Enable
    pCmdUI->Enable();

    // Update
    this->GetDocument()->m_TR_Video.m_videoChannel = GetRibbonEdit(ID_QUAD_TR_CHANNEL);
}
void QuadView::OnUpdateQuadTrResolution(CCmdUI* pCmdUI)
{
    // Enable
    pCmdUI->Enable();

    // Update
    this->GetDocument()->m_TR_Video.m_resolution = GetRibbonEdit(ID_QUAD_TR_RESOLUTION);
}
void QuadView::OnUpdateQuadBlIp(CCmdUI* pCmdUI)
{
    // Enable
    pCmdUI->Enable();

    // Update
    this->GetDocument()->m_BL_Video.m_videoIP = GetRibbonEdit(ID_QUAD_BL_IP);
}
void QuadView::OnUpdateQuadBlChannel(CCmdUI* pCmdUI)
{
    // Enable
    pCmdUI->Enable();

    // Update
    this->GetDocument()->m_BL_Video.m_videoChannel = GetRibbonEdit(ID_QUAD_BL_CHANNEL);
}
void QuadView::OnUpdateQuadBlResolution(CCmdUI* pCmdUI)
{
    // Enable
    pCmdUI->Enable();

    // Update
    this->GetDocument()->m_BL_Video.m_resolution = GetRibbonEdit(ID_QUAD_BL_RESOLUTION);
}
void QuadView::OnUpdateQuadBrIp(CCmdUI* pCmdUI)
{
    // Enable
    pCmdUI->Enable();

    // Update
    this->GetDocument()->m_BR_Video.m_videoIP = GetRibbonEdit(ID_QUAD_BR_IP);
}
void QuadView::OnUpdateQuadBrChannel(CCmdUI* pCmdUI)
{
    // Enable
    pCmdUI->Enable();

    // Update
    this->GetDocument()->m_BR_Video.m_videoChannel = GetRibbonEdit(ID_QUAD_BR_CHANNEL);
}
void QuadView::OnUpdateQuadBrResolution(CCmdUI* pCmdUI)
{
    // Enable
    pCmdUI->Enable();

    // Update
    this->GetDocument()->m_BR_Video.m_resolution = GetRibbonEdit(ID_QUAD_BR_RESOLUTION);
}
void QuadView::OnUpdateQuadviewName(CCmdUI* pCmdUI)
{
    // Enable
    pCmdUI->Enable();

    // Update
    this->GetDocument()->m_viewName = GetRibbonEdit(ID_QUADVIEW_NAME);
}


/* START INDIVIDUAL STREAMS BUTTON HANDLER */
void QuadView::OnQuadConnectTl()
{
    std::cout << "BEGINING STREAM 1 VIDEO THREAD NOW!" << std::endl;
    this->GetDocument()->m_TL_Video.m_videoChannel = GetRibbonEdit(ID_QUAD_TL_CHANNEL);
    StartStream(0, this->GetDocument()->m_TL_Video.m_videoIP, this->GetDocument()->m_TL_Video.m_videoChannel);
}

void QuadView::OnStartTlVid()
{
    std::cout << "BEGINING STREAM 2 VIDEO THREAD NOW!" << std::endl;
    this->GetDocument()->m_TR_Video.m_videoChannel = GetRibbonEdit(ID_QUAD_TR_CHANNEL);
    StartStream(1, this->GetDocument()->m_TR_Video.m_videoIP, this->GetDocument()->m_TR_Video.m_videoChannel);
}
void QuadView::OnQuadConnectBl()
{
    std::cout << "BEGINING STREAM 3 VIDEO THREAD NOW!" << std::endl;
    this->GetDocument()->m_BL_Video.m_videoChannel = GetRibbonEdit(ID_QUAD_BL_CHANNEL);
    StartStream(2, this->GetDocument()->m_BL_Video.m_videoIP, this->GetDocument()->m_BL_Video.m_videoChannel);
}
void QuadView::OnQuadConnectBr()
{
    std::cout << "BEGINING STREAM 4 VIDEO THREAD NOW!" << std::endl;
    this->GetDocument()->m_BR_Video.m_videoChannel = GetRibbonEdit(ID_QUAD_BR_CHANNEL);
    StartStream(3, this->GetDocument()->m_BR_Video.m_videoIP, this->GetDocument()->m_BR_Video.m_videoChannel);
}


/* GIVE THE VIEW A CUSTOM NAME */
void QuadView::OnQuadviewAddname()
{
    // SET WINDOW NAME
    std::cout << "SETTING QUAD VIEW WINDOW NAME NOW" << std::endl;
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

/* FOR HANDLING MOUSE WHEEL UP OR DOWN EVENT */
BOOL QuadView::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_MOUSEWHEEL)
    {
        // Call OnMouseWheel handler logic here
        CPoint point(LOWORD(pMsg->lParam), HIWORD(pMsg->lParam));
        ScreenToClient(&point); // Convert to client coordinates

        // Extract zDelta from wParam (high-order word)
        short zDelta = HIWORD(pMsg->wParam);

        // Implement mouse wheel logic
        CRect rc;
        GetClientRect(&rc);

        // IDENTIFY LEFT OR RIGHT PART OF SCREEN
        if (point.x < (rc.Width() / 2)) {
            // LEFT SIDE OF WINDOW
            std::cout << "LEFT SIDE" << std::endl;

        }
        else {
            //RIGHT SIDE OF WINDOW
            std::cout << "RIGHT SIDE" << std::endl;
        }

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


/* CONTEXT MENU HANDLERS FOR MANIPULATING THE WINDOW */
void QuadView::OnFillWindow()
{
    std::cout << "FILL ENTIRE WINDOW" << std::endl;
    try {
        CFrameWnd* pFrame = GetParentFrame();
        CChildFrame* pChildFrame = dynamic_cast<CChildFrame*>(pFrame);
        pChildFrame->FillWindow();
    }
    catch (const std::runtime_error& e) {
        std::cerr << "CCAMERACONTROLVIEW::ONFILLWINDOW::::EXCEPTION! " << e.what() << std::endl;
    }
}
void QuadView::OnRestoreView()
{
    std::cout << "RESTORE CHILD WINDOW" << std::endl;
    // Cast it to your specific child frame class
    CFrameWnd* pFrame = GetParentFrame();
    CChildFrame* pChildFrame = dynamic_cast<CChildFrame*>(pFrame);
    pChildFrame->RestoreFrame();
}

/// <summary>
/// Handles determining window sizes.
/// The _size can be one of the enum types.
/// </summary>
/// <param name="_size"></param>
void QuadView::GetSizes(const SIZES& _size, const unsigned int& _cL, const unsigned int& _cW) const noexcept
{
    // ASSERT STATEMENT TO CATCH LOGICAL ERRORS
    assert(_size <= MAX_SIZES, "SIZE IS INVALID! TOO LARGE");

    // SWITCH CASE TO DETERMINE CORRECT SIZES
    switch (_size) {
    case SINGLE:
        SetSingle(_cL, _cW);
        break;
    case DOUBLE:
        SetDouble(_cL, _cW);
        break;
    case QUAD:
        SetQuad(_cL, _cW);
        break;
    case TRI_BIG_TOP:
        SetTriTop(_cL, _cW);
        break;
    case TRI_BIG_BOTTOM:
        SetTriBottom(_cL, _cW);
        break;
    case TRI_RIGHT_BIG:
        SetTriRight(_cL, _cW);
        break;
    case TRI_LEFT_BIG:
        SetTriLeft(_cL, _cW);
        break;
    default:
        std::cout << "QUAD::VIEW=ERROR COULD NOT DETERMINE SIZE. THIS IS A DISASTER. END ALL VIDEO THREADS!" << std::endl;
    }
}


void QuadView::SetSingle(const unsigned int& _cL, const unsigned int& _cW) const noexcept
{

}
void QuadView::SetQuad(const unsigned int& _cL, const unsigned int& _cW) const noexcept
{

}
void QuadView::SetDouble(const unsigned int& _cL, const unsigned int& _cW) const noexcept
{

}
void QuadView::SetTriTop(const unsigned int& _cL, const unsigned int& _cW) const noexcept
{

}
void QuadView::SetTriBottom(const unsigned int& _cL, const unsigned int& _cW) const noexcept
{

}
void QuadView::SetTriLeft(const unsigned int& _cL, const unsigned int& _cW) const noexcept
{

}
void QuadView::SetTriRight(const unsigned int& _cL, const unsigned int& _cW) const noexcept
{

}