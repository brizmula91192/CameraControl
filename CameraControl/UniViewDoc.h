#pragma once



class UniViewDoc : public CDocument
{
protected: // create from serialization only
	UniViewDoc() noexcept;
	DECLARE_DYNCREATE(UniViewDoc)

	// Attributes
public:

	// Operations
public:

	// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

	// Implementation
public:
	virtual ~UniViewDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

/* CAMERA VARIABLES */
public:
	/* Video Structs */
	struct m_Video {
		CString m_videoIP = _T("NA");
		CString m_videoChannel = _T("1");
		CString m_resolution = _T("1080");
		int m_cropStype;
	};

	m_Video m_TL_Video;
	m_Video m_TR_Video;
	m_Video m_BL_Video;
	m_Video m_BR_Video;

	// Declare


	CString m_viewName = _T("");

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};