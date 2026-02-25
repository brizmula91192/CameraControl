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

// CameraControlView.h : interface of the CCameraControlView class
//

#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>
#include <windows.h>

class CCameraControlView : public CFormView
{
protected: // create from serialization only
	CCameraControlView() noexcept;
	DECLARE_DYNCREATE(CCameraControlView)

public:
#ifdef AFX_DESIGN_TIME
	enum{ IDD = IDD_CAMERACONTROL_FORM };
#endif

/* PRIVATE MEMBERS FOR HANDLING GUI STATES */
private:
	BOOL m_toolBarState = FALSE;
	BOOL m_ptzONOFF = TRUE;

// Attributes
public:
	CCameraControlDoc* GetDocument() const;

// Operations
public:
	CMainFrame* m_mainFrame;
	virtual BOOL OnEraseBkgnd(CDC* pDC);


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
	virtual ~CCameraControlView();
	void BeginVideoShutdown();
	int m_bgraStride = 0;
	void Stop();
	// View-size buffer (what we actually blit)
	std::vector<uint8_t> m_viewBgra;
	BITMAPINFO           m_viewBmi{};
	int                  m_viewStride = 0;
	int                  m_viewW = 0, m_viewH = 0;

	// Scaler for view-size conversion
	SwsContext* m_swsView = nullptr;
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

	std::atomic<bool> m_needRepaint{ false };     // "a repaint is pending" flag
	size_t m_lastDrawn = static_cast<size_t>(-1); // last painted frame index (UI thread only)

	std::unique_ptr<Gdiplus::Font> m_pRecFont;
	std::unique_ptr<Gdiplus::SolidBrush> m_pRedBrush;
	std::unique_ptr<Gdiplus::SolidBrush> m_pWhiteBrush;
	void InitGdiResources();

	void WorkerProc(CString url);
	void StartVideo(const CString& ip, const CString& user, const CString& pass, const CString& chan, const CString& res,
		const CString& decoder_opts, const CString& frameRate);
	void InitBuffer();
	void PreAllocateRingBuffer();
	BOOL m_isRecording = FALSE;
	BOOL m_isPaused = FALSE;

	// RING BUFFER VARIABLES
	struct VideoFrame {
		std::vector<uint8_t> pixels;
		BITMAPINFO bmi;
		int stride = 0;
		int w = 0, h = 0;
	};
	static const int RING_BUFFER_SIZE = 4;
	VideoFrame m_ring[RING_BUFFER_SIZE];
	std::atomic<size_t> m_writeIdx{ 0 }, m_readIdx{ 0 };
	std::atomic<int> m_targetW{ 0 }, m_targetH{ 0 };


	// FFmpeg contexts (owned by worker thread)
	AVFormatContext* m_fmt = nullptr;
	AVCodecContext* m_dec = nullptr;
	SwsContext* m_sws = nullptr;
	int              m_vindex = -1;
	CString m_wndName;
	void DrawRecOverlay(VideoFrame& slot);
	void DrawPauseOverlay(VideoFrame& slot);

	// Frame storage for painting
	std::mutex           m_frameMtx;
	std::vector<uint8_t> m_bgra;     // top-down BGRA pixels
	BITMAPINFO           m_bmi{};
	int                  m_w = 0, m_h = 0;

	// Thread control
	std::thread       m_thr;
	std::atomic<bool> m_run{ false };
	HWND m_hwnd{ nullptr };      // cached window handle for safe posting


	// FFmpeg interrupt callback
	static int AvInterrupt(void* opaque);

	// Join With Timeout
	template<class Rep, class Period>
	static bool JoinWithTimeout(std::thread& t, std::chrono::duration<Rep, Period> d)
	{
		HANDLE h = (HANDLE)t.native_handle();
		DWORD ms = (DWORD)std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
		DWORD r = WaitForSingleObject(h, ms);
		if (r == WAIT_OBJECT_0) { t.join(); return true; }
		return false;
	}



	// Message id to trigger repaint
	static constexpr UINT WM_NEWFRAME = WM_USER + 101;
public:
	afx_msg void OnUpdateSinglecamIp(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSinglecamChannel(CCmdUI* pCmdUI);
	afx_msg void OnConnectSingleView();

	afx_msg void OnUpdateSinglecamAudioIp(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLeftOverlay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRightOverlay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSinglecamName(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSinglecamaudioIp(CCmdUI* pCmdUI);
	afx_msg void OnAddSingleviewName();
	afx_msg void OnUpdateSingleviewResolution(CCmdUI* pCmdUI);
	afx_msg void OnPauseRecording();
	afx_msg void OnStopRecording();
	afx_msg void OnSnapShot();
	afx_msg void OnUpdateSingleviewCompression(CCmdUI* pCmdUI);
	afx_msg void OnFullscreen();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSingleviewRmOverlays();
	afx_msg void OnSingleviewAddROverlay();
	afx_msg void OnSingleviewAddLOverlay();
	afx_msg void OnUpdateSingleviewPtzOnoff(CCmdUI* pCmdUI);
	afx_msg void OnSingleviewPtzOnoff();
	afx_msg void OnRestoreView();
	afx_msg void OnFillWindow();
	afx_msg void OnHideRibbon();

	afx_msg void OnRecordVidAudio();
	afx_msg void OnRecordVideo();
	afx_msg void OnUpdateRecordVideo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRecordVidAudio(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePauseRecording(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStopRecording(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSingleviewResume(CCmdUI* pCmdUI);
	afx_msg void OnSingleviewResume();
};

#ifndef _DEBUG  // debug version in CameraControlView.cpp
inline CCameraControlDoc* CCameraControlView::GetDocument() const
   { return reinterpret_cast<CCameraControlDoc*>(m_pDocument); }
#endif

