/////////////////////////////////////////////////////////////////////////////
// Name:        ButtonEditDlg.cpp
// Purpose:     Dialog to create new button from SVG file
// Author:      Alex Thuering
// Created:     5.03.2018
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "ButtonEditDlg.h"
#include "MenuObject.h"
#include "ButtonAddParamDlg.h"
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/log.h>

//(*InternalHeaders(ButtonEditDlg)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

class MenuObjectTable: public wxGridTableBase {
public:
	MenuObjectTable(MenuObjectDef& menuObjectDef): wxGridTableBase(),
			m_objectDef(menuObjectDef), m_params(menuObjectDef.GetParams()) {
		// nothing to do
	}

	int GetNumberRows() {
		return m_params.size();
	}

	int GetNumberCols() {
		return 9;
	}

	bool IsEmptyCell(int row, int col) {
		return false;
	}

	wxString GetValue(int row, int col) {
		MenuObjectParam* param = m_params[row];
		switch (col) {
		case 0:
			return param->name;
		case 1:
			return param->title;
		case 2:
			return param->type;
		case 3: {
			wxString elements;
			for (wxString elem : param->element) {
				if (elements.length())
					elements += ",";
				elements += elem;
			}
			return elements;
		}
		case 4:
			return param->attribute;
		case 5:
			return param->changeable ? "1" : "0";
		case 6:
			if (param->element.size()) {
				if (param->changeable)
					return wxSVGPaint(m_objectDef.GetParamColour(param->name)).GetCSSText();
				return m_objectDef.GetParam(param->name);
			}
			return "";
		case 7:
			return param->changeable ? wxSVGPaint(param->highlightedColour).GetCSSText() : "";
		case 8:
			return param->changeable ? wxSVGPaint(param->selectedColour).GetCSSText() : "";
		default:
			break;
		}
		return "";
	}

	void SetValue(int row, int col, const wxString& value) {
		MenuObjectParam* param = m_params[row];
		switch (col) {
		case 0:
			param->name = value;
			break;
		case 1:
			param->title = value;
			break;
		case 2:
			param->type = value;
			break;
		case 3:
			param->element.clear();
			param->element.push_back(value);
			break;
		case 4:
			param->attribute = value;
			break;
		case 5:
			param->changeable = (value == "1");
			break;
		case 6:
			if (param->name.length()) {
				if (param->changeable)
					m_objectDef.SetParamColour(param->name, wxCSSStyleDeclaration::ParseColor(value));
				m_objectDef.SetParam(param->name, value);
			}
			break;
		case 7:
			param->highlightedColour = param->changeable ? wxCSSStyleDeclaration::ParseColor(value) : wxColour();
			break;
		case 8:
			param->selectedColour = param->changeable ? wxCSSStyleDeclaration::ParseColor(value) : wxColour();
			break;
		default:
			break;
		}
	}

	bool AppendRows(size_t numRows = 1) {
		wxString paramName;
		int paramNum = 1;
		while (1) {
			paramName = wxString::Format("param%02d", paramNum);
			for (MenuObjectParam* p : m_params) {
				if (p->name == paramName) {
					paramName = "";
					break;
				}
			}
			if (paramName.length() > 0)
				break;
			paramNum++;
		}

		MenuObjectParam* param = new MenuObjectParam;
		param->name = paramName;
		param->title = "Fill colour";
		param->type = "colour";
		param->attribute = "fill";
		param->changeable = true;
		m_params.push_back(param);

		if (GetView()) {
			wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, numRows);
			GetView()->ProcessTableMessage(msg);
		}
		return true;
	}

	bool DeleteRows(size_t pos = 0, size_t numRows = 1) {
		m_params.erase(m_params.begin() + pos);
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
		wxString cols[] = { _("name"), _("title"), _("type"), _("ID of element"), _("attribute name"), _("dynamic"),
			_("value"), _("highlighted"), _("selected") };
		return cols[col];
	}

private:
	MenuObjectDef& m_objectDef;
	vector<MenuObjectParam*>& m_params;
};


