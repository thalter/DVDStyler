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
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
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
	wxSVGCtrl* m_svgCtrl;
	wxSplitterWindow* m_splitterWindow;
	//*)
	
	int GetSelection() const;

protected:
	//(*Identifiers(ButtonAddParamDlg)
	static const long ID_LISTBOX1;
	static const long ID_PANEL1;
	static const long ID_SVG_CTRL;
	static const long ID_SPLITTERWINDOW1;
	//*)
	
	wxNodeList m_nodeList;
	wxSVGDocument* m_doc;

private:
	//(*Handlers(ButtonAddParamDlg)
	void OnSelectSvgElement(wxCommandEvent& event);
	//*)
	DECLARE_EVENT_TABLE()
};

#endif
