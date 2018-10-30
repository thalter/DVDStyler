/////////////////////////////////////////////////////////////////////////////
// Name:        AnimationDlg.h
// Author:      Alex Thuering
// Created:     08.01.2016
// RCS-ID:      $Id: AnimationDlg.h,v 1.2 2016/05/16 21:15:48 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef ANIMATIONDLG_H
#define ANIMATIONDLG_H

//(*Headers(AnimationDlg)
#include <wx/bmpbuttn.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/sizer.h>
//*)
#include <wxSVG/SVGAnimateElement.h>
#include <vector>
using namespace std;
class MenuObject;

/**
 * Animation dialog to create and edit animations of menu objects
 */
class AnimationDlg: public wxDialog {
public:
	AnimationDlg(wxWindow* parent, MenuObject* menuObject);
	virtual ~AnimationDlg();

private:
	MenuObject* m_object;
	vector<wxSVGAnimateElement*> m_animations;
	void SetCellEditors(int row);
	void OnOkBt(wxCommandEvent& event);
		
	//(*Declarations(AnimationDlg)
	wxBitmapButton* m_addBt;
	wxBitmapButton* m_delBt;
	wxGrid* m_grid;
	//*)

	//(*Identifiers(AnimationDlg)
	static const long ID_GRID1;
	static const long ID_ADD_BT;
	static const long ID_DEL_BT;
	//*)

	//(*Handlers(AnimationDlg)
	void OnAddBt(wxCommandEvent& event);
	void OnDelBtClick(wxCommandEvent& event);
	//*)

	DECLARE_EVENT_TABLE()
};

#endif
