/////////////////////////////////////////////////////////////////////////////
// Name:        VideoPropDlg.cpp
// Purpose:     The video properties dialog
// Author:      Alex Thuering
// Created:     09.09.2010
// RCS-ID:      $Id: VideoPropDlg.cpp,v 1.27 2016/12/17 17:27:38 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "VideoPropDlg.h"
#include "DVD.h"
#include "Utils.h"
#include "Config.h"
#include <wx/artprov.h>
#include <wx/regex.h>

//(*InternalHeaders(VideoPropDlg)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(VideoPropDlg)
const long VideoPropDlg::ID_STATICTEXT2 = wxNewId();
const long VideoPropDlg::ID_STATICTEXT4 = wxNewId();
const long VideoPropDlg::ID_STATICTEXT3 = wxNewId();
const long VideoPropDlg::DST_CHOICE_ID = wxNewId();
const long VideoPropDlg::ASPECT_CHOICE_ID = wxNewId();
const long VideoPropDlg::INTERLACED_CHOICE_ID = wxNewId();
const long VideoPropDlg::FF_CHOICE_ID = wxNewId();
const long VideoPropDlg::KEEP_ASPECT_CB_ID = wxNewId();
const long VideoPropDlg::KEEP_ASPECT_CHOICE_ID = wxNewId();
const long VideoPropDlg::ID_SPINCTRL1 = wxNewId();
const long VideoPropDlg::ID_SPINCTRL4 = wxNewId();
const long VideoPropDlg::ID_SPINCTRL3 = wxNewId();
const long VideoPropDlg::ID_SPINCTRL2 = wxNewId();
const long VideoPropDlg::ID_SPINCTRL5 = wxNewId();
const long VideoPropDlg::ID_SPINCTRL6 = wxNewId();
const long VideoPropDlg::ID_SPINCTRL7 = wxNewId();
const long VideoPropDlg::ID_SPINCTRL8 = wxNewId();
const long VideoPropDlg::ID_SPINCTRL9 = wxNewId();
const long VideoPropDlg::ID_SPINCTRL10 = wxNewId();
const long VideoPropDlg::ID_TEXTCTRL1 = wxNewId();
const long VideoPropDlg::ID_CUSTOM1 = wxNewId();
const long VideoPropDlg::ID_SLIDER = wxNewId();
const long VideoPropDlg::ID_TIME_CTRL = wxNewId();
const long VideoPropDlg::ID_TIME_SPINB = wxNewId();
const long VideoPropDlg::ID_FRAME_SPINBT = wxNewId();
const long VideoPropDlg::ID_TEXTCTRL2 = wxNewId();
const long VideoPropDlg::ID_SPINBUTTON1 = wxNewId();
const long VideoPropDlg::ID_SPINBUTTON2 = wxNewId();
const long VideoPropDlg::START_BT_ID = wxNewId();
const long VideoPropDlg::ID_TEXTCTRL3 = wxNewId();
const long VideoPropDlg::ID_SPINBUTTON3 = wxNewId();
const long VideoPropDlg::ID_SPINBUTTON4 = wxNewId();
const long VideoPropDlg::ID_RESET_TIME_BT = wxNewId();
//*)

BEGIN_EVENT_TABLE(VideoPropDlg,wxDialog)
	//(*EventTable(VideoPropDlg)
	//*)
	EVT_BUTTON(wxID_OK, VideoPropDlg::OnOkBt)
	EVT_COMMAND_SCROLL(ID_SLIDER, VideoPropDlg::OnSliderScroll)
END_EVENT_TABLE()