//(*IdInit(ButtonEditDlg)
const long ButtonEditDlg::ID_SVG_CTRL = wxNewId();
const long ButtonEditDlg::ID_BUTTON_STATE = wxNewId();
const long ButtonEditDlg::ID_STATICTEXT1 = wxNewId();
const long ButtonEditDlg::ID_TEXTCTRL1 = wxNewId();
const long ButtonEditDlg::ID_TEXTCTRL2 = wxNewId();
const long ButtonEditDlg::ID_SPINCTRL1 = wxNewId();
const long ButtonEditDlg::ID_COMBOBOX1 = wxNewId();
const long ButtonEditDlg::ID_SPINCTRL2 = wxNewId();
const long ButtonEditDlg::ID_SPINCTRL3 = wxNewId();
const long ButtonEditDlg::ID_SPINCTRL4 = wxNewId();
const long ButtonEditDlg::ID_COMBOBOX2 = wxNewId();
const long ButtonEditDlg::ID_SPINCTRL5 = wxNewId();
const long ButtonEditDlg::ID_SPINCTRL6 = wxNewId();
const long ButtonEditDlg::ID_SPINCTRL7 = wxNewId();
const long ButtonEditDlg::ID_COMBOBOX3 = wxNewId();
const long ButtonEditDlg::ID_SPINCTRL8 = wxNewId();
const long ButtonEditDlg::ID_SPINCTRL9 = wxNewId();
const long ButtonEditDlg::ID_SPINCTRL10 = wxNewId();
const long ButtonEditDlg::ID_COMBOBOX4 = wxNewId();
const long ButtonEditDlg::ID_SPINCTRL11 = wxNewId();
const long ButtonEditDlg::ID_SPINCTRL12 = wxNewId();
const long ButtonEditDlg::ID_STATICBITMAP1 = wxNewId();
const long ButtonEditDlg::ID_PARAMS_GRID = wxNewId();
//*)

BEGIN_EVENT_TABLE(ButtonEditDlg,wxDialog)
	//(*EventTable(ButtonEditDlg)
	//*)
END_EVENT_TABLE()

