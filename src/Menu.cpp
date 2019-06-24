/////////////////////////////////////////////////////////////////////////////
// Name:        Menu.cpp
// Purpose:     The class to store a DVD Menu
// Author:      Alex Thuering
// Created:	04.11.2003
// RCS-ID:      $Id: Menu.cpp,v 1.127 2016/12/28 20:10:20 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "Menu.h"
#include "MenuPalettes.h"
#include "Palette3D.h"
#include "DVD.h"
#include "Config.h"
#include "Utils.h"
#include <wxVillaLib/SConv.h>
#include <wxVillaLib/utils.h>
#include <wxSVG/svg.h>
#include <wxSVG/SVGCanvas.h>
#include <wxSVGXML/svgxmlhelpr.h>
#include <wx/mstream.h>
#include <wx/filename.h>
#include <wx/sstream.h>
#include <wx/stdpaths.h>

#define OBJECTS_DIR wxFindDataDirectory(_T("objects"))
#define BACKGROUNDS_DIR wxFindDataDirectory(_T("backgrounds"))

#define BUTTON_MAX_COLORS 4

Menu::Menu(DVD* dvd, int tsi, int pgci, VideoFormat videoFormat, AspectRatio aspectRatio) {
	m_dvd = dvd;
	m_tsi = tsi;
	m_pgci = pgci;
	m_videoFormat = videoFormat;
	m_aspectRatio = aspectRatio;
	m_startTime = _T("00:00:00.00");
	m_rememberLastButton = true;
	m_rememberLastButtonRegister = -1;
	m_svg = CreateSVG(GetResolution().GetWidth(), GetResolution().GetHeight());
	SetBackgroundColour(wxColour(0, 0, 0));
}

Menu::~Menu() {
	delete m_svg;
	WX_CLEAR_ARRAY(m_objects);
	VECTOR_CLEAR(m_actions, DVDAction)
}

wxSVGDocument* Menu::CreateSVG(int width, int height) {
	wxSVGDocument* svg = new wxSVGDocument;
	wxSVGSVGElement* svgElement = new wxSVGSVGElement;
	svgElement->SetWidth(width);
	svgElement->SetHeight(height);
	svg->AppendChild(svgElement);
	wxSVGDefsElement* defsElement = new wxSVGDefsElement;
	defsElement->SetId(wxT("defs"));
	svgElement->AppendChild(defsElement);
	wxSVGGElement* gElem = new wxSVGGElement;
	gElem->SetId(wxT("objects"));
	svgElement->AppendChild(gElem);
	gElem = new wxSVGGElement;
	gElem->SetId(wxT("buttons"));
	svgElement->AppendChild(gElem);
	return svg;
}

/** Updates menu location. Call it after moving of menus. */
void Menu::UpdateMenuLocation(int tsi, int pgci) {
	m_tsi = tsi;
	m_pgci = pgci;
	for (unsigned int obji = 0; obji < m_objects.size(); obji++) {
		m_objects[obji]->GetAction().SetVmg(tsi == -1);
	}
	for (unsigned int actIdx = 0; actIdx < m_actions.size(); actIdx++) {
		m_actions[actIdx]->SetVmg(tsi == -1);
	}
}

void Menu::SetVideoFormat(VideoFormat videoFormat) {
	if (!s_config.GetAllowHdMenues() && videoFormat > vfNTSC)
		videoFormat = isNTSC(videoFormat) ? vfNTSC : vfPAL;
	if (m_videoFormat == videoFormat)
		return;
	wxSize resolutionBefore = GetResolution();
	m_videoFormat = videoFormat;
	wxSize resolution = GetResolution();
	if (resolutionBefore == resolution)
		return;
	UpdateResolution(((double) resolution.GetWidth()) / resolutionBefore.GetWidth(),
			((double) resolution.GetHeight()) / resolutionBefore.GetHeight());
}

void Menu::SetAspectRatio(AspectRatio aspectRatio) {
	if (m_aspectRatio == aspectRatio)
		return;
	wxSize resolutionBefore = GetResolution();
	m_aspectRatio = aspectRatio;
	wxSize resolution = GetResolution();
	if (resolutionBefore == resolution)
		return;
	UpdateResolution(((double) resolution.GetWidth()) / resolutionBefore.GetWidth(),
			((double) resolution.GetHeight()) / resolutionBefore.GetHeight());
}

wxSize Menu::GetResolution() {
	if (m_videoFormat >= vfPAL_FULL_HD) {
		return m_aspectRatio == ar16_9 ? wxSize(1920, 1080) : wxSize(1920, 1440);
	} else if (m_videoFormat >= vfPAL_HDV) {
		return m_aspectRatio == ar16_9 ? wxSize(1440, 810) : wxSize(1440, 1080);
	} else if (m_videoFormat >= vfPAL_HALF_HD) {
		return m_aspectRatio == ar16_9 ? wxSize(1280, 720) : wxSize(1280, 960);
	}
	return m_aspectRatio == ar16_9 ? wxSize(720, 405) : wxSize(720, 540);
}

wxSize Menu::GetFrameResolution() {
	return GetFrameSize(m_videoFormat);
}

void Menu::UpdateResolution(double fx, double fy) {
	// svg root element
	m_svg->GetRootElement()->SetWidth(GetResolution().GetWidth());
	m_svg->GetRootElement()->SetHeight(GetResolution().GetHeight());
	// background
	wxSVGElement* bgElement = m_svg->GetElementById(wxT("backgroundColour"));
	if (bgElement && bgElement->GetDtd() == wxSVG_RECT_ELEMENT) {
		((wxSVGRectElement*) bgElement)->SetWidth(GetResolution().GetWidth());
		((wxSVGRectElement*) bgElement)->SetHeight(GetResolution().GetHeight());
	}
	bgElement = m_svg->GetElementById(wxT("background"));
	if (bgElement) {
		if (bgElement->GetDtd() == wxSVG_IMAGE_ELEMENT) {
			((wxSVGImageElement*) bgElement)->SetWidth(GetResolution().GetWidth());
			((wxSVGImageElement*) bgElement)->SetHeight(GetResolution().GetHeight());
		} else if (bgElement->GetDtd() == wxSVG_VIDEO_ELEMENT) {
			((wxSVGVideoElement*) bgElement)->SetWidth(GetResolution().GetWidth());
			((wxSVGVideoElement*) bgElement)->SetHeight(GetResolution().GetHeight());
		}
	}
	// change object coordinates and size
	for (unsigned int obji = 0; obji < m_objects.GetCount(); obji++) {
		MenuObject* obj = m_objects[obji];
		// update font size
		wxFont font = obj->GetParamFont(wxT("text"));
		font.SetPointSize(round(font.GetPointSize()*fy));
		obj->SetParamFont(wxT("text"), font);
		// update object X/Y-coordinate
		obj->SetX(round(obj->GetX()*fx/2)*2);
		obj->SetY(round(obj->GetY()*fy/2)*2);
		// update object size
		if (!obj->IsDefaultSize() || obj->GetParamByName(wxT("text")) == NULL) {
			obj->SetDefaultSize(false);
			obj->SetWidth(round(obj->GetWidth()*fy));
			obj->SetHeight(round(obj->GetHeight()*fy));
		}
		obj->UpdateSize();
	}
	FixButtonCoordinates();
}

