/////////////////////////////////////////////////////////////////////////////
// Name:        ButtonAddParamDlg.h
// Purpose:     Dialog to add new parameter to button
// Author:      Alex Thuering
// Created:     11.10.2018
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "ButtonAddParamDlg.h"
#include <wx/button.h>
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/log.h>
#include <wxSVG/SVGSVGElement.h>
#include <wxSVG/SVGGElement.h>


//*)

//(*IdInit(ButtonAddParamDlg)
const long ButtonAddParamDlg::ID_LISTBOX1 = wxNewId();
const long ButtonAddParamDlg::ID_RADIOBUTTON1 = wxNewId();
const long ButtonAddParamDlg::ID_TEXTCTRL1 = wxNewId();
const long ButtonAddParamDlg::ID_RADIOBUTTON2 = wxNewId();
const long ButtonAddParamDlg::ID_TEXTCTRL2 = wxNewId();
const long ButtonAddParamDlg::ID_RADIOBUTTON3 = wxNewId();
const long ButtonAddParamDlg::ID_TEXTCTRL3 = wxNewId();
const long ButtonAddParamDlg::ID_PANEL1 = wxNewId();
const long ButtonAddParamDlg::ID_SVG_CTRL = wxNewId();
const long ButtonAddParamDlg::ID_SPLITTERWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ButtonAddParamDlg,wxDialog)
	//(*EventTable(ButtonAddParamDlg)
	//*)
END_EVENT_TABLE()

