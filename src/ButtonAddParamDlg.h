/////////////////////////////////////////////////////////////////////////////
// Name:        ButtonAddParamDlg.h
// Purpose:     Dialog to add new parameter to button
// Author:      Alex Thuering
// Created:     11.10.2018
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef BUTTONADDPARAMDLG_H
#define BUTTONADDPARAMDLG_H

//(*Headers(ButtonAddParamDlg)
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wxSVG/svgctrl.h>
//*)
#include <wxSVG/NodeList.h>
#include <vector>
using namespace std;


class ButtonAddParamDlg: public wxDialog {
public:
	ButtonAddParamDlg(wxWindow* parent, wxSVGSVGElement* rootElem, wxNodeList nodeList, vector<wxString> elements);
	virtual ~ButtonAddParamDlg();
	//(*Declarations(ButtonAddParamDlg)
	wxListBox* m_elementsListBox;
	wxRadioButton* m_fillRadio;
	wxRadioButton* m_strokeRadio;
	wxRadioButton* m_strokeWidthRadio;
	wxSVGCtrl* m_svgCtrl;
	wxSplitterWindow* m_splitterWindow;
	wxTextCtrl* m_fillCtrl;
	wxTextCtrl* m_strokeCtrl;
	wxTextCtrl* m_strokeWidthCtrl;
	//*)
	
	int GetSelection() const;
	wxString GetAttribute() const;
	wxString GetType() const;
	wxString GetTitle() const;
	bool IsChangeable() const;

protected:
	//(*Identifiers(ButtonAddParamDlg)
	static const long ID_LISTBOX1;
	static const long ID_RADIOBUTTON1;
	static const long ID_TEXTCTRL1;
	static const long ID_RADIOBUTTON2;
	static const long ID_TEXTCTRL2;
	static const long ID_RADIOBUTTON3;
	static const long ID_TEXTCTRL3;
	static const long ID_PANEL1;
	static const long ID_SVG_CTRL;
	static const long ID_SPLITTERWINDOW1;
	//*)
	
	wxNodeList m_nodeList;
	wxSVGDocument* m_doc;
	wxSVGSVGElement* CreateSvgDoc(double width, double height, const wxSVGRect& viewBox);

private:
	//(*Handlers(ButtonAddParamDlg)
	void OnSelectSvgElement(wxCommandEvent& event);
	//*)
	DECLARE_EVENT_TABLE()
};

#endif
