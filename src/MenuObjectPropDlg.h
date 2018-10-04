/////////////////////////////////////////////////////////////////////////////
// Name:        MenuObjectPropDlg.h
// Purpose:     DVD menu button properties dialog
// Author:      Alex Thuering
// Created:	20.11.2003
// RCS-ID:      $Id: MenuObjectPropDlg.h,v 1.27 2016/06/20 10:42:37 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef BUTTON_PROP_DLG_H
#define BUTTON_PROP_DLG_H

#include "DVD.h"
#include "MenuObject.h"
#include <wxVillaLib/PropDlg.h>
#include <wxSVG/SVGPreserveAspectRatio.h>

class MenuObjectPropDlg: public wxPropDlg {
public:
	MenuObjectPropDlg(wxWindow *parent, wxString id, bool multObjects, Menu *menu, DVD* dvd, int tsi, int pgci);
	virtual ~MenuObjectPropDlg() {}
	
private:
	wxString m_id;
	bool m_multObjects;
	MenuObject* m_object;
	Menu* m_menu;
	DVD* m_dvd;
	int m_tsi; // tsi of menu
	int m_pgci; // pgci of menu
	void CreatePropPanel(wxSizer* sizer);
	void CreateNavigationProp(wxSizer* sizer, NavigationButton navButton);
	void CreateLook(wxBoxSizer* mainSizer);
	void CreateLocAndSize(wxBoxSizer* mainSizer);
	void CreateAspectCtrls(wxSizer* sizer, wxSVG_PRESERVEASPECTRATIO align, wxSVG_MEETORSLICE meetOrSlice);
	void CreateImageCrtls(wxFlexGridSizer* grid, const wxString& title, MenuObjectParam* param);
	void CreateShadowCtrls(wxFlexGridSizer* grid, const wxString& title, MenuObjectParam* param);
	void CreateButtonPane(wxSizer* sizer, bool resetButton, bool dontShowCheckbox);
	
	wxChoice* m_targetChoice;
	wxChoice* m_chapterChoice;
	/** Returns titleset id of selected target */
	int GetSelectedTsi();
	/** Returns PGC id of selected target */
	int GetSelectedPgci();
	/** Returns true if selected target is menu */
	bool IsSelectedMenu();
	/** Returns selected chapter number */
	int GetSelectedChapter();
	
	void* TargetToPtr(int tsi, int pgci, bool menu);
	void FillTargets(wxChoice* ctrl);
	void UpdateChapters();
	void OnChangeTarget(wxCommandEvent& evt);
	void OnChangeChapter(wxCommandEvent& evt);
	
	int m_audioIcon;
	void OnChangeAudio(wxCommandEvent& evt);
	int m_subtitleIcon;
	void OnChangeSubtitle(wxCommandEvent& evt);
	
	bool m_displayVideoFrame; // shows if video frame must be automatically selected if it is available
	long m_defaultPos;
	long m_videoPos;
	int m_videoDuration;
	wxTextCtrl* m_imageEdit;
	int m_imageEditIdx;
	wxRadioButton* m_imageRadio;
	wxRadioButton* m_videoFrameRadio;
	wxChoice* m_videoChoice;
	wxButton* m_videoFrameBt;
	wxString GetVideoFilename(bool withTimestamp = false, long position = -1);
	void OnImageRadio(wxCommandEvent& evt);
	void OnVideoFrame(wxCommandEvent& evt);
	void OnJustifyText(wxCommandEvent& evt);
	bool CanJustifyText(wxString paramName);
	
	wxTextCtrl* m_customActionEdit;
	void OnJumpActionSelected(wxCommandEvent& evt);
	void OnCustomActionSelected(wxCommandEvent& evt);
	
	wxTextCtrl* m_widthEdit;
	wxTextCtrl* m_heightEdit;
	void OnKeepAspectRatio(wxCommandEvent& evt);
	
	void OnAnimationsBt(wxCommandEvent& event);
	
	/** Populate values on the model */
	bool SetValues();
	
	DECLARE_EVENT_TABLE()
};

#endif // BUTTON_PROP_DLG_H
