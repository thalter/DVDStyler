/////////////////////////////////////////////////////////////////////////////
// Name:        VobListBox.h
// Purpose:     The list box to display information about streams in given VOB
// Author:      Alex Thuering
// Created:     03.05.2009
// RCS-ID:      $Id: VobListBox.h,v 1.15 2016/05/11 20:02:37 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////
 
#ifndef VOB_LIST_BOX_H
#define VOB_LIST_BOX_H

#include <wx/wx.h>
#include <wx/vlbox.h>
#include <wx/choice.h>
#include <wx/dynarray.h>
#include "DVD.h"
#include <vector>

WX_DECLARE_OBJARRAY(wxRect, RectList);
WX_DECLARE_OBJARRAY(RectList, RectListOfList);
WX_DECLARE_OBJARRAY(wxArrayString, StringListOfList);
class TitlePropDlg;

/**
 * The list box to display information about streams in given VOB.
 */
class VobListBox: public wxVListBox {
public:
	/**
	 * Constructor
	 */
	VobListBox(wxScrolledWindow* parent, wxWindowID id, Vob* vob, PgcArray* pgcs, DVD* dvd, TitlePropDlg* titlePropDlg);
	
	/**
	 * Returns the height of the specified item (in pixels)
	 */
	wxCoord OnMeasureItem(size_t n) const;
	
	/**
	 * Draws the item with the given index on the provided DC
	 */
	void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const;
	
	/**
	 * Removes item with given index
	 */
	void RemoveItem(int index);
	
	/**
	 * Adds audio file to the vob
	 */
	void AddAudio(wxString filename);
	
	/**
	 * Adds subtitle file to the vob
	 */
	void AddSubtitle(wxString filename);
	
	/**
	 * Disables transcoding: sets Copy for all streams and disables choice controls
	 */
	void SetDoNotTranscode(bool value);
	
	/**
	 * Returns true if "do not transcode active ist"
	 */
	bool GetDoNotTranscode();
	
	/**
	 * Returns true if this VOB contains audio files
	 */
	bool HasAudioFiles();
	
	/**
	 * Updates VOB and PGC objects with values from controls
	 */
	void SetValues();
	
	/**
	 * Returns selected aspect ratio
	 */
	AspectRatio GetAspectRatio() { return m_aspectRatio; }
	
	/**
	 * Shows properties dialog for selected item
	 */
	void ShowPropDialog();
	
private:
	/** TitlePropDlg */
	TitlePropDlg* m_titlePropDlg;
	/** The DVD */
	DVD* m_dvd;
	/** Copy of VOB object to edit */
	Vob* m_vob;
	/** Arrays of PGCs */
	PgcArray* m_pgcs;
	/** selected aspect ratio */
	AspectRatio m_aspectRatio;
	/** The list of information string list for every file in VOB */
	StringListOfList m_info;
	/** The list of rectangle list. Each rectangle stores coordinates and size of one information string. */
	RectListOfList m_infoRect;
	/** The list of choice controls */
	vector<wxChoice*> m_choiceList;
	/** The list of buttons */
	vector<wxButton*> m_buttonList;
	/** The list of stream indexes for buttons */
	vector<int> m_buttonStreamList;
	/** Index of first choice for audio file */
	int m_audioChoiceIdx;
	/** Index of first choice for subtitle */
	int m_subtitleChoiceIdx;
	
	wxBitmap m_videoImg;
	wxBitmap m_audioImg;
	wxBitmap m_subtitleImg;
	
	wxImage LoadFrame(const wxString& filename) const;
	wxBitmap Scale(wxImage image);
	void RefreshInfo();
	void UpdateDoNotTranscodeCheck();
	
	/**
	 * Adds given string m_info[n] and coordinates/size to m_infoRect[n]
	 * @param s The information string
	 * @param n The list item index
	 * @param dc The device context
	 * @param x The x-coordinate of info string
	 * @param y The y-coordinate of info string
	 * @returns The y-coordinate of new line
	 */
	int AddInfo(const wxString& s, int n, wxDC& dc, int x, int y);
	/** Creates choice control and adds it to the list m_choiceList */
	void AddChoiceCtrl(wxArrayString formats, int selection, int x, int y, int& choiceIdx, bool enabled);
	/** Increase y and height if height of control is greater that actual height */
	int UpdateRect(wxRect& rect, wxControl* ctrl);
	/** Creates button and adds it to the list m_buttonList */
	void AddButton(int x, int y, int& buttonIdx, bool enabled, int streamIdx);
	/** Returns index of choice control for given stream */
	int GetChoiceIdx(unsigned int streamIdx);
	/** Returns index of stream for given choice control */
	int GetStreamIdx(wxChoice* choice);
	/** Returns index of button for given stream */
	int GetButtonIdx(unsigned int streamIdx);
	/** Gets video format index */
	int GetVideoFormatIdx(Stream* stream, VideoFormat videoFormat);
	/** Sets video format */
	void SetVideoFormat(unsigned int streamIdx, int videoFormat);
	/** Get video format */
	int GetVideoFormat(unsigned int streamIdx);
	/** Get audio format */
	int GetAudioFormat(unsigned int streamIdx);
	/** Sets audio format */
	void SetAudioFormat(unsigned int streamIdx, int audioFormat);
	/** Get audio language code */
	wxString GetAudioLangCode(unsigned int streamIdx);
	/** Sets audio language code */
	void SetAudioLangCode(unsigned int streamIdx, wxString langCode);
	/** Get subtitle language code */
	wxString GetSubtitleLangCode(int subtitleIndex);
	/** Sets subtitle language code */
	void SetSubtitleLangCode(int subtitleIndex, wxString langCode);
	/** Shows subtitle properties dialog */
	void ShowPropDialog(unsigned int streamIdx);

	/** Processes a double click event */
	void OnDoubleClick(wxMouseEvent& evt);
	/** Processes a button event */
	void OnButton(wxCommandEvent& event);
	/** Processes a format change event */
	void OnFormatChange(wxCommandEvent& event);

	/** Event table */
	DECLARE_EVENT_TABLE()
};

#endif // VOB_LIST_BOX_H
