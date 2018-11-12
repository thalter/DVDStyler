/////////////////////////////////////////////////////////////////////////////
// Name:        SubtitlePropDlg.cpp
// Purpose:     The subtitle properties dialog
// Author:      Alex Thuering
// Created:     24.02.2010
// RCS-ID:      $Id: SubtitlePropDlg.cpp,v 1.15 2016/04/19 23:09:52 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "SubtitlePropDlg.h"
#include "DVD.h"
#include "Config.h"
#include <wx/filename.h>
#include <wx/colordlg.h>
#include <wxVillaLib/utils.h>

//(*InternalHeaders(SubtitlePropDlg)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(SubtitlePropDlg)
const long SubtitlePropDlg::ID_CHOICE1 = wxNewId();
const long SubtitlePropDlg::ID_FORCE_CB = wxNewId();
const long SubtitlePropDlg::ID_CHOICE2 = wxNewId();
const long SubtitlePropDlg::FONT_FAMILY_BOX_ID = wxNewId();
const long SubtitlePropDlg::FONT_STYLE_BOX_ID = wxNewId();
const long SubtitlePropDlg::FONT_SIZE_SPIN_ID = wxNewId();
const long SubtitlePropDlg::FONT_SIZE_BOX_ID = wxNewId();
const long SubtitlePropDlg::ID_STATICTEXT4 = wxNewId();
const long SubtitlePropDlg::ID_TEXTCTRL1 = wxNewId();
const long SubtitlePropDlg::ID_PANEL1 = wxNewId();
const long SubtitlePropDlg::FILL_BT_ID = wxNewId();
const long SubtitlePropDlg::ID_SPINCTRL7 = wxNewId();
const long SubtitlePropDlg::ID_STATICTEXT10 = wxNewId();
const long SubtitlePropDlg::ID_PANEL2 = wxNewId();
const long SubtitlePropDlg::OUTLINE_BT_ID = wxNewId();
const long SubtitlePropDlg::ID_SPINCTRL8 = wxNewId();
const long SubtitlePropDlg::ID_TEXTCTRL2 = wxNewId();
const long SubtitlePropDlg::ID_PANEL3 = wxNewId();
const long SubtitlePropDlg::SHADOW_BT_ID = wxNewId();
const long SubtitlePropDlg::ID_SPINCTRL9 = wxNewId();
const long SubtitlePropDlg::ID_SPINCTRL1 = wxNewId();
const long SubtitlePropDlg::ID_SPINCTRL2 = wxNewId();
const long SubtitlePropDlg::ID_RADIOBUTTON1 = wxNewId();
const long SubtitlePropDlg::ID_RADIOBUTTON2 = wxNewId();
const long SubtitlePropDlg::ID_RADIOBUTTON3 = wxNewId();
const long SubtitlePropDlg::ID_RADIOBUTTON4 = wxNewId();
const long SubtitlePropDlg::ID_RADIOBUTTON6 = wxNewId();
const long SubtitlePropDlg::ID_RADIOBUTTON7 = wxNewId();
const long SubtitlePropDlg::ID_RADIOBUTTON8 = wxNewId();
const long SubtitlePropDlg::ID_RADIOBUTTON9 = wxNewId();
const long SubtitlePropDlg::ID_SPINCTRL3 = wxNewId();
const long SubtitlePropDlg::ID_SPINCTRL4 = wxNewId();
const long SubtitlePropDlg::ID_SPINCTRL5 = wxNewId();
const long SubtitlePropDlg::ID_SPINCTRL6 = wxNewId();
const long SubtitlePropDlg::ID_DEF_BT = wxNewId();
const long SubtitlePropDlg::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(SubtitlePropDlg,wxDialog)
	//(*EventTable(SubtitlePropDlg)
	//*)
END_EVENT_TABLE()

