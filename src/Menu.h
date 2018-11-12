/////////////////////////////////////////////////////////////////////////////
// Name:        Menu.h
// Purpose:     The class to store a DVD Menu
// Author:      Alex Thuering
// Created:	    04.11.2003
// RCS-ID:      $Id: Menu.h,v 1.65 2015/10/03 13:29:57 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef MENU_H
#define MENU_H

#include "MenuObject.h"
#include "mediaenc.h"
#include <wxSVG/SVGPreserveAspectRatio.h>
#include <vector>

using namespace std;

enum MenuDrawType {
	mdALL,
	mdBACKGROUND,
	mdBUTTONS_NORMAL,
	mdBUTTONS_HIGHLIGHTED,
	mdBUTTONS_SELECTED
};

WX_DEFINE_ARRAY(MenuObject*, MenuObjects);
class DVD;
class wxProgressDialog;

class Menu {
public:
	/**
	 * Constructor
	 * @param dvd The reference to DVD
	 * @param tsi The index of titleset
	 * @param videoFormat The video format (PAL/NTSC)
	 * @param aspectRatio The aspectRatio (4:3/16:9)
	 */
	Menu(DVD* dvd, int tsi, int pgci, VideoFormat videoFormat = vfPAL, AspectRatio aspectRatio = ar4_3);
	~Menu();
	
	/** Returns menu SVG (graphic) */
	inline wxSVGDocument* GetSVG() { return m_svg; }
	/** Returns menu tsi */
	inline int GetTsi() { return m_tsi; }
	/** Updates menu location. Call it after moving of menus. */
	void UpdateMenuLocation(int tsi, int pgci);
	
	/** Returns video format */
	inline VideoFormat GetVideoFormat() { return m_videoFormat; }
	/** Sets video format */
	void SetVideoFormat(VideoFormat format);
	
	wxSize GetResolution();
	wxSize GetFrameResolution();
	void UpdateResolution(double fx, double fy);
	
	/** Returns aspect ratio */
	inline AspectRatio GetAspectRatio() { return m_aspectRatio; }
	/** Sets aspect ratio */
	void SetAspectRatio(AspectRatio aspectRatio);
	
	inline wxString GetStartTime() { return m_startTime; }
	inline void 	SetStartTime(wxString time) { m_startTime = time; }
	inline wxString GetEndTime() { return m_endTime; }
	inline void 	SetEndTime(wxString time) { m_endTime = time; }
	
	/** Returns filename of background image/video */
	wxString GetBackground();
	/** Sets background image/video */
	void SetBackground(wxString fileName);
	/** Returns align of background image */
	wxSVG_PRESERVEASPECTRATIO GetBackgroundAlign();
	/** Sets align of background image */
	void SetBackgroundAlign(wxSVG_PRESERVEASPECTRATIO align);
	/** Returns meetOrSlice of background image */
    wxSVG_MEETORSLICE GetBackgroundMeetOrSlice();
    /** Sets meetOrSlice of background image */
    void SetBackgroundMeetOrSlice(wxSVG_MEETORSLICE meetOrSlice);
	/** Returns background colour */
	wxColour GetBackgroundColour();
	/** Sets background colour */
	void SetBackgroundColour(wxColour colour);
	/** Returns true if background is a video */
	bool HasVideoBackground();
	
	wxString AddImage(wxString fileName, int x = -1, int y = -1);
	wxString AddText(wxString text, int x = -1, int y = -1);
	wxString AddObject(wxString fileName, wxString param, int x = -1, int y = -1);
	
	/** Returns the count of menu objects inclusive buttons */
	unsigned int GetObjectsCount() { return m_objects.Count(); }
	/** Returns the count of buttons */
	unsigned int GetButtonsCount();
	/** Returns menu object with given index */
	MenuObject* GetObject(int index) { return m_objects[index]; }
	/** Returns menu object with given id */
	MenuObject* GetObject(wxString id);
	/** Returns index of menu object with given id */
	int GetIndexOfObject(wxString id);
	/** Removes menu object with given id */
	void RemoveObject(wxString id);
	/** Moves object to front */
	inline void ObjectToFront(wxString id) { MenuObject* obj = GetObject(id); if (obj) obj->ToFront(); }
	/** Moves object forward */
	inline void ObjectForward(wxString id) { MenuObject* obj = GetObject(id); if (obj) obj->Forward(); }
	/** Moves object backward */
	inline void ObjectBackward(wxString id) { MenuObject* obj = GetObject(id); if (obj) obj->Backward(); }
	/** Moves object to back */
	inline void ObjectToBack(wxString id) { MenuObject* obj = GetObject(id); if (obj) obj->ToBack(); }
	/** Returns true if the object with given id is first */
	inline bool IsFirstObject(wxString id) { MenuObject* obj = GetObject(id); return obj && obj->IsFirst(); }
	/** Returns true if the object with given id is last */
	inline bool IsLastObject(wxString id) { MenuObject* obj = GetObject(id); return obj && obj->IsLast(); }
	/** Returns true if button with given id is the first button */
	bool IsFirstButton(wxString id);
	/** Makes button with given id the first button */
	void SetFirstButton(wxString id);
	
