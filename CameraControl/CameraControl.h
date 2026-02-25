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

// CameraControl.h : main header file for the CameraControl application
//
#pragma once
#define _WINSOCKAPI_
#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
extern "C" {
#   include <libavformat/avformat.h>
#   include <libavcodec/avcodec.h>
#   include <libavutil/imgutils.h>
#   include <libavutil/pixfmt.h>         // AVColorSpace, AVColorRange, AVPixelFormat
#   include <libswscale/swscale.h>
}

#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream> // For std::cout if you use it
#include "Utilities.h"
#include <vector>


#ifndef SWS_CS_BT709
#   define SWS_CS_BT709 1
#endif
#ifndef SWS_CS_SMPTE170M
#   define SWS_CS_SMPTE170M 5
#endif

// CCameraControlApp:
// See CameraControl.cpp for the implementation of this class
//

class CCameraControlApp : public CWinAppEx
{
public:
	CCameraControlApp() noexcept;
	void CreateConsole();
	void MakeConsoleCornersSquare();
	ULONG_PTR m_gdiplusToken;
	Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;


public:
	CMultiDocTemplate* m_singleView = nullptr;
	CMultiDocTemplate* m_quadView = nullptr;
	CMultiDocTemplate* m_ptzView = nullptr;
	CMultiDocTemplate* m_playbackView = nullptr;
	CMultiDocTemplate* m_markerView = nullptr;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	std::vector<CString> ListChildNames_ByViewClass();

// Implementation
	UINT  m_nAppLook;
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CCameraControlApp theApp;