VideoPropDlg::VideoPropDlg(wxWindow* parent, DVD* dvd, Vob* vob, Stream* stream, AspectRatio aspectRatio) {
	m_dvd = dvd;
	m_vob = vob;
	m_stream = stream;
	m_aspectRatio = aspectRatio;
	m_videoPos = 0;

	//(*Initialize(VideoPropDlg)
	wxGridBagSizer* gridBagSizer;
	wxStaticText* srcLabel;
	wxGridBagSizer* cropBagSizer;
	wxBoxSizer* aspectSizer;
	wxBoxSizer* fadeSizer;
	wxStaticText* StaticText6;
	wxStaticText* filterLabel;
	wxStaticText* StaticText8;
	wxBoxSizer* padCropSizer;
	wxStaticText* durLabel;
	wxBoxSizer* mediaSizer;
	wxStdDialogButtonSizer* stdDialogButtonSizer;
	wxStaticText* staticText1;
	wxStaticText* dstLabel;
	wxGridBagSizer* borderSizer;
	wxStaticText* startLabel;
	wxBoxSizer* dstSizer;
	wxStaticText* fadeLabel;
	wxStaticText* endLabel;
	wxBoxSizer* timeSizer;
	wxStaticText* fileNameLabel;
	wxBoxSizer* mainSizer;
	wxBoxSizer* hSizer;

	Create(parent, wxID_ANY, _("Video properties"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
	SetClientSize(wxSize(1024,500));
	mainSizer = new wxBoxSizer(wxVERTICAL);
	hSizer = new wxBoxSizer(wxHORIZONTAL);
	gridBagSizer = new wxGridBagSizer(2, 2);
	fileNameLabel = new wxStaticText(this, wxID_ANY, _("File Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	gridBagSizer->Add(fileNameLabel, wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_fileNameText = new wxStaticText(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	m_fileNameText->SetMinSize(wxSize(8,8));
	gridBagSizer->Add(m_fileNameText, wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND, 5);
	durLabel = new wxStaticText(this, wxID_ANY, _("Duration:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	gridBagSizer->Add(durLabel, wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_durText = new wxStaticText(this, ID_STATICTEXT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	gridBagSizer->Add(m_durText, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND, 5);
	srcLabel = new wxStaticText(this, wxID_ANY, _("Source Format:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	gridBagSizer->Add(srcLabel, wxGBPosition(2, 0), wxDefaultSpan, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_srcText = new wxStaticText(this, ID_STATICTEXT3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	gridBagSizer->Add(m_srcText, wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND, 5);
	dstLabel = new wxStaticText(this, wxID_ANY, _("Destination Format:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	gridBagSizer->Add(dstLabel, wxGBPosition(3, 0), wxDefaultSpan, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	dstSizer = new wxBoxSizer(wxHORIZONTAL);
	m_dstChoice = new wxChoice(this, DST_CHOICE_ID, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("DST_CHOICE_ID"));
	dstSizer->Add(m_dstChoice, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_aspectChoice = new wxChoice(this, ASPECT_CHOICE_ID, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ASPECT_CHOICE_ID"));
	dstSizer->Add(m_aspectChoice, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_interlacedChoice = new wxChoice(this, INTERLACED_CHOICE_ID, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("INTERLACED_CHOICE_ID"));
	m_interlacedChoice->SetSelection( m_interlacedChoice->Append(_("progressive")) );
	m_interlacedChoice->Append(_("interlace"));
	dstSizer->Add(m_interlacedChoice, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_ffChoice = new wxChoice(this, FF_CHOICE_ID, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("FF_CHOICE_ID"));
	m_ffChoice->SetSelection( m_ffChoice->Append(_("auto")) );
	m_ffChoice->Append(_("BFF"));
	m_ffChoice->Append(_("TFF"));
	dstSizer->Add(m_ffChoice, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	gridBagSizer->Add(dstSizer, wxGBPosition(3, 1), wxDefaultSpan, wxEXPAND, 5);
	aspectSizer = new wxBoxSizer(wxHORIZONTAL);
	m_keepAspectCtrl = new wxCheckBox(this, KEEP_ASPECT_CB_ID, _("Keep Aspect Ratio"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("KEEP_ASPECT_CB_ID"));
	m_keepAspectCtrl->SetValue(false);
	aspectSizer->Add(m_keepAspectCtrl, 0, wxALIGN_CENTER_VERTICAL, 5);
	m_keepAspectChoice = new wxChoice(this, KEEP_ASPECT_CHOICE_ID, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("KEEP_ASPECT_CHOICE_ID"));
	m_keepAspectChoice->SetSelection( m_keepAspectChoice->Append(_("border")) );
	m_keepAspectChoice->Append(_("crop"));
	aspectSizer->Add(m_keepAspectChoice, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	gridBagSizer->Add(aspectSizer, wxGBPosition(4, 0), wxGBSpan(1, 2), wxEXPAND, 5);
	padCropSizer = new wxBoxSizer(wxHORIZONTAL);
	m_padBox = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Border"));
	borderSizer = new wxGridBagSizer(2, 2);
	m_padTop = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxSize(58,-1), 0, 0, 999, 0, _T("ID_SPINCTRL1"));
	m_padTop->SetValue(_T("0"));
	borderSizer->Add(m_padTop, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_padBottom = new wxSpinCtrl(this, ID_SPINCTRL4, _T("0"), wxDefaultPosition, wxSize(58,-1), 0, 0, 999, 0, _T("ID_SPINCTRL4"));
	m_padBottom->SetValue(_T("0"));
	borderSizer->Add(m_padBottom, wxGBPosition(2, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_padRight = new wxSpinCtrl(this, ID_SPINCTRL3, _T("0"), wxDefaultPosition, wxSize(58,-1), 0, 0, 999, 0, _T("ID_SPINCTRL3"));
	m_padRight->SetValue(_T("0"));
	borderSizer->Add(m_padRight, wxGBPosition(1, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_padLeft = new wxSpinCtrl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxSize(58,-1), 0, 0, 999, 0, _T("ID_SPINCTRL2"));
	m_padLeft->SetValue(_T("0"));
	borderSizer->Add(m_padLeft, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_padBox->Add(borderSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	padCropSizer->Add(m_padBox, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_cropBox = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Crop"));
	cropBagSizer = new wxGridBagSizer(2, 2);
	m_cropTop = new wxSpinCtrl(this, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxSize(58,-1), 0, 0, 999, 0, _T("ID_SPINCTRL5"));
	m_cropTop->SetValue(_T("0"));
	cropBagSizer->Add(m_cropTop, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_cropBottom = new wxSpinCtrl(this, ID_SPINCTRL6, _T("0"), wxDefaultPosition, wxSize(58,-1), 0, 0, 999, 0, _T("ID_SPINCTRL6"));
	m_cropBottom->SetValue(_T("0"));
	cropBagSizer->Add(m_cropBottom, wxGBPosition(2, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_cropRight = new wxSpinCtrl(this, ID_SPINCTRL7, _T("0"), wxDefaultPosition, wxSize(58,-1), 0, 0, 999, 0, _T("ID_SPINCTRL7"));
	m_cropRight->SetValue(_T("0"));
	cropBagSizer->Add(m_cropRight, wxGBPosition(1, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_cropLeft = new wxSpinCtrl(this, ID_SPINCTRL8, _T("0"), wxDefaultPosition, wxSize(58,-1), 0, 0, 999, 0, _T("ID_SPINCTRL8"));
	m_cropLeft->SetValue(_T("0"));
	cropBagSizer->Add(m_cropLeft, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_cropBox->Add(cropBagSizer, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	padCropSizer->Add(m_cropBox, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	gridBagSizer->Add(padCropSizer, wxGBPosition(5, 0), wxGBSpan(1, 2), wxEXPAND, 5);
	fadeLabel = new wxStaticText(this, wxID_ANY, _("Fade-In/Out:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	gridBagSizer->Add(fadeLabel, wxGBPosition(6, 0), wxDefaultSpan, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	filterLabel = new wxStaticText(this, wxID_ANY, _("Filters:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	gridBagSizer->Add(filterLabel, wxGBPosition(7, 0), wxDefaultSpan, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	fadeSizer = new wxBoxSizer(wxHORIZONTAL);
	m_fadeInCtrl = new wxSpinCtrl(this, ID_SPINCTRL9, _T("0"), wxDefaultPosition, wxSize(58,-1), 0, 0, 999, 0, _T("ID_SPINCTRL9"));
	m_fadeInCtrl->SetValue(_T("0"));
	fadeSizer->Add(m_fadeInCtrl, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText6 = new wxStaticText(this, wxID_ANY, _("sec"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	fadeSizer->Add(StaticText6, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_fadeOutCtrl = new wxSpinCtrl(this, ID_SPINCTRL10, _T("0"), wxDefaultPosition, wxSize(58,-1), 0, 0, 999, 0, _T("ID_SPINCTRL10"));
	m_fadeOutCtrl->SetValue(_T("0"));
	fadeSizer->Add(m_fadeOutCtrl, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(this, wxID_ANY, _("sec"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	fadeSizer->Add(StaticText8, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	gridBagSizer->Add(fadeSizer, wxGBPosition(6, 1), wxDefaultSpan, wxEXPAND, 5);
	m_filtersCtrl = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	gridBagSizer->Add(m_filtersCtrl, wxGBPosition(7, 1), wxDefaultSpan, wxEXPAND, 5);
	hSizer->Add(gridBagSizer, 0, wxALL|wxEXPAND, 5);
	mediaSizer = new wxBoxSizer(wxVERTICAL);
	m_mediaCtrl = new MediaCtrlFF(this, ID_CUSTOM1, wxT(""), wxDefaultPosition,wxDefaultSize, 0, wxDefaultValidator, _T("ID_CUSTOM1"));
	m_mediaCtrl->SetMinSize(wxSize(300, 200));
	m_mediaCtrl->SetWindowStyle(wxBORDER_NONE);
	mediaSizer->Add(m_mediaCtrl, 1, wxALL|wxEXPAND, 4);
	m_slider = new wxSlider(this, ID_SLIDER, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER"));
	mediaSizer->Add(m_slider, 0, wxEXPAND, 2);
	timeSizer = new wxBoxSizer(wxHORIZONTAL);
	staticText1 = new wxStaticText(this, wxID_ANY, _("Time:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	timeSizer->Add(staticText1, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	m_timeCtrl = new wxTextCtrl(this, ID_TIME_CTRL, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TIME_CTRL"));
	timeSizer->Add(m_timeCtrl, 0, wxLEFT|wxEXPAND, 2);
	m_timeSpinBt = new wxSpinButton(this, ID_TIME_SPINB, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL|wxSP_ARROW_KEYS, _T("ID_TIME_SPINB"));
	m_timeSpinBt->SetRange(0, 100);
	m_timeSpinBt->SetMinSize(wxSize(16,12));
	timeSizer->Add(m_timeSpinBt, 0, wxEXPAND, 5);
	m_frameSpinBt = new wxSpinButton(this, ID_FRAME_SPINBT, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL|wxSP_ARROW_KEYS|wxSP_WRAP, _T("ID_FRAME_SPINBT"));
	m_frameSpinBt->SetRange(-9999, 9999);
	m_frameSpinBt->SetMinSize(wxSize(16,12));
	timeSizer->Add(m_frameSpinBt, 0, wxEXPAND, 5);
	startLabel = new wxStaticText(this, wxID_ANY, _("Start:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	timeSizer->Add(startLabel, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 8);
	m_startCtrl = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	timeSizer->Add(m_startCtrl, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_startSpin = new wxSpinButton(this, ID_SPINBUTTON1, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL|wxSP_ARROW_KEYS|wxSP_WRAP, _T("ID_SPINBUTTON1"));
	m_startSpin->SetRange(-9999, 9999);
	m_startSpin->SetMinSize(wxSize(16,12));
	timeSizer->Add(m_startSpin, 0, wxEXPAND, 5);
	m_startFrameSpin = new wxSpinButton(this, ID_SPINBUTTON2, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL|wxSP_ARROW_KEYS|wxSP_WRAP, _T("ID_SPINBUTTON2"));
	m_startFrameSpin->SetRange(-9999, 9999);
	m_startFrameSpin->SetMinSize(wxSize(16,12));
	timeSizer->Add(m_startFrameSpin, 0, wxEXPAND, 5);
	m_startBt = new wxBitmapButton(this, START_BT_ID, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_CUT")),wxART_MENU), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("START_BT_ID"));
	m_startBt->SetDefault();
	m_startBt->SetToolTip(_("Cut beginning"));
	timeSizer->Add(m_startBt, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	endLabel = new wxStaticText(this, wxID_ANY, _("End:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	timeSizer->Add(endLabel, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_endCtrl = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	timeSizer->Add(m_endCtrl, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_endSpin = new wxSpinButton(this, ID_SPINBUTTON3, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL|wxSP_ARROW_KEYS|wxSP_WRAP, _T("ID_SPINBUTTON3"));
	m_endSpin->SetRange(-9999, 9999);
	m_endSpin->SetMinSize(wxSize(16,12));
	timeSizer->Add(m_endSpin, 0, wxEXPAND, 0);
	m_endFrameSpin = new wxSpinButton(this, ID_SPINBUTTON4, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL|wxSP_ARROW_KEYS|wxSP_WRAP, _T("ID_SPINBUTTON4"));
	m_endFrameSpin->SetRange(-9999, 9999);
	m_endFrameSpin->SetMinSize(wxSize(16,12));
	timeSizer->Add(m_endFrameSpin, 0, wxEXPAND, 0);
	m_endBt = new wxBitmapButton(this, ID_RESET_TIME_BT, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_CUT")),wxART_MENU), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_RESET_TIME_BT"));
	m_endBt->SetDefault();
	m_endBt->SetToolTip(_("Cut ending"));
	timeSizer->Add(m_endBt, 0, wxEXPAND, 2);
	mediaSizer->Add(timeSizer, 0, wxALL|wxEXPAND, 5);
	hSizer->Add(mediaSizer, 1, wxEXPAND, 5);
	mainSizer->Add(hSizer, 1, wxALL|wxEXPAND, 5);
	stdDialogButtonSizer = new wxStdDialogButtonSizer();
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	stdDialogButtonSizer->Realize();
	mainSizer->Add(stdDialogButtonSizer, 0, wxALL|wxEXPAND, 4);
	SetSizer(mainSizer);
	SetSizer(mainSizer);
	Layout();
	Center();

	Connect(DST_CHOICE_ID,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&VideoPropDlg::OnChangeFormat);
	Connect(ASPECT_CHOICE_ID,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&VideoPropDlg::OnChangeAspect);
	Connect(INTERLACED_CHOICE_ID,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&VideoPropDlg::OnInterlaced);
	Connect(KEEP_ASPECT_CB_ID,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&VideoPropDlg::OnChangeKeepAspect);
	Connect(KEEP_ASPECT_CHOICE_ID,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&VideoPropDlg::OnChangeKeepAspect);
	Connect(ID_SPINCTRL1,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&VideoPropDlg::OnChangeBorder);
	Connect(ID_SPINCTRL4,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&VideoPropDlg::OnChangeBorder);
	Connect(ID_SPINCTRL3,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&VideoPropDlg::OnChangeBorder);
	Connect(ID_SPINCTRL2,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&VideoPropDlg::OnChangeBorder);
	Connect(ID_SPINCTRL5,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&VideoPropDlg::OnChangeCrop);
	Connect(ID_SPINCTRL6,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&VideoPropDlg::OnChangeCrop);
	Connect(ID_SPINCTRL7,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&VideoPropDlg::OnChangeCrop);
	Connect(ID_SPINCTRL8,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&VideoPropDlg::OnChangeCrop);
	Connect(ID_TIME_CTRL,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&VideoPropDlg::OnChangeTime);
	Connect(ID_TIME_SPINB,wxEVT_SCROLL_THUMBTRACK,(wxObjectEventFunction)&VideoPropDlg::OnTimeSpin);
	Connect(ID_FRAME_SPINBT,wxEVT_SCROLL_LINEUP,(wxObjectEventFunction)&VideoPropDlg::OnFrameSpin);
	Connect(ID_FRAME_SPINBT,wxEVT_SCROLL_LINEDOWN,(wxObjectEventFunction)&VideoPropDlg::OnFrameSpinDown);
	Connect(ID_SPINBUTTON1,wxEVT_SCROLL_LINEUP,(wxObjectEventFunction)&VideoPropDlg::OnStartSpin);
	Connect(ID_SPINBUTTON1,wxEVT_SCROLL_LINEDOWN,(wxObjectEventFunction)&VideoPropDlg::OnStartSpinDown);
	Connect(ID_SPINBUTTON2,wxEVT_SCROLL_LINEUP,(wxObjectEventFunction)&VideoPropDlg::OnStartFrameSpin);
	Connect(ID_SPINBUTTON2,wxEVT_SCROLL_LINEDOWN,(wxObjectEventFunction)&VideoPropDlg::OnStartFrameSpinDown);
	Connect(START_BT_ID,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoPropDlg::OnStartBt);
	Connect(ID_SPINBUTTON3,wxEVT_SCROLL_LINEUP,(wxObjectEventFunction)&VideoPropDlg::OnEndSpin);
	Connect(ID_SPINBUTTON3,wxEVT_SCROLL_LINEDOWN,(wxObjectEventFunction)&VideoPropDlg::OnEndSpinDown);
	Connect(ID_SPINBUTTON4,wxEVT_SCROLL_LINEUP,(wxObjectEventFunction)&VideoPropDlg::OnEndFrameSpin);
	Connect(ID_SPINBUTTON4,wxEVT_SCROLL_LINEDOWN,(wxObjectEventFunction)&VideoPropDlg::OnEndFrameSpinDown);
	Connect(ID_RESET_TIME_BT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&VideoPropDlg::OnEndBt);
	//*)

    int x = 0;
	m_timeCtrl->GetTextExtent(wxT("00:00:00.000"), &x, NULL);
	m_timeCtrl->SetMinSize(wxSize(x + 10, -1));
	m_startCtrl->SetMinSize(wxSize(x + 10, -1));
	m_endCtrl->SetMinSize(wxSize(x + 10, -1));

    m_fileNameText->SetLabel(m_vob->GetFilenameDisplay());
    m_fileNameText->SetToolTip(m_vob->GetFilenameDisplay());
    // duration
	wxString s = wxT("N/A");
	if (m_vob->GetDuration()>0) {
		int secs = (int) m_vob->GetDuration();
		int mins = secs / 60;
		secs %= 60;
		int hours = mins / 60;
		mins %= 60;
		s = wxString::Format(wxT("%02d:%02d:%02d"), hours, mins, secs);
	}
    m_durText->SetLabel(s);
    m_srcText->SetLabel(m_stream->GetSourceFormat());

    // format
    bool copyEnabled = m_stream->IsCopyPossible();
    m_dstChoice->Append(DVD::GetVideoFormatLabels(copyEnabled, false, false, m_dvd->IsHD()));
	int vf = m_stream->GetVideoFormat();
	if (m_dvd->IsHD()) {
		if (vf >= vfPAL_HALF_HD) {
			vf = vf - vfPAL_HALF_HD + (copyEnabled ? 1 : 0);
		} else
			vf =  vf >= vfPAL && vf < vfPAL_HALF_HD ? vf + (copyEnabled ? 5 : 4) : 0;
	} else {
		vf -= (copyEnabled ? 1 : 2);
	}
    m_dstChoice->SetSelection(vf);
    m_aspectChoice->Append(DVD::GetAspectRatioLabels());
    m_aspectChoice->SetSelection(m_aspectRatio - 1);
    m_interlacedChoice->SetSelection(m_vob->GetInterlaced() ? 1 : 0);
    m_ffChoice->SetSelection(((int)m_vob->GetFirstField()) + 1);
    m_keepAspectCtrl->SetValue(m_vob->GetKeepAspectRatio());
    m_keepAspectChoice->SetSelection(m_vob->GetKeepAspectCrop() ? 1 : 0);
    m_lastFormat = GetVideoFormat();

    // border
    m_padLeft->SetValue(m_vob->GetPad()[0]);
    m_padRight->SetValue(m_vob->GetPad()[1]);
    m_padTop->SetValue(m_vob->GetPad()[2]);
    m_padBottom->SetValue(m_vob->GetPad()[3]);

    // crop
    m_cropLeft->SetValue(m_vob->GetCrop()[0]);
    m_cropRight->SetValue(m_vob->GetCrop()[1]);
    m_cropTop->SetValue(m_vob->GetCrop()[2]);
    m_cropBottom->SetValue(m_vob->GetCrop()[3]);

    // fade-in / fade-out
    m_fadeInCtrl->SetValue(m_vob->GetFadeIn());
    m_fadeOutCtrl->SetValue(m_vob->GetFadeOut());
    m_filtersCtrl->SetValue(m_vob->GetVideoFilters());

    // start time
    int ms = lround(m_vob->GetStartTime() * 1000);
    ms = ms % 1000;
	long t = (long) m_vob->GetStartTime();
	m_startCtrl->SetValue(wxString::Format(wxT("%d:%02d:%02d.%03d"), t/3600, (t/60)%60, t%60, ms));

	// end time
	if (m_vob->GetRecordingTime() > 0) {
		ms = lround((m_vob->GetStartTime() + m_vob->GetRecordingTime()) * 1000);
		ms = ms % 1000;
		t = (long) (m_vob->GetStartTime() + m_vob->GetRecordingTime());
		m_endCtrl->SetValue(wxString::Format(wxT("%d:%02d:%02d.%03d"), t/3600, (t/60) % 60, t%60, ms));
	}

    wxCommandEvent evt;
    OnChangeFormat(evt);
    m_dstChoice->SetFocus();
}

VideoPropDlg::~VideoPropDlg() {
	//(*Destroy(VideoPropDlg)
	//*)
}

double TimeToDouble(wxString timeStr) {
	wxRegEx re(wxT("^(([[:digit:]]+:)?([[:digit:]]?[[:digit:]]):)?([[:digit:]]?[[:digit:]])(\\.[[:digit:]]+)?$"));
	if (!re.Matches(timeStr))
		return -1;
	double result = 0;
	long lval;
	wxString val = re.GetMatch(timeStr, 2).BeforeFirst(wxT(':'));
	if (val.ToLong(&lval))
		result += lval*3600;
	val = re.GetMatch(timeStr, 3).BeforeFirst(wxT(':'));
	if (val.ToLong(&lval))
		result += lval*60;
	val = re.GetMatch(timeStr, 4);
	if (val.ToLong(&lval))
		result += lval;
	val = re.GetMatch(timeStr, 5);
	double dval;
	if (val.ToDouble(&dval))
		result += dval;
	return result;
}

VideoFormat VideoPropDlg::GetVideoFormat() {
	int vf = m_dstChoice->GetSelection();
	if (m_dvd->IsHD()) {
		if (m_stream->IsCopyPossible()) {
			if (vf == 0) {
				vf = vfCOPY;
			} else if (vf <= 6) {
				vf += vfPAL_HALF_HD - 1;
			} else
				vf -= 5;
		} else {
			if (vf <= 5) {
				vf += vfPAL_HALF_HD;
			} else
				vf -= 4;
		}
	} else {
		vf += (m_stream->IsCopyPossible() ? 1 : 2);
	}
	return (VideoFormat) vf;
}

AspectRatio VideoPropDlg::GetAspectRatio() {
	return (AspectRatio) (m_aspectChoice->GetSelection() + 1);
}

vector<int> VideoPropDlg::GetPad() {
	int a[4] = { m_padLeft->GetValue(), m_padRight->GetValue(), m_padTop->GetValue(), m_padBottom->GetValue() };
	return vector<int>(a, a + sizeof(a)/sizeof(*a));
}

vector<int> VideoPropDlg::GetCrop() {
	int a[4] = { m_cropLeft->GetValue(), m_cropRight->GetValue(), m_cropTop->GetValue(), m_cropBottom->GetValue() };
	return vector<int>(a, a + sizeof(a)/sizeof(*a));
}

/**
 * Displays dialog
 */
int VideoPropDlg::ShowModal() {
	if (!m_mediaCtrl->Load(m_vob->GetFilename()))
		return wxID_CANCEL;
	m_slider->SetMax(m_mediaCtrl->Length()/1000);
	m_timeSpinBt->SetMax(m_mediaCtrl->Length()/1000);
	wxScrollEvent scrlEvt;
	OnSliderScroll(scrlEvt);
	int res = wxDialog::ShowModal();
	if (res == wxID_OK) {
		m_stream->SetDestinationFormat(GetVideoFormat());
		m_vob->GetPad() = GetPad();
		m_vob->GetCrop() = GetCrop();
		m_vob->SetInterlaced(m_interlacedChoice->GetSelection() == 1);
		m_vob->SetFirstField((FirstField) (m_ffChoice->GetSelection() - 1));
		m_vob->SetKeepAspectRatio(m_keepAspectCtrl->GetValue());
		m_vob->SetKeepAspectCrop(m_keepAspectChoice->GetSelection() == 1);
		m_vob->SetFadeIn(m_fadeInCtrl->GetValue());
		m_vob->SetFadeOut(m_fadeOutCtrl->GetValue());
		m_vob->SetVideoFilters(m_filtersCtrl->GetValue());
		
		double durationOld = m_vob->GetResultDuration();
		double time = TimeToDouble(m_startCtrl->GetValue());
		m_vob->SetStartTime(time >= 0 ? time : 0);
		time = TimeToDouble(m_endCtrl->GetValue());
		m_vob->SetRecordingTime(time > m_vob->GetStartTime() ? time - m_vob->GetStartTime() : -1);
		if (durationOld != m_vob->GetResultDuration()) {
			// remove chapter at the end
			long duration = (long) durationOld;
			if (s_config.GetAddChapterAtTitleEnd() && m_vob->GetCells().size() > 0 &&
					m_vob->GetCells()[m_vob->GetCells().size() - 1]->GetStart() == (duration - 1)*1000) {
				m_vob->GetCells().erase(m_vob->GetCells().end() - 1);
			}
			// remove chapters > duration
			duration = (long) m_vob->GetResultDuration()*1000;
			unsigned int i = 0;
			while (i < m_vob->GetCells().size()) {
				if (m_vob->GetCells()[i]->GetStart() > duration) {
					m_vob->GetCells().erase(m_vob->GetCells().begin() + i);
				} else
					i++;
			}
			// add chapter at the end
			duration = (long) m_vob->GetResultDuration();
			if (s_config.GetAddChapterAtTitleEnd() && (m_vob->GetCells().size() == 0 ||
					m_vob->GetCells()[m_vob->GetCells().size() - 1]->GetStart() < (duration - 1)*1000)) {
				m_vob->GetCells().push_back(new Cell((duration - 1)* 1000));
			}
		}
	}
	return res;
}

void VideoPropDlg::UpdatePadCrop() {
	if (m_keepAspectCtrl->GetValue()) {
		if (m_keepAspectChoice->GetSelection() == 0) {
			// set border
			int padx = 0;
			int pady = 0;
			if (m_vob->CalcPad(padx, pady, GetVideoFormat(), GetAspectRatio(), GetCrop())) {
				m_padLeft->SetValue(padx);
				m_padRight->SetValue(padx);
				m_padTop->SetValue(pady);
				m_padBottom->SetValue(pady);
			}
		} else  {
			// set crop
			int cropx = 0;
			int cropy = 0;
			if (m_vob->CalcCrop(cropx, cropy, GetVideoFormat(), GetAspectRatio(), GetPad())) {
				m_cropLeft->SetValue(cropx);
				m_cropRight->SetValue(cropx);
				m_cropTop->SetValue(cropy);
				m_cropBottom->SetValue(cropy);
			}
		}
	}
	// update preview
	m_mediaCtrl->SetVideoFormat(GetVideoFormat(), GetAspectRatio(), GetPad(), GetCrop());
}

void VideoPropDlg::SeekVideo(long pos, bool updateTimeCtrl) {
	m_videoPos = pos;
	m_slider->SetValue(lround(pos / 1000));
	m_timeSpinBt->SetValue(lround(pos / 1000));
	m_mediaCtrl->Seek((wxFileOffset) pos);
	if (updateTimeCtrl)
		m_timeCtrl->ChangeValue(Time2String(pos, true));
}

void VideoPropDlg::OnSliderScroll(wxScrollEvent& event) {
	SeekVideo(((long)m_slider->GetValue())*1000);
}

void VideoPropDlg::OnTimeSpin(wxSpinEvent& event) {
	SeekVideo(((long)m_timeSpinBt->GetValue())*1000);
}

void VideoPropDlg::OnChangeTime(wxCommandEvent& event) {
	if (s_timeRE.Matches(m_timeCtrl->GetValue()))
		SeekVideo(String2Time(m_timeCtrl->GetValue(), m_mediaCtrl->GetFps()), false);
}

void VideoPropDlg::OnFrameSpin(wxSpinEvent& event) {
    SeekVideo(String2Time(m_timeCtrl->GetValue(), m_mediaCtrl->GetFps()) + 1000 / m_mediaCtrl->GetFps());
}

void VideoPropDlg::OnFrameSpinDown(wxSpinEvent& event) {
    long pos = String2Time(m_timeCtrl->GetValue(), m_mediaCtrl->GetFps()) - 1000 / m_mediaCtrl->GetFps();
    SeekVideo(pos >= 0 ? pos : 0);
}

void VideoPropDlg::OnStartSpin(wxSpinEvent& event) {
   	long pos = String2Time(m_startCtrl->GetValue(), m_mediaCtrl->GetFps());
   	pos = pos - pos % 1000 + 1000;
	m_startCtrl->ChangeValue(Time2String(pos, true));
}

void VideoPropDlg::OnStartSpinDown(wxSpinEvent& event) {
    long pos = String2Time(m_startCtrl->GetValue(), m_mediaCtrl->GetFps());
    pos = pos - pos % 1000 - (pos % 1000 > 0 ? 0 : 1000);
    m_startCtrl->ChangeValue(Time2String(pos >= 0 ? pos : 0, true));
}

void VideoPropDlg::OnStartFrameSpin(wxSpinEvent& event) {
	long pos = String2Time(m_startCtrl->GetValue(), m_mediaCtrl->GetFps()) + 1000 / m_mediaCtrl->GetFps();
	m_startCtrl->ChangeValue(Time2String(pos, true));
}

void VideoPropDlg::OnStartFrameSpinDown(wxSpinEvent& event) {
	long pos = String2Time(m_startCtrl->GetValue(), m_mediaCtrl->GetFps()) - 1000 / m_mediaCtrl->GetFps();
	m_startCtrl->ChangeValue(Time2String(pos >= 0 ? pos : 0, true));
}

void VideoPropDlg::OnEndSpin(wxSpinEvent& event) {
   	long pos = String2Time(m_endCtrl->GetValue(), m_mediaCtrl->GetFps());
   	pos = pos - pos % 1000 + 1000;
	m_endCtrl->ChangeValue(Time2String(pos, true));
}

void VideoPropDlg::OnEndSpinDown(wxSpinEvent& event) {
	long pos = String2Time(m_endCtrl->GetValue(), m_mediaCtrl->GetFps());
	pos = pos - pos % 1000 - (pos % 1000 > 0 ? 0 : 1000);
	m_endCtrl->ChangeValue(Time2String(pos >= 0 ? pos : 0, true));
}

void VideoPropDlg::OnEndFrameSpin(wxSpinEvent& event) {
	long pos = String2Time(m_endCtrl->GetValue(), m_mediaCtrl->GetFps());
	m_endCtrl->ChangeValue(Time2String(pos + 1000 / m_mediaCtrl->GetFps(), true));
}

void VideoPropDlg::OnEndFrameSpinDown(wxSpinEvent& event) {
	long pos = String2Time(m_endCtrl->GetValue(), m_mediaCtrl->GetFps()) - 1000 / m_mediaCtrl->GetFps();
	m_endCtrl->ChangeValue(Time2String(pos >= 0 ? pos : 0, true));
}

void VideoPropDlg::OnChangeFormat(wxCommandEvent& event) {
	// update keepAspect, border and crop controls
	if (GetVideoFormat() == vfCOPY) {
		m_interlacedChoice->SetSelection(0);
		m_keepAspectCtrl->SetValue(false);
		m_padLeft->SetValue(0);
		m_padRight->SetValue(0);
		m_padTop->SetValue(0);
		m_padBottom->SetValue(0);
		m_cropLeft->SetValue(0);
		m_cropRight->SetValue(0);
		m_cropTop->SetValue(0);
		m_cropBottom->SetValue(0);
	} else if (m_lastFormat == vfCOPY)
		m_keepAspectCtrl->SetValue(true);
	m_lastFormat = GetVideoFormat();

	// enable/disable controls
	bool enable = GetVideoFormat() != vfCOPY;
	bool enablePad = enable && !m_keepAspectCtrl->GetValue();
	m_interlacedChoice->Enable(enable);
	m_keepAspectCtrl->Enable(enable);
	m_keepAspectChoice->Enable(enable && m_keepAspectCtrl->GetValue());
	m_padLeft->Enable(enablePad);
	m_padRight->Enable(enablePad);
	m_padTop->Enable(enablePad);
	m_padBottom->Enable(enablePad);
	m_cropLeft->Enable(enable);
	m_cropRight->Enable(enable);
	m_cropTop->Enable(enable);
	m_cropBottom->Enable(enable);
	m_fadeInCtrl->Enable(enable);
	m_fadeOutCtrl->Enable(enable);
	m_filtersCtrl->Enable(enable);
	wxCommandEvent evt;
	OnInterlaced(evt);
	UpdatePadCrop();
}

void VideoPropDlg::OnInterlaced(wxCommandEvent& event) {
	m_ffChoice->Enable(m_interlacedChoice->GetSelection() == 1);
	if (m_interlacedChoice->GetSelection() == 0)
		m_ffChoice->SetSelection(0);
}

void VideoPropDlg::OnChangeAspect(wxCommandEvent& event) {
	UpdatePadCrop();
}

void VideoPropDlg::OnChangeKeepAspect(wxCommandEvent& event) {
	m_keepAspectChoice->Enable(m_keepAspectCtrl->GetValue());
	bool keepAspect = m_keepAspectCtrl->GetValue();
	bool border = m_keepAspectChoice->GetSelection() == 0;
	m_padLeft->Enable(!keepAspect || !border);
	m_padRight->Enable(!keepAspect || !border);
	m_padTop->Enable(!keepAspect || !border);
	m_padBottom->Enable(!keepAspect || !border);
	m_cropLeft->Enable(!keepAspect || border);
	m_cropRight->Enable(!keepAspect || border);
	m_cropTop->Enable(!keepAspect || border);
	m_cropBottom->Enable(!keepAspect || border);
	if (keepAspect) {
		if (border) {
			m_cropLeft->SetValue(0);
			m_cropRight->SetValue(0);
			m_cropTop->SetValue(0);
			m_cropBottom->SetValue(0);
		} else {
			m_padLeft->SetValue(0);
			m_padRight->SetValue(0);
			m_padTop->SetValue(0);
			m_padBottom->SetValue(0);
		}
	} else {
		if (border) {
			m_padLeft->SetValue(0);
			m_padRight->SetValue(0);
			m_padTop->SetValue(0);
			m_padBottom->SetValue(0);
		} else {
			m_cropLeft->SetValue(0);
			m_cropRight->SetValue(0);
			m_cropTop->SetValue(0);
			m_cropBottom->SetValue(0);
		}
	}
	UpdatePadCrop();
}

void VideoPropDlg::OnChangeBorder(wxSpinEvent& event) {
	UpdatePadCrop();
}

void VideoPropDlg::OnChangeCrop(wxSpinEvent& event) {
	UpdatePadCrop();
}

void VideoPropDlg::OnStartBt(wxCommandEvent& event) {
	m_startCtrl->SetValue(Time2String(m_videoPos, true));
}

void VideoPropDlg::OnEndBt(wxCommandEvent& event) {
	if (m_slider->GetValue() < m_slider->GetMax()) {
		m_endCtrl->SetValue(Time2String(m_videoPos, true));
	} else
		m_endCtrl->SetValue(wxT(""));
}

void VideoPropDlg::OnOkBt(wxCommandEvent& event) {
	if (m_startCtrl->GetValue().length() > 0 && TimeToDouble(m_startCtrl->GetValue()) == -1) {
		wxLogError(_("Invalid time") + wxString(wxT(": ")) + m_startCtrl->GetValue());
		event.Skip();
		return;
	}
	if (m_endCtrl->GetValue().length() > 0 && TimeToDouble(m_endCtrl->GetValue()) == -1) {
		wxLogError(_("Invalid time") + wxString(wxT(": ")) + m_startCtrl->GetValue());
		event.Skip();
		return;
	}
	this->EndModal(wxID_OK);
}