SubtitlePropDlg::SubtitlePropDlg(wxWindow* parent,TextSub* textsub, const wxString& langCode) {
	m_textsub = textsub;
	m_langCode = langCode;

	//(*Initialize(SubtitlePropDlg)
	wxStaticText* staticText2;
	wxGridSizer* alignmentSizer;
	wxButton* resetBt;
	wxStaticText* staticText5;
	wxBoxSizer* boxSizer1;
	wxStaticText* staticText4;
	wxStaticText* staticText8;
	wxBoxSizer* boxSizer2;
	wxStaticText* languageLabel;
	wxStaticBoxSizer* staticBoxSizer2;
	wxStaticBoxSizer* staticBoxSizer1;
	wxStaticText* StaticText3;
	wxStaticText* staticText7;
	wxStaticText* staticText10;
	wxStdDialogButtonSizer* stdDialogButtonSizer;
	wxBoxSizer* fSizer2;
	wxStaticText* staticText1;
	wxBoxSizer* fillSizer;
	wxBoxSizer* propSizer2;
	wxBoxSizer* BoxSizer1;
	wxStaticText* staticText11;
	wxBoxSizer* fSizer1;
	wxBoxSizer* shadowSizer;
	wxStaticText* staticText6;
	wxBoxSizer* btSizer;
	wxBoxSizer* mainSizer;
	wxGridSizer* marginSizer;
	wxBoxSizer* fSizer3;
	wxFlexGridSizer* propSizer;

	Create(parent, wxID_ANY, _("Subtitle Properties"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	mainSizer = new wxBoxSizer(wxVERTICAL);
	propSizer = new wxFlexGridSizer(0, 2, 4, 2);
	propSizer->AddGrowableCol(1);
	propSizer->AddGrowableRow(2);
	languageLabel = new wxStaticText(this, wxID_ANY, _("Language:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	propSizer->Add(languageLabel, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_langChoice = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	BoxSizer1->Add(m_langChoice, 0, wxALL|wxALIGN_CENTER_VERTICAL, 0);
	m_force = new wxCheckBox(this, ID_FORCE_CB, _("Force display"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_FORCE_CB"));
	m_force->SetValue(false);
	BoxSizer1->Add(m_force, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	propSizer->Add(BoxSizer1, 0, wxEXPAND, 0);
	staticText1 = new wxStaticText(this, wxID_ANY, _("Charset:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	propSizer->Add(staticText1, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	m_charsetChoice = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	propSizer->Add(m_charsetChoice, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	staticText2 = new wxStaticText(this, wxID_ANY, _("Font:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	propSizer->Add(staticText2, 0, wxTOP|wxALIGN_LEFT|wxALIGN_TOP, 2);
	boxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_fontFamilyBox = new wxListBox(this, FONT_FAMILY_BOX_ID, wxDefaultPosition, wxSize(-1,250), 0, 0, wxLB_SINGLE|wxLB_SORT, wxDefaultValidator, _T("FONT_FAMILY_BOX_ID"));
	boxSizer1->Add(m_fontFamilyBox, 0, wxRIGHT|wxEXPAND, 6);
	fSizer1 = new wxBoxSizer(wxVERTICAL);
	fSizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_fontStyleBox = new wxListBox(this, FONT_STYLE_BOX_ID, wxDefaultPosition, wxSize(140,40), 0, 0, wxLB_SINGLE|wxLB_SORT, wxDefaultValidator, _T("FONT_STYLE_BOX_ID"));
	fSizer2->Add(m_fontStyleBox, 1, wxRIGHT|wxEXPAND, 6);
	fSizer3 = new wxBoxSizer(wxVERTICAL);
	m_fontSizeSpin = new wxSpinCtrl(this, FONT_SIZE_SPIN_ID, _T("0"), wxDefaultPosition, wxSize(56,-1), 0, 0, 100, 0, _T("FONT_SIZE_SPIN_ID"));
	m_fontSizeSpin->SetValue(_T("0"));
	fSizer3->Add(m_fontSizeSpin, 0, wxBOTTOM|wxEXPAND, 6);
	m_fontSizeBox = new wxListBox(this, FONT_SIZE_BOX_ID, wxDefaultPosition, wxSize(56,40), 0, 0, wxLB_SINGLE, wxDefaultValidator, _T("FONT_SIZE_BOX_ID"));
	fSizer3->Add(m_fontSizeBox, 1, wxALL|wxEXPAND, 0);
	fSizer2->Add(fSizer3, 0, wxBOTTOM|wxEXPAND, 0);
	fSizer1->Add(fSizer2, 1, wxBOTTOM|wxEXPAND, 2);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT4, _("Preview:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	fSizer1->Add(StaticText3, 0, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT, 2);
	m_previewText = new wxTextCtrl(this, ID_TEXTCTRL1, _("ABCabcXYZxyz"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	fSizer1->Add(m_previewText, 0, wxTOP|wxRIGHT|wxEXPAND, 2);
	boxSizer1->Add(fSizer1, 1, wxEXPAND, 0);
	propSizer->Add(boxSizer1, 0, wxALL|wxEXPAND, 0);
	staticText4 = new wxStaticText(this, wxID_ANY, _("Fill:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	propSizer->Add(staticText4, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	fillSizer = new wxBoxSizer(wxHORIZONTAL);
	m_fillPanel = new ColourPanel(this, ID_PANEL1, wxDefaultPosition, wxSize(20,20), wxSUNKEN_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	fillSizer->Add(m_fillPanel, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 4);
	m_fillBt = new wxButton(this, FILL_BT_ID, _("..."), wxDefaultPosition, wxSize(24,24), 0, wxDefaultValidator, _T("FILL_BT_ID"));
	fillSizer->Add(m_fillBt, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	m_fillOpacityCtrl = new wxSpinCtrl(this, ID_SPINCTRL7, _T("100"), wxDefaultPosition, wxSize(48,-1), 0, 0, 100, 100, _T("ID_SPINCTRL7"));
	m_fillOpacityCtrl->SetValue(_T("100"));
	fillSizer->Add(m_fillOpacityCtrl, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT10, _("%"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	fillSizer->Add(StaticText9, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	propSizer->Add(fillSizer, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 0);
	staticText5 = new wxStaticText(this, wxID_ANY, _("Outline:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	propSizer->Add(staticText5, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	boxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_outlinePanel = new ColourPanel(this, ID_PANEL2, wxDefaultPosition, wxSize(20,20), wxSUNKEN_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	boxSizer2->Add(m_outlinePanel, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	m_outlineBt = new wxButton(this, OUTLINE_BT_ID, _("..."), wxDefaultPosition, wxSize(24,24), 0, wxDefaultValidator, _T("OUTLINE_BT_ID"));
	boxSizer2->Add(m_outlineBt, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	m_outlineOpacityCtrl = new wxSpinCtrl(this, ID_SPINCTRL8, _T("100"), wxDefaultPosition, wxDefaultSize, 0, 0, 100, 100, _T("ID_SPINCTRL8"));
	m_outlineOpacityCtrl->SetValue(_T("100"));
	m_outlineOpacityCtrl->SetMinSize(wxSize(48,-1));
	boxSizer2->Add(m_outlineOpacityCtrl, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	staticText10 = new wxStaticText(this, wxID_ANY, _("%"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	boxSizer2->Add(staticText10, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 6);
	staticText6 = new wxStaticText(this, wxID_ANY, _("Thickness:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	boxSizer2->Add(staticText6, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_thicknessCtrl = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxSize(80,-1), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	boxSizer2->Add(m_thicknessCtrl, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	propSizer->Add(boxSizer2, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	staticText7 = new wxStaticText(this, wxID_ANY, _("Shadow:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	propSizer->Add(staticText7, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	shadowSizer = new wxBoxSizer(wxHORIZONTAL);
	m_shadowPanel = new ColourPanel(this, ID_PANEL3, wxDefaultPosition, wxSize(20,20), wxSUNKEN_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	shadowSizer->Add(m_shadowPanel, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	m_shadowBt = new wxButton(this, SHADOW_BT_ID, _("..."), wxDefaultPosition, wxSize(24,24), 0, wxDefaultValidator, _T("SHADOW_BT_ID"));
	shadowSizer->Add(m_shadowBt, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 4);
	m_shadowOpacityCtrl = new wxSpinCtrl(this, ID_SPINCTRL9, _T("100"), wxDefaultPosition, wxSize(48,-1), 0, 0, 100, 100, _T("ID_SPINCTRL9"));
	m_shadowOpacityCtrl->SetValue(_T("100"));
	shadowSizer->Add(m_shadowOpacityCtrl, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	staticText11 = new wxStaticText(this, wxID_ANY, _("%"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	shadowSizer->Add(staticText11, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 6);
	staticText8 = new wxStaticText(this, wxID_ANY, _("Offset:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	shadowSizer->Add(staticText8, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_offsetXCtrl = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxSize(60,-1), 0, 0, 100, 0, _T("ID_SPINCTRL1"));
	m_offsetXCtrl->SetValue(_T("0"));
	shadowSizer->Add(m_offsetXCtrl, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_offsetYCtrl = new wxSpinCtrl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxSize(60,-1), 0, 0, 100, 0, _T("ID_SPINCTRL2"));
	m_offsetYCtrl->SetValue(_T("0"));
	shadowSizer->Add(m_offsetYCtrl, 0, wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	propSizer->Add(shadowSizer, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	propSizer->Add(-1,-1,0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	propSizer2 = new wxBoxSizer(wxHORIZONTAL);
	staticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Alignment"));
	alignmentSizer = new wxGridSizer(0, 3, 12, 16);
	m_alignRadioTL = new wxRadioButton(this, ID_RADIOBUTTON1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
	alignmentSizer->Add(m_alignRadioTL, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_alignRadioTC = new wxRadioButton(this, ID_RADIOBUTTON2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
	alignmentSizer->Add(m_alignRadioTC, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_alignRadioTR = new wxRadioButton(this, ID_RADIOBUTTON3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON3"));
	alignmentSizer->Add(m_alignRadioTR, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_alignRadioCL = new wxRadioButton(this, ID_RADIOBUTTON4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON4"));
	alignmentSizer->Add(m_alignRadioCL, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	alignmentSizer->Add(-1,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_alignRadioCR = new wxRadioButton(this, ID_RADIOBUTTON6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON6"));
	alignmentSizer->Add(m_alignRadioCR, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_alignRadioBL = new wxRadioButton(this, ID_RADIOBUTTON7, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON7"));
	alignmentSizer->Add(m_alignRadioBL, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_alignRadioBC = new wxRadioButton(this, ID_RADIOBUTTON8, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON8"));
	alignmentSizer->Add(m_alignRadioBC, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_alignRadioBR = new wxRadioButton(this, ID_RADIOBUTTON9, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON9"));
	alignmentSizer->Add(m_alignRadioBR, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	staticBoxSizer1->Add(alignmentSizer, 0, wxTOP|wxLEFT|wxEXPAND, 4);
	propSizer2->Add(staticBoxSizer1, 0, wxRIGHT|wxEXPAND, 6);
	staticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Margin"));
	marginSizer = new wxGridSizer(0, 3, 4, 4);
	marginSizer->Add(-1,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_marginTop = new wxSpinCtrl(this, ID_SPINCTRL3, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL3"));
	m_marginTop->SetValue(_T("0"));
	marginSizer->Add(m_marginTop, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	marginSizer->Add(-1,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_marginLeft = new wxSpinCtrl(this, ID_SPINCTRL4, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL4"));
	m_marginLeft->SetValue(_T("0"));
	marginSizer->Add(m_marginLeft, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	marginSizer->Add(-1,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_marginRight = new wxSpinCtrl(this, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL5"));
	m_marginRight->SetValue(_T("0"));
	marginSizer->Add(m_marginRight, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	marginSizer->Add(-1,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	m_marginBottom = new wxSpinCtrl(this, ID_SPINCTRL6, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL6"));
	m_marginBottom->SetValue(_T("0"));
	marginSizer->Add(m_marginBottom, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	marginSizer->Add(-1,-1,0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	staticBoxSizer2->Add(marginSizer, 0, wxTOP|wxLEFT|wxEXPAND, 4);
	propSizer2->Add(staticBoxSizer2, 0, wxALL|wxEXPAND, 0);
	propSizer->Add(propSizer2, 1, wxTOP|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 6);
	mainSizer->Add(propSizer, 1, wxALL|wxEXPAND, 10);
	btSizer = new wxBoxSizer(wxHORIZONTAL);
	m_useAsDefaultBt = new wxButton(this, ID_DEF_BT, _("&Use as defaults"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_DEF_BT"));
	btSizer->Add(m_useAsDefaultBt, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	resetBt = new wxButton(this, ID_BUTTON1, _("Reset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	btSizer->Add(resetBt, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	stdDialogButtonSizer = new wxStdDialogButtonSizer();
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	stdDialogButtonSizer->Realize();
	btSizer->Add(stdDialogButtonSizer, 1, wxEXPAND, 4);
	mainSizer->Add(btSizer, 0, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND, 4);
	SetSizer(mainSizer);
	mainSizer->Fit(this);
	mainSizer->SetSizeHints(this);
	Center();

	Connect(FONT_FAMILY_BOX_ID,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&SubtitlePropDlg::OnFontFamilySelected);
	Connect(FONT_STYLE_BOX_ID,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&SubtitlePropDlg::OnFontStyleSelected);
	Connect(FONT_SIZE_SPIN_ID,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&SubtitlePropDlg::OnFontSizeChanged);
	Connect(FONT_SIZE_BOX_ID,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&SubtitlePropDlg::OnFontSizeSelected);
	Connect(FILL_BT_ID,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubtitlePropDlg::OnSelectFillColor);
	Connect(OUTLINE_BT_ID,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubtitlePropDlg::OnSelectOutlineColor);
	Connect(SHADOW_BT_ID,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubtitlePropDlg::OnSelectShadowColor);
	Connect(ID_DEF_BT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubtitlePropDlg::OnUseAsDefaults);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&SubtitlePropDlg::OnResetBt);
	//*)
	stdDialogButtonSizer->GetAffirmativeButton()->SetDefault();

    m_langChoice->Append(DVD::GetLanguageNames());
    map<wxString, wxString>& langMap = DVD::GetLanguageMap();
	for (map<wxString, wxString>::iterator langIt = langMap.begin(); langIt != langMap.end(); langIt++) {
		if (langIt->second == m_langCode) {
			m_langChoice->SetStringSelection(langIt->first);
		}
	}
	
	m_force->SetValue(m_textsub->GetForce());

	m_charsetChoice->Append(DVD::GetCharsets());
	m_charsetChoice->SetStringSelection(m_textsub->GetCharacterSet());

    FontMap& fonts = TextSub::GetFontMap();
    for (FontMap::iterator fontIt = fonts.begin(); fontIt != fonts.end(); fontIt++) {
    	m_fontFamilyBox->Append(fontIt->first);
    }
    m_fontFamilyBox->SetStringSelection(m_textsub->GetFontFamily());
	wxCommandEvent evt;
	OnFontFamilySelected(evt);
	m_fontStyleBox->SetStringSelection(m_textsub->GetFontStyle());

	for (int i=6; i<72; i++) {
		if (i>18) {
			i++;
		} else if (i>28) {
			i += 3;
		} else if (i>40) {
			i += 7;
		}
		m_fontSizeBox->Append(wxString::Format(wxT("%d"), i));
	}

	m_fontSizeSpin->SetValue((int)m_textsub->GetFontSize());
	wxSpinEvent spnEvt;
	OnFontSizeChanged(spnEvt);

	// colours
	SetFillColour(m_textsub->GetFillColour());
	SetOutlineColour(m_textsub->GetOutlineColour());
	SetShadowColour(m_textsub->GetShadowColour());
	m_thicknessCtrl->SetValue(wxString::Format(wxT("%0.1f"), m_textsub->GetOutlineThickness()));
	m_offsetXCtrl->SetValue(m_textsub->GetShadowOffset().x);
	m_offsetYCtrl->SetValue(m_textsub->GetShadowOffset().y);

	// align
	m_alignRadioTL->SetValue(m_textsub->GetAlignment() == (wxALIGN_TOP | wxALIGN_LEFT));
	m_alignRadioTC->SetValue(m_textsub->GetAlignment() == (wxALIGN_TOP | wxALIGN_CENTER_HORIZONTAL));
	m_alignRadioTR->SetValue(m_textsub->GetAlignment() == (wxALIGN_TOP | wxALIGN_RIGHT));
	m_alignRadioCL->SetValue(m_textsub->GetAlignment() == (wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT));
	m_alignRadioCR->SetValue(m_textsub->GetAlignment() == (wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	m_alignRadioBL->SetValue(m_textsub->GetAlignment() == (wxALIGN_BOTTOM | wxALIGN_LEFT));
	m_alignRadioBC->SetValue(m_textsub->GetAlignment() == (wxALIGN_BOTTOM | wxALIGN_CENTER_HORIZONTAL));
	m_alignRadioBR->SetValue(m_textsub->GetAlignment() == (wxALIGN_BOTTOM | wxALIGN_RIGHT));
	m_marginLeft->SetValue(m_textsub->GetLeftMargin());
	m_marginRight->SetValue(m_textsub->GetRightMargin());
	m_marginTop->SetValue(m_textsub->GetTopMargin());
	m_marginBottom->SetValue(m_textsub->GetBottomMargin());

    m_langChoice->SetMinSize(wxSize(m_fontFamilyBox->GetSize().GetWidth(), -1));
    m_charsetChoice->SetMinSize(wxSize(m_fontFamilyBox->GetSize().GetWidth(), -1));
    OnFontChanged();
    m_langChoice->SetFocus();

	mainSizer->Fit(this);
	mainSizer->SetSizeHints(this);
    SetMinSize(GetSize());
	Center();
}

SubtitlePropDlg::~SubtitlePropDlg() {
	//(*Destroy(SubtitlePropDlg)
	//*)
}


void SubtitlePropDlg::OnFontFamilySelected(wxCommandEvent& event) {
	// update font style choice
	m_fontStyleBox->Clear();
	if (m_fontFamilyBox->GetSelection() < 0)
		return;
	map<wxString, wxString>& styles = TextSub::GetFontMap()[m_fontFamilyBox->GetStringSelection()];
	for (map<wxString, wxString>::iterator styleIt = styles.begin(); styleIt != styles.end(); styleIt++) {
		m_fontStyleBox->Append(styleIt->first);
	}
	m_fontStyleBox->SetStringSelection(wxT("Regular"));
	if (m_fontStyleBox->GetSelection() == -1)
		m_fontStyleBox->SetStringSelection(wxT("Normal"));
	if (m_fontStyleBox->GetSelection() == -1)
		m_fontStyleBox->SetStringSelection(wxT("Standard"));
	if (m_fontStyleBox->GetSelection() == -1)
		m_fontStyleBox->SetSelection(0);
	OnFontChanged();
}

void SubtitlePropDlg::OnFontStyleSelected(wxCommandEvent& event) {
	OnFontChanged();
}

void SubtitlePropDlg::OnFontSizeChanged(wxSpinEvent& event) {
	m_fontSizeBox->SetStringSelection(wxString::Format(wxT("%d"), m_fontSizeSpin->GetValue()));
	OnFontChanged();
}

void SubtitlePropDlg::OnFontSizeSelected(wxCommandEvent& event) {
	long size;
	if (m_fontSizeBox->GetStringSelection().ToLong(&size))
		m_fontSizeSpin->SetValue(size);
	OnFontChanged();
}

void SubtitlePropDlg::OnFontChanged() {
	m_previewText->SetMinSize(wxSize(-1, -1));
	wxFont font = m_previewText->GetFont();
	font.SetFaceName(m_fontFamilyBox->GetStringSelection());
	wxString style = m_fontStyleBox->GetStringSelection().Lower();
	if (style.Find(wxT("slant")) != wxNOT_FOUND || style.Find(wxString(_("slant")).Lower()) != wxNOT_FOUND) {
		font.SetStyle(wxFONTSTYLE_SLANT);
	} else if (style.Find(wxT("italic")) != wxNOT_FOUND || style.Find(wxString(_("italic")).Lower()) != wxNOT_FOUND
			|| style.Find(wxT("cursiva")) != wxNOT_FOUND) {
		font.SetStyle(wxFONTSTYLE_ITALIC);
	} else
		font.SetStyle(wxFONTSTYLE_NORMAL);
	if (style.Find(wxT("light")) != wxNOT_FOUND || style.Find(wxString(_("light")).Lower()) != wxNOT_FOUND)
		font.SetWeight(wxFONTWEIGHT_LIGHT);
	else if (style.Find(wxT("bold")) != wxNOT_FOUND || style.Find(wxString(_("bold")).Lower()) != wxNOT_FOUND)
		font.SetWeight(wxFONTWEIGHT_BOLD);
	else
		font.SetWeight(wxFONTWEIGHT_NORMAL);
	font.SetPointSize(m_fontSizeSpin->GetValue());
	m_previewText->SetFont(font);
	if (GetSizer()) {
		GetSizer()->Fit(this);
		Layout();
		if (m_previewText->GetSize().GetHeight() < 56)
			m_previewText->SetMinSize(wxSize(-1, 56));
		GetSizer()->Fit(this);
		Layout();
	}
}

int SubtitlePropDlg::ShowModal() {
	int res = wxDialog::ShowModal();
	if (res == wxID_OK) {
		m_textsub->SetForce(m_force->IsChecked());
		m_textsub->SetCharacterSet(m_charsetChoice->GetStringSelection());
		m_textsub->SetFontFamily(m_fontFamilyBox->GetStringSelection());
		m_textsub->SetFontStyle(m_fontStyleBox->GetStringSelection());
		m_textsub->SetFontSize(m_fontSizeSpin->GetValue());

		// colours
		m_textsub->SetFillColour(GetFillColour());
		m_textsub->SetOutlineColour(GetOutlineColour());
		m_textsub->SetShadowColour(GetShadowColour());
		double dval;
		if (m_thicknessCtrl->GetValue().ToDouble(&dval))
			m_textsub->SetOutlineThickness(dval);
		m_textsub->SetShadowOffset(wxPoint(m_offsetXCtrl->GetValue(), m_offsetYCtrl->GetValue()));

		// alignment
		if (m_alignRadioTL->GetValue())
			m_textsub->SetAlignment((wxAlignment) (wxALIGN_TOP | wxALIGN_LEFT));
		else if (m_alignRadioTC->GetValue())
			m_textsub->SetAlignment((wxAlignment) (wxALIGN_TOP | wxALIGN_CENTER_HORIZONTAL));
		else if (m_alignRadioTR->GetValue())
			m_textsub->SetAlignment((wxAlignment) (wxALIGN_TOP | wxALIGN_RIGHT));
		else if (m_alignRadioCL->GetValue())
			m_textsub->SetAlignment((wxAlignment) (wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT));
		else if (m_alignRadioCR->GetValue())
			m_textsub->SetAlignment((wxAlignment) (wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
		else if (m_alignRadioBL->GetValue())
			m_textsub->SetAlignment((wxAlignment) (wxALIGN_BOTTOM | wxALIGN_LEFT));
		else if (m_alignRadioBC->GetValue())
			m_textsub->SetAlignment((wxAlignment) (wxALIGN_BOTTOM | wxALIGN_CENTER_HORIZONTAL));
		else if (m_alignRadioBR->GetValue())
			m_textsub->SetAlignment((wxAlignment) (wxALIGN_BOTTOM | wxALIGN_RIGHT));

		// margin
		m_textsub->SetLeftMargin(m_marginLeft->GetValue());
		m_textsub->SetRightMargin(m_marginRight->GetValue());
		m_textsub->SetTopMargin(m_marginTop->GetValue());
		m_textsub->SetBottomMargin(m_marginBottom->GetValue());
	}
	return res;
}

/**
 * Returns selected language code
 */
wxString SubtitlePropDlg::GetLangCode() {
	return DVD::GetLanguageMap()[m_langChoice->GetStringSelection()];
}

void SubtitlePropDlg::OnSelectFillColor(wxCommandEvent &event) {
	wxColourData data;
	data.SetColour(GetFillColour());
	wxColourDialog dialog(this, &data);
	if (dialog.ShowModal() == wxID_OK)
		SetFillColour(dialog.GetColourData().GetColour());
}

void SubtitlePropDlg::OnSelectOutlineColor(wxCommandEvent &event) {
	wxColourData data;
	data.SetColour(GetOutlineColour());
	wxColourDialog dialog(this, &data);
	if (dialog.ShowModal() == wxID_OK)
		SetOutlineColour(dialog.GetColourData().GetColour());
}

void SubtitlePropDlg::OnSelectShadowColor(wxCommandEvent &event) {
	wxColourData data;
	data.SetColour(GetShadowColour());
	wxColourDialog dialog(this, &data);
	if (dialog.ShowModal() == wxID_OK)
		SetShadowColour(dialog.GetColourData().GetColour());
}


wxColour SubtitlePropDlg::GetFillColour() {
	wxColour colour = m_fillPanel->GetColour();
	if (colour.IsOk())
		colour = wxColour(colour.Red(), colour.Green(), colour.Blue(),
				lround(((double) m_fillOpacityCtrl->GetValue()) * 255 / 100));
	return colour;
}

void SubtitlePropDlg::SetFillColour(const wxColour& colour) {
	m_fillPanel->SetColour(colour);
	if (colour.IsOk())
		m_fillOpacityCtrl->SetValue(lround(((double) colour.Alpha()) * 100 / 255));
}

wxColour SubtitlePropDlg::GetOutlineColour() {
	wxColour colour = m_outlinePanel->GetColour();
	if (colour.IsOk())
		colour = wxColour(colour.Red(), colour.Green(), colour.Blue(),
				lround(((double) m_outlineOpacityCtrl->GetValue()) * 255 / 100));
	return colour;
}

void SubtitlePropDlg::SetOutlineColour(const wxColour& colour) {
	m_outlinePanel->SetColour(colour);
	if (colour.IsOk())
		m_outlineOpacityCtrl->SetValue(lround(((double) colour.Alpha()) * 100 / 255));
}

wxColour SubtitlePropDlg::GetShadowColour() {
	wxColour colour = m_shadowPanel->GetColour();
	if (colour.IsOk())
		colour = wxColour(colour.Red(), colour.Green(), colour.Blue(),
				lround(((double) m_shadowOpacityCtrl->GetValue()) * 255 / 100));
	return colour;
}

void SubtitlePropDlg::SetShadowColour(const wxColour& colour) {
	m_shadowPanel->SetColour(colour);
	if (colour.IsOk())
		m_shadowOpacityCtrl->SetValue(lround(((double) colour.Alpha()) * 100 / 255));
}

void SubtitlePropDlg::OnUseAsDefaults(wxCommandEvent& event) {
	s_config.SetSubtitlesCharacterSet(m_charsetChoice->GetStringSelection());
	s_config.SetSubtitlesFontFamily(m_fontFamilyBox->GetStringSelection());
	s_config.SetSubtitlesFontStyle(m_fontStyleBox->GetStringSelection());
	s_config.SetSubtitlesFontSize(m_fontSizeSpin->GetValue());

	// colours
	s_config.SetSubtitlesFillColour(GetFillColour());
	s_config.SetSubtitlesOutlineColour(GetOutlineColour());
	s_config.SetSubtitlesShadowColour(GetShadowColour());
	double dval;
	if (m_thicknessCtrl->GetValue().ToDouble(&dval))
		s_config.SetSubtitlesOutlineThickness(dval);
	s_config.SetSubtitlesShadowOffsetX(m_offsetXCtrl->GetValue());
	s_config.SetSubtitlesShadowOffsetY(m_offsetYCtrl->GetValue());

	// alignment
	if (m_alignRadioTL->GetValue())
		s_config.SetSubtitlesAlignment((wxAlignment) (wxALIGN_TOP | wxALIGN_LEFT));
	else if (m_alignRadioTC->GetValue())
		s_config.SetSubtitlesAlignment((wxAlignment) (wxALIGN_TOP | wxALIGN_CENTER_HORIZONTAL));
	else if (m_alignRadioTR->GetValue())
		s_config.SetSubtitlesAlignment((wxAlignment) (wxALIGN_TOP | wxALIGN_RIGHT));
	else if (m_alignRadioCL->GetValue())
		s_config.SetSubtitlesAlignment((wxAlignment) (wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT));
	else if (m_alignRadioCR->GetValue())
		s_config.SetSubtitlesAlignment((wxAlignment) (wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	else if (m_alignRadioBL->GetValue())
		s_config.SetSubtitlesAlignment((wxAlignment) (wxALIGN_BOTTOM | wxALIGN_LEFT));
	else if (m_alignRadioBC->GetValue())
		s_config.SetSubtitlesAlignment((wxAlignment) (wxALIGN_BOTTOM | wxALIGN_CENTER_HORIZONTAL));
	else if (m_alignRadioBR->GetValue())
		s_config.SetSubtitlesAlignment((wxAlignment) (wxALIGN_BOTTOM | wxALIGN_RIGHT));

	// margin
	s_config.SetSubtitlesLeftMargin(m_marginLeft->GetValue());
	s_config.SetSubtitlesRightMargin(m_marginRight->GetValue());
	s_config.SetSubtitlesTopMargin(m_marginTop->GetValue());
	s_config.SetSubtitlesBottomMargin(m_marginBottom->GetValue());
	s_config.Flush();
}

void SubtitlePropDlg::OnResetBt(wxCommandEvent& event) {
	m_charsetChoice->SetStringSelection(s_config.GetSubtitlesCharacterSet(true));
	m_fontFamilyBox->SetStringSelection(s_config.GetSubtitlesFontFamily(true));
	m_fontStyleBox->SetStringSelection(s_config.GetSubtitlesFontStyle(true));
	m_fontSizeSpin->SetValue(s_config.GetSubtitlesFontSize(true));

	// colours
	SetFillColour(s_config.GetSubtitlesFillColour(true));
	SetOutlineColour(s_config.GetSubtitlesOutlineColour(true));
	SetShadowColour(s_config.GetSubtitlesShadowColour(true));
	m_thicknessCtrl->SetValue(wxString::Format(wxT("%0.1f"), s_config.GetSubtitlesOutlineThickness(true)));
	m_offsetXCtrl->SetValue(s_config.GetSubtitlesShadowOffsetX(true));
	m_offsetYCtrl->SetValue(s_config.GetSubtitlesShadowOffsetY(true));

	// alignment
	m_alignRadioTL->SetValue(s_config.GetSubtitlesAlignment(true) == (wxALIGN_TOP | wxALIGN_LEFT));
	m_alignRadioTC->SetValue(s_config.GetSubtitlesAlignment(true) == (wxALIGN_TOP | wxALIGN_CENTER_HORIZONTAL));
	m_alignRadioTR->SetValue(s_config.GetSubtitlesAlignment(true) == (wxALIGN_TOP | wxALIGN_RIGHT));
	m_alignRadioCL->SetValue(s_config.GetSubtitlesAlignment(true) == (wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT));
	m_alignRadioCR->SetValue(s_config.GetSubtitlesAlignment(true) == (wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT));
	m_alignRadioBL->SetValue(s_config.GetSubtitlesAlignment(true) == (wxALIGN_BOTTOM | wxALIGN_LEFT));
	m_alignRadioBC->SetValue(s_config.GetSubtitlesAlignment(true) == (wxALIGN_BOTTOM | wxALIGN_CENTER_HORIZONTAL));
	m_alignRadioBR->SetValue(s_config.GetSubtitlesAlignment(true) == (wxALIGN_BOTTOM | wxALIGN_RIGHT));
	
	// margin
	m_marginLeft->SetValue(s_config.GetSubtitlesLeftMargin(true));
	m_marginRight->SetValue(s_config.GetSubtitlesRightMargin(true));
	m_marginTop->SetValue(s_config.GetSubtitlesTopMargin(true));
	m_marginBottom->SetValue(s_config.GetSubtitlesBottomMargin(true));
}
