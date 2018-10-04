/////////////////////////////////////////////////////////////////////////////
// Name:        SlideDlg.cpp
// Purpose:     Slide properties dialog
// Author:      Alex Thuering
// Created:     23.12.2013
// RCS-ID:      $Id: SlideDlg.cpp,v 1.2 2014/01/01 19:47:06 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////
#include "SlideDlg.h"
#include "../wxVillaLib/utils.h"
#include <wx/file.h>

//(*InternalHeaders(SlideDlg)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

#include "../wxVillaLib/rc/loading.png.h"
#define TRANSITIONS_DIR wxFindDataDirectory(_T("transitions"))

//(*IdInit(SlideDlg)
const long SlideDlg::ID_FILENAME_TEXT = wxNewId();
const long SlideDlg::ID_THUMBNAILS = wxNewId();
//*)

BEGIN_EVENT_TABLE(SlideDlg, wxDialog)
	//(*EventTable(SlideDlg)
	//*)
	EVT_BUTTON(wxID_OK, SlideDlg::OnOkBt)
END_EVENT_TABLE()

SlideDlg::SlideDlg(wxWindow* parent, Slide* slide, int defaultTranstion) {
	m_slide = slide;
	
	//(*Initialize(SlideDlg)
	wxBoxSizer* boxSizer1;
	wxStdDialogButtonSizer* stdDialogButtonSizer;
	wxStaticText* staticText1;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxBoxSizer* mainSizer;

	Create(parent, wxID_ANY, _("Slide properties"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
	SetClientSize(wxSize(500,380));
	mainSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	staticText1 = new wxStaticText(this, wxID_ANY, _("File name:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	boxSizer1->Add(staticText1, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_fileNameText = new wxStaticText(this, ID_FILENAME_TEXT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_FILENAME_TEXT"));
	boxSizer1->Add(m_fileNameText, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	mainSizer->Add(boxSizer1, 0, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Transition:"));
	m_thumbnails = new wxThumbnails(this,ID_THUMBNAILS);
	StaticBoxSizer1->Add(m_thumbnails, 1, wxLEFT|wxRIGHT|wxEXPAND, 0);
	mainSizer->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
	stdDialogButtonSizer = new wxStdDialogButtonSizer();
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	stdDialogButtonSizer->Realize();
	mainSizer->Add(stdDialogButtonSizer, 0, wxALL|wxEXPAND, 5);
	SetSizer(mainSizer);
	SetSizer(mainSizer);
	Layout();
	Center();
	//*)
	stdDialogButtonSizer->GetAffirmativeButton()->SetDefault();
	
	m_fileNameText->SetLabel(m_slide->GetFilename());m_thumbnails->Clear();
	wxArrayString labels;
	Slideshow::GetTransitions(m_transitions, labels);
	labels[0] = _("Default") + wxString(wxT(" (")) + labels[defaultTranstion] + wxT(")");
	m_transitions[0] = wxT("");
	int sel = 0;
	for (unsigned int i = 0; i < m_transitions.GetCount(); i++) {
		if (m_transitions[i] == m_slide->GetTransition()) {
			sel = i;
		}
		wxImage img = getTransitionImage(m_transitions[i == 0 ? defaultTranstion : i]);
		wxThumb* thumb = new wxThumb(img, labels[i],  m_transitions[i]);
		m_thumbnails->InsertItem(thumb);
	}
	
	if (m_thumbnails->GetItemCount() > 0)
		m_thumbnails->SetSelected(sel);
	m_thumbnails->Update();
	m_thumbnails->SetFocus();
}

SlideDlg::~SlideDlg() {
	//(*Destroy(SlideDlg)
	//*)
}

wxImage SlideDlg::getTransitionImage(const wxString& transition) {
	wxString imgFile;
	if (transition.length() == 0 || transition == wxT("none")) {
		imgFile = TRANSITIONS_DIR + wxT("none.png");
	} else if (transition == wxT("random")) {
		imgFile = TRANSITIONS_DIR + wxT("random.png");
	} else {
		imgFile = TRANSITIONS_DIR + transition.BeforeLast(wxT('.')) + wxT(".png");
	}

	wxImage img;
	if (wxFile::Exists(imgFile)) {
		img.LoadFile(imgFile);
	} else {
		img = wxBITMAP_FROM_MEMORY(loading).ConvertToImage();
	}
	return img;
}

void SlideDlg::OnOkBt(wxCommandEvent& event) {
	m_slide->SetTransition(m_transitions[m_thumbnails->GetSelected() >= 0 ? m_thumbnails->GetSelected() : 0]);
	this->EndModal(wxID_OK);
}

