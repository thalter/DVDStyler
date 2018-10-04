/////////////////////////////////////////////////////////////////////////////
// Name:        PropDlg.h
// Purpose:     Base class for property dialogs
// Author:      Alex Thuering
// Created:     18.11.2003
// RCS-ID:      $Id: PropDlg.h,v 1.34 2016/05/16 21:09:33 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef PROP_DLG_H
#define PROP_DLG_H

#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/dynarray.h>
#include <wx/filedlg.h>
#if wxCHECK_VERSION(2,9,0)
#include <wx/fontdata.h>
#endif
#if wxCHECK_VERSION(2,9,3)
#include <wx/colourdata.h>
#endif
#include <wx/artprov.h>
#include <wx/statbmp.h>
#include <vector>
using namespace std;

WX_DECLARE_OBJARRAY(wxBitmap, BitmapArray);


class ColourPanel: public wxPanel {
public:
	ColourPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxSize(20, 20), long style = wxTAB_TRAVERSAL | wxSUNKEN_BORDER,
			const wxString& name = wxPanelNameStr);
	void SetColour(wxColour colour);

	wxColour GetColour();

protected:
	wxColour m_colour;
	wxColour m_oldColour;
	wxColour m_bgColour;
	
	void OnPaint(wxPaintEvent &event);
	void OnClick(wxMouseEvent &event);
	
private:
	DECLARE_EVENT_TABLE()
};


/** Base class for property dialogs.
  * Allow you to simply create properties dialogs without any GUI designer.
  * Using this class, you can also create dynamic property dialog.
  */
class wxPropDlg: public wxDialog {
public:
	/**
	 * Constructor
	 */
    wxPropDlg(wxWindow *parent, wxString title = _("Properties"));
    /**
     * Destructor
     */
    virtual ~wxPropDlg();
    
    /** Returns list of custom colours for colour selection dialog */
    static wxColourData& GetColourData() { return m_colourData; }
	
protected:
	wxArrayInt m_types;
	wxArrayPtrVoid m_controls;
	wxArrayInt m_groupIds;
	int m_updateIndex; /** index of property by update (reset) */
	int m_currGroupId;
	void* m_currGroupChecker;
	static wxColourData m_colourData;
	
	/**
	 * Creates dialog with properties panel and button pane
	 */
	void Create(bool resetButton = false, bool dontShowCheckbox = false);
	
	/**
	 * Creates properties panel.
	 * if sizer != NULL adds properties, else restores default values (reset function).
	 */
	virtual void CreatePropPanel(wxSizer* sizer) = 0;
	
	/**
	 * Creates panel with buttons
	 */
	virtual void CreateButtonPane(wxSizer* sizer, bool resetButton = false, bool dontShowCheckbox = false);
	
	/**
	 * Restores default values in the properties dialog.
	 * Calls CreatePropPanel with sizer = NULL.
	 */
	virtual void Reset();
	
	/**
	 * Populate values on the model.
	 */
	virtual bool SetValues() = 0;

	// Add properties
	wxWindow* propWindow; // default this
	void SetPropWindow(wxWindow* propWindow) { this->propWindow = propWindow; }

	void AddText(wxSizer* sizer, wxString text, int flag = wxALIGN_CENTER_VERTICAL);