/** Returns filename of background image/video */
wxString Menu::GetBackground() {
	return GetBackground(m_svg->GetRootElement());
}

wxString Menu::GetBackground(wxSVGSVGElement* root) {
	wxSVGElement* bgElement = (wxSVGElement*) root->GetElementById(wxT("background"));
	if (bgElement) {
		if (bgElement->GetDtd() == wxSVG_IMAGE_ELEMENT)
			return ((wxSVGImageElement*) bgElement)->GetHref();
		else if (bgElement->GetDtd() == wxSVG_VIDEO_ELEMENT)
			return ((wxSVGVideoElement*) bgElement)->GetHref();
	}
	return wxT("");
}

/** Sets background image/video */
void Menu::SetBackground(wxString fileName) {
	SetBackground(m_svg->GetRootElement(), fileName);
}

void Menu::SetBackground(wxSVGSVGElement* root, wxString fileName) {
	bool image = wxImage::FindHandler(fileName.AfterLast(wxT('.')).Lower(), BITMAP_TYPE_ANY) != NULL;
	// test if element exists
	wxSVGElement* bgElement = (wxSVGElement*) root->GetElementById(wxT("background"));
	if (fileName.length() == 0) {
		if (bgElement) // remove image/video elment
			bgElement->GetParent()->RemoveChild(bgElement);
		return;
	}
	if (!bgElement || (image && bgElement->GetDtd() != wxSVG_IMAGE_ELEMENT)
			|| (!image && bgElement->GetDtd() != wxSVG_VIDEO_ELEMENT)) {
		if (bgElement) // remove old
			bgElement->GetParent()->RemoveChild(bgElement);
		if (image) { // create image element
			bgElement = new wxSVGImageElement;
			((wxSVGImageElement*) bgElement)->SetWidth(GetResolution().GetWidth());
			((wxSVGImageElement*) bgElement)->SetHeight(GetResolution().GetHeight());
		} else { // create video element
			bgElement = new wxSVGVideoElement;
			((wxSVGVideoElement*) bgElement)->SetWidth(GetResolution().GetWidth());
			((wxSVGVideoElement*) bgElement)->SetHeight(GetResolution().GetHeight());
		}
		bgElement->SetId(wxT("background"));
		root->InsertChild(bgElement,
				root->GetChildren() && ((wxSVGElement*) root->GetChildren())->GetId() == wxT("backgroundColour")
				? root->GetChildren()->GetNextSibling() : root->GetChildren());
	}
	// set href
	if (image)
		((wxSVGImageElement*) bgElement)->SetHref(fileName);
	else
		((wxSVGVideoElement*) bgElement)->SetHref(fileName);
}

/** Returns align of background image */
wxSVG_PRESERVEASPECTRATIO Menu::GetBackgroundAlign() {
	wxSVGElement* bgElement = (wxSVGElement*) m_svg->GetRootElement()->GetElementById(wxT("background"));
	if (bgElement) {
		if (bgElement->GetDtd() == wxSVG_IMAGE_ELEMENT)
			return ((wxSVGImageElement*) bgElement)->GetPreserveAspectRatio().GetBaseVal().GetAlign();
		else if (bgElement->GetDtd() == wxSVG_VIDEO_ELEMENT)
			return ((wxSVGVideoElement*) bgElement)->GetPreserveAspectRatio().GetBaseVal().GetAlign();
	}
	return wxSVG_PRESERVEASPECTRATIO_UNKNOWN;
}

/** Sets align of background image */
void Menu::SetBackgroundAlign(wxSVG_PRESERVEASPECTRATIO align) {
	wxSVGElement* bgElement = (wxSVGElement*) m_svg->GetRootElement()->GetElementById(wxT("background"));
	if (bgElement) {
		if (bgElement->GetDtd() == wxSVG_IMAGE_ELEMENT) {
			wxSVGPreserveAspectRatio ar = ((wxSVGImageElement*) bgElement)->GetPreserveAspectRatio().GetBaseVal();
			ar.SetAlign(align);
			((wxSVGImageElement*) bgElement)->SetPreserveAspectRatio(ar);
		} else if (bgElement->GetDtd() == wxSVG_VIDEO_ELEMENT) {
			wxSVGPreserveAspectRatio ar = ((wxSVGVideoElement*) bgElement)->GetPreserveAspectRatio().GetBaseVal();
			ar.SetAlign(align);
			((wxSVGVideoElement*) bgElement)->SetPreserveAspectRatio(ar);
		}
	}
}

/** Returns meetOrSlice of background image */
wxSVG_MEETORSLICE Menu::GetBackgroundMeetOrSlice() {
	wxSVGElement* bgElement = (wxSVGElement*) m_svg->GetRootElement()->GetElementById(wxT("background"));
	if (bgElement) {
		if (bgElement->GetDtd() == wxSVG_IMAGE_ELEMENT)
			return ((wxSVGImageElement*) bgElement)->GetPreserveAspectRatio().GetBaseVal().GetMeetOrSlice();
		else if (bgElement->GetDtd() == wxSVG_VIDEO_ELEMENT)
			return ((wxSVGVideoElement*) bgElement)->GetPreserveAspectRatio().GetBaseVal().GetMeetOrSlice();
	}
	return wxSVG_MEETORSLICE_UNKNOWN;
}

/** Sets meetOrSlice of background image */
void Menu::SetBackgroundMeetOrSlice(wxSVG_MEETORSLICE meetOrSlice) {
	wxSVGElement* bgElement = (wxSVGElement*) m_svg->GetRootElement()->GetElementById(wxT("background"));
	if (bgElement) {
		if (bgElement->GetDtd() == wxSVG_IMAGE_ELEMENT) {
			wxSVGPreserveAspectRatio ar = ((wxSVGImageElement*) bgElement)->GetPreserveAspectRatio().GetBaseVal();
			ar.SetMeetOrSlice(meetOrSlice);
			((wxSVGImageElement*) bgElement)->SetPreserveAspectRatio(ar);
		} else if (bgElement->GetDtd() == wxSVG_VIDEO_ELEMENT) {
			wxSVGPreserveAspectRatio ar = ((wxSVGVideoElement*) bgElement)->GetPreserveAspectRatio().GetBaseVal();
			ar.SetMeetOrSlice(meetOrSlice);
			((wxSVGVideoElement*) bgElement)->SetPreserveAspectRatio(ar);
		}
	}
}

