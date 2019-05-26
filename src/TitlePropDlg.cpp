/////////////////////////////////////////////////////////////////////////////
// Name:        TitlePropDlg.cpp
// Purpose:     DVD title properties dialog
// Author:      Alex Thuering
// Created:     31.01.2004
// RCS-ID:      $Id: TitlePropDlg.cpp,v 1.65 2016/05/11 20:02:37 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "TitlePropDlg.h"
#include "TitlesetManager.h"
#include "MPEG.h"
#include "Config.h"
#include "ChaptersDlg.h"
#include "SlideDlg.h"
#include <wxVillaLib/utils.h>
#include <wx/tokenzr.h>
#include <wx/regex.h>
#include <wx/file.h>
#include <wx/spinctrl.h>
#include "rc/add.png.h"
#include "rc/remove.png.h"
#include "rc/preferences.png.h"

wxRegEx s_chapterRegEx(wxT("^([[:digit:]][[:digit:]]?:)?([[:digit:]][[:digit:]]?:)?([[:digit:]][[:digit:]]?:)?[[:digit:]][[:digit:]]?(\\.[[:digit:]]+)?$"));

enum {
	VOB_BOX_ID = 7850,
	SLIDESHOW_BOX_ID,
	ADD_BT_ID,
	REMOVE_BT_ID,
	PROP_BT_ID,
	DO_NOT_TRANSCODE_CB_ID,
	CHAPTERS_BT_ID,
	AUDIO_CTRL_ID
};

BEGIN_EVENT_TABLE(TitlePropDlg, wxPropDlg)
	EVT_LISTBOX(VOB_BOX_ID, TitlePropDlg::OnSelectItem)
	EVT_THUMBNAILS_SEL_CHANGED(SLIDESHOW_BOX_ID, TitlePropDlg::OnSelectItem)
	EVT_THUMBNAILS_DCLICK(SLIDESHOW_BOX_ID, TitlePropDlg::OnThumbDoubleClick)
	EVT_BUTTON(ADD_BT_ID, TitlePropDlg::OnAddBt)
	EVT_BUTTON(REMOVE_BT_ID, TitlePropDlg::OnRemoveBt)
	EVT_BUTTON(PROP_BT_ID, TitlePropDlg::OnPropBt)
	EVT_CHECKBOX(DO_NOT_TRANSCODE_CB_ID, TitlePropDlg::OnDoNotTranscodeCheck)
	EVT_BUTTON(CHAPTERS_BT_ID, TitlePropDlg::OnChaptersBt)
	EVT_TEXT(AUDIO_CTRL_ID, TitlePropDlg::OnChangeAudio)
END_EVENT_TABLE()

TitlePropDlg::TitlePropDlg(wxWindow* parent, DVD* dvd, int tsi, int pgci, int vobi): wxPropDlg(parent) {
	m_dvd = dvd;
	m_tsi = tsi;
	m_pgci = pgci;
	m_pgcs = &m_dvd->GetPgcArray(tsi, false);
	m_pgc = (*m_pgcs)[pgci];
	m_vob = new Vob(*m_pgc->GetVobs()[vobi]);
	m_vobi = vobi;
	m_langCodes = &m_pgcs->GetAudioLangCodes();
	m_vobListBox = NULL;
	m_doNotTranscodeCheckBox = NULL;
	m_chaptersCtrl = NULL;
	m_chaptersBt = NULL;
	m_pause = NULL;
	m_slideShowBox = NULL;
	m_audioFormat = NULL;
	// set title
	wxString title = _("Properties") + wxString(wxT(" - "));
	if (dvd->GetTitlesets().Count() > 1)
		title += _("Titleset") + wxString::Format(wxT(" %d "), tsi+1);
	title += _("Title") + wxString::Format(wxT(" %d"), pgci+1);
	if (vobi>0)
		title += wxString::Format(wxT("-%d"), vobi+1);
	SetTitle(title);
	// create
	Create();
	// update vobBox
	if (m_vobListBox)
		m_vobListBox->SetSelection(0);
	wxCommandEvent evt;
	OnSelectItem(evt);
	if (m_slideShowBox)
		m_slideShowBox->SetFocus();
}

TitlePropDlg::~TitlePropDlg() {
	if (m_vob)
		delete m_vob;
}

