#pragma once


#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_6.h> // Shared infrastructure for both
#include <array>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <windows.h>
#include "Utilities.h"
#include "UniViewDoc.h"

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


class UniView : public CFormView
{
protected: // create from serialization only
	UniView() noexcept;
	DECLARE_DYNCREATE(UniView)

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UniView_DLG };
#endif

	// Attributes
public:
	UniViewDoc* GetDocument();

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
	virtual ~UniView();
	void BeginVideoShutdown();

	HWND m_hwnd{ nullptr };      // cached window handle for safe posting
	CString m_wndName;

private:
	// HW ACCELERATION REFERENCE
	AVBufferRef* m_hwDeviceCtx = nullptr;
	static enum AVPixelFormat GetHwFormat(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts);

	// --- NEW: Four-stream support ---
	static constexpr int NUM_STREAMS = 4;
	enum : int { TL = 0, TR = 1, BL = 2, BR = 3 };

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

	// Worker entry
	void WorkerProc(int idx, CString url);

	// Post: WM_NEWFRAME with stream index in wParam
	static constexpr UINT WM_NEWFRAME = WM_USER + 101;

	// Overwrite your old single-stream fields that are no longer needed
	// OR keep them if used elsewhere; but they’re not used for quad rendering now.

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
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnFillWindow();
	afx_msg void OnRestoreView();
};
