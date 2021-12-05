/////////////////////////////////////////////////////////////////////////////
// Name:        PropDlg.cpp
// Purpose:     Properties dialog (base class for all properties dialogs)
// Author:      Alex Thuering
// Created:     18.11.2003
// RCS-ID:      $Id: PropDlg.cpp,v 1.53 2016/08/12 16:17:20 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "PropDlg.h"
#include "utils.h"
#include <wx/fontdlg.h>
#include <wx/colordlg.h>
#ifdef __WXMSW__
#include <wx/generic/colrdlgg.h>
#endif
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/grid.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/bmpcbox.h>
#include <wx/tglbtn.h>
#include <wx/tooltip.h>
#include <wx/richtooltip.h>
#include <wx/wx.h>
#if !wxCHECK_VERSION(2,9,0)
#define wxBitmapToggleButton wxToggleBitmapButton
#endif
#ifdef __WXMSW__
#include <setupapi.h>
#include "wx/msw/private.h"  
#endif

#include "rc/delete.png.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(BitmapArray);

//////////////////////////////////////////////////////////////////////////////
//////////////////////////// ColourPanel ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

ColourPanel::ColourPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size,
		long style, const wxString& name): wxPanel(parent, id, pos, size, style, name) {
	m_bgColour = GetBackgroundColour();
}

void ColourPanel::SetColour(wxColour colour) {
	m_colour = colour.Ok() && colour.Alpha() == 0 ? wxColour() : colour;
	if (m_colour.Ok())
		SetBackgroundColour(m_colour);
	else
		SetBackgroundColour(m_bgColour);
	Refresh();
}

wxColour ColourPanel::GetColour() {
	return m_colour;
}

/**
 * Alpha blending
 * Destination background is white
 */
unsigned char alphaBlending(unsigned char c, unsigned char alpha) {
	return (unsigned int) c*alpha/255 + (255 - alpha);
}

wxColour alphaBlending(wxColour colour) {
	return wxColour(alphaBlending(colour.Red(), colour.Alpha()),
			alphaBlending(colour.Green(), colour.Alpha()),
			alphaBlending(colour.Blue(), colour.Alpha()));
}

void ColourPanel::OnPaint(wxPaintEvent &event) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	if (!m_colour.Ok()) {
		dc.SetPen(*wxBLACK_PEN);
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		int w = GetClientSize().GetWidth();
		int h = GetClientSize().GetHeight();
		dc.DrawLine(0, 0, w, h);
		dc.DrawLine(w, 0, 0, h);
	} else if (m_colour.Alpha() < 255) {
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(wxBrush(alphaBlending(m_colour), wxBRUSHSTYLE_SOLID));
		int w = GetClientSize().GetWidth();
		int h = GetClientSize().GetHeight();
		dc.DrawRectangle(0, 0, w/2, h/2);
		dc.DrawRectangle(w/2, h/2, w/2, h/2);
	}
}

void ColourPanel::OnClick(wxMouseEvent &event) {
	if (m_colour.Ok()) {
		m_oldColour = GetColour();
		SetColour(wxColour());
	} else
		SetColour(m_oldColour);
	Refresh();
}

BEGIN_EVENT_TABLE(ColourPanel, wxPanel)
	EVT_PAINT(ColourPanel::OnPaint)
	EVT_LEFT_DOWN(ColourPanel::OnClick)
END_EVENT_TABLE()

//////////////////////////////////////////////////////////////////////////////
///////////////////////////// wxPropDlg //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

enum {
	CUSTOM_CHOICE_ID = 7830,
	SELECT_FONT_BT_ID,
	SELECT_COLOR_BT_ID,
	SELECT_FILE_BT_ID,
	CLEAR_TEXT_BT_ID,
	SELECT_DIR_BT_ID,
	GROUP_CHECK_ID,
	ROW_DELETE_ID
};

#ifndef EVT_GRID_CELL_CHANGED
#define EVT_GRID_CELL_CHANGED EVT_GRID_CELL_CHANGE
#endif

BEGIN_EVENT_TABLE(wxPropDlg, wxDialog)
	EVT_BUTTON(wxID_CANCEL, wxPropDlg::OnCancel)
	EVT_BUTTON(wxID_OK, wxPropDlg::OnOk)
	EVT_BUTTON(wxID_APPLY, wxPropDlg::OnReset)
	EVT_BUTTON(SELECT_FONT_BT_ID, wxPropDlg::OnSelectFont)
	EVT_BUTTON(SELECT_COLOR_BT_ID, wxPropDlg::OnSelectColour)
	EVT_BUTTON(SELECT_FILE_BT_ID, wxPropDlg::OnSelectFile)
	EVT_BUTTON(CLEAR_TEXT_BT_ID, wxPropDlg::OnClearText)
	EVT_BUTTON(SELECT_DIR_BT_ID, wxPropDlg::OnSelectDir)
	EVT_CHECKBOX(GROUP_CHECK_ID, wxPropDlg::OnGroupCheck)
	EVT_GRID_CELL_LEFT_CLICK(wxPropDlg::OnCellLeftClick)
	EVT_GRID_CELL_RIGHT_CLICK(wxPropDlg::OnCellRightClick)
	EVT_GRID_CELL_CHANGED(wxPropDlg::OnCellChange)
	EVT_MENU(ROW_DELETE_ID, wxPropDlg::OnRowDelete)
	EVT_CHOICE(CUSTOM_CHOICE_ID, wxPropDlg::OnSelectCustomChoice)
END_EVENT_TABLE()

enum PropertyType {
	propTEXT = 0,
	propSTAT_TEXT,
	propSPIN,
	propSPIN_DOUBLE,
	propCOMBO,
	propBITMAP_COMBO,
	propCHOICE,
	propCHECK,
	propRADIO,
	propRADIO_GROUP,
	propGRID,
	propCOLOUR,
	propFONT,
	propTOGGLE_BUTTON
};

wxColourData wxPropDlg::m_colourData;

wxPropDlg::wxPropDlg(wxWindow* parent, wxString title) : wxDialog(parent, -1, title, wxDefaultPosition, wxDefaultSize,
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), m_currGroupChecker(NULL), propWindow(NULL) {
	m_currGroupId = 0;
	m_updateIndex = -1;
	m_currObject = NULL;
	m_currObjectItem = 0;
	SetIcon(((wxTopLevelWindow*) wxGetTopLevelParent(parent))->GetIcon());
}

