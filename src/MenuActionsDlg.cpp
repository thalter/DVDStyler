/////////////////////////////////////////////////////////////////////////////
// Name:        MenuActionsDlg.cpp
// Author:      Alex Thuering
// Created:     13.01.2012
// RCS-ID:      $Id: MenuActionsDlg.cpp,v 1.6 2016/01/06 21:10:04 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////
#include "MenuActionsDlg.h"
#include "MenuPropDlg.h"
#include "Utils.h"
#include <wxVillaLib/utils.h>
#include "rc/add.png.h"
#include "rc/remove.png.h"

//(*InternalHeaders(MenuActionsDlg)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

class ActionsTable: public wxGridTableBase {
public:
	ActionsTable(vector<DVDAction*>& actions, DVD* dvd, int tsi): wxGridTableBase(), m_actions(actions), m_dvd(dvd), m_tsi(tsi) { }

	int GetNumberRows() {
		return m_actions.size();
	}

	int GetNumberCols() {
		return 2;
	}

	bool IsEmptyCell(int row, int col) {
		return false;
	}

	wxString GetValue(int row, int col) {
		DVDAction* action = m_actions[row];
		switch (col) {
		case 0:
			return action->GetId();
		case 1:
			return action->AsString(m_dvd);
		default:
			break;
		}
		return wxT("");
	}

	void SetValue(int row, int col, const wxString& value) {
		DVDAction* action = m_actions[row];
		switch (col) {
		case 0:
			action->SetId(value);
			break;
		case 1:
			action->SetCustom(value);
			break;
		default:
			break;
		}
	}

	bool AppendRows(size_t numRows = 1) {
		int idx = 0;
		wxString id;
		while (true) {
			idx++;
			id = wxString::Format(wxT("action%02d"), idx);
			bool found = false;
			for (vector<DVDAction*>::const_iterator it = m_actions.begin(); it != m_actions.end(); it++) {
				if ((*it)->GetId() == id) {
					found = true;
					break;
				}
			}
			if (!found)
				break;
		}
		m_actions.push_back(new DVDAction(m_tsi == -1, id));
		if (GetView()) {
			wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, numRows);
			GetView()->ProcessTableMessage(msg);
		}
		return true;
	}

	bool DeleteRows(size_t pos = 0, size_t numRows = 1) {
		m_actions.erase(m_actions.begin() + pos);
		if (GetView()) {
			wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED, pos, 1);
			GetView()->ProcessTableMessage(msg);
		}
		return true;
	}

	wxString GetRowLabelValue( int row ) {
		return wxT("");
	}

	wxString GetColLabelValue( int col ) {
		wxString cols[] = { _("ID"), _("Command") };
		return cols[col];
	}

private:
	vector<DVDAction*>& m_actions;
	DVD* m_dvd;
	int m_tsi;
};


//(*IdInit(MenuActionsDlg)
const long MenuActionsDlg::ID_GRID = wxNewId();
const long MenuActionsDlg::ID_ADD_BT = wxNewId();
const long MenuActionsDlg::ID_DEL_BT = wxNewId();
//*)

BEGIN_EVENT_TABLE(MenuActionsDlg,wxDialog)
	//(*EventTable(MenuActionsDlg)
	//*)
END_EVENT_TABLE()

