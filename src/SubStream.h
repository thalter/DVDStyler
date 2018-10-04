/////////////////////////////////////////////////////////////////////////////
// Name:        SubPicture.h
// Author:      Alex Thuering
// Created:	    24.02.2018
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef SUB_STREAM_H
#define SUB_STREAM_H

#include "DVDAction.h"
#include <vector>

using namespace std;

enum SubStreamMode {
	ssmNORMAL = 0,
	ssmWIDESCREEN,
	ssmLETTERBOX,
	ssmPANSCAN
};

enum SubStreamContent {
	sscNORMAL = 0, sscLARGE, sscCHILDREN, sscNORMAL_CC, sscLARGE_CC, sscCHILDREN_CC, sscFORCED, sscDIRECTOR,
	sscLARGE_DIRECTOR, sscCHILDREN_DIRECTOR
};

/**
 * Stores information about subpicture stream
 */
class SubStream {
public:
	/** Default constructor */
	SubStream(): m_mode(ssmNORMAL), m_content(sscNORMAL) {}
	/** Constructor */
	SubStream(wxString id, SubStreamMode mode): m_id(id), m_mode(mode), m_content(sscNORMAL) {}
	
	/** Sets id */
	void SetId(wxString id) { m_id = id; }
	/** Returns id */
	wxString GetId() { return m_id; }
	
	/** Sets mode */
	void SetMode(SubStreamMode mode) { m_mode = mode; }
	/** Returns mode */
	SubStreamMode GetMode() { return m_mode; }
	
	/** Sets content type */
	void SetContent(SubStreamContent content) { m_content = content; }
	/** Returns content type */
	SubStreamContent GetContent() { return m_content; }
	
	wxSvgXmlNode* GetXML(DVDFileType type);
	bool PutXML(wxSvgXmlNode* node);
		
private:
	wxString m_id;
	SubStreamMode m_mode;
	SubStreamContent m_content;
};

#endif // SUB_STREAM_H