/** Sets background colour */
void Menu::SetBackgroundColour(wxColour colour) {
	wxSVGRectElement* bgRect = (wxSVGRectElement*) m_svg->GetElementById(wxT("backgroundColour"));
	if (!bgRect || bgRect->GetDtd() != wxSVG_RECT_ELEMENT) {
		if (bgRect)
			bgRect->GetParent()->RemoveChild(bgRect);
		bgRect = new wxSVGRectElement;
		bgRect->SetId(wxT("backgroundColour"));
		bgRect->SetWidth(GetResolution().GetWidth());
		bgRect->SetHeight(GetResolution().GetHeight());
		m_svg->GetRootElement()->InsertChild(bgRect,
				m_svg->GetRootElement()->GetChildren());
	}
	bgRect->SetFill(wxSVGPaint(colour.Red(), colour.Green(), colour.Blue()));
}

wxColour Menu::GetBackgroundColour() {
	wxSVGRectElement* bgRect = (wxSVGRectElement*) m_svg->GetElementById(wxT("backgroundColour"));
	if (bgRect && bgRect->GetDtd() == wxSVG_RECT_ELEMENT)
		return bgRect->GetFill().GetRGBColor();
	return wxColour(0, 0, 0);
}

bool Menu::HasVideoBackground() {
	wxSVGElement* bgElement = m_svg->GetElementById(wxT("background"));
	return bgElement && bgElement->GetDtd() == wxSVG_VIDEO_ELEMENT;
}

bool Menu::IsDefElement(wxSVGElement* element) {
	if (!element || element->GetDtd() == wxSVG_SVG_ELEMENT)
		return false;
	if (element->GetDtd() == wxSVG_DEFS_ELEMENT)
		return true;
	return IsDefElement((wxSVGElement*) element->GetParent());
}

void Menu::RemoveChangeable(wxSVGElement* element, MenuObject* obj) {
	wxSVGElement* child = (wxSVGElement*) element->GetChildren();
	while (child) {
		wxSVGElement* elem = child;
		child = (wxSVGElement*) child->GetNext();
		if (elem->GetType() != wxSVGXML_ELEMENT_NODE)
			continue;
		// don't remove def elements
		if (IsDefElement(elem))
			continue;
		// check if elem changeable
		if (elem->GetId().length()) {
			// check if element has changeable attributes
			bool isChangeable = false;
			for (unsigned int i = 0; i < obj->GetParams().size(); i++) {
				MenuObjectParam* param = obj->GetParams()[i];
				if (!param->isChangeable())
					continue;
				for (vector<wxString>::const_iterator elemIt = param->element.begin();
						elemIt != param->element.end(); elemIt++) {
					if (*elemIt == elem->GetId()) {
						elem->SetAttribute(param->attribute, wxT("none"));
						isChangeable = true;
					}
				}
			}
			if (isChangeable)
				continue;
		}
		// remove changeable children
		RemoveChangeable(elem, obj);
	}
}

bool Menu::RemoveNotChangeable(wxSVGElement* element, MenuObject* obj) {
	wxSVGElement* child = (wxSVGElement*) element->GetChildren();
	while (child) {
		wxSVGElement* elem = child;
		child = (wxSVGElement*) child->GetNext();
		if (elem->GetType() != wxSVGXML_ELEMENT_NODE) {
			elem->GetParent()->RemoveChild(elem);
			continue;
		}
		// don't remove def elements
		if (IsDefElement(elem))
			continue;
		// check if child changeable
		if (elem->GetId().length()) {
			// check if element has changeable attributes
			bool isChangeable = false;
			for (unsigned int i = 0; i < obj->GetParams().size(); i++) {
				MenuObjectParam* param = obj->GetParams()[i];
				for (vector<wxString>::const_iterator elemIt = param->element.begin();
						elemIt != param->element.end(); elemIt++) {
					if (*elemIt == elem->GetId() && param->changeable) {
						if (param->normalColour == param->highlightedColour
								&& param->normalColour == param->selectedColour)
							elem->SetAttribute(param->attribute, wxT("none"));
						else
							isChangeable = true;
					}
				}
			}
			if (isChangeable) {
				// todo: remove not changeable attributes: SetFill(none),SetStroke(none)
				continue;
			}
		}
		// check if it has changeable children
		if (RemoveNotChangeable(elem, obj))
			elem->GetParent()->RemoveChild(elem);
	}
	return element->GetChildren() == NULL; // return if has no children
}

void CopyCachedImages(wxSVGDocument* doc, wxSVGElement* parent1, wxSVGElement* parent2) {
	wxSVGElement* elem1 = (wxSVGElement*) parent1->GetChildren();
	wxSVGElement* elem2 = (wxSVGElement*) parent2->GetChildren();
	while (elem1 && elem2) {
		if (elem1->GetType() == wxSVGXML_ELEMENT_NODE && elem1->GetDtd() == wxSVG_IMAGE_ELEMENT) {
			wxSVGImageElement* img1 = (wxSVGImageElement*) elem1;
			wxSVGImageElement* img2 = (wxSVGImageElement*) elem2;
			if (img1->GetHref().GetAnimVal().length() && img1->GetCanvasItem() != NULL)
				img2->SetCanvasItem(doc->GetCanvas()->CreateItem(img1));
		} else if (elem1->GetChildren())
			CopyCachedImages(doc, elem1, elem2);
		elem1 = (wxSVGElement*) elem1->GetNext();
		elem2 = (wxSVGElement*) elem2->GetNext();
	}
}

