/////////////////////////////////////////////////////////////////////////////
// Name:        AnimationDlg.cpp
// Author:      Alex Thuering
// Created:     08.01.2016
// RCS-ID:      $Id: AnimationDlg.cpp,v 1.5 2016/08/09 21:19:47 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////
#include "AnimationDlg.h"
#include "Utils.h"
#include "MenuObject.h"
#include <wxVillaLib/utils.h>
#include <algorithm>
#include "rc/add.png.h"
#include "rc/remove.png.h"

//(*InternalHeaders(AnimationDlg)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

class AnimationTable: public wxGridTableBase {
public:
	AnimationTable(vector<wxSVGAnimateElement*>& animations, MenuObject* object): wxGridTableBase(),
			m_animations(animations) {
		// parameters
		const wxString attributes[] = { wxT("x"), wxT("y"), wxT("width"), wxT("height"), wxT("opacity") };
		const wxString labels[] = { wxT("x"), wxT("y"), _("width"), _("height"), _("opacity") };
		for (unsigned int i = 0; i < WXSIZEOF(attributes); i++) {
			wxString label = wxGetTranslation(attributes[i]);
			if (attributes[i] == wxT("opacity")) {
				label += wxT(" (1 = 100%)");
			}
			paramLabels.Add(label);
			paramElements.Add(wxT(""));
			paramAttributes.Add(attributes[i]);
		}
		for (int i = 0; i < object->GetObjectParamsCount(); i++) {
			MenuObjectParam* param = object->GetObjectParam(i);
			if (param->type != wxT("integer") && param->type != wxT("decimal") && param->type != wxT("percent")
					&& param->type != wxT("colour") && param->type != wxT("text")) {
				continue;
			}
			if (param->type == wxT("text")) {
				paramLabels.Add(_("font size"));
				paramElements.Add(param->element.front());
				paramAttributes.Add(wxT("font-size"));
			} else {
				wxString label = wxGetTranslation((const wxChar*)param->title.GetData());
				if (paramLabels.Index(label) >= 0) {
					label += wxT(" (") + param->element.front() + wxT(")");
				}
				paramLabels.Add(label);
				paramElements.Add(param->element.front());
				paramAttributes.Add(param->attribute);
			}
		}
	}

	int GetNumberRows() {
		return m_animations.size();
	}

	int GetNumberCols() {
		return 4;
	}

	bool IsEmptyCell(int row, int col) {
		return false;
	}
	
	wxString GetValue(int row, int col) {
		wxSVGAnimateElement* anim = m_animations[row];
		switch (col) {
		case 0:
			return GetParamLabel((anim->GetHref().length() ? anim->GetHref().Mid(1) : anim->GetHref()),
					anim->GetAttributeName());
		case 1:
			if (anim->GetAttribute(wxT("values")).length() > 0)
				return anim->GetAttribute(wxT("values"));
			if (anim->GetFrom().GetValueAsString().length()
					|| anim->GetTo().GetValueAsString().length())
				return anim->GetFrom().GetValueAsString() + wxT(";") + anim->GetTo().GetValueAsString();
			return wxT("");
		case 2:
			return wxString::Format(wxT("%g"), anim->GetBegin());
		case 3:
			return wxString::Format(wxT("%g"), anim->GetDur());
		default:
			break;
		}
		return wxT("");
	}
	
	void SetValue(int row, int col, const wxString& value) {
		wxSVGAnimateElement* anim = m_animations[row];
		double dval;
		switch (col) {
		case 0:
			if (paramLabels.Index(value) >= 0) {
				int idx = paramLabels.Index(value);
				anim->SetHref(paramElements[idx].length() ? wxT("#") + paramElements[idx] : paramElements[idx]);
				anim->SetAttributeName(paramAttributes[idx]);
			}
			break;
		case 1:
			if (value.Find(wxT(';')) > 0 && value.AfterFirst(wxT(';')).Find(wxT(';')) == 0) {
				anim->SetAttribute(wxT("values"), wxT(""));
				wxSVGAnimatedType from, to;
				from.SetValueAsString(value.BeforeFirst(wxT(';')));
				to.SetValueAsString(value.AfterFirst(wxT(';')));
				anim->SetFrom(from);
				anim->SetTo(to);
			} else {
				anim->SetFrom(wxSVGAnimatedType());
				anim->SetTo(wxSVGAnimatedType());
				anim->SetAttribute(wxT("values"), value);
			}
			break;
		case 2:
			if (value.ToDouble(&dval))
				anim->SetBegin(dval);
			break;
		case 3:
			if (value.ToDouble(&dval))
				anim->SetDur(dval);
			break;
		default:
			break;
		}
	}

	bool AppendRows(size_t numRows = 1) {
		m_animations.push_back(new wxSVGAnimateElement);
		if (GetView()) {
			wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, numRows);
			GetView()->ProcessTableMessage(msg);
		}
		return true;
	}

	bool DeleteRows(size_t pos = 0, size_t numRows = 1) {
		m_animations.erase(m_animations.begin() + pos);
		if (GetView()) {
			wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED, pos, 1);
			GetView()->ProcessTableMessage(msg);
		}
		return true;
	}

	wxString GetRowLabelValue(int row) {
		return wxT("");
	}

	wxString GetColLabelValue(int col) {
		wxString colLabels[] = { _("Attribute"), _("Values (separated by semicolon)"), _("Begin"), _("Duration (sec)")};
		return colLabels[col];
	}
	
	int FindParam(const wxString& element, const wxString& attribute) {
		for (unsigned int i = 0; i < paramElements.size(); i++) {
			if (paramElements[i] == element && paramAttributes[i] == attribute) {
				return i;
			}
		}
		return -1;
	}
	
	wxString GetParamLabel(wxString element, wxString attribute) {
		int idx = FindParam(element, attribute);
		return idx >= 0 ? paramLabels[idx] : wxT("");
	}
	
	wxArrayString GetParamLabels() {
		return paramLabels;
	}