void wxPropDlg::Create(bool resetButton, bool dontShowCheckbox) {
	propWindow = this;
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* propSizer = new wxBoxSizer(wxVERTICAL);
	CreatePropPanel(propSizer);
	sizer->Add(propSizer, 1, wxEXPAND | wxALL, 10);
	CreateButtonPane(sizer, resetButton, dontShowCheckbox);
	SetAutoLayout(true);
	SetSizer(sizer);
	sizer->Fit(this);
	sizer->SetSizeHints(this);
	Layout();
	Centre();
	for (int i = 0; i < (int) m_controls.GetCount(); i++) {
		if (m_types[i] < propCOLOUR && m_types[i] != propRADIO_GROUP && ((wxControl*) m_controls[i])->IsEnabled()
				&& (m_types[i] != propRADIO || ((wxRadioButton*) m_controls[i])->GetValue())) {
			((wxControl*) m_controls[i])->SetFocus();
			break;
		}
	}
}

wxPropDlg::~wxPropDlg() {
	for (int i = 0; i < (int) m_types.GetCount(); i++)
		switch (m_types[i]) {
		case propFONT:
			delete (wxFontData*) m_controls[i];
			break;
		case propRADIO_GROUP:
			delete (wxArrayPtrVoid*) m_controls[i];
			break;
		}
}

void wxPropDlg::OnCancel(wxCommandEvent &WXUNUSED(event)) {
	EndModal(wxID_CANCEL);
}

void wxPropDlg::OnOk(wxCommandEvent &WXUNUSED(event)) {
	if (SetValues())
		EndModal(wxID_OK);
}

void wxPropDlg::OnReset(wxCommandEvent& event) {
	Reset();
}

void wxPropDlg::CreateButtonPane(wxSizer* sizer, bool resetButton, bool dontShowCheckbox) {
	wxStdDialogButtonSizer* buttonPane = new wxStdDialogButtonSizer();
	if (dontShowCheckbox)
		AddCheckProp(buttonPane, _("&Don't show this dialog again"), false);
	buttonPane->Add(10, 10, dontShowCheckbox ? 0 : 1, wxEXPAND);
	
	buttonPane->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	buttonPane->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	if (resetButton) {
		buttonPane->AddButton(new wxButton(this, wxID_APPLY, _("Reset")));
	}
	buttonPane->Add(10, 10, dontShowCheckbox ? 0 : 1, wxEXPAND);
	buttonPane->Realize();
	buttonPane->GetAffirmativeButton()->SetDefault();
	sizer->Add(buttonPane, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 10);
}

void wxPropDlg::Reset() {
	m_updateIndex = 0;
	CreatePropPanel(NULL);
}

void wxPropDlg::AddText(wxSizer* sizer, wxString text, int flag) {
	if (!sizer)
		return;
	// add static text
	if (text.length())
		sizer->Add(new wxStaticText(propWindow, -1, text), 0, flag);
	else
		sizer->Add(10, 10);
}

wxSizer* wxPropDlg::AddTextProp(wxSizer* sizer, const wxString& label, const wxString& value, bool readonly,
		int width, bool addSpacer, long style) {
	if (!sizer) { // only update value
		wxTextCtrl* ctrl = (wxTextCtrl*) m_controls[m_updateIndex];
		ctrl->SetValue(value);
		ctrl->SetEditable(!readonly);
		m_updateIndex++;
		return NULL;
	}
	// add label
	if (label.length())
		sizer->Add(new wxStaticText(propWindow, -1, label), 0, wxALIGN_CENTER_VERTICAL);
	// add text control
	wxSize size = width > 0 ? wxSize(width, -1) : wxDefaultSize;
	wxTextCtrl* ctrl = new wxTextCtrl(propWindow, -1, value, wxDefaultPosition, size, style);
	ctrl->SetEditable(!readonly);
	wxBoxSizer* pane = NULL;
	if (width > 0) {
		pane = new wxBoxSizer(wxHORIZONTAL);
		pane->Add(ctrl);
		if (addSpacer)
			pane->Add(0, 0, 1, wxEXPAND);
		sizer->Add(pane, 1, wxEXPAND, 0);
	} else
		sizer->Add(ctrl, 1, wxEXPAND, 0);
	m_types.Add(propTEXT);
	m_controls.Add(ctrl);
	m_groupIds.Add(m_currGroupId);
	return pane;
}

void wxPropDlg::AddStaticTextProp(wxSizer* sizer, const wxString& label, const wxString& value) {
	if (!sizer) { // only update value
		((wxStaticText*) m_controls[m_updateIndex])->SetLabel(value);
		m_updateIndex++;
		return;
	}
	// add static text
	if (label.length())
		sizer->Add(new wxStaticText(propWindow, -1, label), 0, wxALIGN_CENTER_VERTICAL);
	wxStaticText* ctrl = new wxStaticText(propWindow, -1, value);
	sizer->Add(ctrl, 1, wxEXPAND, 0);
	m_types.Add(propSTAT_TEXT);
	m_controls.Add(ctrl);
	m_groupIds.Add(m_currGroupId);
}