wxSVGSVGElement* Menu::GetSVGCopy(MenuDrawType drawType) {
	wxSVGSVGElement* svgNode = (wxSVGSVGElement*) m_svg->GetRoot()->CloneNode();
	CopyCachedImages(m_svg, (wxSVGElement*) m_svg->GetRoot(), svgNode);
	// remove selection rectangles
	wxSVGElement* child = (wxSVGElement*) svgNode->GetChildren();
	while (child != NULL) {
		wxSVGElement* childToRemove = child->GetId().StartsWith(wxT("selection_")) ? child : NULL;
		child = (wxSVGElement*) child->GetNextSibling();
		if (childToRemove)
			svgNode->RemoveChild(childToRemove);
	}
	// remove safeTV rectangle
	wxSvgXmlElement* elem = svgNode->GetElementById(wxT("safeTV"));
	if (elem)
		elem->GetParent()->RemoveChild(elem);
	// remove grid
	elem = svgNode->GetElementById(wxT("grid"));
	if (elem)
		elem->GetParent()->RemoveChild(elem);
	
	if (drawType == mdBACKGROUND) {
		// remove changeable elements of objects
		for (unsigned int i = 0; i < GetObjectsCount(); i++) {
			MenuObject* obj = GetObject(i);
			if (obj->IsButton()) {
				if (obj->GetAction().IsValid(m_dvd, m_tsi, m_pgci, true)) {
					// remove dynamic part of button
					wxSVGElement* elem = (wxSVGElement*) svgNode->GetElementById(wxT("s_") + obj->GetId());
					if (elem && !s_config.GetDrawButtonsOnBackground())
						RemoveChangeable(elem, obj);
				} else {
					// remove buttons with invalid actions
					wxSVGElement* elem = (wxSVGElement*) svgNode->GetElementById(obj->GetId());
					if (elem && elem->GetParent())
						elem->GetParent()->RemoveChild(elem);
					elem = (wxSVGElement*) svgNode->GetElementById(wxT("s_") + obj->GetId());
					if (elem && elem->GetParent())
						elem->GetParent()->RemoveChild(elem);
				}
			}
		}
	} else if (drawType == mdBUTTONS_NORMAL || drawType == mdBUTTONS_HIGHLIGHTED || drawType == mdBUTTONS_SELECTED) {
		// remove background and static elements of objects
		wxSVGElement* bgElement = (wxSVGElement*) svgNode->GetElementById(wxT("backgroundColour"));
		if (bgElement)
			bgElement->GetParent()->RemoveChild(bgElement);
		bgElement = (wxSVGElement*) svgNode->GetElementById(wxT("background"));
		if (bgElement)
			bgElement->GetParent()->RemoveChild(bgElement);
		for (unsigned int i = 0; i < GetObjectsCount(); i++) {
			MenuObject* obj = GetObject(i);
			if (obj->IsButton() && obj->GetAction().IsValid(m_dvd, m_tsi, m_pgci, true)
					&& (!s_config.GetDrawButtonsOnBackground() || drawType != mdBUTTONS_NORMAL)) {
				wxSVGSVGElement* symbol = (wxSVGSVGElement*)svgNode->GetElementById(wxT("s_") + obj->GetId());
				if (symbol)
					RemoveNotChangeable(symbol, obj);
				for (unsigned int i = 0; i < obj->GetParams().size(); i++) {
					MenuObjectParam* param = obj->GetParams()[i];
					if (param->isChangeable() && drawType != mdBUTTONS_NORMAL) {
						for (vector<wxString>::const_iterator elemIt = param->element.begin();
								elemIt != param->element.end(); elemIt++) {
							wxSVGElement* elem = (wxSVGElement*) symbol->GetElementById(*elemIt);
							if (elem && param->attribute.length()) {
								wxSVGPaint paint(drawType == mdBUTTONS_SELECTED ? param->selectedColour
										: param->highlightedColour);
								elem->SetAttribute(param->attribute, paint.GetCSSText());
							}
						}
					}
				}
			} else {
				wxSVGElement* elem = (wxSVGElement*) svgNode->GetElementById(obj->GetId());
				if (elem && elem->GetParent())
					elem->GetParent()->RemoveChild(elem);
				elem = (wxSVGElement*) svgNode->GetElementById(wxT("s_") + obj->GetId());
				if (elem && elem->GetParent())
					elem->GetParent()->RemoveChild(elem);
			}
		}
	}

	return svgNode;
}

////////////////////////////// Render ////////////////////////////////////////

wxImage Menu::RenderImage(MenuDrawType drawType, int width, int height, const wxSVGRect* rect,
		bool preserveAspectRatio, bool alpha, wxProgressDialog* progressDlg) {
	wxSVGDocument svg;
	svg.AppendChild(GetSVGCopy(drawType));
	return svg.Render(width, height, rect, preserveAspectRatio, alpha, progressDlg);
}

wxImage Menu::GetImage(int width, int height, wxProgressDialog* progressDlg) {
	return RenderImage(mdALL, width, height, NULL, true, false, progressDlg);
}

wxImage Menu::GetBackgroundImage() {
	int width = GetFrameResolution().GetWidth();
	int height = GetFrameResolution().GetHeight();
	return RenderImage(mdBACKGROUND, width, height);
}

int Menu::ReduceColours() {
	// create 3d palette
	Palette3D palette;
	for (unsigned int i = 0; i < GetObjectsCount(); i++) {
		MenuObject& obj = *GetObject(i);
		if (!obj.IsButton() || !obj.GetAction().IsValid(m_dvd, m_tsi, m_pgci, true))
			continue;
		for (unsigned int j = 0; j < obj.GetParams().size(); j++) {
			MenuObjectParam* param = obj.GetParams()[j];
			if (param->isChangeable()) {
				palette.Add(s_config.GetDrawButtonsOnBackground() ? wxColour() : param->normalColour,
						param->highlightedColour, param->selectedColour);
			}
		}
	}
	palette.Add(wxColour(), wxColour(), wxColour());
	wxLogMessage(wxT("Menu has %d group(s) of changeable colours."), palette.GetColoursCount() - 1);
	// reduce the number of colours
	if (palette.GetColoursCount() <= BUTTON_MAX_COLORS)
		return palette.GetColoursCount();
	wxLogMessage(wxT("Warning: Count of changeable colours will be reduced to %d groups."), BUTTON_MAX_COLORS - 1);
	palette.ReduceColours(BUTTON_MAX_COLORS);
	// apply palette
	for (unsigned int i = 0; i < GetObjectsCount(); i++) {
		MenuObject& obj = *GetObject(i);
		if (!obj.IsButton() || !obj.GetAction().IsValid(m_dvd, m_tsi, m_pgci, true))
			continue;
		for (unsigned int j = 0; j < obj.GetParams().size(); j++) {
			MenuObjectParam* param = obj.GetParams()[j];
			if (param->isChangeable()) {
				wxColour tmp;
				if (palette.Apply(param, s_config.GetDrawButtonsOnBackground())) {
					if (!s_config.GetDrawButtonsOnBackground()) 
						obj.SetParamColour(param->name, param->normalColour, mbsNORMAL);
					obj.SetParamColour(param->name, param->highlightedColour, mbsHIGHLIGHTED);
					obj.SetParamColour(param->name, param->selectedColour, mbsSELECTED);
				}
			}
		}
	}
	
	return BUTTON_MAX_COLORS;
}

inline unsigned char GetAlpha(int alphaCount, unsigned char srcAlpha) {
	if (alphaCount == 3)
		return srcAlpha > 192 ? 255 : srcAlpha > 127 ? 170 : 85;
	else
		return 255;
}