ButtonEditDlg::ButtonEditDlg(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size) {
	//(*Initialize(ButtonEditDlg)
	wxBoxSizer* BoxSizer1;
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer3;
	wxBoxSizer* BoxSizer4;
	wxBoxSizer* BoxSizer5;
	wxBoxSizer* BoxSizer6;
	wxBoxSizer* defHeightSizer;
	wxBoxSizer* defWidthSizer;
	wxBoxSizer* mainSizer;
	wxFlexGridSizer* flexGridSizer1;
	wxStaticText* StaticText10;
	wxStaticText* StaticText11;
	wxStaticText* StaticText12;
	wxStaticText* StaticText13;
	wxStaticText* StaticText14;
	wxStaticText* StaticText15;
	wxStaticText* StaticText16;
	wxStaticText* StaticText17;
	wxStaticText* StaticText18;
	wxStaticText* StaticText19;
	wxStaticText* StaticText1;
	wxStaticText* StaticText20;
	wxStaticText* StaticText21;
	wxStaticText* StaticText22;
	wxStaticText* StaticText23;
	wxStaticText* StaticText24;
	wxStaticText* StaticText25;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText4;
	wxStaticText* StaticText5;
	wxStaticText* StaticText6;
	wxStaticText* StaticText7;
	wxStaticText* StaticText8;
	wxStaticText* StaticText9;
	wxStdDialogButtonSizer* stdDialogButtonSizer;

	Create(parent, wxID_ANY, _("Create button"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER, _T("wxID_ANY"));
	SetClientSize(wxSize(900,600));
	mainSizer = new wxBoxSizer(wxVERTICAL);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	m_svgCtrl = new wxSVGCtrl(this,ID_SVG_CTRL,wxDefaultPosition,wxDefaultSize,0,_T("ID_SVG_CTRL"));
	BoxSizer2->Add(m_svgCtrl, 1, wxALL|wxEXPAND, 5);
	BoxSizer5 = new wxBoxSizer(wxVERTICAL);
	wxString __wxRadioBoxChoices_1[3] =
	{
		_("normal"),
		_("highlighted"),
		_("selected")
	};
	m_buttonStateCtrl = new wxRadioBox(this, ID_BUTTON_STATE, _("Button state"), wxDefaultPosition, wxDefaultSize, 3, __wxRadioBoxChoices_1, 1, 0, wxDefaultValidator, _T("ID_BUTTON_STATE"));
	BoxSizer5->Add(m_buttonStateCtrl, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer2->Add(BoxSizer5, 0, wxRIGHT|wxALIGN_TOP, 5);
	mainSizer->Add(BoxSizer2, 1, wxEXPAND, 5);
	flexGridSizer1 = new wxFlexGridSizer(0, 2, 2, 2);
	flexGridSizer1->AddGrowableCol(1);
	flexGridSizer1->AddGrowableRow(3);
	StaticText4 = new wxStaticText(this, wxID_ANY, _("Directory:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	flexGridSizer1->Add(StaticText4, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_dirCtrl = new wxStaticText(this, ID_STATICTEXT1, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	flexGridSizer1->Add(m_dirCtrl, 1, wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("File name:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	flexGridSizer1->Add(StaticText1, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	m_filenameCtrl = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	BoxSizer1->Add(m_filenameCtrl, 1, wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, wxID_ANY, _(".xml"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer1->Add(StaticText3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	flexGridSizer1->Add(BoxSizer1, 1, wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Title:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	flexGridSizer1->Add(StaticText2, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 0);
	m_titleCtrl = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	flexGridSizer1->Add(m_titleCtrl, 1, wxEXPAND, 0);
	StaticText6 = new wxStaticText(this, wxID_ANY, _("Default width:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	flexGridSizer1->Add(StaticText6, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	defWidthSizer = new wxBoxSizer(wxHORIZONTAL);
	m_defWidth = new wxSpinCtrl(this, ID_SPINCTRL1, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL1"));
	m_defWidth->SetValue(_T("0"));
	defWidthSizer->Add(m_defWidth, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(this, wxID_ANY, _("or"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	defWidthSizer->Add(StaticText7, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_defWidthElem = new wxComboBox(this, ID_COMBOBOX1, wxEmptyString, wxDefaultPosition, wxSize(80,-1), 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX1"));
	defWidthSizer->Add(m_defWidthElem, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText8 = new wxStaticText(this, wxID_ANY, _("+"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	defWidthSizer->Add(StaticText8, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_defWidthInc = new wxSpinCtrl(this, ID_SPINCTRL2, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL2"));
	m_defWidthInc->SetValue(_T("0"));
	defWidthSizer->Add(m_defWidthInc, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText10 = new wxStaticText(this, wxID_ANY, _("+"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	defWidthSizer->Add(StaticText10, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_defWidthPercent = new wxSpinCtrl(this, ID_SPINCTRL3, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL3"));
	m_defWidthPercent->SetValue(_T("0"));
	defWidthSizer->Add(m_defWidthPercent, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText9 = new wxStaticText(this, wxID_ANY, _("%"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	defWidthSizer->Add(StaticText9, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	defWidthSizer->Add(-1,-1,1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText11 = new wxStaticText(this, wxID_ANY, _("Min width:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	defWidthSizer->Add(StaticText11, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 8);
	BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
	m_minWidth = new wxSpinCtrl(this, ID_SPINCTRL4, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL4"));
	m_minWidth->SetValue(_T("0"));
	BoxSizer3->Add(m_minWidth, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText12 = new wxStaticText(this, wxID_ANY, _("or"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer3->Add(StaticText12, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_minWidthElem = new wxComboBox(this, ID_COMBOBOX2, wxEmptyString, wxDefaultPosition, wxSize(80,-1), 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX2"));
	BoxSizer3->Add(m_minWidthElem, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText13 = new wxStaticText(this, wxID_ANY, _("+"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer3->Add(StaticText13, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_minWidthInc = new wxSpinCtrl(this, ID_SPINCTRL5, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL5"));
	m_minWidthInc->SetValue(_T("0"));
	BoxSizer3->Add(m_minWidthInc, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText14 = new wxStaticText(this, wxID_ANY, _("+"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer3->Add(StaticText14, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_minWidthPercent = new wxSpinCtrl(this, ID_SPINCTRL6, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL6"));
	m_minWidthPercent->SetValue(_T("0"));
	BoxSizer3->Add(m_minWidthPercent, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText15 = new wxStaticText(this, wxID_ANY, _("%"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer3->Add(StaticText15, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	defWidthSizer->Add(BoxSizer3, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	flexGridSizer1->Add(defWidthSizer, 1, wxEXPAND, 5);
	StaticText16 = new wxStaticText(this, wxID_ANY, _("Default height:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	flexGridSizer1->Add(StaticText16, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	defHeightSizer = new wxBoxSizer(wxHORIZONTAL);
	m_defHeight = new wxSpinCtrl(this, ID_SPINCTRL7, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL7"));
	m_defHeight->SetValue(_T("0"));
	defHeightSizer->Add(m_defHeight, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText17 = new wxStaticText(this, wxID_ANY, _("or"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	defHeightSizer->Add(StaticText17, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_defHeightElem = new wxComboBox(this, ID_COMBOBOX3, wxEmptyString, wxDefaultPosition, wxSize(80,-1), 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX3"));
	defHeightSizer->Add(m_defHeightElem, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText18 = new wxStaticText(this, wxID_ANY, _("+"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	defHeightSizer->Add(StaticText18, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_defHeightInc = new wxSpinCtrl(this, ID_SPINCTRL8, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL8"));
	m_defHeightInc->SetValue(_T("0"));
	defHeightSizer->Add(m_defHeightInc, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText19 = new wxStaticText(this, wxID_ANY, _("+"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	defHeightSizer->Add(StaticText19, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_defHeightPercent = new wxSpinCtrl(this, ID_SPINCTRL9, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL9"));
	m_defHeightPercent->SetValue(_T("0"));
	defHeightSizer->Add(m_defHeightPercent, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText20 = new wxStaticText(this, wxID_ANY, _("%"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	defHeightSizer->Add(StaticText20, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	defHeightSizer->Add(-1,-1,1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText21 = new wxStaticText(this, wxID_ANY, _("Min height:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	defHeightSizer->Add(StaticText21, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 8);
	BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
	m_minHeight = new wxSpinCtrl(this, ID_SPINCTRL10, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL10"));
	m_minHeight->SetValue(_T("0"));
	BoxSizer4->Add(m_minHeight, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText22 = new wxStaticText(this, wxID_ANY, _("or"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer4->Add(StaticText22, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_minHeightElem = new wxComboBox(this, ID_COMBOBOX4, wxEmptyString, wxDefaultPosition, wxSize(80,-1), 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX4"));
	BoxSizer4->Add(m_minHeightElem, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText23 = new wxStaticText(this, wxID_ANY, _("+"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer4->Add(StaticText23, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_minHeightInc = new wxSpinCtrl(this, ID_SPINCTRL11, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL11"));
	m_minHeightInc->SetValue(_T("0"));
	BoxSizer4->Add(m_minHeightInc, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText24 = new wxStaticText(this, wxID_ANY, _("+"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer4->Add(StaticText24, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	m_minHeightPercent = new wxSpinCtrl(this, ID_SPINCTRL12, _T("0"), wxDefaultPosition, wxSize(48,-1), 0, 0, 999, 0, _T("ID_SPINCTRL12"));
	m_minHeightPercent->SetValue(_T("0"));
	BoxSizer4->Add(m_minHeightPercent, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	StaticText25 = new wxStaticText(this, wxID_ANY, _("%"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer4->Add(StaticText25, 0, wxLEFT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
	defHeightSizer->Add(BoxSizer4, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	flexGridSizer1->Add(defHeightSizer, 1, wxEXPAND, 5);
	BoxSizer6 = new wxBoxSizer(wxVERTICAL);
	StaticText5 = new wxStaticText(this, wxID_ANY, _("Parameters:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	BoxSizer6->Add(StaticText5, 0, wxEXPAND, 5);
	m_infoIcon = new wxStaticBitmap(this, ID_STATICBITMAP1, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_INFORMATION")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER, _T("ID_STATICBITMAP1"));
	BoxSizer6->Add(m_infoIcon, 0, wxALL|wxALIGN_LEFT, 5);
	flexGridSizer1->Add(BoxSizer6, 0, wxEXPAND, 5);
	m_grid = new wxGrid(this, ID_PARAMS_GRID, wxDefaultPosition, wxSize(500,200), 0, _T("ID_PARAMS_GRID"));
	flexGridSizer1->Add(m_grid, 1, wxEXPAND, 5);
	mainSizer->Add(flexGridSizer1, 0, wxLEFT|wxRIGHT|wxEXPAND, 5);
	stdDialogButtonSizer = new wxStdDialogButtonSizer();
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_OK, _("Save")));
	stdDialogButtonSizer->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	stdDialogButtonSizer->Realize();
	mainSizer->Add(stdDialogButtonSizer, 0, wxTOP|wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(mainSizer);
	SetSizer(mainSizer);
	Layout();
	Center();

	Connect(ID_BUTTON_STATE,wxEVT_COMMAND_RADIOBOX_SELECTED,(wxObjectEventFunction)&ButtonEditDlg::OnButtonState);
	//*)
	Connect(wxID_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ButtonEditDlg::OnOkBt);
	Connect(ID_PARAMS_GRID, wxEVT_GRID_CELL_CHANGED, (wxObjectEventFunction)&ButtonEditDlg::OnCellChanged);
	m_grid->GetGridWindow()->Bind(wxEVT_RIGHT_DOWN, &ButtonEditDlg::OnGridRightClick, this);
	m_svgCtrl->Bind(wxEVT_LEFT_UP, &ButtonEditDlg::OnSvgLeftClick, this);
	stdDialogButtonSizer->GetAffirmativeButton()->SetDefault();

	m_dirCtrl->SetLabel(wxStandardPaths::Get().GetUserDataDir() + wxFILE_SEP_PATH + "buttons" + wxFILE_SEP_PATH);
	m_infoIcon->Connect(wxEVT_ENTER_WINDOW, wxMouseEventHandler(ButtonEditDlg::OnShowTooltip), NULL, this);
}

ButtonEditDlg::~ButtonEditDlg() {
	//(*Destroy(ButtonEditDlg)
	//*)
}

void ButtonEditDlg::InitGrid() {
	MenuObjectTable* table = new MenuObjectTable(m_objectDef);
	m_grid->SetTable(table);
	m_grid->SetRowLabelSize(10);
	m_grid->SetColSize(3, 120);
	m_grid->SetColSize(4, 120);
	m_grid->SetColSize(5, 75);
	m_grid->SetColSize(7, 75);
	m_grid->SetColSize(8, 75);
	m_grid->SetColLabelSize(m_grid->GetCharHeight() + 8);
	m_grid->SetDefaultRowSize(m_grid->GetCharHeight() + 8);
	m_grid->SetSelectionMode(wxGrid::wxGridSelectRows);
	for (int i = 0; i < m_grid->GetNumberRows(); i++)
		SetCellEditors(i);

	m_defWidthElem->Append("");
	m_defWidthElem->Append(m_objectDef.GetIdList());
	m_minWidthElem->Append("");
	m_minWidthElem->Append(m_objectDef.GetIdList());
	m_defHeightElem->Append("");
	m_defHeightElem->Append(m_objectDef.GetIdList());
	m_minHeightElem->Append("");
	m_minHeightElem->Append(m_objectDef.GetIdList());
}

bool ButtonEditDlg::LoadSVG(const wxString& filename) {
	if (!m_objectDef.LoadSVG(filename))
		return false;
	m_svgCtrl->SetSVG(m_objectDef.GetSVG());
	wxString basename = filename.AfterLast(wxFILE_SEP_PATH).BeforeLast('.');
	m_filenameCtrl->SetValue(basename);
	m_titleCtrl->SetValue(basename.substr(0, 1).Upper() + basename.substr(1));
	m_defWidth->SetValue(48);
	m_defHeight->SetValue(48);
	InitGrid();
	return true;
}

bool ButtonEditDlg::LoadButton(const wxString& filename) {
	SetTitle(_("Edit button"));
	if (!m_objectDef.Load(filename))
		return false;
	m_svgCtrl->SetSVG(m_objectDef.GetSVG());
	wxString basename = filename.AfterLast(wxFILE_SEP_PATH).BeforeLast('.');
	m_filenameCtrl->SetValue(basename);
	m_titleCtrl->SetValue(m_objectDef.GetTitle());
	m_defWidth->SetValue(m_objectDef.GetDefaultWidth().value);
	if (m_objectDef.GetDefaultWidth().elements.size())
		m_defWidthElem->SetValue(m_objectDef.GetDefaultWidth().elements[0]);
	m_defWidthInc->SetValue(m_objectDef.GetDefaultWidth().valueInc);
	m_defWidthPercent->SetValue(m_objectDef.GetDefaultWidth().valuePercent);
	m_defHeight->SetValue(m_objectDef.GetDefaultHeight().value);
	if (m_objectDef.GetDefaultHeight().elements.size())
		m_defHeightElem->SetValue(m_objectDef.GetDefaultHeight().elements[0]);
	m_defHeightInc->SetValue(m_objectDef.GetDefaultHeight().valueInc);
	m_defHeightPercent->SetValue(m_objectDef.GetDefaultHeight().valuePercent);
	InitGrid();
	return true;
}

void ButtonEditDlg::SetCellEditors(int row) {
	m_grid->SetCellEditor(row, 2, new wxGridCellChoiceEditor(MenuObjectDef::GetParamTypes(), false));
	m_grid->SetCellEditor(row, 3, new wxGridCellChoiceEditor(m_objectDef.GetIdList(), false));

	m_grid->SetCellRenderer(row, 5, new wxGridCellBoolRenderer);
	m_grid->SetCellEditor(row, 5, new wxGridCellBoolEditor);
}

void ButtonEditDlg::OnCellChanged(wxGridEvent& event) {
	if (event.GetCol() >= 2 && event.GetCol() <= 5) {
		MenuObjectParam* param = m_objectDef.GetParams()[event.GetRow()];
		if (param->changeable && param->element.size() && param->attribute.length() > 0) {
			param->changeable = false;
			param->normalColour = m_objectDef.GetParamColour(param->name);
			param->changeable = true;
		}
	}
	m_svgCtrl->Refresh();
}

void ButtonEditDlg::OnGridRightClick(wxMouseEvent& event) {
	// context menu
	wxMenu mnu;
	mnu.Append(wxID_ADD, _("Add"));
	if (!m_grid->GetSelectedRows().IsEmpty()) {
		mnu.Append(wxID_REMOVE, _("Remove selected"));
	}
	mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(ButtonEditDlg::OnPopupClick), NULL, this);
	PopupMenu(&mnu);
}

void ButtonEditDlg::OnPopupClick(wxCommandEvent &evt) {
	switch (evt.GetId()) {
	case wxID_ADD:
		m_grid->AppendRows(1);
		SetCellEditors(m_grid->GetNumberRows() - 1);
		m_grid->SelectRow(m_grid->GetGridCursorRow(), false);
		m_grid->SetFocus();
		break;
	case wxID_REMOVE:
		if (!m_grid->GetSelectedRows().IsEmpty())
			while (m_grid->GetSelectedRows().size() > 0)
				m_grid->DeleteRows(m_grid->GetSelectedRows()[0]);
		if (m_grid->GetNumberRows() > 0)
			m_grid->SelectRow(m_grid->GetGridCursorRow(), false);
		m_grid->SetFocus();
		break;
	}
}

void ButtonEditDlg::OnSvgLeftClick(wxMouseEvent & event) {
	// get list of elements
	vector<wxString> elements;
	wxSVGSVGElement* rootElem = m_svgCtrl->GetSVG()->GetRootElement();
	wxSVGRect rect(event.m_x / m_svgCtrl->GetScaleX(), event.m_y / m_svgCtrl->GetScaleY(), 1, 1);
	wxNodeList nodeList = rootElem->GetIntersectionList(rect, *rootElem);
	for (unsigned int i = 1; i < nodeList.GetCount(); i++) {
		wxSVGElement* obj = nodeList.Item(i);
		if (obj->GetId() == "DVDStyler_bt_bg") {
			nodeList.RemoveAt(i);
			i--;
			continue;
		}
		if (obj->GetId().length())
			elements.push_back(wxString::Format(_T("%s, id: %s"), obj->GetName().c_str(), obj->GetId().c_str()));
		else
			elements.push_back(obj->GetName());
	}
	if (elements.size() == 0) {
		wxMessageBox(_("No SVG element is selected"), this->GetTitle(), wxOK, this);
		return;
	}
	ButtonAddParamDlg dlg(this, rootElem, nodeList, elements);
//	wxSingleChoiceDialog dlg(this, "Please select an SVG element for new parameter", "Add parameter", elements);
//	dlg.SetSelection(elements.size() - 1);
	if (dlg.ShowModal() != wxID_OK)
		return;
	wxSVGElement* element = nodeList[dlg.GetSelection() + 1];

	// set element ID, if it is empty
	wxString id = element->GetId();
	if (id.length() == 0) {
		int idNum = 1;
		while (1) {
			id = wxString::Format("id%002d", idNum);
			if (rootElem->GetElementById(id) == NULL)
				break;
			idNum++;
		}
		element->SetId(id);
	}

	// add parameter
	m_grid->AppendRows(1);
	MenuObjectParam* param = m_objectDef.GetParams().back();
	param->element.push_back(id);
	if (param->changeable) {
		param->changeable = false;
		param->normalColour = m_objectDef.GetParamColour(param->name);
		param->changeable = true;
	}

	SetCellEditors(m_grid->GetNumberRows() - 1);
	m_grid->SelectRow(m_grid->GetGridCursorRow(), false);
	m_grid->SetFocus();
}

void ButtonEditDlg::OnOkBt(wxCommandEvent& event) {
	// update object definition
	m_objectDef.GetDefaultWidth().value = m_defWidth->GetValue();
	if (m_objectDef.GetDefaultWidth().elements.size())
		m_objectDef.GetDefaultWidth().elements[0] = m_defWidthElem->GetValue();
	else
		m_objectDef.GetDefaultWidth().elements.push_back(m_defWidthElem->GetValue());
	m_objectDef.GetDefaultWidth().valueInc = m_defWidthInc->GetValue();
	m_objectDef.GetDefaultWidth().valuePercent = m_defWidthPercent->GetValue();

	m_objectDef.GetDefaultHeight().value = m_defHeight->GetValue();
	if (m_objectDef.GetDefaultHeight().elements.size())
		m_objectDef.GetDefaultHeight().elements[0] = m_defHeightElem->GetValue();
	else
		m_objectDef.GetDefaultHeight().elements.push_back(m_defHeightElem->GetValue());
	m_objectDef.GetDefaultHeight().valueInc = m_defHeightInc->GetValue();
	m_objectDef.GetDefaultHeight().valuePercent = m_defHeightPercent->GetValue();
	
	// save
	wxString dir = wxStandardPaths::Get().GetUserDataDir() + wxFILE_SEP_PATH;
	if (!wxDirExists(dir))
		wxDir::Make(dir);
	dir = dir + "buttons" + wxFILE_SEP_PATH;
	if (!wxDirExists(dir))
		wxDir::Make(dir);
	wxString fname = dir + m_filenameCtrl->GetValue() + ".xml";
	if (wxFileExists(fname) && wxMessageBox(wxString::Format(
			_("File '%s' already exist. Do you want to overwrite it?"), fname.c_str()),
			this->GetTitle(), wxYES_NO|wxICON_QUESTION, this) == wxNO)
		return;
	m_objectDef.SetButtonState(mbsNORMAL);
	if (!m_objectDef.Save(fname))
		return;
	m_destFileName = fname;
	EndModal(wxID_OK);
}

void ButtonEditDlg::OnButtonState(wxCommandEvent& event) {
	m_objectDef.SetButtonState((MenuButtonState) m_buttonStateCtrl->GetSelection());
	m_svgCtrl->Refresh();
}

void ButtonEditDlg::OnShowTooltip(wxMouseEvent& event) {
	wxRichToolTip toolTip(_("Info"), _("Click on image to select an SVG\n element for a new parameter"));
	toolTip.SetIcon(wxICON_INFORMATION);
	toolTip.SetTipKind(wxTipKind_Bottom);
	toolTip.SetTimeout(3000);
	toolTip.ShowFor(m_infoIcon);
}
