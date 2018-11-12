/////////////////////////////////////////////////////////////////////////////
// Name:        Pgc.h
// Purpose:     The class to store a PGC
// Author:      Alex Thuering
// Created:	    29.01.2003
// RCS-ID:      $Id: Pgc.h,v 1.24 2015/09/19 17:20:01 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef PGC_H
#define PGC_H

#include "Vob.h"
#include <set>

class PgcArray;
class wxProgressDialog;
#define StringSet std::set<wxString>

class Pgc {
public:
	Pgc() { }
	~Pgc() { WX_CLEAR_ARRAY(m_vobs) }
	
	wxString GetEntriesAsString(bool vmgm = false);
	StringSet& GetEntries() { return m_entries; }
	void SetEntries(const StringSet& entries) { m_entries = entries; }
	void RemoveEntries(const StringSet& entries);
	
	void SetPreCommands(wxString value) { m_pre = value; }
	wxString GetPreCommands() { return m_pre; }
	void SetPostCommands(wxString value) { m_post = value; }
	wxString GetPostCommands() { return m_post; }
	void SetPalette(wxString value) { m_palette = value; }
	wxString GetPalette() { return m_palette; }
	
	VobArray& GetVobs() { return m_vobs; }
	
	/** Returns menu */
	Menu* GetMenu();
	/** Returns slideshow */
    Slideshow* GetSlideshow();
    
    /** Returns count of chapters in the title */
    unsigned int GetChapterCount(int lastVobi = -1);
	/** Returns URI of video frame */
	wxString GetVideoFrameUri(int chapter, long position = -1, bool fileNameOnly = false);
	/** Returns true if "call last menu;" is used in cell commands */
	bool IsCellsUseCallLastMenu();
	
	wxSvgXmlNode* GetXML(DVDFileType type, DVD* dvd, PgcArray& pgcs, bool vmgm = false, int nextTitle = -1,
			int nextTitleset = -1);
	bool PutXML(wxSvgXmlNode* node, DVD* dvd, int tsi, int pgci, bool menu, wxProgressDialog* progressDlg);
	
private:
	VobArray m_vobs;
	StringSet m_entries;
	wxString m_pre, m_post, m_palette;
};

WX_DEFINE_ARRAY(Pgc*, PgcArrayBase);

#endif // PGC_H
