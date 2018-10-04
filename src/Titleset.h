/////////////////////////////////////////////////////////////////////////////
// Name:        Titleset.h
// Author:      Alex Thuering
// Created:	    29.01.2003
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef TITLESET_H
#define TITLESET_H

#include "SubStream.h"
#include "Pgc.h"

/** Video parameters of titleset */
class Video {
public:
	/** Constructor */
	Video(): m_format(vfCOPY), m_aspect(arAUTO), m_widescreen(wtNOPANSCAN) {}

	void SetFormat(VideoFormat value) { m_format = value; }
	VideoFormat GetFormat() { return m_format; }
	
	void SetAspect(AspectRatio value) { m_aspect = value; }
	AspectRatio GetAspect() { return m_aspect; }
	wxArrayString GetAspectStrings(wxString def = wxT(""));
	
	void SetResolution(wxString value) { m_resolution = value; }
	wxString GetResolution() { return m_resolution; }
	
	void SetCaption(wxString value) { m_caption = value; }
	wxString GetCaption() { return m_caption; }
	
	void SetWidescreen(WidescreenType value) { m_widescreen = value; }
	WidescreenType GetWidescreen() { return m_widescreen; }
	wxArrayString GetWidescreenStrings(wxString def = wxT(""));
	
	wxSvgXmlNode* GetXML(DVDFileType type, bool menu);
	bool PutXML(wxSvgXmlNode* node, bool menu);
	
private:
	VideoFormat m_format;
	AspectRatio m_aspect;
	wxString m_resolution;
	wxString m_caption;
	WidescreenType m_widescreen;
};

/**
 * Stores information about subpictures
 */
class SubPicture {
public:
	/** Constructor */
	SubPicture() { }
	/** Constructor */
	SubPicture(wxString langCode): m_langCode(langCode) { }
	/** Destructor */
	~SubPicture();
	
	/** Returns language code */
	wxString GetLangCode() { return m_langCode; }
	/** Sets language code */
	void SetLangCode(wxString value) { m_langCode = value; }
	
	/** Returns list of subpicture streams */
	vector<SubStream*>& GetStreams() { return m_streams; }
	
	wxSvgXmlNode* GetXML(DVDFileType type, Video& video, bool menu);
	bool PutXML(wxSvgXmlNode* node);
	
private:
	wxString m_langCode;
	vector<SubStream*> m_streams;
};

class PgcArray: public PgcArrayBase {
public:
	/** Destructor */
	~PgcArray();
	
	/** Returns list of audio codes */
	wxArrayString& GetAudioLangCodes() { return m_audioLangCodes; }
	/** Retuens list of subpictures */
	vector<SubPicture*>& GetSubPictures() { return m_subPictures; }
	/** Returns video parameters */
	Video& GetVideo() { return m_video; }
	
	/** Returns count of audio streams */
	unsigned int GetAudioStreamCount();
	/** Returns count of subtitle streams */
	unsigned int GetSubtitleStreamsCount();
	
	/** Sets video format */
	void SetVideoFormat(VideoFormat videoFormat);
	/** Sets aspect ratio */
	void SetAspectRatio(AspectRatio aspectRatio);
	/** Sets post commands */
	void SetPostCommands(wxString postCommands);
	
	/** Returns true if at least one title use "call last menu" command */
	bool HasCallLastMenu();
protected:
	wxArrayString m_audioLangCodes;
	vector<SubPicture*> m_subPictures;
	Video m_video;
	void UpdateAudioLangCodes();
	void UpdateSubtitleLangCodes();
};

class Menus: public PgcArray {
public:
	StringSet GetEntries();
	int GetPgciByEntry(wxString entry);
	
	wxSvgXmlNode* GetXML(DVDFileType type, DVD* dvd, bool vmgm = false);
	bool PutXML(wxSvgXmlNode* node, DVD* dvd, int tsi, wxProgressDialog* progressDlg);
};

class Titles: public PgcArray {
public:
	wxSvgXmlNode* GetXML(DVDFileType type, DVD* dvd, int nextTitleset);
	bool PutXML(wxSvgXmlNode* node, DVD* dvd, int tsi, wxProgressDialog* progressDlg);
};

/**
 * Stores information about DVD titleset
 */
class Titleset {
public:
	Titleset() { WX_CLEAR_ARRAY(m_menus); WX_CLEAR_ARRAY(m_titles); }
	
	Menus& GetMenus() { return m_menus; }
	Titles& GetTitles() { return m_titles; }
	
	inline bool IsEmpty() { return GetMenus().Count() == 0 && GetTitles().Count() == 0; }
	
	wxSvgXmlNode* GetXML(DVDFileType type, DVD* dvd, int nextTitleset = -1);
	bool PutXML(wxSvgXmlNode* node, DVD* dvd, int tsi, wxProgressDialog* progressDlg);
	
private:
	Menus m_menus;
	Titles m_titles;
};

class Vmgm: public Menus {
public:
	/** Returns the fpc (First Program Chain) commands */
    wxString GetFpc() { return m_fpc; }
    /** Sets the FPC (First Program Chain) commands */
    void SetFpc(wxString fpc) { m_fpc = fpc; }
	
    wxSvgXmlNode* GetXML(DVDFileType type, DVD* dvd);
	bool PutXML(wxSvgXmlNode* node, DVD* dvd, wxProgressDialog* progressDlg);
	
private:
	wxString m_fpc;
};

WX_DEFINE_ARRAY(Titleset*, TitlesetArray);

#endif // TITLESET_H