	wxSizer* AddTextProp(wxSizer* sizer, const wxString& label, const wxString& value,
	  bool readonly = false, int width = -1, bool addSpacer = true, long style = 0);
	void AddStaticTextProp(wxSizer* sizer, const wxString& label, const wxString& value);
	/**
	 * Adds a static line with given caption to the window
	 */
	void AddStaticLine(wxSizer* sizer, wxString caption);
	/**
	 * Creates label and spin control.
	 * @returns subSizer if width > 0, null otherwise
	 */
	wxSizer* AddSpinProp(wxSizer* sizer, const wxString& label, int value, int min = 0, int max = 100,
			int width = -1, const wxString& label2 = wxT(""), bool addSpacer = true);
	/**
	 * Creates label and spin double control.
	 * @returns subSizer if width > 0, null otherwise
	 */
	wxSizer* AddSpinDoubleProp(wxSizer* sizer, const wxString& label, double value, double min  = 0, double max = 100,
			int width = -1, const wxString& label2 = wxT(""), bool addSpacer = true);
	/**
	 * Creates label and check control
	 */
	void AddCheckProp(wxSizer* sizer, const wxString& label, bool value, bool readonly = false, int id = -1);
	/**
	 * Creates label and radio control
	 */
	void AddRadioProp(wxSizer* sizer, const wxString& label, bool value, long style = wxRB_SINGLE,
			bool readonly = false, int id = -1);
	/**
	 * Creates label and radio group control
	 */
	void AddRadioGroupProp(wxSizer* sizer, const wxArrayString& text, int value, bool readonly = false);
	/**
	 * Creates label and combobox.
	 * @returns subSizer if width > 0, null otherwise
	 */
	wxSizer* AddComboProp(wxSizer* sizer, const wxString& label, const wxString& value, const wxArrayString& choices,
			long style = 0, int width = -1, bool addSpacer = true, int id = -1);
	/**
	 * Creates label and bitmap combobox.
	 * @returns subSizer if width > 0, null otherwise
	 */
	wxSizer* AddBitmapComboProp(wxSizer* sizer, const wxString& label, const wxString& value, const wxArrayString& choices,
			const BitmapArray& bitmaps, long style = 0, int width = -1, bool addSpacer = true, int id = -1);
	/**
	 * Creates label and choice control.
	 * @returns subSizer if width > 0, null otherwise
	 */
	wxSizer* AddChoiceProp(wxSizer* sizer, const wxString& label, const wxString& value,
	  const wxArrayString& choices, int width = -1, bool addSpacer = true, int id = -1);
	/**
	 * Creates choice control.
	 * @returns Choice control
	 */
	wxChoice* AddChoiceProp(const wxString& value, const wxArrayString& choices, int id = -1);
	/**
	 * Creates label and choice control with item 'Custom'.
	 * @returns subSizer if width > 0, null otherwise
	 */
	wxSizer* AddChoiceCustomProp(wxSizer* sizer, const wxString& label, const wxString& value,
	  const wxArrayString& choices, int customItemIdx, int width = -1, bool addSpacer = true);
	/** Sets the last control as control to enter the custom value. */
	void SetLastControlCustom(int choiceCtrlIdx, bool enable);
	void AddGridProp(wxSizer* sizer, const wxArrayPtrVoid& data, const wxString& rowTitle, bool editable);
	void SetGridData(wxGrid* grid, const wxArrayPtrVoid& data, const wxString& rowTitle, bool editable);
	void AddFontProp(wxSizer* sizer, const wxString& label, wxFont font, wxString caption = _T("..."));
	void AddFontProp(wxSizer* sizer, const wxString& label, wxFontData font, wxString caption = _T("..."));
	void AddColourProp(wxSizer* sizer, const wxString& label,
	  wxColour colour, int opacity = -1, wxString caption = _T("..."));
	wxSizer* AddFileProp(wxSizer* sizer, const wxString& label, const wxString& value,
	  int dlgStyle = wxFD_OPEN, wxString caption = _T("..."), wxString wildcard = _T("*.*"), int id = -1);
	wxSizer* AddDirectoryProp(wxSizer* sizer, const wxString& label, const wxString& value, wxString caption = _T("..."));
	void AddButton(wxSizer* sizer, const wxString& label, const long id, const wxString& buttonLabel = _T("..."));
	void AddBitmapToggleButton(wxSizer* sizer, bool value, const long id, const wxBitmap& bitmap, const wxSize& size);

	wxSizer* BeginGroup(wxSizer* sizer, wxString title = wxEmptyString, wxString checkTitle = wxEmptyString,
			bool value = false, bool readonly = false, bool vertical = true, int proportion = 0);
	void BeginCheckGroup(wxSizer* sizer, wxString label, bool value, bool readonly = false);
	void EndGroup();

	int AddIcon(wxSizer* sizer, const wxString& title, const wxString& tooltip, wxArtID artId = wxART_INFORMATION);
	void UpdateIcon(int index, const wxString& title, const wxString& tooltip, wxArtID artId = wxART_INFORMATION);
	wxStaticBitmap* GetIcon(int index) { return m_icons[index]; }
	
	/**
	 * @return the pointer to the control with given index
	 */
	wxControl* GetControl(int index);
	/**
	 * @return the pointer to last added control
	 */
	wxControl* GetLastControl();
	/**
	 * @return index of the last added control
	 */
	int GetLastControlIndex();

	// Get values
	wxString GetString(int index);
	int GetInt(int index);
	bool GetBool(int index);
	double GetDouble(int index);
	void* GetClientData(int index);
	wxArrayPtrVoid GetGrid(int index);
	wxFontData GetFont(int index);
	wxColour GetColour(int index);
	
	void* m_currObject;
	int m_currObjectItem;
  	virtual void OnCancel(wxCommandEvent& event);
    virtual void OnOk(wxCommandEvent& event);
    virtual void OnReset(wxCommandEvent& event);
    virtual void OnSelectCustomChoice(wxCommandEvent& evt);
	virtual void OnSelectFont(wxCommandEvent& event);
	virtual void OnSelectColour(wxCommandEvent& event);
	virtual void OnSelectFile(wxCommandEvent& event);
	virtual void OnClearText(wxCommandEvent& event);
	virtual void OnSelectDir(wxCommandEvent& event);
	virtual void OnGroupCheck(wxCommandEvent& event);
	virtual void OnCellLeftClick(wxGridEvent& event);
	virtual void OnCellRightClick(wxGridEvent& event);
	virtual void OnCellChange(wxGridEvent& event);
	virtual void OnRowDelete(wxCommandEvent& event);
	virtual void OnShowTooltip(wxMouseEvent& event);
	
private:
	vector<wxStaticBitmap*> m_icons;
	wxArrayInt m_tooltipIcon;
	wxArrayString m_tooltipTitle;
	wxArrayString m_tooltipText;

    DECLARE_EVENT_TABLE()
};

#endif // PROP_DLG_H
