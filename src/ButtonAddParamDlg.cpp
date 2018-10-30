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

//*)

//(*IdInit(ButtonAddParamDlg)
const long ButtonAddParamDlg::ID_LISTBOX1 = wxNewId();
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
	wxPanel* panel1;
	wxStaticText* StaticText1;
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
	m_doc = new wxSVGDocument;
	m_doc->SetRootElement(new wxSVGSVGElement());
	m_doc->GetRootElement()->SetWidth(rootElem->GetWidth());
	m_doc->GetRootElement()->SetHeight(rootElem->GetHeight());
	m_doc->GetRootElement()->SetViewBox(rootElem->GetViewBox());
	m_svgCtrl->SetSVG(m_doc);
	
	m_elementsListBox->Set(elements);
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

void ButtonAddParamDlg::OnSelectSvgElement(wxCommandEvent& event) {
	if (m_doc->GetRootElement()->GetFirstChild()) {
		wxSvgXmlNode* child = m_doc->GetRootElement()->GetFirstChild();
		m_doc->GetRootElement()->RemoveChild(child);
		delete child;
	}
	m_doc->GetRootElement()->AppendChild(m_nodeList[GetSelection() + 1]->CloneNode());
	m_svgCtrl->Refresh();
}
