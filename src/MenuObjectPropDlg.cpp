/////////////////////////////////////////////////////////////////////////////
// Name:        MenuObjectPropDlg.cpp
// Purpose:     DVD menu button properties dialog
// Author:      Alex Thuering
// Created:	20.11.2003
// RCS-ID:      $Id: MenuObjectPropDlg.cpp,v 1.89 2017/01/08 22:21:14 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "MenuObjectPropDlg.h"
#include "Menu.h"
#include "VideoFrameDlg.h"
#include "AnimationDlg.h"
#include "Config.h"
#include <wxVillaLib/utils.h>
#include <wxSVG/svg.h>
#include <wx/statline.h>
#include <wx/collpane.h>
#include <wx/tglbtn.h>
#include <stdint.h>
#include "rc/justifyLeft.png.h"
#include "rc/justifyLeftSelected.png.h"
#include "rc/justifyCenter.png.h"
#include "rc/justifyCenterSelected.png.h"
#include "rc/justifyRight.png.h"
#include "rc/justifyRightSelected.png.h"

enum {
	JUMP_ACTION_RADIO_ID = 7900,
	CUSTOM_ACTION_RADIO_ID,
	TARGET_CHOICE_ID,
	CHAPTER_CHOICE_ID,
	AUDIO_CHOICE_ID,
	SUBTITLE_CHOICE_ID,
	AUTO_SIZE_RADIO_ID,
	KEEP_ASPECT_CB_ID,
	IMAGE_RADIO_ID,
	VIDEOFRAME_RADIO_ID,
	VIDEOFRAME_BT_ID,
	JUSTIFY_LEFT_BT_ID,
	JUSTIFY_CENTER_BT_ID,
	JUSTIFY_RIGHT_BT_ID,
	ANIMATIONS_BT_ID,
};

BEGIN_EVENT_TABLE(MenuObjectPropDlg, wxPropDlg)
	EVT_RADIOBUTTON(JUMP_ACTION_RADIO_ID, MenuObjectPropDlg::OnJumpActionSelected)
	EVT_RADIOBUTTON(CUSTOM_ACTION_RADIO_ID, MenuObjectPropDlg::OnCustomActionSelected)
	EVT_CHOICE(TARGET_CHOICE_ID, MenuObjectPropDlg::OnChangeTarget)
	EVT_CHOICE(CHAPTER_CHOICE_ID, MenuObjectPropDlg::OnChangeChapter)
	EVT_CHOICE(AUDIO_CHOICE_ID, MenuObjectPropDlg::OnChangeAudio)
	EVT_CHOICE(SUBTITLE_CHOICE_ID, MenuObjectPropDlg::OnChangeSubtitle)
	EVT_RADIOBUTTON(KEEP_ASPECT_CB_ID, MenuObjectPropDlg::OnKeepAspectRatio)
	EVT_RADIOBUTTON(IMAGE_RADIO_ID, MenuObjectPropDlg::OnImageRadio)
	EVT_RADIOBUTTON(VIDEOFRAME_RADIO_ID, MenuObjectPropDlg::OnImageRadio)
	EVT_BUTTON(VIDEOFRAME_BT_ID, MenuObjectPropDlg::OnVideoFrame)
	EVT_TOGGLEBUTTON(JUSTIFY_LEFT_BT_ID, MenuObjectPropDlg::OnJustifyText)
	EVT_TOGGLEBUTTON(JUSTIFY_CENTER_BT_ID, MenuObjectPropDlg::OnJustifyText)
	EVT_TOGGLEBUTTON(JUSTIFY_RIGHT_BT_ID, MenuObjectPropDlg::OnJustifyText)
	EVT_BUTTON(ANIMATIONS_BT_ID, MenuObjectPropDlg::OnAnimationsBt)
END_EVENT_TABLE()

MenuObjectPropDlg::MenuObjectPropDlg(wxWindow* parent, wxString id, bool multObjects, Menu* menu, DVD* dvd, int tsi,
		int pgci): wxPropDlg(parent, wxString(_("Properties"))) {
	m_id = id;
	m_multObjects = multObjects;
	m_menu = menu;
	m_dvd = dvd;
	m_tsi = tsi;
	m_pgci = pgci;
	m_imageEdit = NULL;
	m_imageEditIdx = -1;
	m_imageRadio = NULL;
	m_videoFrameRadio = NULL;
	m_videoChoice = NULL;
	m_videoFrameBt = NULL;
	m_customActionEdit = NULL;
	m_widthEdit = NULL;
	m_heightEdit = NULL;
	m_object = menu->GetObject(id);
	m_displayVideoFrame = m_object->IsDisplayVideoFrame();
	m_defaultPos = 0;
	m_videoPos = 0;
	m_videoDuration = 0;
	m_targetChoice = NULL;
	m_chapterChoice = NULL;
	m_audioIcon = -1;
	m_subtitleIcon = -1;
	Create();
	if (!multObjects)
		SetTitle(GetTitle()+ wxT(" - ") + menu->GetObject(id)->GetId(true));
}