private:
	vector<wxSVGAnimateElement*>& m_animations;
	wxArrayString paramLabels;
	wxArrayString paramElements;
	wxArrayString paramAttributes;
};

//(*IdInit(AnimationDlg)
const long AnimationDlg::ID_GRID1 = wxNewId();
const long AnimationDlg::ID_ADD_BT = wxNewId();
const long AnimationDlg::ID_DEL_BT = wxNewId();
//*)

BEGIN_EVENT_TABLE(AnimationDlg,wxDialog)
	//(*EventTable(AnimationDlg)
	//*)
END_EVENT_TABLE()

AnimationDlg::AnimationDlg(wxWindow* parent, MenuObject* menuObject) {
	m_object = menuObject;
	vector<wxSVGAnimateElement*> v = m_object->GetAnimations();
	VECTOR_COPY(v, m_animations, wxSVGAnimateElement)
	
	//(*Initialize(AnimationDlg)
	wxBoxSizer* boxSizer3;
	wxBoxSizer* boxSizer2;
	wxStdDialogButtonSizer* stdDialogButtonSizer;
	wxBoxSizer* mainSizer;

	Create(parent, wxID_ANY, _("Animations"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	mainSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_grid = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxSize(520,300), 0, _T("ID_GRID1"));
	boxSizer2->Add(m_grid, 1, wxRIGHT|wxEXPAND, 6);
	boxSizer3 = new wxBoxSizer(wxVERTICAL);
	m_addBt = new wxBitmapButton(this, ID_ADD_BT, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_ADD_BT"));
	m_addBt->SetDefault();
	boxSizer3->Add(m_addBt, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_delBt = new wxBitmapButton(this, ID_DEL_BT, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_DEL_BT"));
	m_delBt->SetDefault();
	boxSizer3->Add(m_delBt, 0, wxTOP|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	boxSizer2->Add(boxSizer3, 0, wxLEFT|wxRIGHT|wxEXPAND, 4);
	mainSizer->Add(boxSizer2, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	stdDialogButtonSizer = new wxStdDialogButtonSizer();
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	stdDialogButtonSizer->Realize();
	mainSizer->Add(stdDialogButtonSizer, 0, wxALL|wxEXPAND, 5);
	SetSizer(mainSizer);
	mainSizer->Fit(this);
	mainSizer->SetSizeHints(this);

	Connect(ID_ADD_BT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&AnimationDlg::OnAddBt);
	Connect(ID_DEL_BT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&AnimationDlg::OnDelBtClick);
	//*)
	Connect(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&AnimationDlg::OnOkBt);
	stdDialogButtonSizer->GetAffirmativeButton()->SetDefault();
		
	AnimationTable* table = new AnimationTable(m_animations, m_object);
	m_grid->SetTable(table);
	m_grid->SetRowLabelSize(10);
	m_grid->SetColLabelSize(m_grid->GetCharHeight() + 8);
	m_grid->SetDefaultRowSize(m_grid->GetCharHeight() + 8);
	m_grid->SetSelectionMode(wxGrid::wxGridSelectRows);
	for (int col = 0; col < m_grid->GetNumberCols(); col++) {
		m_grid->AutoSizeColLabelSize(col);
		m_grid->SetColSize(col, m_grid->GetColSize(col) + (col == 0 ? 80 : 10));
	}
	for (int row = 0; row < m_grid->GetNumberRows(); row++) {
		SetCellEditors(row);
	}

	m_addBt->SetBitmapLabel(wxBITMAP_FROM_MEMORY(add));
	m_delBt->SetBitmapLabel(wxBITMAP_FROM_MEMORY(remove));
	
	m_addBt->SetFocus();
	mainSizer->Fit(this);
}

AnimationDlg::~AnimationDlg() {
	//(*Destroy(AnimationDlg)
	//*)
}

void AnimationDlg::SetCellEditors(int row) {
	m_grid->SetCellEditor(row, 0, new wxGridCellChoiceEditor(
			((AnimationTable*)m_grid->GetTable())->GetParamLabels(), false));
	
//	const wxString elements[] = { _T(""), _T("main"), _T("text"), _T("background") };
//	m_grid->SetCellEditor(row, 0, new wxGridCellChoiceEditor(WXSIZEOF(elements), elements, true));
//	m_grid->SetCellEditor(row, 1, new wxGridCellChoiceEditor(WXSIZEOF(attributes), attributes, true));
	
	m_grid->SetCellEditor(row, 1, new wxGridCellTextEditor);
	m_grid->SetCellEditor(row, 2, new wxGridCellFloatEditor);
	m_grid->SetCellEditor(row, 3, new wxGridCellFloatEditor);
}

void AnimationDlg::OnAddBt(wxCommandEvent& event) {
	m_grid->AppendRows(1);
	SetCellEditors(m_grid->GetNumberRows() - 1);
	m_grid->SelectRow(m_grid->GetGridCursorRow(), false);
	m_grid->SetFocus();
	m_delBt->Enable(true);
}

void AnimationDlg::OnDelBtClick(wxCommandEvent& event) {
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

void AnimationDlg::OnOkBt(wxCommandEvent& event) {
	m_object->SetAnimations(m_animations);
	EndModal(wxID_OK);
}
