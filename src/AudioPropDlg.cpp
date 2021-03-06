/////////////////////////////////////////////////////////////////////////////
// Name:        AudioPropDlg.cpp
// Author:      Alex Thuering
// Created:     23.07.2011
// RCS-ID:      $Id: AudioPropDlg.cpp,v 1.16 2016/06/04 22:05:36 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "AudioPropDlg.h"
#include "mediatrc_ffmpeg.h"
#include "ProcessTranscode.h"
#include "Utils.h"
#include <wxVillaLib/utils.h>

#include "rc/refresh.png.h"

//(*InternalHeaders(AudioPropDlg)
#include <wx/bitmap.h>
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(AudioPropDlg)
const long AudioPropDlg::ID_STATICTEXT2 = wxNewId();
const long AudioPropDlg::ID_STATICTEXT4 = wxNewId();
const long AudioPropDlg::ID_STATICTEXT6 = wxNewId();
const long AudioPropDlg::ID_DST_CHOICE = wxNewId();
const long AudioPropDlg::ID_A51_CHECK = wxNewId();
const long AudioPropDlg::ID_LANG_CHOICE = wxNewId();
const long AudioPropDlg::ID_TEXTCTRL1 = wxNewId();
const long AudioPropDlg::ID_SPINBUTTON2 = wxNewId();
const long AudioPropDlg::ID_TEXTCTRL2 = wxNewId();
const long AudioPropDlg::ID_RADIOBUTTON1 = wxNewId();
const long AudioPropDlg::ID_RADIOBUTTON2 = wxNewId();
const long AudioPropDlg::ID_SPINCTRL1 = wxNewId();
const long AudioPropDlg::ID_RADIOBUTTON4 = wxNewId();
const long AudioPropDlg::ID_RADIOBUTTON3 = wxNewId();
const long AudioPropDlg::ID_TEXTCTRL3 = wxNewId();
const long AudioPropDlg::ID_SPINBUTTON3 = wxNewId();
const long AudioPropDlg::ID_CALC_GAIN_BT = wxNewId();
const long AudioPropDlg::ID_STATICTEXT_GAIN = wxNewId();
//*)

BEGIN_EVENT_TABLE(AudioPropDlg,wxDialog)
	//(*EventTable(AudioPropDlg)
	//*)
END_EVENT_TABLE()