void MenuObjectPropDlg::CreatePropPanel(wxSizer* sizer) {
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* paneSz = mainSizer;
#if wxUSE_COLLPANE
	wxCollapsiblePane* collpane = NULL;
#endif
	
	// action
	if (m_object->IsButton() && !m_multObjects) {
		DVDAction& action = m_object->GetAction();
#if wxUSE_COLLPANE
		collpane = new wxCollapsiblePane(this, wxID_ANY, _("Action"));
#ifdef __WXMSW__
		collpane->GetControlWidget()->SetFont(wxFont(collpane->GetControlWidget()->GetFont().GetPointSize() + 2,
				wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
#endif
		mainSizer->Add(collpane, 0, wxEXPAND);
		paneSz = new wxBoxSizer(wxVERTICAL);
		collpane->GetPane()->SetSizer(paneSz);
		SetPropWindow(collpane->GetPane());
#else
		AddStaticLine(mainSizer, _("Action"));
#endif
		wxFlexGridSizer* grid = new wxFlexGridSizer(2, 4, 16);
		grid->AddGrowableCol(1);
		// check menu entry if it is set
		wxString customAction = action.GetCustom();
		if (action.GetEntry().length() > 0 && customAction.length() == 0) {
			// find menu with given entry
			Menus& menus = (Menus&) m_dvd->GetPgcArray(action.GetTsi(), true);
			int pgci = menus.GetPgciByEntry(action.GetEntry());
			if (pgci >= 0) {
				action.SetPgci(pgci);
			} else {
				// entry is not valid => set custom action
				customAction = action.AsString(m_dvd);
			}
		}
		// jump to
		AddRadioProp(grid, _("Jump to") + wxString(wxT(":")), customAction.length() == 0, wxRB_GROUP, false,
				JUMP_ACTION_RADIO_ID);
		wxSizer* jumpSizer = new wxFlexGridSizer(4, 4, 4);
		AddChoiceProp(jumpSizer, wxT(""), wxT(""), wxArrayString(), -1, false, TARGET_CHOICE_ID);
		m_targetChoice = (wxChoice*) m_controls[m_controls.Count()-1];
		FillTargets(m_targetChoice);
		AddChoiceProp(jumpSizer, wxT(""), wxT(""), wxArrayString(), -1, true, CHAPTER_CHOICE_ID);
		m_chapterChoice = (wxChoice*) m_controls[m_controls.Count()-1];
		UpdateChapters();
		BeginCheckGroup(jumpSizer, _("Play all titles"), action.IsPlayAll());
		wxArrayString labels;
		labels.Add(_("single titleset"));
		labels.Add(_("all titlesets"));
		jumpSizer->Add(AddChoiceProp(labels[action.IsPlayAllTitlesets() ? 1 : 0], labels));
		EndGroup();
		
		grid->Add(jumpSizer);
		// customAction
		if (!customAction.length()) {
			// check if selection was ok, otherwise set this action as custom action
			int tsi = action.GetTsi() > -2 ? action.GetTsi() : m_tsi;
			int pgci = action.GetPgci() > -1 ? action.GetPgci() : m_pgci;
			bool menu = action.GetPgci() > -1 ? action.IsMenu() : true;
			if (GetSelectedTsi() != tsi || GetSelectedPgci() != pgci || IsSelectedMenu() != menu
					|| GetSelectedChapter() != action.GetChapter())
				customAction = action.AsString(m_dvd);
		}
		AddRadioProp(grid, _("Custom") + wxString(wxT(":")), customAction.length() > 0, 0, false,
				CUSTOM_ACTION_RADIO_ID);
		AddTextProp(grid, wxT(""), customAction);
		m_customActionEdit = (wxTextCtrl*) m_controls[m_controls.Count()-1];
		wxCommandEvent evt;
		if (customAction.length())
			OnCustomActionSelected(evt);
		else
			OnJumpActionSelected(evt);

		// audio & subtitle
		labels.Clear();
		labels.Add(_("last selected"));
		for (int i = 0; i < (int) m_dvd->GetAudioStreamCount(); i++)
			labels.Add(_("track") + wxString::Format(wxT(" %d"), i + 1));
		AddText(grid, _("Audio") + wxString(wxT(":")), wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT);
		wxSizer* s = AddChoiceProp(grid, wxT(""), labels[action.GetAudio()+1], labels, 0, false, AUDIO_CHOICE_ID);
		bool showAudioIcon = ((wxChoice*)GetLastControl())->GetSelection() > 0;
		if (m_dvd->GetAudioStreamCount() == 1) {
			GetLastControl()->Enable(false);
		} else if (m_tsi == -1) {
			s->Add(2, 2);
			m_audioIcon = AddIcon(s, _("Warning"), _("Please use titleset menu to select audio tracks."), wxART_WARNING);
		}
		s->Add(16, 16);
		labels.Clear();
		labels.Add(_("last selected"));
		labels.Add(_("off"));
		for (int i = 0; i < (int) m_dvd->GetSubtitleStreamsCount(); i++)
			labels.Add(_("track") + wxString::Format(wxT(" %d"), i + 1));
		AddText(s, _("subtitle") + wxString(wxT(":")));
		s->Add(16, 16);
		int subtitleIdx = action.GetSubtitle() + 1 < (int) labels.size() ? action.GetSubtitle() + 1 : 0;
		s = AddChoiceProp(s, wxT(""), labels[subtitleIdx], labels, 0, false, SUBTITLE_CHOICE_ID);
		bool showSubtitleIcon = ((wxChoice*)GetLastControl())->GetSelection() > 0;
		if (m_dvd->GetSubtitleStreamsCount() == 0) {
			GetLastControl()->Enable(false);
		} else if (m_tsi == -1) {
			s->Add(2, 2);
			m_subtitleIcon = AddIcon(s, _("Warning"), _("Please use titleset menu to select subtitle tracks."), wxART_WARNING);
		}
		s->AddStretchSpacer();

		paneSz->Add(grid, 0, wxEXPAND|wxBOTTOM, 4);
		AddCheckProp(paneSz, _("Auto execute command by select"), m_object->IsAutoExecute());

		grid = new wxFlexGridSizer(3, 4, 16);
		AddText(grid, _("Navigation"));
		CreateNavigationProp(grid, nbUP);
		grid->Add(10,10);
		CreateNavigationProp(grid, nbLEFT);
		grid->Add(10,10);
		CreateNavigationProp(grid, nbRIGHT);
		grid->Add(10,10);
		CreateNavigationProp(grid, nbDOWN);
		grid->Add(10,10);
		paneSz->Add(grid, 0, wxEXPAND|wxTOP|wxBOTTOM, 4);
		
#if wxUSE_COLLPANE
		paneSz->SetSizeHints(collpane->GetPane());
		collpane->Expand();
		
		collpane = new wxCollapsiblePane(this, wxID_ANY, _("Look"));
#ifdef __WXMSW__
		collpane->GetControlWidget()->SetFont(wxFont(collpane->GetControlWidget()->GetFont().GetPointSize() + 2,
				wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
#endif
		mainSizer->Add(collpane, 0, wxEXPAND);
		paneSz = new wxBoxSizer(wxVERTICAL);
		collpane->GetPane()->SetSizer(paneSz);
		SetPropWindow(collpane->GetPane());
#else
		AddStaticLine(mainSizer, _("Look"));
#endif
		if (m_audioIcon >= 0)
			GetIcon(m_audioIcon)->Show(showAudioIcon);
		if (m_subtitleIcon >= 0)
			GetIcon(m_subtitleIcon)->Show(showSubtitleIcon);
	}
    CreateLook(paneSz);
    if (!m_multObjects)
		CreateLocAndSize(paneSz);
    
#if wxUSE_COLLPANE
    if (collpane) {
		paneSz->SetSizeHints(collpane->GetPane());
		collpane->Expand();
		SetPropWindow(this);
    }
#endif
	sizer->Add(mainSizer, 1, wxEXPAND|wxALL, 4);
}

void MenuObjectPropDlg::CreateNavigationProp(wxSizer* sizer, NavigationButton navButton) {
	wxFlexGridSizer* grid = new wxFlexGridSizer(2, 1, 4, 16);
	AddChoiceProp(grid, wxT(""), wxT(""), wxArrayString());
	wxChoice* ctrl = (wxChoice*) m_controls[m_controls.Count()-1];
	
	// default navigation
	wxString defBtId = m_object->GetDefaultFocusDest(navButton);
	wxString text = _("none");
	if (defBtId != m_object->GetId()) {
		long l = -1;
		if (defBtId.Mid(6).ToLong(&l))
			text = _("button") + wxString::Format(wxT(" %d"), (int) l);
	}
	ctrl->Append(_("auto") + wxString(wxT(" (")) + text + wxT(")"), new wxString);
	ctrl->Append(_("none"), new wxString(m_object->GetId()));

	// add all buttons except current
	int sel = m_object->GetFocusDest(navButton) == m_object->GetId() ? 1 : 0;
	for (unsigned int i=0; i<m_menu->GetObjectsCount(); i++) {
		MenuObject* obj = m_menu->GetObject(i);
		if (obj->IsButton() && obj != m_object) {
			wxString id = obj->GetId();
			if (m_object->GetFocusDest(navButton) == id)
				sel = ctrl->GetCount();
			long l = -1;
			id.Mid(6).ToLong(&l);
			wxString text = _("button") + wxString::Format(wxT(" %d"), (int) l);
			if (obj->GetParam(wxT("text")).length())
				text += wxT(": ") + obj->GetParam(_T("text"));
			ctrl->Append(text, new wxString(id));
		}
	}
	for (unsigned int i=0; i<m_menu->GetActionsCount(); i++) {
		DVDAction* action = m_menu->GetAction(i);
		wxString id = action->GetId();
		if (m_object->GetFocusDest(navButton) == id)
			sel = ctrl->GetCount();
		ctrl->Append(id, new wxString(id));
	}
	ctrl->SetSelection(sel);
	sizer->Add(grid, 0, 0, 0);
}

void MenuObjectPropDlg::CreateLook(wxBoxSizer* mainSizer) {
    // object look parameters
    bool lastChangeable = false;
    wxFlexGridSizer* grid = NULL;
    for (unsigned int i=0; i<m_object->GetParams().size(); i++) {
		MenuObjectParam* param = m_object->GetParams()[i];
		if (param->name == wxT("rotation")) {
			continue;
		}

		if (grid == NULL || param->changeable != lastChangeable) {
			if (param->changeable) {
				grid = new wxFlexGridSizer(4, 4, 16);
				AddText(grid, wxT(""));
				AddText(grid, _("Normal"));
				AddText(grid, _("Highlighted"));
				AddText(grid, _("Selected"));
			} else {
				grid = new wxFlexGridSizer(2, 4, 16);
				grid->AddGrowableCol(1);
			}
			if (i==0)
				mainSizer->Add(grid, 0, wxEXPAND|wxTOP|wxBOTTOM, 4);
			else
				mainSizer->Add(grid, 0, wxEXPAND|wxBOTTOM, 4);
			lastChangeable = param->changeable;
		}

		wxString title = wxGetTranslation((const wxChar*)param->title.GetData()) + wxString(wxT(":"));

		if (param->changeable && param->type == _T("colour")) {
			AddColourProp(grid, title, m_object->GetParamColour(param->name));
			AddColourProp(grid, wxT(""), m_object->GetParamColour(param->name, mbsHIGHLIGHTED));
			AddColourProp(grid, wxT(""), m_object->GetParamColour(param->name, mbsSELECTED));
		} else {
			if (param->type == _T("text")) {
				AddText(grid, title);
				wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
				AddTextProp(sizer2, wxT(""), m_object->GetParam(param->name), m_multObjects, -1, true, wxTE_MULTILINE);
				((wxTextCtrl*) GetLastControl())->SetSelection(-1, -1);
				if (m_multObjects)
					GetLastControl()->Enable(false);
				wxBoxSizer* sizer3 = new wxBoxSizer(wxVERTICAL);
				AddFontProp(sizer3, wxT(""), m_object->GetParamFont(param->name), _("Font..."));
				wxBoxSizer* sizer4 = new wxBoxSizer(wxHORIZONTAL);
				bool enableJustify = CanJustifyText(param->name);
				bool selected = m_object->GetParam(param->name, wxT("text-anchor")) == wxT("start");
				AddBitmapToggleButton(sizer4, selected, JUSTIFY_LEFT_BT_ID, selected ? wxBITMAP_FROM_MEMORY(
						justifyLeftSelected) : wxBITMAP_FROM_MEMORY(justifyLeft), wxSize(22, 22));
				GetLastControl()->Enable(enableJustify);
				selected = m_object->GetParam(param->name, wxT("text-anchor")) == wxT("middle");
				AddBitmapToggleButton(sizer4, selected, JUSTIFY_CENTER_BT_ID, selected ? wxBITMAP_FROM_MEMORY(
						justifyCenterSelected) : wxBITMAP_FROM_MEMORY(justifyCenter), wxSize(22, 22));
				GetLastControl()->Enable(enableJustify);
				selected = m_object->GetParam(param->name, wxT("text-anchor")) == wxT("end");
				AddBitmapToggleButton(sizer4, selected, JUSTIFY_RIGHT_BT_ID, selected ? wxBITMAP_FROM_MEMORY(
						justifyRightSelected) : wxBITMAP_FROM_MEMORY(justifyRight), wxSize(22, 22));
				GetLastControl()->Enable(enableJustify);
				sizer3->Add(sizer4, 0, wxEXPAND);
				sizer2->Add(sizer3, 0, wxEXPAND);
				grid->Add(sizer2, 0, wxEXPAND);
			} else if (param->type == _T("string")) {
				AddTextProp(grid, title, m_object->GetParam(param->name));
			} else if (param->type == _T("decimal")) {
				long min = 0, max = 999;
				if (param->type.Find(wxT('(')) != -1 && param->type.Find(wxT(')')) != -1) {
					param->type.AfterFirst(wxT('(')).BeforeFirst(wxT(',')).Strip(wxString::both).ToLong(&min);
					param->type.BeforeFirst(wxT(')')).AfterFirst(wxT(',')).Strip(wxString::both).ToLong(&max);
				}
				AddSpinDoubleProp(grid, title, m_object->GetParamDouble(param->name), min, max, 54);
				if (m_multObjects && param->attribute.Find(wxT('#')) != wxNOT_FOUND)
					GetLastControl()->Enable(false);
			} else if (param->type.Mid(0,7) == _T("integer")) {
				long min = 0, max = 999;
				if (param->type.Find(wxT('(')) != -1 && param->type.Find(wxT(')')) != -1) {
					param->type.AfterFirst(wxT('(')).BeforeFirst(wxT(',')).Strip(wxString::both).ToLong(&min);
					param->type.BeforeFirst(wxT(')')).AfterFirst(wxT(',')).Strip(wxString::both).ToLong(&max);
				}
				AddSpinProp(grid, title, m_object->GetParamInt(param->name), min, max, max < 1000 ? 54 : 80);
				if (m_multObjects && param->attribute.Find(wxT('#')) != wxNOT_FOUND)
					GetLastControl()->Enable(false);
			} else if (param->type.Mid(0,7) == _T("percent")) {
				AddSpinProp(grid, title, m_object->GetParamDouble(param->name)*100, 0, 100, 54, wxT("%"));
				if (m_multObjects && param->attribute.Find(wxT('#')) != wxNOT_FOUND)
					GetLastControl()->Enable(false);
			} else if (param->type == _T("image") && !m_multObjects) {
				CreateImageCrtls(grid, title, param);
			} else if (param->type == _T("colour")) {
				wxColour colour = m_object->GetParamColour(param->name);
				wxString opacityStr = m_object->GetParam(param->name, wxT("-opacity"));
				if (colour.IsOk() && opacityStr.length() > 0) {
					double dval;
					if (opacityStr.ToDouble(&dval))
						colour = wxColour(colour.Red(), colour.Green(), colour.Blue(), lround(dval*255));
				}
				AddColourProp(grid, title, colour);
			} else if (param->type == wxT("shadow")) {
				CreateShadowCtrls(grid, title, param);
			}
		}
	}
}

void MenuObjectPropDlg::CreateAspectCtrls(wxSizer* sizer, wxSVG_PRESERVEASPECTRATIO align,
		wxSVG_MEETORSLICE meetOrSlice) {
	sizer->AddSpacer(4);
	bool keepAspectRatio = align > wxSVG_PRESERVEASPECTRATIO_NONE;
	BeginCheckGroup(sizer, _("Keep Aspect Ratio"), keepAspectRatio);
	sizer->AddSpacer(4);
	wxArrayString alignXStrings;
	alignXStrings.Add(_("X Min"));
	alignXStrings.Add(_("X Mid"));
	alignXStrings.Add(_("X Max"));
	int alignX = keepAspectRatio ? (align - wxSVG_PRESERVEASPECTRATIO_XMINYMIN) % 3 : 1;
	sizer->Add(AddChoiceProp(alignXStrings[alignX], alignXStrings));
	wxArrayString alignYStrings;
	alignYStrings.Add(_("Y Min"));
	alignYStrings.Add(_("Y Mid"));
	alignYStrings.Add(_("Y Max"));
	int alignY = keepAspectRatio ? (align - wxSVG_PRESERVEASPECTRATIO_XMINYMIN) / 3 : 1;
	sizer->Add(AddChoiceProp(alignYStrings[alignY], alignYStrings), 0, wxLEFT, 4);
	wxArrayString meetStrings;
	meetStrings.Add(_("Meet"));
	meetStrings.Add(_("Slice"));
	int meet = align == wxSVG_PRESERVEASPECTRATIO_NONE || meetOrSlice == wxSVG_MEETORSLICE_SLICE ? 1 : 0;
	sizer->Add(AddChoiceProp(meetStrings[meet], meetStrings), 0, wxLEFT, 4);
	sizer->AddStretchSpacer(1);
	EndGroup();
}

void MenuObjectPropDlg::CreateImageCrtls(wxFlexGridSizer* grid, const wxString& title, MenuObjectParam* param) {
	// video duration
	m_videoDuration = m_object->GetParamVideoDuration(param->name);
	// file name
	wxString imageFile = m_object->GetParam(param->name);
	wxString fname = imageFile.Find(wxT('#')) != -1 && m_videoDuration == 0 ? imageFile.BeforeLast(wxT('#')) : imageFile;
	if (!m_object->IsButton()) {
		m_videoChoice = AddChoiceProp(wxT(""), wxArrayString());
		int sel = 0;
		for (unsigned int tsi = 0; tsi < m_dvd->GetTitlesets().size(); tsi++) {
			Titleset* ts = m_dvd->GetTitlesets()[tsi];
			for (unsigned int pgci = 0; pgci < ts->GetTitles().size(); pgci++) {
				wxString label = _("title") + wxString::Format(wxT(" %d"), pgci + 1);
				if (m_dvd->GetTitlesets().size() > 1)
					label = _("titleset") + wxString::Format(wxT(" %d "), tsi + 1) + label;
				int id = DVD::MakeId(tsi, pgci, 0, false);
				m_videoChoice->Append(label, (void*)(intptr_t) id);
				if (id == m_object->GetDisplayVobId()) {
					sel = m_videoChoice->GetCount() - 1;
				}
			}
		}
		if (m_videoChoice->GetCount() > 0)
			m_videoChoice->SetSelection(sel);
	}
	bool hasVideo = GetVideoFilename(false).length();
	// image
	bool image = !m_displayVideoFrame || !hasVideo || fname != GetVideoFilename(false);
	AddRadioProp(grid, wxString(_("Image")) + wxT(":"), image, wxRB_GROUP, false, IMAGE_RADIO_ID);
	m_imageRadio = (wxRadioButton*) GetLastControl();
	wxString wildcard = _("Image Files ") + wxImage::GetImageExtWildcard()
		+ wxT("|") + wxString(_("All Files")) + wxT(" (*.*)|*.*"); 
	AddFileProp(grid, wxT(""), wxString(image ? imageFile : wxT("")), wxFD_OPEN, wxT("..."), wildcard);
	m_imageEdit = (wxTextCtrl*) GetLastControl();
	m_imageEditIdx = GetLastControlIndex();
	// video frame
	AddRadioProp(grid, wxString(_("Video")) + wxT(":"), !image, 0, false, VIDEOFRAME_RADIO_ID);
	m_videoFrameRadio = (wxRadioButton*) GetLastControl();
	m_videoFrameRadio->Enable(hasVideo);
	wxBoxSizer* videoFrameSizer = new wxBoxSizer(wxHORIZONTAL);
	if (!m_object->IsButton()) {
		videoFrameSizer->Add(m_videoChoice, 0, wxALIGN_CENTER_VERTICAL);
	}
	m_videoFrameBt = new wxButton(propWindow, VIDEOFRAME_BT_ID, wxT("..."));
	int h = m_videoFrameBt->GetSize().GetHeight() > 24 ? m_videoFrameBt->GetSize().GetHeight() : 24;
	m_videoFrameBt->SetSizeHints(h, h, h, h);
	videoFrameSizer->Add(m_videoFrameBt);
	grid->Add(videoFrameSizer);
	wxCommandEvent evt;
	OnImageRadio(evt);
	// default video position
	if (m_object->IsButton()) {
		wxString url = GetVideoFilename(true);
		if (url.length()) {
			url.AfterLast(wxT('#')).ToLong(&m_defaultPos);
		}
	} else {
		m_defaultPos = 0;
	}
	// video position
	if (!image) {
		m_videoPos = lround(m_object->GetParamVideoClipBegin(param->name) * 1000);
		if (!image && imageFile.Find(wxT('#')) != -1) {
			wxString vf = imageFile.AfterLast(wxT('#'));
			if (vf.length() > 0)
				vf.ToLong(&m_videoPos);
		}
	} else
		m_videoPos = m_defaultPos;
	// keep aspect
	wxSVGPreserveAspectRatio aspectRatio;
	aspectRatio.SetValueAsString(m_object->GetParam(param->name, wxT("preserveAspectRatio")));
	CreateAspectCtrls(videoFrameSizer, aspectRatio.GetAlign(), aspectRatio.GetMeetOrSlice());
}

void MenuObjectPropDlg::CreateShadowCtrls(wxFlexGridSizer* grid, const wxString& title, MenuObjectParam* param) {
	bool shadow = m_object->GetParam(param->name, wxT("visibility")) != wxT("hidden");
	BeginCheckGroup(grid, title, shadow, false);
	wxSizer* shadowSizer = new wxBoxSizer(wxHORIZONTAL);
	grid->Add(shadowSizer);
	
	wxColour colour = m_object->GetParamColour(param->name);
	wxString opacityStr = m_object->GetParam(param->name, wxT("-opacity"));
	if (colour.IsOk() && opacityStr.length() > 0) {
		double dval;
		if (opacityStr.ToDouble(&dval))
			colour = wxColour(colour.Red(), colour.Green(), colour.Blue(), lround(dval*255));
	}
	AddColourProp(shadowSizer, wxT(""), colour);
	
	shadowSizer->AddSpacer(8);
	AddText(shadowSizer, _("Offset:"));
	shadowSizer->AddSpacer(4);
	AddSpinProp(shadowSizer, wxT(""), m_object->GetParamInt(param->name, wxT("x")), -999, 999, 46, wxT(""), false);
	AddSpinProp(shadowSizer, wxT(""), m_object->GetParamInt(param->name, wxT("y")), -999, 999, 46, wxT(""), false);
	shadowSizer->AddSpacer(4);
	wxSVGElement* filter = m_object->GetElementById(param->element.front() + wxT("Filter"));
	if (filter && ((wxSVGElement*)filter->GetFirstChild())->GetDtd() == wxSVG_FEGAUSSIANBLUR_ELEMENT) {
		wxSVGFEGaussianBlurElement* blur = (wxSVGFEGaussianBlurElement*) filter->GetFirstChild();
		AddText(shadowSizer, _("Deviation:"));
		shadowSizer->AddSpacer(4);
		AddSpinDoubleProp(shadowSizer, wxT(""), blur->GetStdDeviationX(), 0, 99, 46);
	}
	EndGroup();
}

void MenuObjectPropDlg::CreateLocAndSize(wxBoxSizer* mainSizer) {
	wxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
	mainSizer->Add(hSizer, 0, wxTOP, 8);

	wxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
	hSizer->Add(vSizer);

	// object location
	wxSizer* grpSizer = BeginGroup(vSizer, _("Location"));
	wxFlexGridSizer* grid = new wxFlexGridSizer(2, 4, 16);
	wxSize menuSize = m_menu->GetResolution();
	AddSpinProp(grid, _("X:"), m_object->GetX(), m_object->IsButton() ? 0 : -menuSize.GetWidth(), menuSize.GetWidth()-8);
	AddSpinProp(grid, _("Y:"), m_object->GetY(), m_object->IsButton() ? 0 : -menuSize.GetHeight(), menuSize.GetHeight()-8);
	grpSizer->Add(grid, 0, wxALL, 4);
	EndGroup();
	vSizer->AddSpacer(8);
	
	// object rotation
	grpSizer = BeginGroup(vSizer, _("Rotation"));
	grid = new wxFlexGridSizer(2, 4, 16);
	AddSpinDoubleProp(grid, _("Angle:"), m_object->GetAngle(), 0, 360);
	grpSizer->Add(grid, 0, wxALL, 4);
	EndGroup();

	// object size
	hSizer->AddSpacer(8);
	grpSizer = BeginGroup(hSizer, _("Size"), _("Custom"), !m_object->IsDefaultSize());
	grid = new wxFlexGridSizer(2, 4, 16);
	AddSpinProp(grid, _("Width:"), m_object->GetWidth(), 0, m_menu->GetResolution().GetWidth());
	m_widthEdit = (wxTextCtrl*) m_controls[m_controls.Count()-1];
	AddSpinProp(grid, _("Height:"), m_object->GetHeight(), 0, m_menu->GetResolution().GetHeight());
	m_heightEdit = (wxTextCtrl*) m_controls[m_controls.Count()-1];
	grid->AddSpacer(4);
	AddCheckProp(grid, _("Keep Aspect Ratio"), m_object->IsKeepAspectRatio(), false, KEEP_ASPECT_CB_ID);
	grpSizer->Add(grid, 0, wxALL, 4);
	EndGroup();
}

/**
 * Creates panel with buttons
 */
void MenuObjectPropDlg::CreateButtonPane(wxSizer* sizer, bool resetButton, bool dontShowCheckbox) {
	wxStdDialogButtonSizer* buttonPane = new wxStdDialogButtonSizer();
	
	// cells & actions buttons
	buttonPane->Add(new wxButton(this, ANIMATIONS_BT_ID, _("Animations...")));
	buttonPane->Add(10, 10, 1, wxEXPAND);
	
	// ok & cancel buttons
	buttonPane->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	buttonPane->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	
	buttonPane->Realize();
	buttonPane->GetAffirmativeButton()->SetDefault();
	sizer->Add(buttonPane, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
}

int MenuObjectPropDlg::GetSelectedTsi() {
	int tsi = 0;
	if (m_targetChoice->GetSelection() >= 0)
		tsi = ((intptr_t) m_targetChoice->GetClientData(m_targetChoice->GetSelection()))/1000 - 1;
	return tsi;
}

int MenuObjectPropDlg::GetSelectedPgci() {
	if (m_targetChoice->GetSelection() >= 0)
		return (((intptr_t) m_targetChoice->GetClientData(m_targetChoice->GetSelection())) % 1000) / 2;
	return 0;
}

bool MenuObjectPropDlg::IsSelectedMenu() {
	if (m_targetChoice->GetSelection() >= 0)
		return (((intptr_t) m_targetChoice->GetClientData(m_targetChoice->GetSelection())) % 1000) % 2 == 0;
	return true;
}

int MenuObjectPropDlg::GetSelectedChapter() {
	if (m_chapterChoice->GetSelection() >= 0)
		return m_chapterChoice->GetSelection();
	return 0;
}

void* MenuObjectPropDlg::TargetToPtr(int tsi, int pgci, bool menu) {
	return (void*) (((long) tsi+1)*1000 + pgci*2 + (menu ? 0 : 1));
}

/**
 * Fills choice "targets" with possible action targets
 */
void MenuObjectPropDlg::FillTargets(wxChoice* ctrl) {
	int sel = 0;
	DVDAction& action = m_object->GetAction(); // current action
	int actionTsi = action.GetTsi() > -2 ? action.GetTsi() : m_tsi; // tsi of current action target
	int actionPgci = action.GetPgci() > -1 ? action.GetPgci() : m_pgci; // pgci of current action target
	int actionMenu = action.GetPgci() > -1 ? action.IsMenu() : true; // menu/title is target of current action

	ctrl->Append(_("start of current menu"), TargetToPtr(m_tsi, m_pgci, true));

	// add a jump target for every title and menu in every titleset if possible
	for (int tsi = -1; tsi < (int)m_dvd->GetTitlesets().Count(); tsi++) {
		if (tsi == -1) { // Video-Manager-Domain
			// jumpt to vmgm menu
			for (int pgci = 0; pgci < (int) m_dvd->GetVmgm().Count(); pgci++) {
				if (tsi == m_tsi && pgci == m_pgci)
					continue; // current menu
				ctrl->Append(_("VMGM menu") + wxString::Format(wxT(" %d"), pgci + 1), TargetToPtr(tsi, pgci, true));
				if (tsi == actionTsi && pgci == actionPgci && actionMenu)
					sel = ctrl->GetCount() - 1; // current action target
			}
		} else { // Titleset
			Titleset* ts = m_dvd->GetTitlesets()[tsi];
			// jumpt to menu
			for (int pgci = 0; pgci < (int) ts->GetMenus().Count(); pgci++) {
				if (tsi == m_tsi && pgci == m_pgci)
					continue; // current menu
				Pgc* pgc = ts->GetMenus()[pgci];
				wxString label = _("menu") + wxString::Format(wxT(" %d"), pgci + 1);
				if (!m_dvd->IsJumppadsEnabled() && m_tsi != tsi) {
					int rootMenu = ts->GetMenus().GetPgciByEntry(wxT("root"));
					if (pgci != rootMenu && pgc->GetEntries().size() == 0)
						continue; // cannot jump to specific menu from other titleset, only to an entry
					const wxString entries[6] = {
							_("title menu"), _("root menu"), _("subtitle menu"),
							_("audio menu"), _("angle menu"), _("chapter menu")};
					int entryIdx = pgc->GetEntries().size() > 0 ? s_entries.Index(*pgc->GetEntries().begin()) : 1;
					if (entryIdx >= 0)
						label = label + wxT(" (") + entries[entryIdx] + wxT(")");
				}
				if (m_dvd->GetTitlesets().size() > 1 || m_dvd->GetVmgm().size() > 0)
					label = _("titleset") + wxString::Format(wxT(" %d "), tsi + 1) + label;
				ctrl->Append(label, TargetToPtr(tsi, pgci, true));
				if (tsi == actionTsi && pgci == actionPgci && actionMenu)
					sel = ctrl->GetCount() - 1;
			}
			// jump to not existing menu
			if (tsi == actionTsi && actionPgci >= (int) ts->GetMenus().Count() && actionMenu) {
				wxString label = _("menu") + wxString::Format(wxT(" %d*"), actionPgci + 1);
				if (m_dvd->GetTitlesets().size() > 1 || m_dvd->GetVmgm().size() > 0)
					label = _("titleset") + wxString::Format(wxT(" %d "), tsi + 1) + label;
				ctrl->Append(label, TargetToPtr(tsi, actionPgci, true));
				sel = ctrl->GetCount() - 1;
			}
			// jumping from titleset menu is allowed only to titles from the same titleset
			if (m_dvd->IsJumppadsEnabled() || m_tsi == -1 || m_tsi == tsi) {
				// jump to title
				for (int pgci = 0; pgci < (int) ts->GetTitles().Count(); pgci++) {
					wxString label = _("title") + wxString::Format(wxT(" %d"), pgci + 1);
					if (m_dvd->GetTitlesets().size() > 1 || m_dvd->GetVmgm().size() > 0)
						label = _("titleset") + wxString::Format(wxT(" %d "), tsi + 1) + label;
					ctrl->Append(label, TargetToPtr(tsi, pgci, false));
					if (tsi == actionTsi && pgci == actionPgci && !actionMenu)
						sel = ctrl->GetCount() - 1;
				}
				// jump to not existing title
				if (tsi == actionTsi && actionPgci >= (int) ts->GetTitles().Count() && !actionMenu) {
					wxString label = _("title") + wxString::Format(wxT(" %d*"), actionPgci + 1);
					if (m_dvd->GetTitlesets().size() > 1 || m_dvd->GetVmgm().size() > 0)
						label = _("titleset") + wxString::Format(wxT(" %d "), tsi + 1) + label;
					ctrl->Append(label, TargetToPtr(tsi, actionPgci, false));
					sel = ctrl->GetCount() - 1;
				}
			}
		}
	}
	// jump to not existing title in not existing titleset
	if (actionTsi >= (int) m_dvd->GetTitlesets().size() && !actionMenu) {
		wxString label = _("titleset") + wxString::Format(wxT(" %d "), actionTsi + 1)
				+ _("title") + wxString::Format(wxT(" %d*"), actionPgci + 1);
		ctrl->Append(label, TargetToPtr(actionTsi, actionPgci, false));
		sel = ctrl->GetCount() - 1;
	}
	ctrl->SetSelection(sel);
}

void MenuObjectPropDlg::UpdateChapters() {
	wxChoice* ctrl = m_chapterChoice;
	int tsi = GetSelectedTsi();
	int pgci = GetSelectedPgci();
	bool menu = IsSelectedMenu();
	int chapter = ctrl->GetCount() ? GetSelectedChapter() : m_object->GetAction().GetChapter();
	ctrl->Clear();
	ctrl->Append(wxString::Format(_("chapter %d"), 1));
	PgcArray& pgcs = m_dvd->GetPgcArray(tsi, false);
	if (tsi >= 0 && !menu && pgci < (int) pgcs.Count())
		for (int i = 1; i < (int) pgcs[pgci]->GetChapterCount(); i++)
			ctrl->Append(wxString::Format(_("chapter %d"), i + 1));
	ctrl->SetSelection(chapter);
	ctrl->Enable(ctrl->GetCount() > 1 && (m_dvd->IsJumppadsEnabled() || m_tsi != -1));
}

void MenuObjectPropDlg::OnChangeTarget(wxCommandEvent& evt) {
	UpdateChapters();
	if (m_videoFrameRadio) {
		m_videoFrameRadio->Enable(GetVideoFilename(false).length());
		if (!m_videoFrameRadio->IsEnabled() && !m_imageRadio->GetValue()) {
			m_imageRadio->SetValue(true);
			wxCommandEvent event(wxEVT_COMMAND_RADIOBUTTON_SELECTED, m_imageRadio->GetId());
			GetEventHandler()->ProcessEvent(event);
		} else if (m_displayVideoFrame && !m_videoFrameRadio->GetValue()) {
			m_videoFrameRadio->SetValue(true);
			wxCommandEvent event(wxEVT_COMMAND_RADIOBUTTON_SELECTED, m_imageRadio->GetId());
			GetEventHandler()->ProcessEvent(event);
		}
		// reset video position
		wxString url = GetVideoFilename(true);
		if (url.length()) {
			url.AfterLast(wxT('#')).ToLong(&m_defaultPos);
			m_videoPos = m_defaultPos;
		}
	}
}

void MenuObjectPropDlg::OnChangeChapter(wxCommandEvent& evt) {
	// reset video position
	wxString url = GetVideoFilename(true);
	if (url.length()) {
		bool isDefaultPos = m_videoPos == m_defaultPos;
		url.AfterLast(wxT('#')).ToLong(&m_defaultPos);
		if (isDefaultPos)
			m_videoPos = m_defaultPos;
	}
}


void MenuObjectPropDlg::OnChangeAudio(wxCommandEvent& evt) {
	if (m_audioIcon >= 0)
		GetIcon(m_audioIcon)->Show(evt.GetSelection() > 0);
}

void MenuObjectPropDlg::OnChangeSubtitle(wxCommandEvent& evt) {
	if (m_subtitleIcon >= 0)
		GetIcon(m_subtitleIcon)->Show(evt.GetSelection() > 0);
}

void MenuObjectPropDlg::OnJumpActionSelected(wxCommandEvent& evt) {
	m_targetChoice->Enable(true);
	m_chapterChoice->Enable(m_chapterChoice->GetCount() > 1 && (m_dvd->IsJumppadsEnabled() || m_tsi != -1));
	m_customActionEdit->Enable(false);
	if (m_videoFrameRadio) {
		m_videoFrameRadio->Enable(GetVideoFilename(false).length());
		if (GetVideoFilename(false).length() == 0) {
			m_imageRadio->SetValue(true);
			wxCommandEvent event(wxEVT_COMMAND_RADIOBUTTON_SELECTED, m_imageRadio->GetId());
			GetEventHandler()->ProcessEvent(event);
		}
		else if (GetVideoFilename(false).length() && m_imageEdit->GetValue().length() == 0)
			m_videoFrameRadio->SetValue(true);
		wxCommandEvent event(wxEVT_COMMAND_RADIOBUTTON_SELECTED, m_videoFrameRadio->GetId());
		GetEventHandler()->ProcessEvent(event);
	}
}

void MenuObjectPropDlg::OnCustomActionSelected(wxCommandEvent& evt) {
	m_targetChoice->Enable(false);
	m_chapterChoice->Enable(false);
	m_customActionEdit->Enable(true);
	if (m_customActionEdit->GetValue().length() == 0) {
		int tsi = m_object->GetAction().GetTsi();
		m_object->GetAction().SetTsi(GetSelectedTsi() != m_tsi ? GetSelectedTsi() : -2);
		m_customActionEdit->SetValue(m_object->GetAction().AsString(m_dvd));
		m_object->GetAction().SetTsi(tsi);
	}
}

wxString MenuObjectPropDlg::GetVideoFilename(bool withTimestamp, long position) {
	int tsi = -1;
	int pgci = 0;
	int chapter = 0;
	if (m_object->IsButton()) {
		if (!GetBool(0)) { // custom action
			DVDAction action(m_object->GetAction());
			action.SetCustom(m_customActionEdit->GetValue());
			int id = action.FindFirstJump(m_tsi);
			if (id >= 0) {
				tsi = DVD::GetTsi(id);
				pgci = DVD::GetPgci(id);
				chapter = DVD::GetVobi(id);
			}
		} else if (!IsSelectedMenu()) {
			tsi = GetSelectedTsi();
			pgci = GetSelectedPgci();
			chapter = GetSelectedChapter();
		}
	} else {
		if (m_videoChoice != NULL) {
			int id = (intptr_t) m_videoChoice->GetClientData(m_videoChoice->GetSelection());
			tsi = DVD::GetTsi(id);
			pgci = DVD::GetPgci(id);
		} else {
			tsi = 0;
		}
	}

	if (tsi < 0 || tsi >= (int) m_dvd->GetTitlesets().size())
		return wxT("");
	Pgc* pgc = m_dvd->GetPgc(tsi, false, pgci);
	if (!pgc)
		return wxT("");
	return withTimestamp ? pgc->GetVideoFrameUri(chapter, position) : pgc->GetVideoFrameUri(chapter, -1, true);
}

void MenuObjectPropDlg::OnImageRadio(wxCommandEvent& evt){
	m_imageEdit->Enable(m_imageRadio->GetValue());
	wxWindow* button = FindWindowByName(wxString::Format(wxT("SelectFileButton_%d"), m_imageEditIdx));
	if (button != NULL)
		button->Enable(m_imageRadio->GetValue());
	if (m_videoFrameRadio->IsEnabled())
		m_displayVideoFrame = m_videoFrameRadio->GetValue();
	if (m_videoChoice != NULL)
		m_videoChoice->Enable(m_videoFrameRadio->GetValue());
	m_videoFrameBt->Enable(m_videoFrameRadio->GetValue());
}

void MenuObjectPropDlg::OnVideoFrame(wxCommandEvent& evt) {
	VideoFrameDlg dlg(this, GetVideoFilename(false), false, m_defaultPos, m_videoPos, m_videoDuration);
	if (dlg.ShowModal() == wxID_OK) {
		m_videoPos = dlg.GetPos();
		m_videoDuration = dlg.GetDuration();
	}
}

bool MenuObjectPropDlg::CanJustifyText(wxString paramName) {
	wxString val = m_object->GetParam(paramName, wxT("x"));
	wxString right = wxT("100%");
	if (m_object->GetTextOffset().GetValueInSpecifiedUnits() != 0) {
		if (m_object->GetTextOffset().GetUnitType() == wxSVG_LENGTHTYPE_PERCENTAGE) {
			right = wxString::Format(wxT("%g%%"), 100 - m_object->GetTextOffset().GetValueInSpecifiedUnits());
		}
	}
	return val == m_object->GetTextOffset().GetValueAsString() || val == wxT("50%") || val == right;
}

void MenuObjectPropDlg::OnJustifyText(wxCommandEvent& evt) {
	((wxBitmapToggleButton*) this->FindWindow(JUSTIFY_LEFT_BT_ID))->SetValue(evt.GetId() == JUSTIFY_LEFT_BT_ID);
	((wxBitmapToggleButton*) this->FindWindow(JUSTIFY_CENTER_BT_ID))->SetValue(evt.GetId() == JUSTIFY_CENTER_BT_ID);
	((wxBitmapToggleButton*) this->FindWindow(JUSTIFY_RIGHT_BT_ID))->SetValue(evt.GetId() == JUSTIFY_RIGHT_BT_ID);
#if wxCHECK_VERSION(2,9,0)
	((wxBitmapToggleButton*) this->FindWindow(JUSTIFY_LEFT_BT_ID))->SetBitmap(evt.GetId() == JUSTIFY_LEFT_BT_ID
			? wxBITMAP_FROM_MEMORY(justifyLeftSelected) : wxBITMAP_FROM_MEMORY(justifyLeft));
	((wxBitmapToggleButton*) this->FindWindow(JUSTIFY_CENTER_BT_ID))->SetBitmap(evt.GetId() == JUSTIFY_CENTER_BT_ID
			? wxBITMAP_FROM_MEMORY(justifyCenterSelected) : wxBITMAP_FROM_MEMORY(justifyCenter));
	((wxBitmapToggleButton*) this->FindWindow(JUSTIFY_RIGHT_BT_ID))->SetBitmap(evt.GetId() == JUSTIFY_RIGHT_BT_ID
			? wxBITMAP_FROM_MEMORY(justifyRightSelected) : wxBITMAP_FROM_MEMORY(justifyRight));
#endif
}

void MenuObjectPropDlg::OnKeepAspectRatio(wxCommandEvent& evt) {
	m_heightEdit->SetValue(m_widthEdit->GetValue());
}

void MenuObjectPropDlg::OnAnimationsBt(wxCommandEvent& event) {
	AnimationDlg dlg(this, m_object);
	dlg.ShowModal();
}

/**
 *  Populate values on the model
 */
bool MenuObjectPropDlg::SetValues() {
	// check Action
	int customRadioIdx = 5;
	if (m_object->IsButton() && !m_multObjects) {
		if (GetBool(customRadioIdx) && GetString(customRadioIdx + 1).length() == 0) { // empty custom action
			wxMessageBox(_("Please enter the action command."),
			GetTitle(), wxOK|wxICON_ERROR, this);
			return false;
		}
		if (GetBool(customRadioIdx)) {
			DVDAction action(m_object->GetAction());
			action.SetCustom(GetString(customRadioIdx + 1));
			if (!action.IsValid(m_dvd, m_tsi, m_pgci, true, m_object->GetId(), true, false)
					&& !s_config.GetAcceptInvalidActions())
				return false;
		}
	}

	int n = 0;

	// set action
	if (m_object->IsButton() && !m_multObjects) {
		DVDAction& action = m_object->GetAction();
		action.SetTsi(GetSelectedTsi() != m_tsi ? GetSelectedTsi() : -2);
		action.SetPgci(GetSelectedTsi() != m_tsi || GetSelectedPgci() != m_pgci || !IsSelectedMenu() ? GetSelectedPgci() : -1);
		action.SetMenu(IsSelectedMenu());
		if (GetSelectedTsi() != m_tsi && GetSelectedTsi() != -1 && IsSelectedMenu() && GetSelectedPgci() > 0) {
			const StringSet& entries = m_dvd->GetPgcArray(GetSelectedTsi(), true)[GetSelectedPgci()]->GetEntries();
			action.SetEntry(wxString(entries.size() > 0 ? *entries.begin() : wxT("")));
		} else
			action.SetEntry(wxT(""));
		action.SetChapter(GetSelectedChapter());
		n += 3;
		action.SetPlayAll(GetBool(n++));
		action.SetPlayAllTitlesets(GetInt(n++) == 1);
		if (GetBool(n++)) {
			if (GetString(n) == wxT("jump title 1;")) {
				action.SetTsi(-2);
				action.SetPgci(0);
				action.SetMenu(false);
				action.SetChapter(0);
				action.SetCustom(_T(""));
			} else
				action.SetCustom(GetString(n));
		} else
			action.SetCustom(_T(""));
		n++;
		action.SetAudio(GetInt(n++) - 1);
		action.SetSubtitle(GetInt(n++) - 1);
		m_object->SetAutoExecute(GetBool(n++));

		wxString* id = (wxString*) GetClientData(n++);
		m_object->SetFocusDest(nbUP, *id);
		delete id;
		id = (wxString*) GetClientData(n++);
		m_object->SetFocusDest(nbLEFT, *id);
		delete id;
		id = (wxString*) GetClientData(n++);
		m_object->SetFocusDest(nbRIGHT, *id);
		delete id;
		id = (wxString*) GetClientData(n++);
		m_object->SetFocusDest(nbDOWN, *id);
		delete id;
	}

	// set look parameters
	for (unsigned int i = 0; i < m_object->GetParams().size(); i++) {
		MenuObjectParam* param = m_object->GetParams()[i];
		if (param->name == wxT("rotation")) {
			continue;
		}
		if (param->changeable && param->type == _T("colour")) {
			m_object->SetParamColour(param->name, GetColour(n++));
			m_object->SetParamColour(param->name, GetColour(n++), mbsHIGHLIGHTED);
			m_object->SetParamColour(param->name, GetColour(n++), mbsSELECTED);
		} else {
			if (param->type == _T("text")) {
				m_object->SetParam(param->name, GetString(n++));
				m_object->SetParamFont(param->name, GetFont(n++).GetChosenFont());
				if (CanJustifyText(param->name)) {
					if (GetBool(n++)) {
						m_object->SetParam(param->name, wxT("start"), wxT("text-anchor"));
						m_object->SetParam(param->name, m_object->GetTextOffset().GetValueAsString(), wxT("x"));
					}
					if (GetBool(n++)) {
						m_object->SetParam(param->name, wxT("middle"), wxT("text-anchor"));
						m_object->SetParam(param->name, wxT("50%"), wxT("x"));
					}
					if (GetBool(n++)) {
						m_object->SetParam(param->name, wxT("end"), wxT("text-anchor"));
						wxString xVal = wxT("100%");
						if (m_object->GetTextOffset().GetValueInSpecifiedUnits() != 0) {
							if (m_object->GetTextOffset().GetUnitType() == wxSVG_LENGTHTYPE_PERCENTAGE) {
								xVal = wxString::Format(wxT("%g%%"), 100
										- m_object->GetTextOffset().GetValueInSpecifiedUnits());
							}
						}
						m_object->SetParam(param->name, xVal, wxT("x"));
					}
				} else {
					n += 3;
				}
			} else if (param->type == _T("string")) {
				m_object->SetParam(param->name, GetString(n++));
			} else if (param->type.Mid(0, 7) == _T("integer")) {
				m_object->SetParamInt(param->name, GetInt(n++));
			} else if (param->type.Mid(0, 7) == _T("decimal")) {
				m_object->SetParamDouble(param->name, GetDouble(n++));
			} else if (param->type == _T("percent")) {
				m_object->SetParamDouble(param->name, ((double)GetInt(n++))/100);
			} else if (param->type == _T("colour")) {
				wxColour colour = GetColour(n++);
				m_object->SetParamColour(param->name, wxColour(colour.Red(), colour.Green(), colour.Blue()));
				wxString opacity = wxString::Format(wxT("%.3f"), (double) colour.Alpha() / 255);
				m_object->SetParam(param->name, opacity, wxT("-opacity"));
			} else if (param->type == _T("image") && !m_multObjects) {
				m_object->SetDisplayVideoFrame(m_displayVideoFrame);
				n += m_object->IsButton() ? 0 : 1; // video choice
				bool image = GetBool(n++); // image radio button
				wxString imgFile = GetString(n++);
				n++; // video radio button
				if (!image) {
					m_object->SetCustomVideoFrame(m_videoPos != m_defaultPos);
					m_object->SetParamImageVideo(param->name, GetVideoFilename(false), m_videoPos, m_videoDuration);
					if (!m_object->IsButton()) {
						m_object->SetDisplayVobId((intptr_t) m_videoChoice->GetClientData(m_videoChoice->GetSelection()));
					}
				} else {
					m_object->SetParamImageVideo(param->name, imgFile, -1, -1);
					m_object->SetDisplayVobId(DVD::MakeId(0, 0, 0));
				}
				// aspect ratio
				if (GetBool(n++)) {
					wxSVGPreserveAspectRatio aspectRatio;
					int alignX = GetInt(n++);
					int alignY = GetInt(n++);
					aspectRatio.SetAlign((wxSVG_PRESERVEASPECTRATIO) (wxSVG_PRESERVEASPECTRATIO_XMINYMIN + alignY * 3 + alignX));
					int meet = GetInt(n++);
					aspectRatio.SetMeetOrSlice(meet == 0 ? wxSVG_MEETORSLICE_MEET : wxSVG_MEETORSLICE_SLICE);
					m_object->SetParam(param->name, aspectRatio.GetValueAsString(), wxT("preserveAspectRatio"));
				} else {
					n += 3;
					m_object->SetParam(param->name, wxT("none"), wxT("preserveAspectRatio"));
				}
			} else if (param->type == wxT("shadow")) {
				m_object->SetParam(param->name, GetBool(n++) ? wxT("visible") : wxT("hidden"), wxT("visibility"));
				wxColour colour = GetColour(n++);
				m_object->SetParamColour(param->name, wxColour(colour.Red(), colour.Green(), colour.Blue()));
				wxString opacity = wxString::Format(wxT("%.3f"), (double) colour.Alpha() / 255);
				m_object->SetParam(param->name, opacity, wxT("-opacity"));
				m_object->SetParamInt(param->name, GetInt(n++), wxT("x"));
				m_object->SetParamInt(param->name, GetInt(n++), wxT("y"));
				wxSVGElement* filter = m_object->GetElementById(param->element.front() + wxT("Filter"));
				if (filter && ((wxSVGElement*)filter->GetFirstChild())->GetDtd() == wxSVG_FEGAUSSIANBLUR_ELEMENT) {
					wxSVGFEGaussianBlurElement* blur = (wxSVGFEGaussianBlurElement*) filter->GetFirstChild();
					double stdDeviation = GetDouble(n++);
					blur->SetStdDeviation(stdDeviation, stdDeviation);
				}
			}
		}
	}

	if (!m_multObjects) {
		// set position
		m_object->SetX(GetInt(n++));
		m_object->SetY(GetInt(n++));
		
		// set angle
		m_object->SetAngle(GetDouble(n++));

		// set size
		m_object->SetDefaultSize(!GetBool(n++));
		if (!m_object->IsDefaultSize()) {
			m_object->SetWidth(GetInt(n++));
			m_object->SetHeight(GetInt(n++));
			m_object->SetKeepAspectRatio(GetBool(n++));
		}
	}
	m_object->UpdateSize();

	return true;
}