/** Returns subPictures */
wxImage* Menu::GetSubPictures(SubStreamMode mode) {
	ReduceColours();
	// render images
	int width = GetFrameResolution().GetWidth();
	int height = GetFrameResolution().GetHeight();
	int padY = 0;
	wxSVGRect* cropRect = NULL;
	if (mode == ssmLETTERBOX) {
		padY = height*0.125;
		height *= 0.75;
	} else if (mode == ssmPANSCAN) {
		wxSize res = GetResolution();
		cropRect = new wxSVGRect(res.GetWidth()*0.125, 0, res.GetWidth()*0.75, res.GetHeight());
		width *= 4.0/3; // increase width to get same width as without crop rect
	}
	wxImage* images = new wxImage[3];
	images[0] = RenderImage(mdBUTTONS_NORMAL, width, height, cropRect, false, true);
	images[1] = RenderImage(mdBUTTONS_HIGHLIGHTED, width, height, cropRect, false, true);
	images[2] = RenderImage(mdBUTTONS_SELECTED, width, height, cropRect, false, true);

	// make aliasing for buttons
	for (unsigned int i = 0; i < GetObjectsCount(); i++) {
		MenuObject& obj = *GetObject(i);
		if (!obj.IsButton() || !obj.GetAction().IsValid(m_dvd, m_tsi, m_pgci, true))
			continue;
		// make palette
		MenuPalettes objPalette(obj, s_config.GetDrawButtonsOnBackground());
		int alphaCount = obj.GetChangebaleColourCount(s_config.GetDrawButtonsOnBackground()) == 1 ? 3 : 1;

		// apply palette
		wxRect rect = obj.GetFrameBBox(mode, true);
		unsigned char* img1 = images[0].GetData() + rect.GetY() * images[0].GetWidth() * 3 + rect.GetX() * 3;
		unsigned char* img1Alpha = images[0].GetAlpha() + rect.GetY() * images[0].GetWidth() + rect.GetX();
		unsigned char* img2 = images[1].GetData() + rect.GetY() * images[1].GetWidth() * 3 + rect.GetX() * 3;
		unsigned char* img2Alpha = images[1].GetAlpha() + rect.GetY() * images[1].GetWidth() + rect.GetX();
		unsigned char* img3 = images[2].GetData() + rect.GetY() * images[2].GetWidth() * 3 + rect.GetX() * 3;
		unsigned char* img3Alpha = images[2].GetAlpha() + rect.GetY() * images[2].GetWidth() + rect.GetX();
		for (int y = 0; y < rect.GetHeight(); y++) {
			for (int x = 0; x < rect.GetWidth(); x++) {
				unsigned char alpha = 0;
				objPalette.Apply(img1, img1, img1Alpha);
				if (*img1Alpha > 0) {
					alpha = GetAlpha(alphaCount, *img1Alpha);
					*img1Alpha = alpha;
				}
				objPalette.Apply(img1, *img1Alpha, img2, img2, img2Alpha);
				if (*img2Alpha > 0) {
					if (alpha == 0)
						alpha = GetAlpha(alphaCount, *img2Alpha);
					*img2Alpha = alpha;
				}
				objPalette.Apply(img1, *img1Alpha, img2, *img2Alpha, img3, img3, img3Alpha);
				if (*img3Alpha > 0) {
					if (alpha == 0)
						alpha = GetAlpha(alphaCount, *img3Alpha);
					*img3Alpha = alpha;
				}
				img1 += 3;
				img1Alpha++;
				img2 += 3;
				img2Alpha++;
				img3 += 3;
				img3Alpha++;
			}
			img1 += (images[0].GetWidth() - rect.GetWidth()) * 3;
			img1Alpha += images[0].GetWidth() - rect.GetWidth();
			img2 += (images[1].GetWidth() - rect.GetWidth()) * 3;
			img2Alpha += images[1].GetWidth() - rect.GetWidth();
			img3 += (images[2].GetWidth() - rect.GetWidth()) * 3;
			img3Alpha += images[2].GetWidth() - rect.GetWidth();
		}
	}

	// all pixels that don't belong to objects must be transparent
	unsigned char* img1 = images[0].GetAlpha();
	unsigned char* img2 = images[1].GetAlpha();
	unsigned char* img3 = images[2].GetAlpha();
	for (int y = 0; y < images[0].GetHeight(); y++) {
		for (int x = 0; x < images[0].GetWidth(); x++) {
			bool found = false;
			for (unsigned int i = 0; i < GetObjectsCount(); i++) {
				MenuObject& obj = *GetObject(i);
				if (!obj.IsButton())
					continue;
				
				wxRect rect = obj.GetFrameBBox(mode, true);
				if (x >= rect.GetX() && x < rect.GetX() + rect.GetWidth()
						&& y >= rect.GetY() && y < rect.GetY() + rect.GetHeight()) {
					found = true;
					break;
				}
			}
			if (!found) {
				*img1 = 0;
				*img2 = 0;
				*img3 = 0;
			}
			img1++;
			img2++;
			img3++;
		}
	}
	
	if (mode == ssmLETTERBOX) {
		for (int i = 0; i < 3; i++) {
			wxSize size(images[i].GetWidth(), images[i].GetHeight() + 2*padY);
			wxPoint pos(0, padY);
			images[i].Resize(size, pos);
		}
	}

	return images;
}


wxSVGDocument* Menu::GetBackgroundSVG() {
	wxSVGDocument* svg = new wxSVGDocument;
	svg->AppendChild(GetSVGCopy(mdBACKGROUND));
	return svg;
}

/** Returns the count of buttons */
unsigned int Menu::GetButtonsCount() {
	int count = 0;
	for (int i=0; i<(int)m_objects.Count(); i++)
		if (m_objects[i]->IsButton())
			count++;
	return count;
}

/** Returns menu object with given id */
MenuObject* Menu::GetObject(wxString id) {
	int idx = GetIndexOfObject(id);
	return idx >= 0 ? m_objects[idx] : NULL;
}

/** Returns index of menu object with given id */
int Menu::GetIndexOfObject(wxString id) {
	for (int i = 0; i < (int) m_objects.Count(); i++)
		if (m_objects[i]->GetId() == id)
			return i;
	return -1;
}

/** Removes menu object with given id */
void Menu::RemoveObject(wxString id) {
	MenuObject* obj = GetObject(id);
	if (obj) {
		m_objects.Remove(obj);
		delete obj;
	}
}


/** Returns true if button with given id is the first button */
bool Menu::IsFirstButton(wxString id) {
	for (int i=0; i<(int)m_objects.Count(); i++)
		if (m_objects[i]->IsButton())
			return m_objects[i]->GetId() == id;
	return false;
}

/** Makes button with given id the first button */
void Menu::SetFirstButton(wxString id) {
	MenuObject* obj = GetObject(id);
	m_objects.Remove(obj);
	m_objects.Insert(obj, 0);
}

wxString Menu::AddImage(wxString fileName, int x, int y) {
	return AddObject(OBJECTS_DIR + wxT("/image.xml"), fileName, x, y);
}

wxString Menu::AddText(wxString text, int x, int y) {
	return AddObject(OBJECTS_DIR + wxT("/text-v2.xml"), text, x, y);
}

wxString Menu::AddObject(wxString fileName, wxString param, int x, int y) {
	MenuObject* obj = new MenuObject(this, m_tsi == -1, fileName, x, y, param);
	m_objects.Add(obj);
	return obj->GetId();
}

/** Returns button with given jump action */
MenuObject* Menu::GetButtonByJumpAction(int actionTsi, int actionPgci, bool actionIsMenu) {
	for (unsigned int obji = 0; obji < m_objects.size(); obji++) {
		MenuObject* obj = m_objects[obji];
		if (obj->IsButton() && !obj->GetAction().IsCustom() && obj->GetAction().GetTsi() == actionTsi
				&& obj->GetAction().GetPgci() == actionPgci && obj->GetAction().IsMenu() == actionIsMenu) {
			return obj;
		}
	}
	return NULL;
}