	inline vector<DVDAction*>& GetActions() { return m_actions; }
	inline unsigned int GetActionsCount() { return m_actions.size(); }
	inline DVDAction* GetAction(int index) { return m_actions[index]; }
	
	/** Returns true if the last selected button must be remembered */
	inline bool GetRememberLastButton() { return m_rememberLastButton; }
	/** Sets if the last selected button must be remembered */
	inline void SetRememberLastButton(bool rememberLastButton) { m_rememberLastButton = rememberLastButton; }
	/** Returns g-register for "remember last selected button" */
	inline int GetRememberLastButtonRegister() { return m_rememberLastButtonRegister; }
	/** Sets g-register for "remember last selected button" */
	inline void SetRememberLastButtonRegister(int gregister) { m_rememberLastButtonRegister = gregister; }
	
	/** Checks if buttons overlapping */
	bool isButtonsOverlapping(const wxString& msgPrefix);

	wxImage GetImage(int width, int height, wxProgressDialog* progressDlg = NULL);
	wxImage GetBackgroundImage();
	
	int ReduceColours();
	/** Returns subPictures */
	wxImage* GetSubPictures(SubStreamMode mode);
	wxSVGDocument* GetBackgroundSVG();
	
	bool SaveSpumux(wxString fileName, SubStreamMode mode, wxString btFile, wxString hlFile, wxString selFile);
	wxSvgXmlNode* GetXML(DVDFileType type, wxSvgXmlNode* node = NULL);
	/** Initializes object with XML data */
	bool PutXML(wxSvgXmlNode* node);
	/** Stores object data to string */
	wxString Serialize();
	/** Restores object from data */
	void Deserialize(const wxString& data);
	
	wxString AddObject(wxSvgXmlNode* node, bool fixPosition = false);

	/** Returns button with given jump action */
	MenuObject* GetButtonByJumpAction(int tsi, int pgci, bool isMenu = false);
	/** Returns object displaying video frame of given title */
	MenuObject* GetObjectByTitle(int tsi, int pgci, DVD* dvd);
	/** Updates image in buttons with given jump action */
	bool UpdateButtonImageFor(int actionTsi, int actionPgci, DVD* dvd);
	/** Fix coordinates of buttons if they are out of range */
	void FixButtonCoordinates();
	
	/** Creates and initializes SVG Document */
	static wxSVGDocument* CreateSVG(int width, int height);
private:
	DVD* m_dvd;
	int m_tsi;
	int m_pgci;
	wxSVGDocument* m_svg;
	VideoFormat m_videoFormat;
	AspectRatio m_aspectRatio;
	wxString 	m_startTime, m_endTime;
	MenuObjects m_objects;
	vector<DVDAction*> m_actions;
	bool m_rememberLastButton;
	int m_rememberLastButtonRegister;
	void SetBackground(wxSVGSVGElement* svg, wxString fileName);
	wxString GetBackground(wxSVGSVGElement* svg);
	void FixPosition(MenuObject* obj); /** for copy & paste */
	bool IsDefElement(wxSVGElement* element);
	void RemoveChangeable(wxSVGElement* element, MenuObject* obj);
	bool RemoveNotChangeable(wxSVGElement* element, MenuObject* obj);
	wxSVGSVGElement* GetSVGCopy(MenuDrawType drawType = mdALL);
	wxImage RenderImage(MenuDrawType drawType, int width, int height, const wxSVGRect* rect = NULL,
			bool preserveAspectRatio = false, bool alpha = false, wxProgressDialog* progressDlg = NULL);
	wxImage GetImage(MenuDrawType drawType, int width = -1, int height = -1);
};

#endif // MENU_H