void TitlePropDlg::CreatePropPanel(wxSizer* sizer) {
	if (m_vob->GetSlideshow()) {
		CreateSlideshowGroup(sizer);
	} else
		CreateVobGroup(sizer);
	CreateTitleGroup(sizer);
}

void TitlePropDlg::CreateVobGroup(wxSizer* sizer) {
	wxSizer* grpSizer = BeginGroup(sizer, _("Video object"), wxT(""), false, false, true, 1);
	wxBoxSizer* vobPane = new wxBoxSizer(wxHORIZONTAL);
	grpSizer->Add(vobPane, 1, wxEXPAND);
	
	wxScrolledWindow* scrollWin = new wxScrolledWindow(this);
	m_vobListBox = new VobListBox(scrollWin, VOB_BOX_ID, m_vob, m_pgcs, m_dvd, this);
	m_vobListBox->Fit();
	wxBoxSizer* scrollSize = new wxBoxSizer(wxVERTICAL);
	scrollSize->Add(m_vobListBox, 1, wxEXPAND);
	scrollWin->SetSizer(scrollSize);
	scrollWin->SetMinSize(wxSize(m_vobListBox->GetClientSize().GetWidth() + 32, 300));
	scrollWin->FitInside();
	scrollWin->SetScrollRate(5, 5);
	vobPane->Add(scrollWin, 1, wxEXPAND);
	
	wxBoxSizer* buttonPane = new wxBoxSizer(wxVERTICAL);
	vobPane->Add(buttonPane, 0, wxEXPAND|wxLEFT, 4);
	m_addBt = new wxBitmapButton(this, ADD_BT_ID, wxBITMAP_FROM_MEMORY(add));
	m_addBt->SetHelpText(_("Add file..."));
	buttonPane->Add(m_addBt, 0, wxALIGN_CENTER);
	buttonPane->Add(4, 4);
	m_delBt = new wxBitmapButton(this, REMOVE_BT_ID, wxBITMAP_FROM_MEMORY(remove));
	m_delBt->SetHelpText(_("Remove file"));
	buttonPane->Add(m_delBt, 0, wxALIGN_CENTER);
	buttonPane->Add(4, 4);
	m_propBt = new wxBitmapButton(this, PROP_BT_ID, wxBITMAP_FROM_MEMORY(preferences));
	m_propBt->SetHelpText(_("Properties..."));
	buttonPane->Add(m_propBt, 0, wxALIGN_CENTER);
	buttonPane->Add(10, 10, 1, wxEXPAND);
	
	wxFlexGridSizer* grid = new wxFlexGridSizer(2, 4, 4);
	grpSizer->Add(grid, 0, wxEXPAND|wxALL, 6);
	grid->AddGrowableCol(1);
	AddText(grid, wxT(""));
	AddCheckProp(grid, _("do not remultiplex/transcode"), m_vob->GetDoNotTranscode(), false, DO_NOT_TRANSCODE_CB_ID);
	m_doNotTranscodeCheckBox = (wxCheckBox*) GetLastControl();
	UpdateDoNotTranscodeCheck();
	grid->Add(2, 2);
	grid->Add(2, 2);
	// chapters
	AddText(grid, _("Chapters:"));
	wxBoxSizer* chaptersPane = new wxBoxSizer(wxHORIZONTAL);
	grid->Add(chaptersPane, 0, wxEXPAND);
	AddTextProp(chaptersPane, wxT(""), m_vob->GetChapters());
	m_chaptersCtrl = (wxTextCtrl*) GetLastControl();
	m_chaptersBt = new wxButton(this, CHAPTERS_BT_ID, wxT("..."));
	int h = m_chaptersBt->GetSize().GetHeight() > 24 ? m_chaptersBt->GetSize().GetHeight() : 24;
	m_chaptersBt->SetSizeHints(h, h, h, h);
	chaptersPane->Add(m_chaptersBt, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
	// Pause
	AddSpinProp(grid, _("Pause:"), m_vob->GetPause(), -1, 254, 100, _("sec"));
	m_pause = (wxSpinCtrl*) GetLastControl();
	sizer->AddSpacer(6);
	// update/enable chapters control
	UpdateChaptersCtrl();
}

void TitlePropDlg::CreateSlideshowGroup(wxSizer* sizer) {
	Slideshow* slideshow = m_vob->GetSlideshow();
	wxSizer* grpSizer = BeginGroup(sizer, _("Slideshow"), wxT(""), false, false, true, 1);
	wxBoxSizer* vobPane = new wxBoxSizer(wxHORIZONTAL);
	vobPane->SetMinSize(646, 320);
	grpSizer->Add(vobPane, 1, wxEXPAND);
	
	m_slideShowBox = new wxThumbnails(this, SLIDESHOW_BOX_ID);
	m_slideShowBox->EnableDragging(false);
	for (unsigned int i = 0; i < slideshow->GetCount(); i++) {
		m_slideShowBox->InsertItem(new wxThumb(slideshow->GetSlide(i)->GetFilename(),
				slideshow->GetSlide(i)->GetFilename().AfterLast(wxFILE_SEP_PATH)));
	}
	vobPane->Add(m_slideShowBox, 1, wxEXPAND);
	
	wxBoxSizer* buttonPane = new wxBoxSizer(wxVERTICAL);
	vobPane->Add(buttonPane, 0, wxEXPAND|wxLEFT, 4);
	m_addBt = new wxBitmapButton(this, ADD_BT_ID, wxBITMAP_FROM_MEMORY(add));
	m_addBt->SetHelpText(_("Add image..."));
	buttonPane->Add(m_addBt, 0, wxALIGN_CENTER);
	buttonPane->Add(4, 4);
	m_delBt = new wxBitmapButton(this, REMOVE_BT_ID, wxBITMAP_FROM_MEMORY(remove));
	m_delBt->SetHelpText(_("Remove image"));
	buttonPane->Add(m_delBt, 0, wxALIGN_CENTER);
	buttonPane->Add(4, 4);
	m_propBt = new wxBitmapButton(this, PROP_BT_ID, wxBITMAP_FROM_MEMORY(preferences));
	m_propBt->SetHelpText(_("Properties..."));
	buttonPane->Add(m_propBt, 0, wxALIGN_CENTER);
	buttonPane->Add(10, 10, 1, wxEXPAND);
	
	wxFlexGridSizer* grid = new wxFlexGridSizer(2, 4, 4);
	grid->AddGrowableCol(1);
	wxArrayString formats = DVD::GetVideoFormatLabels();
	int vf = slideshow->GetVideoFormat() - 2;
	wxSizer* formatSizer = AddChoiceProp(grid, _("Format:"), formats[vf], formats, GetCharWidth()*15, false);
	formatSizer->AddSpacer(4);
	formats = DVD::GetAspectRatioLabels();
	int ar = slideshow->GetAspectRatio() >= 1 ? slideshow->GetAspectRatio() - 1 : 0;
	AddChoiceProp(formatSizer, wxT(""), formats[ar], formats, GetCharWidth()*8);
	AddSpinProp(grid, _("Duration:"), slideshow->GetDuration(), 1, 9999, GetCharWidth()*8, _("sec"));
	
	// transitions
	wxArrayString labels;
	Slideshow::GetTransitions(m_transitions, labels);
	int sel = 0;
	for (unsigned int i = 0; i < m_transitions.GetCount(); i++) {
		if (m_transitions[i] == slideshow->GetTransition()) {
			sel = i;
		}
	}
	AddChoiceProp(grid, _("Transition:"), labels[sel], labels, 0, false);
	wxSize minSize = GetLastControl()->GetBestSize();
	minSize.SetWidth(GetCharWidth()*23 + 4);
	GetLastControl()->SetMinSize(minSize);
//	AddText(grid, _("Transition:"), wxALIGN_CENTER_HORIZONTAL);
//	m_transitionBox = new wxThumbnails(this, SLIDESHOW_BOX_ID);
//	m_transitionBox->SetThumbSize(70, 60, 8);
//	m_transitionBox->EnableDragging(false);
//	int sel = 0;
//	for (unsigned int i = 0; i < m_transitions.GetCount(); i++) {
//		if (m_transitions[i] == slideshow->GetTransition()) {
//			sel = i;
//		}
//		wxImage img =  SlideDlg::getTransitionImage(m_transitions[i]);
//		wxThumb* thumb = new wxThumb(img, labels[i],  m_transitions[i]);
//		m_transitionBox->InsertItem(thumb);
//	}
//	
//	if (m_transitionBox->GetItemCount() > 0)
//		m_transitionBox->SetSelected(sel);
//	m_transitionBox->Update();
//	grid->Add(m_transitionBox, 0, wxEXPAND);
	
	// audio file
	wxString audioFile = m_vob->GetAudioFilenames().size() ? m_vob->GetAudioFilenames()[0] : wxT("");
	wxString audio = wxThumbnails::GetAudioExtWildcard();
	wxSizer* audioSizer = AddFileProp(grid, _("Audio file:"), audioFile, wxFD_OPEN, wxT("..."),
			_("Audio Files") + wxString::Format(wxT(" (%s)|%s|"), audio.c_str(), audio.c_str())
			+ _("All Files") + wxString(wxT(" (*.*)|*.*")), AUDIO_CTRL_ID);
	// audio format
	Stream* stream = audioFile.length() > 0 ? m_vob->GetStreams()[m_vob->GetStreams().size() - 1] : NULL;
	int af = stream != NULL ? stream->GetAudioFormat() - 1 : afCOPY - 1;
	formats = DVD::GetAudioFormatLabels(true);
	audioSizer->Add(AddChoiceProp(formats[af], formats));
	m_audioFormat = (wxChoice*) GetLastControl();
	m_audioFormat->Enable(audioFile.length() > 0);
	labels = DVD::GetAudioLanguageCodes();
	if (m_langCodes->GetCount() < 1)
		m_langCodes->Add(s_config.GetDefAudioLanguage());
	audioSizer->AddSpacer(4);
	audioSizer->Add(AddChoiceProp((*m_langCodes)[0], labels));
	
	grpSizer->Add(grid, 0, wxEXPAND|wxALL, 6);
	sizer->AddSpacer(6);
}

void TitlePropDlg::CreateAudioGroup(wxSizer* sizer) {
	wxSizer* grpSizer = BeginGroup(sizer, _("Audio"));
	wxFlexGridSizer* grid = new wxFlexGridSizer(2, 4, 16);
	grid->AddGrowableCol(1);
	
	grpSizer->Add(grid, 0, wxEXPAND|wxALL, 4);
	sizer->AddSpacer(6);
}

void TitlePropDlg::CreateTitleGroup(wxSizer* sizer) {
	wxSizer* grpSizer = BeginGroup(sizer, _("Title"));
	wxFlexGridSizer* grid = new wxFlexGridSizer(3, 2, 4, 4);
	grid->AddGrowableCol(1);
	
	AddFileProp(grid, _("Palette:"), m_pgc->GetPalette(), wxFD_OPEN, wxT("..."), wxT("*.rgb;*.yuv"));
	wxArrayString commands = GetCommandList(m_dvd, m_tsi);
	AddComboProp(grid, _("Pre commands:"), m_pgc->GetPreCommands(), commands);
	commands.Add(wxT("exit;"));
	AddComboProp(grid, _("Post commands:"), m_pgc->GetPostCommands(), commands);
	grpSizer->Add(grid, 0, wxEXPAND|wxALL, 6);
}

/**
 * Returns list of commands for choice
 */
wxArrayString TitlePropDlg::GetCommandList(DVD* dvd, int currentTsi) {
	wxArrayString commands;
#ifdef __WXMAC__
	commands.Add(wxT(""));
#endif
	commands.Add(wxT("call last menu;"));
	for (unsigned int pgci=0; pgci < dvd->GetVmgm().Count(); pgci++)
		commands.Add(wxString::Format(wxT("call vmgm menu %d;"),pgci+1));
	if (dvd->GetVmgm().Count() == 0 && dvd->IsEmptyMenuEnabled())
		commands.Add(wxT("call vmgm menu 1;"));
	for (int tsi=0; tsi<(int)dvd->GetTitlesets().Count(); tsi++) {
		Titleset* ts = dvd->GetTitlesets()[tsi];
		if (tsi == currentTsi) { // same titleset
			if (ts->GetMenus().GetCount())
				commands.Add(wxT("call menu;"));
			const StringSet& entrySet = ts->GetMenus().GetEntries();
			for (StringSet::const_iterator entry = entrySet.begin(); entry != entrySet.end(); entry++)
				if (*entry != wxT("root"))
					commands.Add(wxString::Format(wxT("call menu entry %s;"), entry->c_str()));
			commands.Add(wxT("jump next title;"));
		}
		if (dvd->IsJumppadsEnabled()) {
			for (int pgci=1; pgci<(int)ts->GetMenus().Count(); pgci++)
				commands.Add(wxString::Format(wxT("call titleset %d menu %d;"), tsi+1, pgci+1));
		}
		if (dvd->IsJumppadsEnabled() || tsi == currentTsi) {
			for (int pgci=0; pgci<(int)ts->GetTitles().Count(); pgci++)
				if (tsi == currentTsi) // same titleset
					commands.Add(wxString::Format(wxT("jump title %d;"), pgci+1));
				else
					commands.Add(wxString::Format(wxT("jump titleset %d title %d;"), tsi+1, pgci+1));
		}
	}
	return commands;
}

bool TitlePropDlg::SetValues() {
	// validate audio file
	if (m_vob->GetSlideshow()) {
		wxString audioFile = GetString(4);
		if (audioFile.length() > 0) {
			if (!wxFile::Exists(audioFile)) {
				wxLogError(_("File %s doesn't exist."), audioFile.c_str());
				return false;
			}
			wxFfmpegMediaDecoder ffmpeg;
			if (!ffmpeg.Load(audioFile))
				return false;
			if (ffmpeg.GetStreamCount() == 0 || ffmpeg.GetStreamType(0) != stAUDIO) {
				wxLogError(_("File %s isn't audio file."), audioFile.c_str());
				return false;
			}
		}
	}
	
	// check commands
	int n = m_vob->GetSlideshow() ? 8 : 4;
	wxString preCommands = GetString(n++);
	wxString postCommands = GetString(n++);
	DVDAction action;
	action.SetCustom(preCommands);
	if (preCommands.length() && !action.IsValid(m_dvd, m_tsi, m_pgci, false, wxT(""), true, false)) {
		return false;
	}
	action.SetCustom(postCommands);
	if (postCommands.length() && !action.IsValid(m_dvd, m_tsi, m_pgci, false, wxT(""), true, false)) {
		return false;
	}

	// check chapters
	if (!m_vob->GetSlideshow() && m_chaptersCtrl->IsEnabled()) {
		wxString chapters = m_chaptersCtrl->GetValue();
		wxStringTokenizer tkz(chapters, _T(","));
		while (tkz.HasMoreTokens()) {
			wxString token = tkz.GetNextToken().Strip(wxString::both);
			if (!s_chapterRegEx.Matches(token)) {
				wxLogError(_("Invalid chapter timestamp '%s'"), token.c_str());
				return false;
			}
		}
	}
	
	// update values
	n = 0;
	if (m_vob->GetSlideshow()) {
		Slideshow* slideshow = m_vob->GetSlideshow();
		slideshow->SetVideoFormat((VideoFormat) (GetInt(n++)+2));
		slideshow->SetAspectRatio((AspectRatio) (GetInt(n++)+1));
		m_pgcs->GetVideo().SetAspect(slideshow->GetAspectRatio());
		slideshow->SetDuration(GetInt(n++));
		slideshow->SetTransition(m_transitions[GetInt(n++)]);
		
		// audio properties
		wxString audioFile = GetString(n++).Strip(wxString::both);
		while (m_vob->GetAudioFilenames().GetCount() > 0)
			m_vob->RemoveAudioFile(0);
		if (audioFile.length() > 0) {
			m_vob->AddAudioFile(audioFile);
			if (m_vob->GetStreams().size() > 0) {
				Stream* stream = m_vob->GetStreams()[m_vob->GetStreams().size() - 1];
				if (stream->GetType() == stAUDIO)
					stream->SetAudioFormat((AudioFormat) (GetInt(n++) + 1));
			}
		} else
			n++;
		m_langCodes->RemoveAt(0);
		m_langCodes->Insert(GetString(n++), 0);
	} else {
		// update vob
		m_vobListBox->SetValues();
		m_vob->SetDoNotTranscode(GetBool(n++) && m_vob->GetAudioFilenames().GetCount() == 0);
		if (m_chaptersCtrl->IsEnabled()) {
			m_vob->SetChapters(GetString(n++), m_vobi == 0);
			m_vob->SetPause(GetInt(n++));
		} else {
			m_vob->SetPause(0);
			n += 2;
		}
		// update video parameters
		m_pgcs->GetVideo().SetAspect(m_vobListBox->GetAspectRatio());
		if (m_pgci == 0 && m_vobi == 0 && m_vob->GetVideoStream())
			m_pgcs->GetVideo().SetFormat(m_vob->GetVideoStream()->GetVideoFormat());
	}
	m_pgc->SetPalette(GetString(n++));
	m_pgc->SetPreCommands(GetString(n++));
	m_pgc->SetPostCommands(GetString(n++));
	
	// replace old VOB with new on in PGC
	delete m_pgc->GetVobs()[m_vobi];
	m_pgc->GetVobs()[m_vobi] = m_vob;
	m_vob = NULL;
	return true;
}

void TitlePropDlg::OnSelectItem(wxCommandEvent& event) {
	if (m_slideShowBox) {
		m_delBt->Enable(m_slideShowBox->GetSelectedCount() > 0);
		m_propBt->Enable(m_slideShowBox->GetSelectedCount() > 0);
	} else {
		m_delBt->Enable(m_vobListBox->GetSelection() > 0);
	}
}

void TitlePropDlg::OnAddBt(wxCommandEvent& event) {
	if (m_slideShowBox != NULL) {
		wxFileDialog fileDlg(this, _("Choose a file"), wxT(""), wxT(""),
				_("Image Files ") + wxImage::GetImageExtWildcard()
				+ wxT("|") + wxString(_("All Files")) + wxT(" (*.*)|*.*"), wxFD_OPEN | wxFD_MULTIPLE);
		fileDlg.SetDirectory(s_config.GetLastAddDir() + wxFILE_SEP_PATH);
		if (fileDlg.ShowModal() == wxID_OK) {
			wxArrayString paths;
			fileDlg.GetPaths(paths);
			for (unsigned int i = 0; i < paths.GetCount(); i++) {
				m_vob->GetSlideshow()->AddSlide(new Slide(paths[i]));
				m_slideShowBox->InsertItem(new wxThumb(paths[i], paths[i].AfterLast(wxFILE_SEP_PATH)));
			}
		}
		return;
	}
	wxString audio = wxThumbnails::GetAudioExtWildcard();
	wxString subtitle = wxThumbnails::GetSubtitleExtWildcard();
	wxFileDialog dialog(this, _("Choose a file"), wxT(""), wxT(""),
			_("Audio & Subtitle Files") + wxString::Format(wxT("|%s%s|"), audio.c_str(), subtitle.c_str())
			+ _("Audio Files") + wxString::Format(wxT(" (%s)|%s|"), audio.c_str(), audio.c_str())
			+ _("Subtitle Files") + wxString::Format(wxT(" (%s)|%s"), subtitle.c_str(), subtitle.c_str()),
			wxFD_OPEN);
	// set file browser dir
	dialog.SetDirectory(s_config.GetLastAddDir() + wxFILE_SEP_PATH);
	if (dialog.ShowModal() == wxID_OK) {
		if (wxThumbnails::IsAudio(dialog.GetPath()))
			m_vobListBox->AddAudio(dialog.GetPath());
		else
			m_vobListBox->AddSubtitle(dialog.GetPath());
		s_config.SetLastAddDir(dialog.GetDirectory());
		UpdateDoNotTranscodeCheck();
	}
}

int cmpIntRev(int* i1, int* i2)  {
	return *i1 < *i2 ? 1 : (*i1 > *i2 ? -1 : 0);
}

void TitlePropDlg::OnRemoveBt(wxCommandEvent& event) {
	if (m_slideShowBox != NULL) {
		wxArrayInt selected = m_slideShowBox->GetSelectedArray();
		selected.Sort(cmpIntRev);
		for (unsigned int i = 0; i < selected.GetCount(); i++) {
			m_vob->GetSlideshow()->RemoveSlide(selected[i]);
			m_slideShowBox->RemoveItemAt(selected[i]);
		}
		m_slideShowBox->SetSelected(-1);
		m_slideShowBox->Refresh();
		return;
	}
	m_vobListBox->RemoveItem(m_vobListBox->GetSelection());
	wxCommandEvent evt;
	OnSelectItem(evt);
	UpdateDoNotTranscodeCheck();
}

void TitlePropDlg::OnPropBt(wxCommandEvent& event) {
	if (m_slideShowBox != NULL) {
		SlideDlg slideDlg(this, m_vob->GetSlideshow()->GetSlide(m_slideShowBox->GetSelected()), GetInt(3));
		slideDlg.ShowModal();
		return;
	}
	m_vobListBox->ShowPropDialog();
	UpdateDoNotTranscodeCheck();
}

void TitlePropDlg::OnThumbDoubleClick(wxCommandEvent& event) {
	if (m_slideShowBox->GetSelected() >= 0) {
		SlideDlg slideDlg(this, m_vob->GetSlideshow()->GetSlide(m_slideShowBox->GetSelected()), GetInt(3));
		slideDlg.ShowModal();
	}
}

void TitlePropDlg::OnDoNotTranscodeCheck(wxCommandEvent& event) {
	m_vobListBox->SetDoNotTranscode(event.IsChecked());
}

void TitlePropDlg::UpdateDoNotTranscodeCheck() {
	if (m_doNotTranscodeCheckBox == NULL)
		return;
	m_doNotTranscodeCheckBox->SetValue(m_vobListBox->GetDoNotTranscode());
	m_doNotTranscodeCheckBox->Enable(!m_vobListBox->HasAudioFiles() && MPEG::HasNavPacks(m_vob->GetFilename()));
}
void TitlePropDlg::OnChaptersBt(wxCommandEvent& event) {
	if (!ApplyChaptersCtrl())
		return;
	ChaptersDlg dialog(this, m_dvd, m_tsi, m_pgci, m_vobi, m_vob);
	if (dialog.ShowModal() == wxID_OK)
		UpdateChaptersCtrl();
}

bool TitlePropDlg::ApplyChaptersCtrl() {
	// update
	if (m_chaptersCtrl->IsEnabled()) {
		wxStringTokenizer tkz(m_chaptersCtrl->GetValue(), _T(","));
		while (tkz.HasMoreTokens()) {
			wxString token = tkz.GetNextToken().Strip(wxString::both);
			if (!s_chapterRegEx.Matches(token)) {
				wxLogError
				(_("Invalid chapter timestamp '%s'"), token.c_str());
				return false;
			}
		}
		m_vob->SetChapters(m_chaptersCtrl->GetValue(), m_vobi == 0);
	}
	return true;
}

void TitlePropDlg::UpdateChaptersCtrl() {
	m_chaptersCtrl->SetValue(m_vob->GetChapters());
	bool readonly = false;
	if (m_vob->GetCells().size() > 0 && m_vob->GetCells().front()->GetStart() != 0)
		readonly = true;
	for (vector<Cell*>::iterator it = m_vob->GetCells().begin(); it != m_vob->GetCells().end(); it++) {
		if (!(*it)->IsChapter() || (*it)->GetEnd() >= 0 || (*it)->GetPause() != 0
				|| (*it)->GetCommands().length() > 0) {
			readonly = true;
			break;
		}
	}
	m_chaptersCtrl->Enable(!readonly);
	if (readonly)
		m_pause->SetValue(0);
	m_pause->Enable(!readonly);
}

void TitlePropDlg::OnChangeAudio(wxCommandEvent& event) {
	if (!m_audioFormat)
		return;
	wxString audioFile = event.GetString();
	m_audioFormat->Enable(audioFile.length() > 0);
	// set audio format
	wxFfmpegMediaDecoder ffmpeg;
	if (wxFile::Exists(audioFile) && ffmpeg.Load(audioFile) && ffmpeg.GetStreamCount() > 0
			&& ffmpeg.GetStreamType(0) == stAUDIO) {
		AudioFormat af = afCOPY;
		wxString codecName = ffmpeg.GetCodecName(0);
		int sampleRate = ffmpeg.GetSampleRate(0);
		if (codecName != wxT("mp2") && codecName != wxT("ac3") && codecName != wxT("liba52"))
			af = m_dvd->GetAudioFormat();
		else if (sampleRate != 48000)
			af = codecName == wxT("mp2") ? afMP2 : afAC3;
		m_audioFormat->SetSelection(af - 1);
	}
}