/** Updates image in buttons with given jump action */
bool Menu::UpdateButtonImageFor(int actionTsi, int actionPgci, DVD* dvd) {
	Pgc* pgc = dvd->GetPgc(actionTsi == -2 ? m_tsi : actionTsi, false, actionPgci);
	int vobId = DVD::MakeId(actionTsi == -2 ? m_tsi : actionTsi, actionPgci, 0);
	bool result = false;
	for (unsigned int obji = 0; obji < m_objects.size(); obji++) {
		MenuObject* obj = m_objects[obji];
		MenuObjectParam* param = obj->GetImageParam();
		if (param == NULL)
			continue;
		DVDAction& action = obj->GetAction();
		bool isUpdateButton = obj->IsButton() && !action.IsCustom() && obj->IsDisplayVideoFrame()
				&& action.GetTsi() == actionTsi && action.GetPgci() == actionPgci && !action.IsMenu();
		MenuObjectParam* imgParam = obj->GetImageParam();
		bool isUpdateObject = !obj->IsButton() && imgParam != NULL && obj->IsDisplayVideoFrame()
				&& obj->GetDisplayVobId() == vobId;
		if (isUpdateButton || isUpdateObject) {
			result = true; // button/object is found
			wxString uri;
			long int pos = 0;
			if (pgc != NULL) {
				if (isUpdateButton) {
					uri = pgc->GetVideoFrameUri(action.GetChapter());
					if (uri.Find(wxT('#')) >= 0) {
						uri.AfterLast(wxT('#')).ToLong(&pos);
						uri = uri.BeforeLast(wxT('#'));
					}
				} else {
					uri = pgc->GetVideoFrameUri(-1, -1);
				}
			}
			wxString objUri = obj->GetParam(param->name);
			objUri = objUri.Find(wxT('#')) >= 0 ? objUri.BeforeLast(wxT('#')) : objUri;
			
			// update image if custom video frame isn't selected and filename isn't changed
			if (!obj->IsCustomVideoFrame() || uri.length() == 0 || uri != objUri) {
				double dur = obj->GetParamVideoDuration(param->name);
				obj->SetParamImageVideo(param->name, uri, pos, (int) lround(dur));
				obj->UpdateSize();
			}
		}
	}
	return result;
}

/** Returns object displaying video frame of given title */
MenuObject* Menu::GetObjectByTitle(int tsi, int pgci, DVD* dvd) {
	int vobId = DVD::MakeId(tsi == -2 ? m_tsi : tsi, pgci, 0);
	for (unsigned int obji = 0; obji < m_objects.size(); obji++) {
		MenuObject* obj = m_objects[obji];
		if (obj->IsButton() || !obj->IsDisplayVideoFrame() || obj->GetImageParam() == NULL)
			continue;
		if (vobId == obj->GetDisplayVobId())
			return obj;
	}
	return NULL;
}

/** Fix coordinates of buttons if they are out of range */
void Menu::FixButtonCoordinates() {
	for (unsigned int obji = 0; obji < m_objects.GetCount(); obji++) {
		MenuObject* obj = m_objects[obji];
		if (obj->IsButton()) {
			if (obj->GetX() < 0)
				obj->SetX(0);
			if (obj->GetY() < 0)
				obj->SetY(0);
			if (obj->GetX() + obj->GetWidth() >= GetResolution().GetWidth())
				obj->SetX(GetResolution().GetWidth() - obj->GetWidth() - 1);
			if (obj->GetY() + obj->GetHeight() >= GetResolution().GetHeight())
				obj->SetY(GetResolution().GetHeight() - obj->GetHeight() - 1);
		}
	}
}

/** Checks if buttons overlapping */
bool Menu::isButtonsOverlapping(const wxString& msgPrefix) {
	for (unsigned int obji = 0; obji < m_objects.GetCount(); obji++) {
		MenuObject* obj = m_objects[obji];
		if (!obj->IsButton())
			continue;
		for (unsigned int obji2 = 0; obji2 < m_objects.GetCount(); obji2++) {
			if (obji == obji2)
				continue;
			MenuObject* obj2 = m_objects[obji2];
			if (!obj2->IsButton())
				continue;
			if (obj->GetResultBBox().Intersects(obj2->GetResultBBox())) {
				wxLogError(msgPrefix + _("%s and %s are overlapping"), obj->GetId(true).c_str(),
						obj2->GetId(true).c_str());
				return true;
			}
		}
	}
	return false;
}

bool Menu::SaveSpumux(wxString fileName, SubStreamMode mode, wxString btFile, wxString hlFile, wxString selFile) {
	wxSvgXmlDocument xml;
	wxSvgXmlNode* root = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, _T("subpictures"));
	root->AddProperty(wxT("format"), isNTSC(m_videoFormat) ? wxT("NTSC") : wxT("PAL"));
	wxSvgXmlNode* streamNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, _T("stream"));
	wxSvgXmlNode* spuNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, _T("spu"));
	if (GetStartTime().Length())
		spuNode->AddProperty(_T("start"), GetStartTime());
	if (GetEndTime().Length())
		spuNode->AddProperty(_T("end"), GetEndTime());
	spuNode->AddProperty(_T("image"), btFile);
	spuNode->AddProperty(_T("highlight"), hlFile);
	spuNode->AddProperty(_T("select"), selFile);
	spuNode->AddProperty(_T("force"), _T("yes"));
	if (s_config.GetButtonsOffset2px()) {
		spuNode->AddProperty(_T("xoffset"), _T("2"));
		spuNode->AddProperty(_T("yoffset"), _T("2"));
	}
	// buttons
	for (int i=0; i<(int)GetObjectsCount(); i++) {
		MenuObject* obj = GetObject(i);
		if (obj->IsButton() && obj->GetAction().IsValid(m_dvd, m_tsi, m_pgci, true, obj->GetId(), false))
			spuNode->AddChild(obj->GetXML(SPUMUX_XML, m_dvd, mode));
	}
	// actions
	for (unsigned int i=0; i<m_actions.size(); i++)
		spuNode->AddChild(m_actions[i]->GetXML(SPUMUX_XML, NULL));
	streamNode->AddChild(spuNode);
	root->AddChild(streamNode);
	xml.SetRoot(root);
	return xml.Save(fileName);
}