AudioPropDlg::AudioPropDlg(wxWindow* parent,Vob* vob, const wxString& audioFile, const wxString& langCode,
		int streamIdx) {
	m_vob = vob;
	m_audioFile = audioFile;
	m_langCode = langCode;
	m_streamIdx = streamIdx;
	m_stream = vob->GetStreams()[streamIdx];

	//(*Initialize(AudioPropDlg)
	wxStaticText* label9;
	wxBoxSizer* normBoxSizer;
	wxFlexGridSizer* propGridSizer;
	wxBoxSizer* boxSizer3;
	wxStaticText* label7;
	wxStaticText* label4;
	wxGridBagSizer* GridBagSizer2;
	wxStaticText* label8;
	wxBoxSizer* boxSizer2;
	wxStaticText* label3;
	wxBoxSizer* mainVSizer;
	wxStaticText* label1;
	wxStdDialogButtonSizer* stdDialogButtonSizer;
	wxStaticText* label2;
	wxBoxSizer* BoxSizer1;
	wxStaticBoxSizer* volAdjustBox;
	wxStaticText* label5;

	Create(parent, wxID_ANY, _("Audio properties"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
	mainVSizer = new wxBoxSizer(wxVERTICAL);
	propGridSizer = new wxFlexGridSizer(0, 2, 4, 4);
	propGridSizer->AddGrowableCol(1);
	label1 = new wxStaticText(this, wxID_ANY, _("File Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	propGridSizer->Add(label1, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_fileNameText = new wxStaticText(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	propGridSizer->Add(m_fileNameText, 1, wxEXPAND, 5);
	label2 = new wxStaticText(this, wxID_ANY, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	propGridSizer->Add(label2, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_durText = new wxStaticText(this, ID_STATICTEXT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	propGridSizer->Add(m_durText, 1, wxEXPAND, 5);
	label3 = new wxStaticText(this, wxID_ANY, _("Source Format:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	propGridSizer->Add(label3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	m_srcText = new wxStaticText(this, ID_STATICTEXT6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	propGridSizer->Add(m_srcText, 1, wxALL|wxEXPAND, 0);
	label4 = new wxStaticText(this, wxID_ANY, _("Destination Format:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	propGridSizer->Add(label4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	boxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_dstChoice = new wxChoice(this, ID_DST_CHOICE, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_DST_CHOICE"));
	boxSizer3->Add(m_dstChoice, 0, wxALIGN_CENTER_VERTICAL, 0);
	m_audio51 = new wxCheckBox(this, ID_A51_CHECK, _("5.1"), wxDefaultPosition, wxSize(-1,-1), 0, wxDefaultValidator, _T("ID_A51_CHECK"));
	m_audio51->SetValue(false);
	boxSizer3->Add(m_audio51, 0, wxLEFT|wxEXPAND, 5);
	propGridSizer->Add(boxSizer3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	label5 = new wxStaticText(this, wxID_ANY, _("Language:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	propGridSizer->Add(label5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	m_langChoice = new wxChoice(this, ID_LANG_CHOICE, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LANG_CHOICE"));
	propGridSizer->Add(m_langChoice, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	label7 = new wxStaticText(this, wxID_ANY, _("Time delay:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	propGridSizer->Add(label7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	boxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_offsetCtrl = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	boxSizer2->Add(m_offsetCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_offsetSpin = new wxSpinButton(this, ID_SPINBUTTON2, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL|wxSP_ARROW_KEYS, _T("ID_SPINBUTTON2"));
	m_offsetSpin->SetRange(0, 999);
	m_offsetSpin->SetMinSize(wxSize(16,12));
	boxSizer2->Add(m_offsetSpin, 0, wxEXPAND, 0);
	label8 = new wxStaticText(this, wxID_ANY, _("ms"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	boxSizer2->Add(label8, 0, wxALL|wxEXPAND, 5);
	propGridSizer->Add(boxSizer2, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	label9 = new wxStaticText(this, wxID_ANY, _("Filters:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	propGridSizer->Add(label9, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_filtersCtrl = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	propGridSizer->Add(m_filtersCtrl, 0, wxEXPAND, 5);
	mainVSizer->Add(propGridSizer, 0, wxTOP|wxLEFT|wxRIGHT|wxEXPAND, 5);
	volAdjustBox = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Volume adjustment"));
	GridBagSizer2 = new wxGridBagSizer(4, 4);
	GridBagSizer2->AddGrowableCol(1);
	m_volNoBt = new wxRadioButton(this, ID_RADIOBUTTON1, _("No"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
	GridBagSizer2->Add(m_volNoBt, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	m_volChange1Bt = new wxRadioButton(this, ID_RADIOBUTTON2, _("Change (%)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
	GridBagSizer2->Add(m_volChange1Bt, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	m_volume1Ctrl = new wxSpinCtrl(this, ID_SPINCTRL1, _T("100"), wxDefaultPosition, wxSize(60,-1), 0, 0, 999, 100, _T("ID_SPINCTRL1"));
	m_volume1Ctrl->SetValue(_T("100"));
	GridBagSizer2->Add(m_volume1Ctrl, wxGBPosition(1, 1), wxDefaultSpan, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_volNormalizeBt = new wxRadioButton(this, ID_RADIOBUTTON4, _("Normalize"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON4"));
	GridBagSizer2->Add(m_volNormalizeBt, wxGBPosition(3, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	m_volChange2Bt = new wxRadioButton(this, ID_RADIOBUTTON3, _("Change (dB)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON3"));
	GridBagSizer2->Add(m_volChange2Bt, wxGBPosition(2, 0), wxDefaultSpan, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_volume2Ctrl = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxSize(42,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	BoxSizer1->Add(m_volume2Ctrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_volume2Spin = new wxSpinButton(this, ID_SPINBUTTON3, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL|wxSP_ARROW_KEYS, _T("ID_SPINBUTTON3"));
	m_volume2Spin->SetRange(-999999, 999999);
	m_volume2Spin->SetMinSize(wxSize(16,12));
	BoxSizer1->Add(m_volume2Spin, 0, wxLEFT|wxEXPAND, 2);
	GridBagSizer2->Add(BoxSizer1, wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND, 5);
	normBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	m_calcGainBt = new wxBitmapButton(this, ID_CALC_GAIN_BT, wxBITMAP_FROM_MEMORY(refresh), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_CALC_GAIN_BT"));
	normBoxSizer->Add(m_calcGainBt, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_gainText = new wxStaticText(this, ID_STATICTEXT_GAIN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_GAIN"));
	normBoxSizer->Add(m_gainText, 1, wxALIGN_CENTER_VERTICAL, 5);
	GridBagSizer2->Add(normBoxSizer, wxGBPosition(3, 1), wxDefaultSpan, wxEXPAND, 5);
	volAdjustBox->Add(GridBagSizer2, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	mainVSizer->Add(volAdjustBox, 1, wxALL|wxEXPAND, 5);
	stdDialogButtonSizer = new wxStdDialogButtonSizer();
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	stdDialogButtonSizer->Realize();
	mainVSizer->Add(stdDialogButtonSizer, 0, wxALL|wxEXPAND, 5);
	SetSizer(mainVSizer);
	mainVSizer->Fit(this);
	mainVSizer->SetSizeHints(this);
	Center();

	Connect(ID_DST_CHOICE,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&AudioPropDlg::OnChangeFormat);
	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&AudioPropDlg::OnChangeOffset);
	Connect(ID_SPINBUTTON2,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&AudioPropDlg::OnOffsetSpin);
	Connect(ID_RADIOBUTTON1,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&AudioPropDlg::OnSelectVolumeBt);
	Connect(ID_RADIOBUTTON2,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&AudioPropDlg::OnSelectVolumeBt);
	Connect(ID_RADIOBUTTON4,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&AudioPropDlg::OnSelectVolumeBt);
	Connect(ID_RADIOBUTTON3,wxEVT_COMMAND_RADIOBUTTON_SELECTED,(wxObjectEventFunction)&AudioPropDlg::OnSelectVolumeBt);
	Connect(ID_TEXTCTRL3,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&AudioPropDlg::OnChangeVolume);
	Connect(ID_SPINBUTTON3,wxEVT_SCROLL_LINEUP,(wxObjectEventFunction)&AudioPropDlg::OnVolume2SpinUp);
	Connect(ID_SPINBUTTON3,wxEVT_SCROLL_LINEDOWN,(wxObjectEventFunction)&AudioPropDlg::OnVolume2SpinDown);
	Connect(ID_CALC_GAIN_BT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&AudioPropDlg::OnCalculateGain);
	//*)
	Connect(wxID_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&AudioPropDlg::OnOk);
	stdDialogButtonSizer->GetAffirmativeButton()->SetDefault();

	m_fileNameText->SetLabel(m_audioFile.length() ? m_audioFile : m_vob->GetFilename());
	// duration
	double duration = m_vob->GetDuration();
	if (m_audioFile.length()) {
		wxFfmpegMediaDecoder ffmpeg;
		if (ffmpeg.Load(m_audioFile) && ffmpeg.GetDuration() > 0)
			duration = ffmpeg.GetDuration();
	}
	wxString s = wxT("N/A");
	if (duration > 0) {
		int secs = (int) duration;
		int ms = lround((duration - secs) * 1000);
		int mins = secs / 60;
		secs %= 60;
		int hours = mins / 60;
		mins %= 60;
		s = wxString::Format(wxT("%02d:%02d:%02d.%03d"), hours, mins, secs, ms);
	}
	m_durText->SetLabel(s);
	m_srcText->SetLabel(m_stream->GetSourceFormat());

	bool copyEnabled = m_stream->IsCopyPossible();
	int afIdx = m_stream->GetAudioFormat();
	if (!copyEnabled && afIdx > 0)
		afIdx -= 1;
	m_dstChoice->Append(DVD::GetAudioFormatLabels(copyEnabled, true));
	m_dstChoice->SetSelection(afIdx);

	// upmix to 5.1 oder donwmix to stereo
	if (m_stream->GetSourceChannelNumber() >= 6) {
		m_audio51->SetLabel(_("stereo"));
		m_audio51->SetValue(m_stream->GetChannelNumber() == 2);
	} else {
		m_audio51->SetValue(m_stream->GetChannelNumber() == 6);
	}

	m_langChoice->Append(DVD::GetLanguageNames());
	map<wxString, wxString>& langMap = DVD::GetLanguageMap();
	for (map<wxString, wxString>::iterator langIt = langMap.begin(); langIt != langMap.end(); langIt++) {
		if (langIt->second == m_langCode) {
			m_langChoice->SetStringSelection(langIt->first);
		}
	}

	m_volume2Ctrl->SetValue(wxT("0.0"));
	switch (m_stream->GetAudioAdjustType()) {
	case aatChangePercent:
		if (m_stream->GetAudioVolume() == 1.0) {
			m_volNoBt->SetValue(true);
		} else {
			m_volChange1Bt->SetValue(true);
			m_volume1Ctrl->SetValue(lround(m_stream->GetAudioVolume()*100));
		}
		break;
	case aatChangeDB:
		if (m_stream->GetAudioVolume() == 0.0) {
			m_volNoBt->SetValue(true);
		} else {
			m_volChange2Bt->SetValue(true);
			m_volume2Ctrl->SetValue(wxString::Format(wxT("%0.1f"), m_stream->GetAudioVolume()));
			m_volume2Spin->SetValue(lround(m_stream->GetAudioVolume()*100));
		}
		break;
	case aatNormalize:
		m_volNormalizeBt->SetValue(true);
		break;
	}
	if (m_stream->IsReplayGainCalculated()) {
		m_gainText->SetLabel(_("Track gain:") + wxString::Format(wxT(" %+0.1f dB"), m_stream->GetTrackGain()));
	}

	m_offsetCtrl->ChangeValue(Time2String(m_stream->GetTsOffset()));
	m_offsetCtrl->Enable(m_audioFile.length() > 0);
	m_offsetCtrl->SetMinSize(wxSize(m_dstChoice->GetBestSize().x - m_offsetSpin->GetMinWidth(), -1));
	m_offsetSpin->SetValue(m_stream->GetTsOffset()/1000);
	m_offsetSpin->Enable(m_audioFile.length() > 0);

	m_filtersCtrl->SetValue(m_stream->GetFilters());

	wxCommandEvent evt;
	OnChangeFormat(evt);
	
	bool replayGainCalculated = m_stream->IsReplayGainCalculated();
	m_stream->SetReplayGainCalculated(true);
	OnSelectVolumeBt(evt);
	m_stream->SetReplayGainCalculated(replayGainCalculated);

	m_dstChoice->SetFocus();
	mainVSizer->Fit(this);
	mainVSizer->SetSizeHints(this);
	Center();
}

AudioPropDlg::~AudioPropDlg() {
	//(*Destroy(AudioPropDlg)
	//*)
}

void AudioPropDlg::OnChangeFormat(wxCommandEvent& event) {
	if (!m_audio51->IsEnabled())
		m_audio51->SetValue(false);
	m_audio51->Enable(GetAudioFormat() >= afAC3);
	if (!m_audio51->IsEnabled()) {
		m_audio51->SetValue(GetAudioFormat() == afMP2 && m_stream->GetSourceChannelNumber() > 2);
	}
	bool allowChangeVolume = GetAudioFormat() != afNONE && GetAudioFormat() != afCOPY;
	m_volNoBt->Enable(allowChangeVolume);
	m_volChange1Bt->Enable(allowChangeVolume);
	m_volChange2Bt->Enable(allowChangeVolume);
	m_volNormalizeBt->Enable(allowChangeVolume);
	if (!allowChangeVolume) {
		m_volNoBt->SetValue(true);
		wxCommandEvent evt;
		OnSelectVolumeBt(evt);
	}
}

AudioFormat AudioPropDlg::GetAudioFormat() {
	bool copyEnabled = m_stream->IsCopyPossible();
	int afIdx = m_dstChoice->GetSelection();
	return (AudioFormat) (copyEnabled || afIdx == 0 ? afIdx : afIdx + 1);
}

wxString AudioPropDlg::GetLangCode() {
	return DVD::GetLanguageMap()[m_langChoice->GetStringSelection()];
}

void AudioPropDlg::OnChangeVolume(wxCommandEvent& event) {
	wxString value = m_volume2Ctrl->GetValue();
	for (unsigned int i = 0; i < value.length(); i++)
		if ((value[i] < wxT('0') || value[i] > wxT('9')) && value[i] != wxT('.') && value[i] != wxT('-'))
			value.Remove(i, 1);
	if (m_volume2Ctrl->GetValue() != value) {
		m_volume2Ctrl->SetValue(value);
	}
}

void AudioPropDlg::OnVolume2SpinUp(wxSpinEvent& event) {
	double volume = 0;
	if (m_volume2Ctrl->GetValue().ToDouble(&volume)) {
		volume++;
	}
	m_volume2Ctrl->SetValue(wxString::Format(wxT("%0.1f"), volume));
}

void AudioPropDlg::OnVolume2SpinDown(wxSpinEvent& event) {
	double volume = 0;
	if (m_volume2Ctrl->GetValue().ToDouble(&volume)) {
		volume--;
	}
	m_volume2Ctrl->SetValue(wxString::Format(wxT("%0.1f"), volume));
}

void AudioPropDlg::OnChangeOffset(wxCommandEvent& event) {
	if (s_timeRE.Matches(m_offsetCtrl->GetValue()))
		m_offsetSpin->SetValue(lround(String2Time(m_offsetCtrl->GetValue())/1000));
}

void AudioPropDlg::OnOffsetSpin(wxSpinEvent& event) {
	m_offsetCtrl->ChangeValue(Time2String(((long)m_offsetSpin->GetValue()) * 1000));
}

void AudioPropDlg::OnSelectVolumeBt(wxCommandEvent& event){
	if (m_volNormalizeBt->GetValue() && !m_stream->IsReplayGainCalculated()) {
		OnCalculateGain(event);
	}
	m_volume1Ctrl->Enable(m_volChange1Bt->GetValue());
	m_volume2Ctrl->Enable(m_volChange2Bt->GetValue());
	m_volume2Spin->Enable(m_volChange2Bt->GetValue());
	m_calcGainBt->Enable(m_volNormalizeBt->GetValue());
}

void AudioPropDlg::OnOk(wxCommandEvent& event) {
	if (!s_timeRE.Matches(m_offsetCtrl->GetValue())) {
		wxLogError(_("Invalid time value '%s'"), m_offsetCtrl->GetValue().c_str());
		return;
	}
	double volume = 0;
	if (m_volChange2Bt->GetValue()) {
		if (!m_volume2Ctrl->GetValue().ToDouble(&volume)) {
			wxLogError(_("Invalid volume value '%s'"), m_volume2Ctrl->GetValue().c_str());
			return;
		}
	}
	// upmix to 5.1 oder donwmix to stereo
	m_stream->SetChannelNumber(m_audio51->IsEnabled() && m_audio51->GetValue() ?
			(m_stream->GetSourceChannelNumber() >= 6 ? 2 : 6) : -1);
	if (m_volNoBt->GetValue()) {
		m_stream->SetAudioVolume(1.0);
		m_stream->SetAudioAdjustType(aatChangePercent);
	} else if (m_volChange1Bt->GetValue()) {
		m_stream->SetAudioVolume((double)m_volume1Ctrl->GetValue()/100);
		m_stream->SetAudioAdjustType(aatChangePercent);
	} else if (m_volChange2Bt->GetValue()) {
		m_stream->SetAudioVolume(volume);
		m_stream->SetAudioAdjustType(aatChangeDB);
	} else {
		m_stream->SetAudioAdjustType(aatNormalize);
	}
	m_stream->SetTsOffset(String2Time(m_offsetCtrl->GetValue()));
	m_stream->SetFilters(m_filtersCtrl->GetValue().Strip(wxString::both));
	EndModal(wxID_OK);
}

void AudioPropDlg::OnCalculateGain(wxCommandEvent& event) {
	int audioStreamIdx = 0;
	if (m_audioFile.length() == 0) {
		for (int i = 0; i < m_streamIdx; i++) {
			if (m_vob->GetStreams()[i]->GetType() == stAUDIO)
				audioStreamIdx++;
		}
	}
	wxFfmpegMediaTranscoder transcoder;
	transcoder.AddInputFile(m_audioFile.length() ? m_audioFile : m_vob->GetFilename());
	transcoder.ReplayGain(audioStreamIdx);
	wxString msg = _("Analysis of audio") + wxString(wxT(". ")) + _("Please wait...");
	wxProgressDialog pDlg(wxT("DVDStyler"), msg, 99, this, wxPD_APP_MODAL | wxPD_CAN_ABORT | wxPD_SMOOTH);
	pDlg.Show();
	pDlg.SetFocus();
	pDlg.Pulse();
	
	AVConvTimeProgExecute prog(&pDlg);
	if (!prog.Execute(transcoder.GetCmd(true)) || !prog.IsOk()) {
		pDlg.Hide();
#if wxCHECK_VERSION(2,9,0)
		if (!pDlg.WasCancelled())
			wxLogError(wxT("Failed calculation of replay gain"));
#endif
		return;
	}
	
	double trackGain = prog.GetTrackGain();
	if (trackGain > 0) {
		wxFfmpegMediaTranscoder transcoder2;
		transcoder2.AddInputFile(m_audioFile.length() ? m_audioFile : m_vob->GetFilename());
		transcoder2.VolumeDetect(audioStreamIdx);
		AVConvTimeProgExecute prog2(&pDlg);
		if (!prog2.Execute(transcoder2.GetCmd(true)) || !prog2.IsOk()) {
			pDlg.Hide();
#if wxCHECK_VERSION(2,9,0)
			if (!pDlg.WasCancelled())
				wxLogError(wxT("Failed calculation of replay gain"));
#endif
			return;
		}
		if (trackGain > -prog2.GetMaxVolume())
			trackGain = -prog2.GetMaxVolume();
	}
	
	pDlg.Hide();
	m_stream->SetTrackGain(trackGain);
	m_stream->SetReplayGainCalculated(true);
	m_gainText->SetLabel(_("Track gain:") + wxString::Format(wxT(" %+0.1f dB"), m_stream->GetTrackGain()));
	Fit();
}
