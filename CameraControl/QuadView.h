#pragma once

#include <array>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <windows.h>
#include "Utilities.h"
#include "QuadViewDoc.h"

/* FFMPEG LIBRARIES */
extern "C" {
#   include <libavformat/avformat.h>
#   include <libavcodec/avcodec.h>
#   include <libavutil/imgutils.h>
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


typedef int SIZES;


class QuadView : public CFormView
{
protected: // create from serialization only
	QuadView() noexcept;
	DECLARE_DYNCREATE(QuadView)

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_QUADVIEW_DLG };
#endif

	// Attributes
public:
	QuadViewDoc* GetDocument();

	// Operations
public:
	CMainFrame* m_mainFrame = nullptr;


	// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnDraw(CDC* pDC) override;
	afx_msg void OnDestroy();
	afx_msg LRESULT OnNewFrame(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBeginVideoShutdown(WPARAM, LPARAM);
	CString GetRibbonEdit(UINT nID);

	// Implementation
public:
	virtual ~QuadView();
	void BeginVideoShutdown();

	HWND m_hwnd{ nullptr };      // cached window handle for safe posting
	CString m_wndName;

private:
	/// <summary>
	/// Main get sizes function. This sizes the video streams
	/// based on the size of the child window and if toolbar is 
	/// visible. This is no except!
	/// </summary>
	/// <param name="_size"></param>
	/// <param name="_cL"></param>
	/// <param name="_cW"></param>
	void GetSizes(const SIZES& _size = SINGLE, const unsigned int& _cL, const unsigned int& _cW) const noexcept;

	/// <summary>
	/// 
	/// </summary>
	/// <param name="_cL"></param>
	/// <param name="_cW"></param>
	inline void SetSingle(const unsigned int& _cL, const unsigned int& _cW) const noexcept;

	/// <summary>
	/// 
	/// </summary>
	/// <param name="_cL"></param>
	/// <param name="_cW"></param>
	inline void SetQuad(const unsigned int& _cL, const unsigned int& _cW) const noexcept;
	inline void SetDouble(const unsigned int& _cL, const unsigned int& _cW) const noexcept;
	inline void SetTriTop(const unsigned int& _cL, const unsigned int& _cW) const noexcept;
	inline void SetTriBottom(const unsigned int& _cL, const unsigned int& _cW) const noexcept;
	inline void SetTriLeft(const unsigned int& _cL, const unsigned int& _cW) const noexcept;
	inline void SetTriRight(const unsigned int& _cL, const unsigned int& _cW) const noexcept;

	// TOOL BAR SIZES
	std::atomic<int> targetW_tool{ 0 }, targetH_tool{ 0 };

	// HW ACCELERATION REFERENCE
	AVBufferRef* m_hwDeviceCtx = nullptr;
	static enum AVPixelFormat GetHwFormat(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts);

	// --- NEW: Four-stream support ---
	static constexpr int NUM_STREAMS = 4;
	enum : int { TL = 0, TR = 1, BL = 2, BR = 3 };
	static constexpr unsigned int MAX_SIZES = 6;
	enum : int { SINGLE = 0, QUAD = 1, DOUBLE = 2, TRI_BIG_TOP = 3, TRI_BIG_BOTTOM = 4,
				 TRI_LEFT_BIG = 5, TRI_RIGHT_BIG = 6 };
	

	struct VideoFrame {
		std::vector<uint8_t> pixels;
		BITMAPINFO bmi{};
		int stride = 0;
		int w = 0, h = 0;
	};

	struct StreamCtx {
		// thread + run flag
		std::thread thr;
		std::atomic<bool> run{ false };

		// ring buffer
		static constexpr int RING_BUFFER_SIZE = 4;
		std::array<VideoFrame, RING_BUFFER_SIZE> ring{};
		std::atomic<size_t> writeIdx{ 0 }, readIdx{ 0 };

		// per-stream FFmpeg
		AVFormatContext* fmt = nullptr;
		AVCodecContext* dec = nullptr;
		SwsContext* swsFmtToBgra = nullptr; // source->BGRA (src size)
		SwsContext* swsView = nullptr;      // BGRA -> quadrant size
		int vindex = -1;

		// desired output size for this stream (quadrant)
		std::atomic<int> targetW{ 0 }, targetH{ 0 };

		// optional: name/label for debugging
		CString name;
	};

	// One window, four streams
	std::array<StreamCtx, NUM_STREAMS> m_stream{};

	// Cached quadrant rectangles
	CRect m_quadrant[NUM_STREAMS];

	// Identify which stream asked for the current paint
	std::atomic<int> m_lastPaintStream{ -1 };

	// Helpers
	void StartStream(int idx, const CString& ip, const CString& chan);
	void StopStream(int idx);
	void StopAll();
	void LayoutQuadrants(); // recompute m_quadrant[*] on size

	// DECODING THREAD
	void WorkerProc(int idx, CString url);

	// Post: WM_NEWFRAME with stream index in wParam
	static constexpr UINT WM_NEWFRAME = WM_USER + 101;

	// Variables for view type
	

public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	inline void SetRibbon(CMFCRibbonEdit* pRibbon, const CString& val, const UINT ID);
	void PreallocateRingBuffersForStream(int idx, int maxW, int maxH);




	// FFmpeg interrupt callback
	static int AvInterrupt(void* opaque);

	// Message id to trigger repaint
	static constexpr UINT WM_NEWFRAME_QUAD = WM_USER + 1089;
public:
	afx_msg void OnConnectSingleView();
	afx_msg void OnUpdateQuadTlIp(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuadTlChannel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuadTlResolution(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuadTrIp(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuadTrChannel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuadTrResolution(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuadBlIp(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuadBlChannel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuadBlResolution(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuadBrIp(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuadBrChannel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateQuadBrResolution(CCmdUI* pCmdUI);
	afx_msg void OnQuadConnectTl();
	afx_msg void OnStartTlVid();
	afx_msg void OnQuadConnectBl();
	afx_msg void OnQuadConnectBr();
	afx_msg void OnUpdateQuadviewName(CCmdUI* pCmdUI);
	afx_msg void OnQuadviewAddname();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	
	afx_msg void OnFillWindow();
	afx_msg void OnRestoreView();
};
