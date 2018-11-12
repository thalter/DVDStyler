/////////////////////////////////////////////////////////////////////////////
// Name:        MenuObjectDef.h
// Purpose:     The class to store a DVD menu object definition
// Author:      Alex Thuering
// Created:     5.03.2018
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef MENU_OBJECT_DEF_H
#define MENU_OBJECT_DEF_H

#include <wx/string.h>
#include <wx/colour.h>
#include <wxSVG/SVGDocument.h>
#include <wxSVG/SVGLength.h>
#include <vector>
#include <set>

using namespace std;
enum MenuButtonState { mbsNORMAL, mbsHIGHLIGHTED, mbsSELECTED };

struct MenuObjectParam {
	wxString name;
	wxString title;
	wxString type;
	vector<wxString> element; // id of element
	wxString attribute; // attribute name
	bool changeable; // changed if select the button
	wxColour normalColour;
	wxColour highlightedColour;
	wxColour selectedColour;
	inline bool isChangeable() {
		return changeable && (normalColour != highlightedColour || normalColour != selectedColour);
	}
	/** Initializes object with XML data */
  	bool PutXML(wxSvgXmlNode* node);
  	/** Returns object data as XML */
  	wxSvgXmlNode* GetXML();
};

struct MenuObjectSize {
	unsigned int value;
	unsigned int valueInc;
	unsigned int valuePercent;
	wxArrayString elements;
	
	wxString GetValueAsString() const;
	void SetValueAsString(wxString value);
};


class MenuObjectDef {
public:
	/** Constructor */
	MenuObjectDef(wxSVGDocument* svg = NULL);
	/** Destructor */
	virtual ~MenuObjectDef();
	
	/** Load SVG file */
	bool LoadSVG(const wxString& filename);
	/** Loads object definition */
	bool Load(const wxString& filename);
	/** Saves object definition */
	bool Save(const wxString& fileName);
	
	/** Returns SVG document */
	virtual wxSVGDocument* GetSVG() const { return m_svg; }
	/** Returns button SVG root element */
	virtual wxSVGSVGElement* GetButtonSVG() const;
	/** Returns object svg element by element id */
	wxSVGElement* GetElementById(wxString id) const;
		
	const wxString& GetTitle() const { return m_title; }
	void SetTitle(const wxString& title) { m_title = title;	}

	const wxArrayString& GetIdList() const { return m_idList; }
	void UpdateIdList();
	
	vector<MenuObjectParam*>& GetParams() { return m_params; }
	MenuObjectParam* GetParamByName(wxString name) const;
	MenuObjectParam* GetInitParam();
	MenuObjectParam* GetImageParam();

	const wxString& GetInitParameter() const { return m_initParameter; }
	void SetInitParameter(const wxString& initParameter) {	m_initParameter = initParameter; }
	
	bool IsPreviewHighlighted() const {	return m_previewHighlighted; }
	void SetPreviewHighlighted(bool previewHighlighted) { m_previewHighlighted = previewHighlighted; }
	
	const wxSVGLength& GetTextOffset() const { return m_textOffset; }
	void SetTextOffset(const wxSVGLength& textOffset) { m_textOffset = textOffset; }
	
	inline bool IsDefaultSize() { return m_defaultSize; }
	inline void SetDefaultSize(bool value = true) { m_defaultSize = value; }
	
	MenuObjectSize& GetDefaultWidth() { return m_defaultWidth; }
	MenuObjectSize& GetDefaultHeight() { return m_defaultHeight; }
	
	MenuObjectSize& GetMinWidth() { return m_minWidth; }
	MenuObjectSize& GetMinHeight() { return m_minHeight; }
	
	wxString GetParam(wxString name, wxString attribute = wxT("")) const;
	void SetParam(wxString name, wxString value, wxString attribute = wxT(""));
	
	int GetParamInt(wxString name, wxString attribute = wxT("")) const;
	void SetParamInt(wxString name, int value, wxString attribute = wxT(""));
	
	double GetParamDouble(wxString name) const;
	void SetParamDouble(wxString name, double value);
	
	wxFont GetParamFont(wxString name) const;
	void SetParamFont(wxString name, wxFont value);
	
	wxColour GetParamColour(wxString name, MenuButtonState state = mbsNORMAL) const;
	void SetParamColour(wxString name, wxColour value, MenuButtonState state = mbsNORMAL);
	
	double GetParamVideoClipBegin(const wxString& name);
	double GetParamVideoDuration(const wxString& name);
	void SetParamImageVideo(const wxString& name, const wxString& filename, long pos, int duration);

	static void SetImageVideoParams(wxSVGSVGElement* svgElem, const wxString& id, const wxString& filename, long pos,
			int duration);
	static const wxArrayString& GetParamTypes();
	
	void SetButtonState(MenuButtonState buttonState);
	MenuButtonState GetButtonState() { return m_buttonState; }
	
protected:
	bool m_deleteSVG;
	wxSVGDocument* m_svg;
	wxArrayString m_idList;
	MenuButtonState m_buttonState;
	
	wxString m_title;
	bool m_previewHighlighted;
	vector<MenuObjectParam*> m_params;
	wxString m_initParameter;
	wxSVGLength m_textOffset;
	
	bool m_defaultSize;
	MenuObjectSize m_defaultWidth;
	MenuObjectSize m_defaultHeight;
	
	MenuObjectSize m_minWidth;
	MenuObjectSize m_minHeight;
	
	wxString ReadNodeContent(wxSvgXmlNode* node);
	void InitSize(wxSvgXmlNode* node, MenuObjectSize& width, MenuObjectSize& height);
};

#endif /* MENUOBJECTDEF_H_ */
