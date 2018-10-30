/////////////////////////////////////////////////////////////////////////////
// Name:        MenuObject.h
// Purpose:     The class to store a DVD Menu Object
// Author:      Alex Thuering
// Created:	04.11.2006
// RCS-ID:      $Id: MenuObject.h,v 1.34 2016/01/29 16:43:00 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef MENU_OBJECT_H
#define MENU_OBJECT_H

#include <src/SubStream.h>
#include "DVDAction.h"
#include "MenuObjectDef.h"
#include <wx/image.h>
#include <wx/dynarray.h>
#include <wxSVG/SVGLength.h>
#include <vector>
using namespace std;

class wxSvgXmlNode;
class wxSVGDocument;
class wxSVGElement;
class wxSVGUseElement;
class wxSVGSVGElement;
class wxSVGImageElement;
class wxSVGRect;
class wxSVGMatrix;
class wxSVGTransformList;
class wxSVGAnimateElement;
class Menu;
class DVD;


enum NavigationButton {
  nbLEFT = 0,
  nbRIGHT,
  nbUP,
  nbDOWN
};

class MenuObject: public MenuObjectDef {
public:
	/** Constructor */
	MenuObject(Menu* menu, bool vmg, wxString fileName = wxT(""), int x = 0, int y = 0, wxString param = wxT(""));
	/** Destructor */
	virtual ~MenuObject();
	
	/** Returns id of menu object */
	virtual wxString GetId(bool translate = false);
	/** Returns true id this menu object is a button */
	inline bool IsButton() { return m_button; }
	/** Returns button action */
	inline DVDAction& GetAction() { return m_action; }
	
	/** Returns true, if it's an auto-execute button. */
	inline bool IsAutoExecute() { return m_autoExecute; }
	/** Sets, if it's an auto-execute button. */
	inline void SetAutoExecute(bool autoExecute) { m_autoExecute = autoExecute; }
    
    inline wxString GetFileName() { return m_fileName; }
    inline wxString GetTitle() { return m_title; }
    
    int GetX() const;
    void SetX(int value);
    int GetY() const;
    void SetY(int value);
    int GetWidth() const;
    void SetWidth(int value);
    int GetHeight() const;
    void SetHeight(int value);
    void SetRect(wxRect rect);
    double GetAngle() const;
    void SetAngle(double value);
    wxRect GetBBox() const;
    wxRect GetResultBBox() const;
    wxRect GetFrameBBox(SubStreamMode mode, bool ignorePadding = false);
	
	inline bool IsDefaultSize() { return m_defaultSize; }
	inline void SetDefaultSize(bool value = true) { m_defaultSize = value; }
	inline bool IsKeepAspectRatio() { return m_keepAspectRatio; }
	inline void SetKeepAspectRatio(bool value = true) { m_keepAspectRatio = value; }
    void FixSize(int& width, int& height);
    void UpdateSize();
    void UpdateMatrix(wxSVGMatrix& matrix) const;
    
    int GetChangebaleColourCount(bool drawButtonsOnBackground);
    const wxSVGLength& GetTextOffset() const { return m_textOffset; }
    
    /** Returns animation elements */
	vector<wxSVGAnimateElement*> GetAnimations();
	/** Sets animation elements */
	void SetAnimations(vector<wxSVGAnimateElement*>& animations);
	
	/** Returns true, if video frame must be displayed (if button has image parameter and "jump to title" action) */
	inline bool IsDisplayVideoFrame() { return m_displayVideoFrame; }
	/** Sets, if video frame must be displayed */
	inline void SetDisplayVideoFrame(bool displayVideoFrame) { m_displayVideoFrame = displayVideoFrame; }
	/** Returns true, if custom video frame is selected */
	inline bool IsCustomVideoFrame() { return m_customVideoFrame; }
	/** Sets, if custom video frame is selected */
	inline void SetCustomVideoFrame(bool customVideoFrame) { m_customVideoFrame = customVideoFrame; }
	/** Returns VOB-ID of displayed frame */
	inline int GetDisplayVobId() { return m_displayVobId; }
	/** Sets VOB-ID of displayed frame */
	inline void SetDisplayVobId(int displayVobId) { m_displayVobId = displayVobId; }
	
    void ToFront();
    void Forward();
    void Backward();
    void ToBack();
    bool IsFirst();
    bool IsLast();
	
	wxString GetFocusDest(NavigationButton navButton) { return m_direction[navButton]; }
	void SetFocusDest(NavigationButton navButton, wxString value) { m_direction[navButton] = value; }
	wxString GetDefaultFocusDest(NavigationButton navButton);
	
	/** Returns image (thumbnail) */
	wxImage GetImage(int maxWidth, int maxHeight);
	
	virtual wxSvgXmlNode* GetXML(DVDFileType type, DVD* dvd, SubStreamMode mode = ssmNORMAL, bool withSVG = false);
	virtual bool PutXML(wxSvgXmlNode* node);
	
protected:
	Menu* m_menu; // can be null
	wxString m_id;
	bool m_button;
	bool m_autoExecute;
	DVDAction m_action;
	wxString m_fileName;
	
	wxString m_direction[4]; // left, right, up, down button names
	
	bool m_displayVideoFrame; // sets if video frame must be displayed
	bool m_customVideoFrame; // shows if custom video frame is selected
	int m_displayVobId; // VOB-ID of displayed frame
	
	bool m_keepAspectRatio;
	wxSVGImageElement* m_aspectRatioElem;
		
	wxSVGUseElement* m_use;
	wxSVGSVGElement* m_buttonSVG;
	wxSVGSVGElement* AddButtonSVG(wxString id, wxSVGElement* content);
	void AddUseElement(wxString id, int x, int y, int width, int height, const wxSVGTransformList* transforms = NULL);
	virtual wxSVGSVGElement* GetButtonSVG() const;
	
	void SetScale(double scaleX, double scaleY);
	
	bool Init(wxString filename, int x = 0, int y = 0, wxString param = wxT(""));
	bool LoadSVG(wxSVGDocument& svg, wxSvgXmlNode* node);
	wxString GenerateId(wxString prefix);
	unsigned int CalcSize(MenuObjectSize& size, bool width);
	bool IsAlignRight(MenuObjectSize& size);
	void UpdateTransform();
};

#endif // MENU_OBJECT_H