ButtonAddParamDlg::ButtonAddParamDlg(wxWindow* parent, wxSVGSVGElement* rootElem,
		wxNodeList nodeList, vector<wxString> elements) {
	//(*Initialize(ButtonAddParamDlg)
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* mainSizer;
	wxFlexGridSizer* FlexGridSizer1;
	wxPanel* panel1;
	wxStaticText* StaticText1;
	wxStaticText* attributeLabel;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, wxID_ANY, _("Add parameter"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
	SetClientSize(wxSize(750,400));
	mainSizer = new wxBoxSizer(wxVERTICAL);
	m_splitterWindow = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	m_splitterWindow->SetMinimumPaneSize(10);
	m_splitterWindow->SetSashGravity(0.5);
	panel1 = new wxPanel(m_splitterWindow, ID_PANEL1, wxPoint(6,5), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	StaticText1 = new wxStaticText(panel1, wxID_ANY, _("Please select an SVG element\nfor new parameter:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer1->Add(StaticText1, 0, wxBOTTOM|wxEXPAND, 5);
	m_elementsListBox = new wxListBox(panel1, ID_LISTBOX1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_LISTBOX1"));
	BoxSizer1->Add(m_elementsListBox, 1, wxEXPAND, 5);
	attributeLabel = new wxStaticText(panel1, wxID_ANY, _("Attribute:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer1->Add(attributeLabel, 0, wxTOP|wxALIGN_LEFT, 5);
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 2, 2);
	m_fillRadio = new wxRadioButton(panel1, ID_RADIOBUTTON1, _("fill"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
	FlexGridSizer1->Add(m_fillRadio, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_fillCtrl = new wxTextCtrl(panel1, ID_TEXTCTRL1, _("Text"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	m_fillCtrl->Disable();
	FlexGridSizer1->Add(m_fillCtrl, 1, wxLEFT|wxEXPAND, 5);
	m_strokeRadio = new wxRadioButton(panel1, ID_RADIOBUTTON2, _("stroke"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
	FlexGridSizer1->Add(m_strokeRadio, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_strokeCtrl = new wxTextCtrl(panel1, ID_TEXTCTRL2, _("Text"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	m_strokeCtrl->Disable();
	FlexGridSizer1->Add(m_strokeCtrl, 1, wxLEFT|wxEXPAND, 5);
	m_strokeWidthRadio = new wxRadioButton(panel1, ID_RADIOBUTTON3, _("stroke-width"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON3"));
	FlexGridSizer1->Add(m_strokeWidthRadio, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_strokeWidthCtrl = new wxTextCtrl(panel1, ID_TEXTCTRL3, _("Text"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	m_strokeWidthCtrl->Disable();
	FlexGridSizer1->Add(m_strokeWidthCtrl, 1, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(FlexGridSizer1, 0, wxEXPAND, 5);
	panel1->SetSizer(BoxSizer1);
	BoxSizer1->Fit(panel1);
	BoxSizer1->SetSizeHints(panel1);
	m_svgCtrl = new wxSVGCtrl(m_splitterWindow,ID_SVG_CTRL,wxDefaultPosition,wxDefaultSize,0,_T("ID_SVG_CTRL"));
	m_splitterWindow->SplitVertically(panel1, m_svgCtrl);
	mainSizer->Add(m_splitterWindow, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	mainSizer->Add(StdDialogButtonSizer1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(mainSizer);
	SetSizer(mainSizer);
	Layout();

	Connect(ID_LISTBOX1,wxEVT_COMMAND_LISTBOX_SELECTED,(wxObjectEventFunction)&ButtonAddParamDlg::OnSelectSvgElement);
	//*)
	
	m_nodeList = nodeList;
	CreateSvgDoc(rootElem->GetWidth().GetBaseVal(), rootElem->GetHeight().GetBaseVal(), rootElem->GetViewBox());
	
	wxArrayString s;
	s.assign(elements.begin(), elements.end());
	m_elementsListBox->Set(s);
	m_elementsListBox->SetSelection(elements.size() - 1);
	wxCommandEvent evt;
	OnSelectSvgElement(evt);
	
	m_splitterWindow->SetSashPosition(100);
}

ButtonAddParamDlg::~ButtonAddParamDlg() {
	//(*Destroy(ButtonAddParamDlg)
	//*)
	delete m_doc;
}

int ButtonAddParamDlg::GetSelection() const {
	return m_elementsListBox->GetSelection();
}

wxString ButtonAddParamDlg::GetAttribute() const {
	if (m_fillRadio->GetValue())
		return "fill";
	if (m_strokeRadio->GetValue())
		return "stroke";
	return "stroke-width";
}

wxString ButtonAddParamDlg::GetType() const {
	if (m_fillRadio->GetValue())
		return "colour";
	if (m_strokeRadio->GetValue())
		return "colour";
	return "integer";
}

wxString ButtonAddParamDlg::GetTitle() const {
	if (m_fillRadio->GetValue())
		return "Fill colour";
	if (m_strokeRadio->GetValue())
		return "Stroke colour";
	return "Stroke width";
}

bool ButtonAddParamDlg::IsChangeable() const {
	return m_fillRadio->GetValue() || m_strokeRadio->GetValue();
}

wxSVGSVGElement* ButtonAddParamDlg::CreateSvgDoc(double width, double height, const wxSVGRect& viewBox) {
	m_doc = new wxSVGDocument;
	m_doc->SetRootElement(new wxSVGSVGElement());
	m_doc->GetRootElement()->SetWidth(width);
	m_doc->GetRootElement()->SetHeight(height);
	m_doc->GetRootElement()->SetViewBox(viewBox);
	m_svgCtrl->SetSVG(m_doc);
	return m_doc->GetRootElement();
}

void ButtonAddParamDlg::OnSelectSvgElement(wxCommandEvent& event) {
	wxSVGElement* selectedElem = (wxSVGElement*) m_nodeList[GetSelection() + 1];
	// clear
	double width = m_doc->GetRootElement()->GetWidth().GetBaseVal();
	double height = m_doc->GetRootElement()->GetHeight().GetBaseVal();
	wxSVGRect viewBox = m_doc->GetRootElement()->GetViewBox();
	delete m_doc;
	wxSVGSVGElement* root = CreateSvgDoc(width, height, viewBox);
	// clione defs
	wxSVGElement* defs = (wxSVGElement*) m_nodeList[0]->GetFirstChild();
	if (defs != NULL && defs->GetDtd() == wxSVG_DEFS_ELEMENT) {
		root->AppendChild(defs->CloneNode());
	}
	// find out transforms, fill and stroke
	vector<wxSVGElement*> parentList;
	wxSVGElement* parent = (wxSVGElement*) selectedElem->GetParent();
	while (parent != NULL) {
		parentList.insert(parentList.begin(), parent);
		parent = (wxSVGElement*) parent->GetParent();
	}
	wxSVGTransformList tList;
	wxSVGPaint fill;
	wxSVGPaint stroke;
	double strokeWidth = 0;
	for (wxSVGElement* parentElem : parentList) {
		wxSVGTransformable* transformable = wxSVGTransformable::GetSVGTransformable(*parentElem);
		if (transformable != NULL && transformable->GetTransform().GetBaseVal().size() > 0) {
			tList.Add(transformable->GetTransform().GetBaseVal()[0]);
		}
		wxSVGStylable* stylable = wxSVGStylable::GetSVGStylable(*parentElem);
		if (stylable != NULL) {
			if (stylable->GetFill().Ok()) {
				fill = stylable->GetFill();
			}
			if (stylable->GetStroke().Ok()) {
				stroke = stylable->GetStroke();
				strokeWidth = stylable->GetStrokeWidth();
			}
		}
	}
	// create g-element and clone selected
	wxSVGGElement* gElem = new wxSVGGElement();
	gElem->SetTransform(tList);
	gElem->SetFill(fill);
	gElem->SetStroke(stroke);
	gElem->SetStrokeWidth(strokeWidth);
	gElem->AddChild(selectedElem->CloneNode());
	root->AppendChild(gElem);
	m_svgCtrl->Refresh();
	
	// display fill and stroke
	wxSVGStylable* stylable = wxSVGStylable::GetSVGStylable(*selectedElem);
	if (stylable != NULL) {
		if (stylable->GetFill().Ok()) {
			fill = stylable->GetFill();
		}
		if (stylable->GetStroke().Ok()) {
			stroke = stylable->GetStroke();
			strokeWidth = stylable->GetStrokeWidth();
		}
	}
	m_fillCtrl->SetValue(fill.GetCSSText());
	m_strokeCtrl->SetValue(stroke.GetCSSText());
	m_strokeWidthCtrl->SetValue(wxString::Format("%f", strokeWidth));
}
