/////////////////////////////////////////////////////////////////////////////
// Name:        ButtonEditDlg.cpp
// Purpose:     Dialog to create new button from SVG file
// Author:      Alex Thuering
// Created:     5.03.2018
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef BUTTONEDITDLG_H
#define BUTTONEDITDLG_H

//(*Headers(ButtonEditDlg)
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wxSVG/svgctrl.h>
//*)

#include "MenuObjectDef.h"
#include <wx/richtooltip.h>

class ButtonEditDlg: public wxDialog {
public:
	ButtonEditDlg(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
	virtual ~ButtonEditDlg();
	
	bool LoadSVG(const wxString& filename);
	bool LoadButton(const wxString& filename);
	wxString GetDestFileName() const { return m_destFileName; }
	
	//(*Declarations(ButtonEditDlg)
	wxComboBox* m_defHeightElem;
	wxComboBox* m_defWidthElem;
	wxComboBox* m_minHeightElem;
	wxComboBox* m_minWidthElem;
	wxGrid* m_grid;
	wxRadioBox* m_buttonStateCtrl;
	wxSVGCtrl* m_svgCtrl;
	wxSpinCtrl* m_defHeight;
	wxSpinCtrl* m_defHeightInc;
	wxSpinCtrl* m_defHeightPercent;
	wxSpinCtrl* m_defWidth;
	wxSpinCtrl* m_defWidthInc;
	wxSpinCtrl* m_defWidthPercent;
	wxSpinCtrl* m_minHeight;
	wxSpinCtrl* m_minHeightInc;
	wxSpinCtrl* m_minHeightPercent;
	wxSpinCtrl* m_minWidth;
	wxSpinCtrl* m_minWidthInc;
	wxSpinCtrl* m_minWidthPercent;
	wxStaticBitmap* m_infoIcon;
	wxStaticText* m_dirCtrl;
	wxTextCtrl* m_filenameCtrl;
	wxTextCtrl* m_titleCtrl;
	//*)

protected:
	//(*Identifiers(ButtonEditDlg)
	static const long ID_SVG_CTRL;
	static const long ID_BUTTON_STATE;
	static const long ID_STATICTEXT1;
	static const long ID_TEXTCTRL1;
	static const long ID_TEXTCTRL2;
	static const long ID_SPINCTRL1;
	static const long ID_COMBOBOX1;
	static const long ID_SPINCTRL2;
	static const long ID_SPINCTRL3;
	static const long ID_SPINCTRL4;
	static const long ID_COMBOBOX2;
	static const long ID_SPINCTRL5;
	static const long ID_SPINCTRL6;
	static const long ID_SPINCTRL7;
	static const long ID_COMBOBOX3;
	static const long ID_SPINCTRL8;
	static const long ID_SPINCTRL9;
	static const long ID_SPINCTRL10;
	static const long ID_COMBOBOX4;
	static const long ID_SPINCTRL11;
	static const long ID_SPINCTRL12;
	static const long ID_STATICBITMAP1;
	static const long ID_PARAMS_GRID;
	//*)

private:
	//(*Handlers(ButtonEditDlg)
	void OnButtonState(wxCommandEvent& event);
	void OnShowTooltip(wxMouseEvent& event);
	//*)
	
	MenuObjectDef m_objectDef;
	wxString m_destFileName;
	void OnOkBt(wxCommandEvent& event);
	void SetCellEditors(int row);
	void OnGridRightClick(wxMouseEvent& event);
	void OnCellChanged(wxGridEvent& event);
	void OnSvgLeftClick(wxMouseEvent & event);
	void OnPopupClick(wxCommandEvent& event);
	void InitGrid();

	DECLARE_EVENT_TABLE()
};

#endif