wxSvgXmlNode* Menu::GetXML(DVDFileType type, wxSvgXmlNode* node) {
	if (node == NULL)
		node = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, _T("menu"));
  
	if (type == DVDSTYLER_XML) {
		node->AddProperty(wxT("videoFormat"), (m_videoFormat == vfPAL) ? _T("PAL") : _T("NTSC"));
		node->AddProperty(wxT("aspectRatio"), wxString::Format(wxT("%d"), m_aspectRatio));
		if (GetStartTime().Length() && GetStartTime() != wxT("00:00:00.00"))
			node->AddProperty(_T("startTime"), GetStartTime());
		if (GetEndTime().Length())
			node->AddProperty(_T("endTime"), GetEndTime());
		if (m_rememberLastButton)
			node->AddProperty(wxT("rememberLastButton"), wxString::Format(wxT("%d"), m_rememberLastButtonRegister));
		
		// add svg
		if (m_svg && m_svg->GetRoot()) {
			wxSVGSVGElement* svg = GetSVGCopy();
			// fix path
			wxString bgFile = GetBackground(svg);
			if (bgFile.length() > 0) {
				wxString dir = BACKGROUNDS_DIR;
				wxString bgDir = wxStandardPaths::Get().GetUserDataDir() + wxFILE_SEP_PATH + wxT("backgrounds")
						+ wxFILE_SEP_PATH;
				if (bgFile.StartsWith(dir))
	 				SetBackground(svg, bgFile.substr(dir.length()));
				else if (bgFile.StartsWith(bgDir))
	 				SetBackground(svg, bgFile.substr(bgDir.length()));
			}
			// set relative path
			wxSVGElement* elem = svg;
			while (elem != NULL) {
				if (elem->GetDtd() == wxSVG_IMAGE_ELEMENT || elem->GetDtd() == wxSVG_VIDEO_ELEMENT) {
					wxSVGURIReference* uriElem = NULL;
					if (elem->GetDtd() == wxSVG_IMAGE_ELEMENT)
						uriElem = (wxSVGImageElement*) elem;
					else // elem->GetDtd() == wxSVG_VIDEO_ELEMENT
						uriElem = (wxSVGVideoElement*) elem;
					wxString href = uriElem->GetHref();
					wxString frameStr;
					if (href.Find(wxT('#')) > 0) {
						frameStr = wxT('#') + href.AfterLast(wxT('#'));
						href = href.BeforeLast(wxT('#'));
					}
					wxFileName fname(href);
					if (fname.GetPath() == m_dvd->GetPath(false))
						uriElem->SetHref(fname.GetFullName() + frameStr);
					else if (m_dvd->GetPath(false).length() > 0 && fname.GetPath().StartsWith(m_dvd->GetPath(false)))
						uriElem->SetHref(fname.GetPath().substr(m_dvd->GetPath(false).length() + 1)
								+ wxFILE_SEP_PATH + fname.GetFullName() + frameStr);
				}
				if (elem->GetChildren() != NULL && (elem->GetDtd() == wxSVG_SVG_ELEMENT
						|| elem->GetDtd() == wxSVG_DEFS_ELEMENT || elem->GetDtd() == wxSVG_G_ELEMENT)) {
					elem = (wxSVGElement*)elem->GetChildren();
				} else {
					while (elem != NULL && elem->GetNextSibling() == NULL)
						elem = (wxSVGElement*) elem->GetParent();
					if (elem != NULL)
						elem = (wxSVGElement*) elem->GetNextSibling();
				}
			}
			// append node
			node->AppendChild(svg);
		}
	}
	
	// add buttons info (action, etc.)
	for (unsigned int i=0; i<GetObjectsCount(); i++) {
		MenuObject* obj = GetObject(i);
		if (type == DVDSTYLER_XML || (obj->IsButton() && obj->GetAction().IsValid(m_dvd, m_tsi, m_pgci, true)))
			node->AddChild(obj->GetXML(type, m_dvd, ssmNORMAL, false));
	}
	
	// actions
	for (unsigned int i=0; i<m_actions.size(); i++)
		node->AddChild(m_actions[i]->GetXML(type, m_dvd));
	
	return node;
}