void wxPropDlg::AddStaticLine(wxSizer* sizer, wxString caption) {
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* staticText = new wxStaticText(propWindow, -1, caption);
	int fontSize = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).GetPointSize() + 2;
	staticText->SetFont(wxFont(fontSize, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
	sizer2->Add(staticText, 0, wxRIGHT | wxTOP | wxBOTTOM, 5);
	sizer2->Add(new wxStaticLine(propWindow, -1), 1, wxALIGN_CENTER_VERTICAL, 5);
	sizer->Add(sizer2, 0, wxEXPAND, 0);
}

wxSizer* wxPropDlg::AddSpinProp(wxSizer* sizer, const wxString& label, int value,
		int min, int max, int width, const wxString& label2, bool addSpacer) {
	if (!sizer) { // only update value
		wxSpinCtrl* ctrl = (wxSpinCtrl*) m_controls[m_updateIndex];
		ctrl->SetValue(value);
		m_updateIndex++;
		return NULL;
	}
	// add text control
	if (label.length())
		sizer->Add(new wxStaticText(propWindow, -1, label), 0, wxALIGN_CENTER_VERTICAL);
	wxSize size = width > 0 ? wxSize(width, -1) : wxDefaultSize;
	wxSpinCtrl* ctrl = new wxSpinCtrl(propWindow, -1, wxEmptyString, wxDefaultPosition, size,
			wxSP_ARROW_KEYS, min, max);
	ctrl->SetValue(value);
	wxBoxSizer* pane = NULL;
	if (width > 0) {
		pane = new wxBoxSizer(wxHORIZONTAL);
		pane->Add(ctrl, 0, wxALIGN_CENTER_VERTICAL, 0);
		if (label2.length() > 0) {
			pane->AddSpacer(2);
			AddText(pane, label2);
		}
		if (addSpacer)
			pane->Add(0, 0, 1, wxEXPAND);
		sizer->Add(pane, 1, wxEXPAND, 0);
	} else
		sizer->Add(ctrl, 1, wxEXPAND, 0);
	m_types.Add(propSPIN);
	m_controls.Add(ctrl);
	m_groupIds.Add(m_currGroupId);
	return pane;
}

wxSizer* wxPropDlg::AddSpinDoubleProp(wxSizer* sizer, const wxString& label, double value,
		double min, double max, int width, const wxString& label2, bool addSpacer) {
	if (!sizer) { // only update value
#if wxCHECK_VERSION(2,9,3)
		wxSpinCtrlDouble* ctrl = (wxSpinCtrlDouble*) m_controls[m_updateIndex];
		ctrl->SetValue(value);
#else
		wxTextCtrl* ctrl = (wxTextCtrl*) m_controls[m_updateIndex];
		ctrl->SetValue(wxString::Format(wxT("%g"), value));
#endif
		m_updateIndex++;
		return NULL;
	}
	// add text control
	if (label.length())
		sizer->Add(new wxStaticText(propWindow, -1, label), 0, wxALIGN_CENTER_VERTICAL);
	wxSize size = width > 0 ? wxSize(width, -1) : wxDefaultSize;
#if wxCHECK_VERSION(2,9,3)
	wxSpinCtrlDouble* ctrl = new wxSpinCtrlDouble(propWindow, -1, wxEmptyString, wxDefaultPosition, size,
			wxSP_ARROW_KEYS, min, max);
	ctrl->SetValue(value);
#else
	wxTextCtrl* ctrl = new wxTextCtrl(propWindow, -1, wxString::Format(wxT("%g"), value), wxDefaultPosition, size);
#endif
	wxBoxSizer* pane = NULL;
	if (width > 0) {
		pane = new wxBoxSizer(wxHORIZONTAL);
		pane->Add(ctrl, 0, wxALIGN_CENTER_VERTICAL, 0);
		if (label2.length() > 0) {
			pane->AddSpacer(4);
			AddText(pane, label2);
		}
		if (addSpacer)
			pane->Add(0, 0, 1, wxEXPAND);
		sizer->Add(pane, 1, wxEXPAND, 0);
	} else
		sizer->Add(ctrl, 1, wxEXPAND, 0);
	m_types.Add(propSPIN_DOUBLE);
	m_controls.Add(ctrl);
	m_groupIds.Add(m_currGroupId);
	return pane;
}

void wxPropDlg::AddCheckProp(wxSizer* sizer, const wxString& label, bool value, bool readonly, int id) {
	if (!sizer) { // only update value
		wxCheckBox* ctrl = (wxCheckBox*) m_controls[m_updateIndex];
		ctrl->SetValue(value);
		ctrl->Enable(!readonly);
		m_updateIndex++;
		return;
	}
	// add checkbox control
	wxCheckBox* ctrl = new wxCheckBox(propWindow, id, label);
	ctrl->SetValue(value);
	ctrl->Enable(!readonly);
	ctrl->SetName(wxString::Format(_T("%d"), (int) m_controls.GetCount()));
	sizer->Add(ctrl, 0, wxEXPAND, 0);
	m_types.Add(propCHECK);
	m_controls.Add(ctrl);
	m_groupIds.Add(m_currGroupId);
}

void wxPropDlg::AddRadioProp(wxSizer* sizer, const wxString& label, bool value, long style, bool readonly, int id) {
	if (!sizer) { // only update value
		wxRadioButton* ctrl = (wxRadioButton*) m_controls[m_updateIndex];
		ctrl->SetValue(value);
		ctrl->Enable(!readonly);
		m_updateIndex++;
		return;
	}
	// add radio button
	wxRadioButton* ctrl = new wxRadioButton(propWindow, id, label, wxDefaultPosition, wxDefaultSize, style);
	ctrl->SetValue(value);
	ctrl->Enable(!readonly);
	ctrl->SetName(wxString::Format(_T("%d"), (int) m_controls.GetCount()));
	sizer->Add(ctrl, 1, wxEXPAND, 0);
	m_types.Add(propRADIO);
	m_controls.Add(ctrl);
	m_groupIds.Add(m_currGroupId);
}

void wxPropDlg::AddRadioGroupProp(wxSizer* sizer, const wxArrayString& text, int value, bool readonly) {
	if (!sizer) { // only update value
		wxArrayPtrVoid* radioControls = (wxArrayPtrVoid*) m_controls[m_updateIndex];
		for (int i = 0; i < (int) radioControls->GetCount(); i++) {
			wxRadioButton* ctrl = (wxRadioButton*) (*radioControls)[i];
			ctrl->SetValue(i == value);
			ctrl->Enable(!readonly);
		}
		m_updateIndex++;
		return;
	}
	wxArrayPtrVoid* radioControls = new wxArrayPtrVoid();
	for (int i = 0; i < (int) text.GetCount(); i++) {
		// add checkbox control
		wxRadioButton* ctrl = new wxRadioButton(propWindow, -1, text[i], wxDefaultPosition, wxDefaultSize,
				i == 0 ? wxRB_GROUP : 0);
		ctrl->SetValue(i == value);
		ctrl->Enable(!readonly);
		ctrl->SetName(wxString::Format(_T("%d_%d"), (int) m_controls.GetCount(), i));
		sizer->Add(ctrl, 0, wxEXPAND, 0);
		radioControls->Add(ctrl);
	}
	m_types.Add(propRADIO_GROUP);
	m_controls.Add(radioControls);
	m_groupIds.Add(m_currGroupId);
}

wxSizer* wxPropDlg::AddComboProp(wxSizer* sizer, const wxString& label, const wxString& value,
		const wxArrayString& choices, long style, int width, bool addSpacer, int id) {
	if (!sizer) { // only update value
		wxComboBox* ctrl = (wxComboBox*)m_controls[m_updateIndex];
		ctrl->SetStringSelection(value);
		m_updateIndex++;
		return NULL;
	}
	if (label.length())
		sizer->Add(new wxStaticText(propWindow, -1, label), 0, wxALIGN_CENTER_VERTICAL);
	// add combobox control
	wxComboBox* ctrl = new wxComboBox(propWindow, id, value, wxDefaultPosition, wxDefaultSize, choices, style);
	ctrl->SetStringSelection(value);
	wxBoxSizer* pane = NULL;
	if (width > 0) {
		pane = new wxBoxSizer(wxHORIZONTAL);
		pane->Add(ctrl);
		if (addSpacer)
			pane->Add(10, 10, 1, wxEXPAND);
		sizer->Add(pane, 1, wxEXPAND, 0);
	} else
		sizer->Add(ctrl, 1, wxEXPAND, 0);
	m_types.Add(propCOMBO);
	m_controls.Add(ctrl);
	m_groupIds.Add(m_currGroupId);
	return pane;
}

wxSizer* wxPropDlg::AddBitmapComboProp(wxSizer* sizer, const wxString& label, const wxString& value,
		const wxArrayString& choices, const BitmapArray& bitmaps, long style, int width, bool addSpacer, int id) {
	if (!sizer) { // only update value
		wxBitmapComboBox* ctrl = (wxBitmapComboBox*)m_controls[m_updateIndex];
		ctrl->SetStringSelection(value);
		m_updateIndex++;
		return NULL;
	}
	if (label.length())
		sizer->Add(new wxStaticText(propWindow, -1, label), 0, wxALIGN_CENTER_VERTICAL);
	// add bitmap combobox control
	wxBitmapComboBox* ctrl =
		new wxBitmapComboBox(propWindow, id, value, wxDefaultPosition, wxDefaultSize, choices, style);
#if wxCHECK_VERSION(2,9,3)
	wxAssertHandler_t oldAssertHandler = wxSetAssertHandler(NULL);
#endif
	for (unsigned int i = 0; i < bitmaps.size(); i++)
		ctrl->SetItemBitmap(i, bitmaps[i]);
#if wxCHECK_VERSION(2,9,3)
	wxSetAssertHandler(oldAssertHandler);
#endif
	ctrl->SetStringSelection(value);
	wxBoxSizer* pane = NULL;
	if (width > 0) {
		pane = new wxBoxSizer(wxHORIZONTAL);
		pane->Add(ctrl);
		if (addSpacer)
			pane->Add(10, 10, 1, wxEXPAND);
		sizer->Add(pane, 1, wxEXPAND, 0);
	} else
		sizer->Add(ctrl, 1, wxEXPAND, 0);
	m_types.Add(propBITMAP_COMBO);
	m_controls.Add(ctrl);
	m_groupIds.Add(m_currGroupId);
	return pane;
}

wxSizer* wxPropDlg::AddChoiceProp(wxSizer* sizer, const wxString& label, const wxString& value,
		const wxArrayString& choices, int width, bool addSpacer, int id) {
	if (!sizer) { // only update value
		wxChoice* ctrl = (wxChoice*)m_controls[m_updateIndex];
		ctrl->SetStringSelection(value);
		m_updateIndex++;
		return NULL;
	}
	if (label.length())
		sizer->Add(new wxStaticText(propWindow, -1, label), 0, wxALIGN_CENTER_VERTICAL);
	// add choice control
	wxChoice* ctrl = AddChoiceProp(value, choices, id);
	wxBoxSizer* pane = NULL;
	if (width >= 0) {
		pane = new wxBoxSizer(wxHORIZONTAL);
		pane->Add(ctrl);
		if (addSpacer)
			pane->Add(10, 10, 1, wxEXPAND);
		sizer->Add(pane, 1, wxEXPAND, 0);
		if (width > 0)
			ctrl->SetMinSize(wxSize(width, -1));
	} else
		sizer->Add(ctrl, width == -2 ? 0 : 1, wxEXPAND, 0);
	return pane;
}

/**
 * Creates choice control.
 * @returns Choice control
 */
wxChoice* wxPropDlg::AddChoiceProp(const wxString& value, const wxArrayString& choices, int id) {
	wxChoice* ctrl = new wxChoice(propWindow, id, wxDefaultPosition, wxDefaultSize, choices);
	ctrl->SetStringSelection(value);
	m_types.Add(propCHOICE);
	m_controls.Add(ctrl);
	m_groupIds.Add(m_currGroupId);
	return ctrl;
}


/**
 * Creates label and choice control with item 'Custom'.
 * @returns subSizer if width > 0, null otherwise
 */
wxSizer* wxPropDlg::AddChoiceCustomProp(wxSizer* sizer, const wxString& label, const wxString& value,
		const wxArrayString& choices, int customItemIdx, int width, bool addSpacer) {
	wxSizer* s = AddChoiceProp(sizer, label, value, choices, width, addSpacer, CUSTOM_CHOICE_ID);
	GetLastControl()->SetName(wxString::Format(wxT("%d-%d"), GetLastControlIndex(), customItemIdx));
	return s;
}

/** Sets the last control as control to enter the custom value. */
void wxPropDlg::SetLastControlCustom(int choiceCtrlIdx, bool enable) {
	GetLastControl()->SetName(wxString::Format(wxT("customCtrlFor%d"), choiceCtrlIdx));
	GetLastControl()->Enable(enable);
}

void wxPropDlg::OnSelectCustomChoice(wxCommandEvent& evt) {
	wxString name = ((wxWindow*)evt.GetEventObject())->GetName();
	long idx = 0;
	long itemIdx = 0;
	if (name.BeforeLast(wxT('-')).ToLong(&idx) && name.AfterFirst(wxT('-')).ToLong(&itemIdx)) {
		wxWindow* ctrl = this->FindWindowByName(wxString::Format(wxT("customCtrlFor%d"), (int) idx));
		ctrl->Enable(evt.GetSelection() == itemIdx && ((wxWindow*)evt.GetEventObject())->IsEnabled());
		if (!ctrl->IsEnabled() && ctrl->GetClassInfo()->IsKindOf(wxCLASSINFO(wxChoice))) {
			((wxChoice*) ctrl)->SetSelection(0);
			((wxChoice*) ctrl)->SendSelectionChangedEvent(wxEVT_CHOICE);
		}
	}
}

//--- Grid property ---//
void wxPropDlg::AddGridProp(wxSizer* sizer, const wxArrayPtrVoid& data, const wxString& rowTitle, bool editable) {
	if (!sizer) { // only update value
		wxGrid* grid = (wxGrid*) m_controls[m_updateIndex];
		grid->ClearGrid();
		SetGridData(grid, data, rowTitle, editable);
		m_updateIndex++;
		return;
	}
	wxGrid* grid = new wxGrid(propWindow, -1, wxPoint(0, 0), wxSize(100, 100));
	grid->CreateGrid(0, 0);
	SetGridData(grid, data, rowTitle, editable);

	// todo: autosize RowLableSize
	int width = 50;
	grid->SetRowLabelSize(50);
	//grid->SetRowSize(0, 60 );
	//grid->SetColSize(0, 120);
	//grid->AutoSize();
	grid->Fit();
	wxArrayString* cols = (wxArrayString*) data.Item(0);
	for (int c = 0; c < (int) cols->GetCount(); c++)
		width += grid->GetColSize(c);
	grid->SetSize(width + 20, 100);

	sizer->Add(grid, 1, wxEXPAND);
	m_types.Add(propGRID);
	m_controls.Add(grid);
	m_groupIds.Add(m_currGroupId);
}

void wxPropDlg::SetGridData(wxGrid* grid, const wxArrayPtrVoid& data, const wxString& rowTitle, bool editable) {
	grid->EnableEditing(editable);

	wxArrayString* cols = (wxArrayString*) data.Item(0);
	int cnt = cols->GetCount() - grid->GetNumberCols();
	if (cnt > 0)
		grid->AppendCols(cnt);
	else if (cnt < 0)
		grid->DeleteCols(0, -cnt);
	grid->SetColLabelSize(grid->GetDefaultRowSize());
	for (int c = 0; c < (int) cols->GetCount(); c++)
		grid->SetColLabelValue(c, cols->Item(c));

	cnt = data.GetCount() - (editable ? 0 : 1) - grid->GetNumberRows();
	if (cnt > 0)
		grid->AppendRows(cnt);
	else if (cnt < 0)
		grid->DeleteRows(0, -cnt);
	for (int r = 1; r <= (int) grid->GetNumberRows(); r++) {
		grid->SetRowLabelValue(r - 1, rowTitle + wxString::Format(_T("%d"), r));
		if (r < (int) data.GetCount()) {
			wxArrayString* row = (wxArrayString*) data.Item(r);
			for (int c = 0; c < (int) cols->GetCount(); c++)
				grid->SetCellValue(r - 1, c, row->Item(c));
		}
	}
}

void wxPropDlg::OnCellLeftClick(wxGridEvent& event) {
	wxGrid* grid = (wxGrid*) event.GetEventObject();
	grid->ClearSelection();
	grid->SetGridCursor(event.GetRow(), event.GetCol());
	grid->EnableCellEditControl();
}

void wxPropDlg::OnCellChange(wxGridEvent& event) {
	wxGrid* grid = (wxGrid*) event.GetEventObject();
	if (event.GetRow() == grid->GetNumberRows() - 1)
		grid->AppendRows();
	wxString rowTitle = grid->GetRowLabelValue(0);
	rowTitle = rowTitle.Mid(0, rowTitle.length() - 1);
	grid->SetRowLabelValue(grid->GetNumberRows() - 1,
			rowTitle + wxString::Format(_T("%d"), grid->GetNumberRows()));
}

void wxPropDlg::OnCellRightClick(wxGridEvent& event) {
	wxGrid* grid = (wxGrid*) event.GetEventObject();
	if (grid->IsEditable()) {
		m_currObject = grid;
		m_currObjectItem = event.GetRow();
		wxMenu* menu = new wxMenu;
		menu->Append(ROW_DELETE_ID, _("&Delete"));
		menu->GetMenuItems()[0]->Enable(
				event.GetRow() < grid->GetNumberRows() - 1);
		grid->PopupMenu(menu, event.GetPosition());
	}
}

void wxPropDlg::OnRowDelete(wxCommandEvent& event) {
	wxGrid* grid = (wxGrid*) m_currObject;
	grid->DeleteRows(m_currObjectItem);
}

//--- Font property ---//
void wxPropDlg::AddFontProp(wxSizer* sizer, const wxString& label, wxFont font, wxString caption) {
	wxFontData data;
	data.SetInitialFont(font);
	AddFontProp(sizer, label, data, caption);
}

void wxPropDlg::AddFontProp(wxSizer* sizer, const wxString& label,
		wxFontData font, wxString caption) {
	if (!sizer) { // only update value
		delete (wxFontData*) m_controls[m_updateIndex];
		m_controls[m_updateIndex] = new wxFontData(font);
		m_updateIndex++;
		return;
	}
	if (label.length())
		sizer->Add(new wxStaticText(propWindow, -1, label), 0, wxALIGN_CENTER_VERTICAL);
	wxButton* button = new wxButton(propWindow, SELECT_FONT_BT_ID, caption);
	if (caption == _T("..."))
		button->SetSizeHints(24, 24, 24, 24);
	button->SetName(wxString::Format(_T("%d"), (int) m_controls.GetCount()));
	sizer->Add(button);
	m_types.Add(propFONT);
	m_controls.Add(new wxFontData(font));
	m_groupIds.Add(m_currGroupId);
}

void wxPropDlg::OnSelectFont(wxCommandEvent& event) {
	long index = -1;
	((wxButton*) event.GetEventObject())->GetName().ToLong(&index);
	wxFontData* data = (wxFontData*) m_controls[index];
#if wxCHECK_VERSION(2,8,0)
	wxFontDialog dialog(propWindow, *data);
#else
	wxFontDialog dialog(propWindow, data);
#endif
	if (dialog.ShowModal() == wxID_OK)
		*data = dialog.GetFontData();
}

void wxPropDlg::AddColourProp(wxSizer* sizer, const wxString& label, wxColour colour, int opacity, wxString caption) {
	if (!sizer) { // only update value
		((ColourPanel*) m_controls[m_updateIndex])->SetColour(colour);
		m_updateIndex++;
		return;
	}
	if (label.length())
		sizer->Add(new wxStaticText(propWindow, -1, label), 0, wxALIGN_CENTER_VERTICAL);
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	ColourPanel* panel = new ColourPanel(propWindow, -1);
	panel->SetSize(wxSize(20, 20));
	panel->SetMinSize(wxSize(20, 20));
	panel->SetColour(colour);
	sizer2->Add(panel, 1, wxALL, 2);
	wxButton* button = new wxButton(propWindow, SELECT_COLOR_BT_ID, caption);
	if (caption == _T("..."))
		button->SetSizeHints(24, 24, 24, 24);
	button->SetName(wxString::Format(wxT("PropColourBt%d"), (int) m_controls.GetCount()));
	sizer2->Add(button);
	m_types.Add(propCOLOUR);
	m_controls.Add(panel);
	m_groupIds.Add(m_currGroupId);
	if (opacity >= 0) {
		// add spinctrl
		wxSpinCtrl* ctrl = new wxSpinCtrl(propWindow, -1, wxEmptyString, wxDefaultPosition, wxSize(54, -1),
				wxSP_ARROW_KEYS, 0, 100);
		ctrl->SetValue(opacity);
		sizer2->Add(ctrl, 0, wxALL, 2);
		AddText(sizer2, wxT("%"));
		m_types.Add(propSPIN);
		m_controls.Add(ctrl);
		m_groupIds.Add(m_currGroupId);
	}
	sizer->Add(sizer2);
}

void wxPropDlg::OnSelectColour(wxCommandEvent& event) {
	long index = -1;
	((wxButton*) event.GetEventObject())->GetName().Mid(12).ToLong(&index);
	ColourPanel* panel = ((ColourPanel*) m_controls[index]);
	m_colourData.SetColour(panel->GetColour());
	m_colourData.SetChooseAlpha(true);
#ifdef __WXMSW__
	wxGenericColourDialog dialog(propWindow, &m_colourData);
#else
	wxColourDialog dialog(propWindow, &m_colourData);
#endif
	if (dialog.ShowModal() == wxID_OK) {
		panel->SetColour(dialog.GetColourData().GetColour());
		m_colourData = dialog.GetColourData();
	}
}

wxSizer* wxPropDlg::AddFileProp(wxSizer* sizer, const wxString& label, const wxString& value, int dlgStyle,
		wxString buttonLabel, wxString wildcard, int id) {
	if (!sizer) // only update value
	{
		wxTextCtrl* ctrl = (wxTextCtrl*) m_controls[m_updateIndex];
		ctrl->SetValue(value);
		m_updateIndex++;
		return NULL;
	}
	// add text control
	if (label.length())
		sizer->Add(new wxStaticText(propWindow, -1, label), 0, wxALIGN_CENTER_VERTICAL);
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxTextCtrl* ctrl = new wxTextCtrl(propWindow, id, value);
	sizer2->Add(ctrl, 1, wxEXPAND);
	wxButton* button = new wxButton(propWindow, SELECT_FILE_BT_ID, buttonLabel);
	int h = ctrl->GetSize().GetHeight() > 24 ? ctrl->GetSize().GetHeight() : 24;
	if (buttonLabel == _T("..."))
		button->SetSizeHints(h, h, h, h);
	button->SetName(wxString::Format(_T("SelectFileButton_%d"), (int) m_controls.GetCount()));
	button->SetClientData(new wxStringClientData(wxString::Format(_T("%d:%s"), dlgStyle, wildcard.c_str())));
	sizer2->Add(button);
	wxBitmapButton* delBt = new wxBitmapButton(propWindow, CLEAR_TEXT_BT_ID, wxBITMAP_FROM_MEMORY(delete));
	delBt->SetName(wxString::Format(_T("ClearTextButton_%d"), (int) m_controls.GetCount()));
	sizer2->Add(delBt);
	sizer->Add(sizer2, 1, wxEXPAND);
	m_types.Add(propTEXT);
	m_controls.Add(ctrl);
	m_groupIds.Add(m_currGroupId);
	return sizer2;
}

void wxPropDlg::OnSelectFile(wxCommandEvent& event) {
	long index = -1;
	((wxButton*) event.GetEventObject())->GetName().Mid(17).ToLong(&index);
	wxString clientData = ((wxStringClientData*) ((wxButton*) event.GetEventObject())->GetClientData())->GetData();
	long dlgStyle = wxFD_DEFAULT_STYLE;
	clientData.BeforeFirst(wxT(':')).ToLong(&dlgStyle);
	wxString wildcard = clientData.AfterFirst(wxT(':'));
	wxTextCtrl* ctrl = ((wxTextCtrl*) m_controls[index]);
	wxFileDialog dialog(propWindow, _("Choose a file"), ctrl->GetValue(), wxT(""), wildcard, dlgStyle);
	dialog.SetPath(ctrl->GetValue());
	if (dialog.ShowModal() == wxID_OK)
		ctrl->SetValue(dialog.GetPath());
}

void wxPropDlg::OnClearText(wxCommandEvent& event) {
	long index = -1;
	((wxBitmapButton*) event.GetEventObject())->GetName().Mid(16).ToLong(&index);
	((wxTextCtrl*) m_controls[index])->SetValue(wxT(""));
}

wxSizer* wxPropDlg::AddDirectoryProp(wxSizer* sizer, const wxString& label, const wxString& value, wxString buttonLabel) {
	if (!sizer) { // only update value
		wxTextCtrl* ctrl = (wxTextCtrl*) m_controls[m_updateIndex];
		ctrl->SetValue(value);
		m_updateIndex++;
		return NULL;
	}
	// add text control
	if (label.length())
		sizer->Add(new wxStaticText(propWindow, -1, label), 0, wxALIGN_CENTER_VERTICAL);
	wxBoxSizer* sizer2 = new wxBoxSizer(wxHORIZONTAL);
	wxTextCtrl* ctrl = new wxTextCtrl(propWindow, -1, value);
	sizer2->Add(ctrl, 1, wxEXPAND, 0);
	wxButton* button = new wxButton(propWindow, SELECT_DIR_BT_ID, buttonLabel);
	int h = ctrl->GetSize().GetHeight() > 24 ? ctrl->GetSize().GetHeight() : 24;
	if (buttonLabel == _T("..."))
		button->SetSizeHints(h, h, h, h);
	button->SetName(wxString::Format(wxT("SelectDirButton_%d"), (int) m_controls.GetCount()));
	sizer2->Add(button);
	sizer->Add(sizer2, 1, wxEXPAND);
	m_types.Add(propTEXT);
	m_controls.Add(ctrl);
	m_groupIds.Add(m_currGroupId);
	return sizer2;
}

void wxPropDlg::OnSelectDir(wxCommandEvent& event) {
	long index = -1;
	((wxButton*) event.GetEventObject())->GetName().AfterLast(wxT('_')).ToLong(&index);
	wxTextCtrl* ctrl = ((wxTextCtrl*) m_controls[index]);
	wxDirDialog dialog(propWindow, _("Choose a directory"), ctrl->GetValue(), wxDD_DEFAULT_STYLE | wxDD_NEW_DIR_BUTTON);
	dialog.SetPath(ctrl->GetValue());
	if (dialog.ShowModal() == wxID_OK)
		ctrl->SetValue(dialog.GetPath());
}

void wxPropDlg::AddButton(wxSizer* sizer, const wxString& label, const long id, const wxString& buttonLabel) {
	if (!sizer) // skip
		return;
	// add text control
	if (label.length())
		sizer->Add(new wxStaticText(propWindow, -1, label), 0, wxALIGN_CENTER_VERTICAL);
	wxButton* bt = new wxButton(propWindow, id, buttonLabel);
	if (buttonLabel == wxT("...")) {
		int h = bt->GetSize().GetHeight() > 24 ? bt->GetSize().GetHeight() : 24;
		bt->SetSizeHints(h, h, h, h);
	}
	sizer->Add(bt);
}

void wxPropDlg::AddBitmapToggleButton(wxSizer* sizer, bool value, const long id, const wxBitmap& bitmap,
		const wxSize& size) {
	if (!sizer) { // only update value
		wxBitmapToggleButton* ctrl = (wxBitmapToggleButton*) m_controls[m_updateIndex];
		ctrl->SetValue(value);
		m_updateIndex++;
		return;
	}
	wxBitmapToggleButton* ctrl = new wxBitmapToggleButton(propWindow, id, bitmap, wxDefaultPosition, size);
	ctrl->SetValue(value);
	sizer->Add(ctrl);
	m_types.Add(propTOGGLE_BUTTON);
	m_controls.Add(ctrl);
	m_groupIds.Add(m_currGroupId);
}

int wxPropDlg::AddIcon(wxSizer* sizer, const wxString& title, const wxString& tooltip, wxArtID artId) {
	if (!sizer)
		return -1;
	m_tooltipTitle.Add(title);
	m_tooltipText.Add(tooltip);
	m_tooltipIcon.Add(artId == wxART_WARNING ? wxICON_WARNING : wxICON_INFORMATION);
	wxStaticBitmap* icon = new wxStaticBitmap(propWindow, wxID_ANY, wxArtProvider::GetBitmap(artId, wxART_BUTTON));
	icon->SetName(wxString::Format(_T("%d"), m_tooltipTitle.GetCount() - 1));
	sizer->Add(icon, 0, wxALIGN_CENTER_VERTICAL);
	m_icons.push_back(icon);
	icon->Connect(wxEVT_ENTER_WINDOW, wxMouseEventHandler(wxPropDlg::OnShowTooltip), NULL, this);
	return m_tooltipTitle.GetCount() - 1;
}

void wxPropDlg::UpdateIcon(int index, const wxString& title, const wxString& tooltip, wxArtID artId) {
	if (title.length())
		m_tooltipTitle[index] = title;
	if (tooltip.length())
		m_tooltipText[index] = tooltip;
	m_tooltipIcon[index] = artId == wxART_WARNING ? wxICON_WARNING : wxICON_INFORMATION;
	m_icons[index]->SetBitmap(wxArtProvider::GetBitmap(artId, wxART_BUTTON));
}

void wxPropDlg::OnShowTooltip(wxMouseEvent& event) {
	long idx;
	if (!((wxWindow*)event.GetEventObject())->GetName().ToLong(&idx))
		return;
#ifdef __WXMSW__2
	HWND hwndToolTips = CreateWindowExW(WS_EX_TOPMOST,
			TOOLTIPS_CLASSW,0,WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
			CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT, 0,0,wxGetInstance(),0);
	if (hwndToolTips) {
		TOOLINFOW ti = {};
		ti.cbSize = sizeof(TOOLINFOW);
		ti.uFlags = TTF_ABSOLUTE | TTF_IDISHWND;
		ti.hwnd   = hwndToolTips;
		ti.hinst  = NULL;
		ti.uId    = (UINT)hwndToolTips;
		ti.lpszText = (LPWSTR) m_tooltipText[idx].wc_str();
		SendMessageW(hwndToolTips, TTM_ADDTOOLW, 0, (LPARAM)&ti);
		SendMessageW(hwndToolTips, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&ti);
		SendMessageW(hwndToolTips, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(event.GetX(), event.GetY()));
	}
#else
	wxRichToolTip tip(m_tooltipTitle[idx], m_tooltipText[idx]);
	tip.SetIcon(m_tooltipIcon[idx]);
	tip.SetTipKind(wxTipKind_Bottom);
	tip.SetTimeout(3000, 0);
	tip.ShowFor((wxWindow*) event.GetEventObject());
#endif
}

////////////////////////////// Group /////////////////////////////////////////

wxSizer* wxPropDlg::BeginGroup(wxSizer* sizer, wxString title, wxString checkTitle, bool value, bool readonly,
		bool vertical, int proportion) {
	if (!sizer) { // only update value
		if (checkTitle.length())
			AddCheckProp(NULL, wxEmptyString, value, readonly, GROUP_CHECK_ID);
		return NULL;
	}
	m_currGroupId++;
	if (title.size() > 0) {
		wxStaticBox* sb = new wxStaticBox(propWindow, -1, title);
		wxStaticBoxSizer* sbsizer = new wxStaticBoxSizer(sb, vertical ? wxVERTICAL : wxHORIZONTAL);
		sizer->Add(sbsizer, proportion, wxEXPAND);
		if (checkTitle.length()) {
			wxFlexGridSizer* groupSizer = new wxFlexGridSizer(2, 2, 8, 4);
			groupSizer->AddGrowableCol(1);
			sbsizer->Add(groupSizer, 0, wxEXPAND | wxALL, 4);
			AddCheckProp(groupSizer, wxEmptyString, value, readonly, GROUP_CHECK_ID);
			m_currGroupChecker = m_controls[m_controls.GetCount() - 1];
			AddText(groupSizer, checkTitle);
			AddText(groupSizer, wxEmptyString);
			return groupSizer;
		}
		m_currGroupChecker = NULL;
		return sbsizer;
	} else {
		wxBoxSizer* pane = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(pane, 1, wxEXPAND, 0);
		return pane;
	}
}

void wxPropDlg::BeginCheckGroup(wxSizer* sizer, wxString label, bool value, bool readonly) {
	if (!sizer) { // only update value
		AddCheckProp(NULL, label, value, readonly, GROUP_CHECK_ID);
		return;
	}
	m_currGroupId++;
	AddCheckProp(sizer, label, value, readonly, GROUP_CHECK_ID);
	m_currGroupChecker = m_controls[m_controls.GetCount() - 1];
}

void wxPropDlg::EndGroup() {
	if (m_currGroupChecker) {
		wxCommandEvent evt;
		evt.SetEventObject((wxObject*) m_currGroupChecker);
		OnGroupCheck(evt);
	}
	m_currGroupChecker = NULL;
	m_currGroupId++;
}

void wxPropDlg::OnGroupCheck(wxCommandEvent& event) {
	long index = -1;
	wxCheckBox* ctrl = (wxCheckBox*) event.GetEventObject();
	ctrl->GetName().ToLong(&index);
	int groupId = m_groupIds[index];
	for (int i = 0; i < (int) m_groupIds.GetCount(); i++)
		if (i != index && m_groupIds[i] == groupId && m_types[i] != propFONT) {
			((wxWindow*) m_controls[i])->Enable(ctrl->GetValue());
			if (m_types[i] == propCOLOUR) {
				wxWindow* button = FindWindowByName(wxString::Format(wxT("PropColourBt%d"), i));
				if (button != NULL)
					button->Enable(ctrl->GetValue());
			}
		}
}

wxControl* wxPropDlg::GetControl(int index) {
	return (wxControl*) m_controls[index];
}

wxControl* wxPropDlg::GetLastControl() {
	return (wxControl*) m_controls[GetLastControlIndex()];
}

int wxPropDlg::GetLastControlIndex() {
	return m_updateIndex > 0 ? m_updateIndex - 1 : m_controls.GetCount() -1;
}

///////////////////////// Get Values /////////////////////////////////////////

wxString wxPropDlg::GetString(int index) {
	switch (m_types[index]) {
	case propTEXT:
		return ((wxTextCtrl*) m_controls[index])->GetValue();
	case propSTAT_TEXT:
		return ((wxStaticText*) m_controls[index])->GetLabel();
	case propCOMBO:
		return ((wxComboBox*) m_controls[index])->GetValue();
	case propCHOICE:
		return ((wxChoice*) m_controls[index])->GetStringSelection();
	}
	return wxEmptyString;
}

int wxPropDlg::GetInt(int index) {
	switch (m_types[index]) {
	case propSPIN:
		return ((wxSpinCtrl*) m_controls[index])->GetValue();
	case propCOMBO:
		return ((wxComboBox*) m_controls[index])->GetSelection();
	case propBITMAP_COMBO:
		return ((wxBitmapComboBox*) m_controls[index])->GetSelection();
	case propCHOICE:
		return ((wxChoice*) m_controls[index])->GetSelection();
	case propRADIO_GROUP: {
		wxArrayPtrVoid* radioControls = (wxArrayPtrVoid*) m_controls[index];
		for (int i = 0; i < (int) radioControls->GetCount(); i++) {
			wxRadioButton* ctrl = (wxRadioButton*) (*radioControls)[i];
			if (ctrl->GetValue())
				return i;
		}
		return -1;
	}
	}
	return -1;
}

bool wxPropDlg::GetBool(int index) {
	switch (m_types[index]) {
	case propCHECK:
		return ((wxCheckBox*) m_controls[index])->GetValue();
	case propRADIO:
		return ((wxRadioButton*) m_controls[index])->GetValue();
	case propTOGGLE_BUTTON:
		return ((wxBitmapToggleButton*) m_controls[index])->GetValue();
	}
	return true;
}

double wxPropDlg::GetDouble(int index) {
	switch (m_types[index]) {
	case propSPIN_DOUBLE:
#if wxCHECK_VERSION(2,9,3)
		return ((wxSpinCtrlDouble*) m_controls[index])->GetValue();
#else
		{
			double val;
			return ((wxTextCtrl*) m_controls[index])->GetValue().ToDouble(&val) ? val : -1;
		}
#endif
	}
	return -1;
}

void* wxPropDlg::GetClientData(int index) {
	switch (m_types[index]) {
	case propCOMBO: {
		wxComboBox* ctrl = (wxComboBox*) m_controls[index];
		if (ctrl->GetSelection() >= 0)
			return ctrl->GetClientData(ctrl->GetSelection());
		break;
	}
	case propCHOICE: {
		wxChoice* ctrl = (wxChoice*) m_controls[index];
		if (ctrl->GetSelection() >= 0)
			return ctrl->GetClientData(ctrl->GetSelection());
		break;
	}
	default:
		break;
	}
	return NULL;
}

wxArrayPtrVoid wxPropDlg::GetGrid(int index) {
	wxArrayPtrVoid data;
	if (m_types[index] != propGRID)
		return data;
	wxGrid* grid = (wxGrid*) m_controls[index];
	grid->SaveEditControlValue();
	int count = grid->GetNumberRows() - (grid->IsEditable() ? 1 : 0);
	for (int r = 0; r < count; r++) {
		wxArrayString* row = new wxArrayString;
		for (int c = 0; c < (int) grid->GetNumberCols(); c++)
			row->Add(grid->GetCellValue(r, c));
		data.Add(row);
	}
	return data;
}

wxFontData wxPropDlg::GetFont(int index) {
	if (m_types[index] != propFONT)
		return wxFontData();
	wxFontData* data = (wxFontData*) m_controls[index];
	if (!data->GetChosenFont().Ok())
		data->SetChosenFont(data->GetInitialFont());
	return *data;
}

wxColour wxPropDlg::GetColour(int index) {
	if (m_types[index] != propCOLOUR)
		return wxColour();
	return ((ColourPanel*) m_controls[index])->GetColour();
}

