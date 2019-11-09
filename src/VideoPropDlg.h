/////////////////////////////////////////////////////////////////////////////
// Name:        VideoPropDlg.h
// Purpose:     The video properties dialog
// Author:      Alex Thuering
// Created:     09.09.2010
// RCS-ID:      $Id: VideoPropDlg.h,v 1.13 2014/09/27 06:00:25 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////
#ifndef VIDEO_PROP_DLG_H
#define VIDEO_PROP_DLG_H

#include "Pgc.h"
#include "mediactrl_ffmpeg.h"

//(*Headers(VideoPropDlg)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/slider.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/gbsizer.h>
#include <wx/dialog.h>
#include <wx/spinbutt.h>
//*)

class VideoPropDlg: public wxDialog {
public:
	/** Constructor */
	VideoPropDlg(wxWindow* parent, DVD* dvd, Vob* vob, Stream* stream, AspectRatio aspectRatio);
	virtual ~VideoPropDlg();

    /** Displays dialog */
	virtual int ShowModal();

	/** Returns selected video format */
    VideoFormat GetVideoFormat();

	/** Returns selected aspect ratio */
    AspectRatio GetAspectRatio();

private:
	//(*Declarations(VideoPropDlg)
	wxTextCtrl* m_filtersCtrl;
	wxStaticText* m_durText;
	wxCheckBox* m_keepAspectCtrl;
	wxChoice* m_interlacedChoice;
	wxSlider* m_slider;
	wxSpinButton* m_frameSpinBt;
	wxSpinCtrl* m_cropBottom;
	wxStaticBoxSizer* m_padBox;
	wxSpinCtrl* m_padLeft;
	wxSpinButton* m_startFrameSpin;
	wxTextCtrl* m_startCtrl;
	wxSpinCtrl* m_cropTop;
	MediaCtrlFF* m_mediaCtrl;
	wxSpinButton* m_endSpin;
	wxSpinCtrl* m_fadeOutCtrl;
	wxSpinCtrl* m_fadeInCtrl;
	wxChoice* m_dstChoice;
	wxTextCtrl* m_timeCtrl;
	wxBitmapButton* m_startBt;
	wxTextCtrl* m_endCtrl;
	wxStaticText* m_fileNameText;
	wxChoice* m_ffChoice;
	wxBitmapButton* m_endBt;
	wxChoice* m_aspectChoice;
	wxStaticBoxSizer* m_cropBox;
	wxSpinCtrl* m_padTop;
	wxSpinCtrl* m_padBottom;
	wxChoice* m_keepAspectChoice;
	wxStaticText* m_srcText;
	wxSpinButton* m_endFrameSpin;
	wxSpinCtrl* m_cropRight;
	wxSpinButton* m_timeSpinBt;
	wxSpinCtrl* m_cropLeft;
	wxSpinCtrl* m_padRight;
	wxSpinButton* m_startSpin;
	//*)

	//(*Identifiers(VideoPropDlg)
	static const long ID_STATICTEXT2;
	static const long ID_STATICTEXT4;
	static const long ID_STATICTEXT3;
	static const long DST_CHOICE_ID;
	static const long ASPECT_CHOICE_ID;
	static const long INTERLACED_CHOICE_ID;
	static const long FF_CHOICE_ID;
	static const long KEEP_ASPECT_CB_ID;
	static const long KEEP_ASPECT_CHOICE_ID;
	static const long ID_SPINCTRL1;
	static const long ID_SPINCTRL4;
	static const long ID_SPINCTRL3;
	static const long ID_SPINCTRL2;
	static const long ID_SPINCTRL5;
	static const long ID_SPINCTRL6;
	static const long ID_SPINCTRL7;
	static const long ID_SPINCTRL8;
	static const long ID_SPINCTRL9;
	static const long ID_SPINCTRL10;
	static const long ID_TEXTCTRL1;
	static const long ID_CUSTOM1;
	static const long ID_SLIDER;
	static const long ID_TIME_CTRL;
	static const long ID_TIME_SPINB;
	static const long ID_FRAME_SPINBT;
	static const long ID_TEXTCTRL2;
	static const long ID_SPINBUTTON1;
	static const long ID_SPINBUTTON2;
	static const long START_BT_ID;
	static const long ID_TEXTCTRL3;
	static const long ID_SPINBUTTON3;
	static const long ID_SPINBUTTON4;
	static const long ID_RESET_TIME_BT;
	//*)

	/** The DVD */
	DVD* m_dvd;
	Vob* m_vob;
	Stream* m_stream;
	AspectRatio m_aspectRatio;
	VideoFormat m_lastFormat;
	long m_videoPos;
	vector<int> GetPad();
	vector<int> GetCrop();
	void UpdatePadCrop();
	void SeekVideo(long pos, bool updateTimeCtrl = true);
	//(*Handlers(VideoPropDlg)
	void OnChangeFormat(wxCommandEvent& event);
	void OnChangeAspect(wxCommandEvent& event);
	void OnChangeKeepAspect(wxCommandEvent& event);
	void OnChangeBorder(wxSpinEvent& event);
	void OnChangeCrop(wxSpinEvent& event);
	void OnChangeTime(wxCommandEvent& event);
	void OnStartBt(wxCommandEvent& event);
	void OnEndBt(wxCommandEvent& event);
	void OnSliderScroll(wxScrollEvent& event);
	void OnTimeSpin(wxSpinEvent& event);
	void OnInterlaced(wxCommandEvent& event);
	void OnOkBt(wxCommandEvent& event);
	void OnFrameSpin(wxSpinEvent& event);
	void OnStartSpin(wxSpinEvent& event);
	void OnStartFrameSpin(wxSpinEvent& event);
	void OnEndSpin(wxSpinEvent& event);
	void OnEndFrameSpin(wxSpinEvent& event);
	void OnFrameSpinDown(wxSpinEvent& event);
	void OnStartFrameSpinDown(wxSpinEvent& event);
	void OnEndFrameSpinDown(wxSpinEvent& event);
	void OnStartSpinDown(wxSpinEvent& event);
	void OnEndSpinDown(wxSpinEvent& event);
	//*)
	DECLARE_EVENT_TABLE()
};

#endif // VIDEO_PROP_DLG_H