bool Menu::PutXML(wxSvgXmlNode* node) {
	if (node->GetName() == _T("spumux"))
		node = node->GetChildren();
	if (node != NULL && node->GetName() == _T("menu")) {
		wxString val;
		long lval;
		m_videoFormat = vfPAL;
		if (node->GetPropVal(wxT("videoFormat"), &val) && val == wxT("NTSC"))
			m_videoFormat = vfNTSC;
		if (node->GetPropVal(wxT("aspectRatio"), &val) && val.ToLong(&lval))
		    m_aspectRatio = AspectRatio(lval);
		node->GetPropVal(wxT("startTime"), &m_startTime);
		node->GetPropVal(wxT("endTime"), &m_endTime);
		m_rememberLastButton = node->GetPropVal(wxT("rememberLastButton"), &val) && val.ToLong(&lval);
		if (m_rememberLastButton)
			m_rememberLastButtonRegister = lval;
		
		wxSvgXmlNode* svgNode = XmlFindNode(node, wxT("svg"));
		if (svgNode) {
			wxSvgXmlDocument xml;
			xml.SetRoot(svgNode->CloneNode());
			wxMemoryOutputStream output;
			xml.Save(output);
			wxMemoryInputStream input(output);
			m_svg->Load(input);
			// fix background
			wxSVGRectElement* bgRect = (wxSVGRectElement*) m_svg->GetElementById(wxT("background"));
			if (bgRect && bgRect->GetDtd() == wxSVG_RECT_ELEMENT)
				bgRect->SetId(wxT("backgroundColour"));
			// fix path
			if (GetBackground().length() > 0) {
				wxString bgDir = wxStandardPaths::Get().GetUserDataDir() + wxFILE_SEP_PATH + wxT("backgrounds")
						+ wxFILE_SEP_PATH;
				wxFileName fn(GetBackground());
				if (fn.IsRelative() && !GetBackground().StartsWith(wxT("concat:"))) {
					if (wxFileExists(m_dvd->GetPath() + fn.GetFullPath()))
						SetBackground(m_dvd->GetPath() + fn.GetFullPath());
					else if (wxFileExists(m_dvd->GetPath() + fn.GetFullName()))
						SetBackground(m_dvd->GetPath() + fn.GetFullName());
					else if (wxFileExists(bgDir + fn.GetFullName()))
						SetBackground(bgDir + fn.GetFullName());
					else if (wxFileExists(BACKGROUNDS_DIR + fn.GetFullName()))
						SetBackground(BACKGROUNDS_DIR + fn.GetFullName());
					else
						SetBackground(m_dvd->GetPath() + fn.GetFullPath());
				}
			}
			// fix relative path
			wxSVGElement* elem = m_svg->GetRootElement();
			while (elem != NULL) {
				if (elem->GetDtd() == wxSVG_IMAGE_ELEMENT || elem->GetDtd() == wxSVG_VIDEO_ELEMENT) {
					wxSVGURIReference* uriElem = NULL;
					if (elem->GetDtd() == wxSVG_IMAGE_ELEMENT)
						uriElem = (wxSVGImageElement*) elem;
					else // elem->GetDtd() == wxSVG_VIDEO_ELEMENT
						uriElem = (wxSVGVideoElement*) elem;
					wxString href = uriElem->GetHref();
					if (href.length() && !href.StartsWith(wxT("concat:"))) {
						wxString frameStr;
						if (href.Find(wxT('#')) > 0) {
							frameStr = wxT('#') + href.AfterLast(wxT('#'));
							href = href.BeforeLast(wxT('#'));
						}
						wxFileName fname(href);
						if (fname.IsRelative())
							uriElem->SetHref(m_dvd->GetPath() + fname.GetFullPath() + frameStr);
						else if (!wxFileExists(href) && wxFileExists(m_dvd->GetPath() + fname.GetFullName()))
							uriElem->SetHref(m_dvd->GetPath() + fname.GetFullName() + frameStr);
					}
				}
				if (elem->GetChildren() != NULL && (elem->GetDtd() == wxSVG_SVG_ELEMENT
						|| elem->GetDtd() == wxSVG_DEFS_ELEMENT || elem->GetDtd() == wxSVG_G_ELEMENT)) {
					elem = (wxSVGElement*)elem->GetChildren();
				} else {
					while (elem != NULL && elem->GetNextSibling() == NULL)
						elem = (wxSVGElement*) elem->GetParent();
					if (elem != NULL)
						elem = (wxSVGElement*) elem->GetNextSibling();
				}
			}
			// Init SVG Canvas
			m_svg->Render();
	    } else { // deprecated
	    	if (node->GetPropVal(wxT("bgFile"), &val))
	    		SetBackground(val);
	    	else if (node->GetPropVal(wxT("bgColour"), &val))
	    		SetBackgroundColour(SConv::ToColour(val));
		}
		
		for (wxSvgXmlNode* child = node->GetChildren(); child != NULL; child = child->GetNext())
			if (child->GetType() == wxSVGXML_ELEMENT_NODE)
				AddObject(child);
		
		// fix for old version (<=1.5b5)
		if (m_svg->GetElementById(wxT("objects")) == NULL) {
		  	wxSVGGElement* objectsElem = new wxSVGGElement;
		  	objectsElem->SetId(wxT("objects"));
	  		m_svg->GetRoot()->AppendChild(objectsElem);
	  		wxSVGGElement* buttonsElem = new wxSVGGElement;
	  		buttonsElem->SetId(wxT("buttons"));
	  		m_svg->GetRoot()->AppendChild(buttonsElem);
	  		m_svg->GetRootElement()->GetChildren();
	  		for (unsigned int i = 0; i < m_objects.GetCount(); i++) {
	  			MenuObject& obj = *m_objects.Item(i);
	  			wxSVGElement* useElem = m_svg->GetElementById(obj.GetId());
	  			useElem->GetParent()->RemoveChild(useElem);
	  			if (obj.IsButton())
		  			buttonsElem->AppendChild(useElem);
		  		else
		  			objectsElem->AppendChild(useElem);
	  		}
		}
		
		// fix resolution (DVDStyler <= v1.8.1)
		if (m_svg->GetRootElement()->GetHeight().GetBaseVal() != GetResolution().GetHeight()) {
			double f = (double) GetResolution().GetHeight() / m_svg->GetRootElement()->GetHeight().GetBaseVal();
			// svg root element
			m_svg->GetRootElement()->SetWidth(GetResolution().GetWidth());
			m_svg->GetRootElement()->SetHeight(GetResolution().GetHeight());
			// background
			wxSVGElement* bgElement = m_svg->GetElementById(wxT("backgroundColour"));
			if (bgElement && bgElement->GetDtd() == wxSVG_RECT_ELEMENT) {
				((wxSVGRectElement*) bgElement)->SetWidth(GetResolution().GetWidth());
				((wxSVGRectElement*) bgElement)->SetHeight(GetResolution().GetHeight());
			}
			bgElement = m_svg->GetElementById(wxT("background"));
			if (bgElement) {
				if (bgElement->GetDtd() == wxSVG_IMAGE_ELEMENT) {
					((wxSVGImageElement*) bgElement)->SetWidth(GetResolution().GetWidth());
					((wxSVGImageElement*) bgElement)->SetHeight(GetResolution().GetHeight());
				} else if (bgElement->GetDtd() == wxSVG_VIDEO_ELEMENT) {
					((wxSVGVideoElement*) bgElement)->SetWidth(GetResolution().GetWidth());
					((wxSVGVideoElement*) bgElement)->SetHeight(GetResolution().GetHeight());
				}
			}
			// change object coordinates
			for (unsigned int obji = 0; obji < m_objects.GetCount(); obji++) {
				MenuObject* obj = m_objects[obji];
				obj->SetY(round(obj->GetY()*f/2)*2);
				if (!obj->IsDefaultSize()) {
					obj->SetWidth(round(obj->GetWidth()*f));
					obj->SetHeight(round(obj->GetHeight()*f));
				}
				obj->UpdateSize();
			}
		}
	}
	return true;
}

/** Stores object data to string */
wxString Menu::Serialize() {
	wxSvgXmlDocument xml;
	xml.SetRoot(GetXML(DVDSTYLER_XML));
	wxStringOutputStream stream;
	xml.Save(stream);
	return stream.GetString();
}

/** Restores object from data */
void Menu::Deserialize(const wxString& data) {
	wxStringInputStream stream(data);
	wxSvgXmlDocument xml;
	xml.Load(stream);
	PutXML(xml.GetRoot());
}

wxString Menu::AddObject(wxSvgXmlNode* node, bool fixPosition) {
    if (node->GetName() == wxT("action")) {
        DVDAction* action = new DVDAction(m_tsi == -1);
        if (!action->PutXML(node)) {
            delete action;
            return wxT("");
        }
        m_actions.push_back(action);
        return action->GetId();
    } else if (node->GetName() == wxT("button") || node->GetName() == wxT("object")) {
    	if (node->GetName() == wxT("button")) {
    		if (GetAspectRatio() == ar16_9) {
    			bool wtAuto = m_dvd == NULL || m_dvd->GetPgcArray(m_tsi, true).GetVideo().GetWidescreen() == wtAUTO;
    			if (wtAuto && GetButtonsCount() >= 12) {
					wxLogError(_("Wide screen DVD menu can contain maximal 12 buttons"));
					return wxT("");
				} else if (GetButtonsCount() >= 18) {
					wxLogError(_("Wide screen DVD menu (nopanscan/noletterbox) can contain maximal 18 buttons"));
					return wxT("");
				}
			} else if (GetButtonsCount() >= 34) {
				wxLogError(_("DVD menu can contain maximal 34 buttons"));
				return wxT("");
			}
		}
    		
        MenuObject* obj = new MenuObject(this, m_tsi == -1);
        if (!obj->PutXML(node)) {
            delete obj;
            return wxT("");
        }
        if (fixPosition) // for copy & paste
            FixPosition(obj);
        m_objects.Add(obj);
        return obj->GetId();
    }
    return wxT("");
}

void Menu::FixPosition(MenuObject* obj) {
	if (obj->GetX() == -1)
		obj->SetX(GetResolution().x / 4);
	if (obj->GetY() == -1)
		obj->SetY(GetResolution().y / 4);
	while (obj->GetX() < GetResolution().x - 48 && obj->GetY() < GetResolution().y - 48) {
		bool found = false;
		for (int i = 0; i < (int) GetObjectsCount(); i++) {
			MenuObject* obj1 = GetObject(i);
			if (obj1->GetX() == obj->GetX() && obj1->GetY() == obj->GetY()) {
				found = true;
				break;
			}
		}
		if (!found)
			break;
		obj->SetX(obj->GetX() + 16);
		obj->SetY(obj->GetY() + 16);
	}
}