MenuActionsDlg::MenuActionsDlg(MenuPropDlg* parent, DVD* dvd, int tsi, int pgci, vector<DVDAction*> actions):
		parentDlg(parent), m_dvd(dvd), m_tsi(tsi), m_pgci(pgci) {
	VECTOR_COPY(actions, m_actions, DVDAction)

	//(*Initialize(MenuActionsDlg)
	wxBoxSizer* boxSizer1;
	wxBoxSizer* boxSizer2;
	wxStdDialogButtonSizer* stdDialogButtonSizer;
	wxStaticText* staticText1;
	wxBoxSizer* mainSizer;

	Create(parent, wxID_ANY, _("Menu Actions"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
	mainSizer = new wxBoxSizer(wxVERTICAL);
	staticText1 = new wxStaticText(this, wxID_ANY, _("Actions:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	mainSizer->Add(staticText1, 0, wxALL|wxALIGN_LEFT, 5);
	boxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_grid = new wxGrid(this, ID_GRID, wxDefaultPosition, wxSize(300,300), 0, _T("ID_GRID"));
	boxSizer1->Add(m_grid, 1, wxRIGHT|wxEXPAND, 6);
	boxSizer2 = new wxBoxSizer(wxVERTICAL);
	m_addBt = new wxBitmapButton(this, ID_ADD_BT, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_ADD_BT"));
	boxSizer2->Add(m_addBt, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_delBt = new wxBitmapButton(this, ID_DEL_BT, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_DEL_BT"));
	boxSizer2->Add(m_delBt, 0, wxTOP|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	boxSizer1->Add(boxSizer2, 0, wxEXPAND, 5);
	mainSizer->Add(boxSizer1, 1, wxEXPAND, 5);
	stdDialogButtonSizer = new wxStdDialogButtonSizer();
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	stdDialogButtonSizer->Realize();
	mainSizer->Add(stdDialogButtonSizer, 0, wxALL|wxEXPAND, 5);
	SetSizer(mainSizer);
	mainSizer->Fit(this);
	mainSizer->SetSizeHints(this);
	Center();

	Connect(ID_ADD_BT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MenuActionsDlg::OnAddBt);
	Connect(ID_DEL_BT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MenuActionsDlg::OnDelBt);
	//*)
	Connect(wxID_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MenuActionsDlg::OnOkBt);
	stdDialogButtonSizer->GetAffirmativeButton()->SetDefault();
	
	ActionsTable* table = new ActionsTable(m_actions, m_dvd, m_tsi);
	m_grid->SetTable(table);
	m_grid->SetRowLabelSize(10);
	m_grid->SetColSize(1, 190);
	m_grid->SetColLabelSize(m_grid->GetCharHeight() + 8);
	m_grid->SetDefaultRowSize(m_grid->GetCharHeight() + 8);
	m_grid->SetSelectionMode(wxGrid::wxGridSelectRows);
	for (int i = 0; i < m_grid->GetNumberRows(); i++)
		SetCellEditors(i);

	m_addBt->SetBitmapLabel(wxBITMAP_FROM_MEMORY(add));
	m_delBt->SetBitmapLabel(wxBITMAP_FROM_MEMORY(remove));
	
	m_addBt->SetFocus();
	mainSizer->Fit(this);
}

MenuActionsDlg::~MenuActionsDlg() {
	//(*Destroy(MenuActionsDlg)
	//*)
	VECTOR_CLEAR(m_actions, DVDAction)
}

void MenuActionsDlg::SetCellEditors(int row) {
	m_grid->SetCellEditor(row, 1, new wxGridCellChoiceEditor(parentDlg->GetCommandList(), true));
}

void MenuActionsDlg::OnAddBt(wxCommandEvent& event) {
	m_grid->AppendRows(1);
	SetCellEditors(m_grid->GetNumberRows() - 1);
	m_grid->SelectRow(m_grid->GetGridCursorRow(), false);
	m_grid->SetFocus();
	m_delBt->Enable(true);
}

void MenuActionsDlg::OnDelBt(wxCommandEvent& event) {
	if (!m_grid->GetSelectedRows().IsEmpty())
		while (m_grid->GetSelectedRows().size() > 0)
			m_grid->DeleteRows(m_grid->GetSelectedRows()[0]);
	else if (m_grid->GetGridCursorRow() >= 0)
		m_grid->DeleteRows(m_grid->GetGridCursorRow());
	if (m_grid->GetNumberRows() > 0)
		m_grid->SelectRow(m_grid->GetGridCursorRow(), false);
	m_grid->SetFocus();
	m_delBt->Enable(m_grid->GetNumberRows() > 0);
}

void MenuActionsDlg::OnOkBt(wxCommandEvent& event) {
	// check ids
	int row = 0;
	for (vector<DVDAction*>::const_iterator it = m_actions.begin(); it != m_actions.end(); it++) {
		for (vector<DVDAction*>::const_iterator it2 = it + 1; it2 != m_actions.end(); it2++) {
			if ((*it)->GetId() == (*it2)->GetId()) {
				m_grid->SelectRow(row, false);
				wxLogError(_("Duplicated action ID '%s'."), (*it)->GetId().c_str());
				return;
			}
		}
		row++;
	}
	// check commands
	for (unsigned int i = 0; i < m_actions.size(); i++) {
		if (m_actions[i]->GetCustom().length()
				&& !m_actions[i]->IsValid(m_dvd, m_tsi, m_pgci, true, m_actions[i]->GetId(), true, true)) {
			return;
		}
	}
	EndModal(wxID_OK);
}
