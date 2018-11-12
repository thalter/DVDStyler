/////////////////////////////////////////////////////////////////////////////
// Name:        MenuObject.cpp
// Purpose:     The class to store a DVD Menu Object
// Author:      Alex Thuering
// Created:     04.11.2006
// RCS-ID:      $Id: MenuObject.cpp,v 1.64 2016/06/20 10:41:04 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "DVD.h"
#include "Menu.h"
#include "Utils.h"
#include <wxVillaLib/utils.h>
#include <wxSVG/svg.h>
#include <wxSVGXML/svgxmlhelpr.h>
#include <wx/mstream.h>
#include <wx/filename.h>
#include <wx/regex.h>
#include <wx/log.h>

#define BUTTONS_DIR wxFindDataDirectory(_T("buttons"))
#define OBJECTS_DIR wxFindDataDirectory(_T("objects"))

const wxString TRANS_ELEM_ID = wxT("s_trans");
wxRegEx s_jumpTitlesetMenu(wxT("jump titleset ([[:digit:]]+) menu;"));

///////////////////////////// MenuObject /////////////////////////////////////

MenuObject::MenuObject(Menu* menu, bool vmg, wxString fileName, int x, int y, wxString param):
		MenuObjectDef(menu != NULL ? menu->GetSVG() : NULL), m_action(vmg), m_use(NULL), m_buttonSVG(NULL) {
	m_menu = menu;
	if (menu == NULL) {
		m_deleteSVG = true;
		m_svg = Menu::CreateSVG(400, 400);
		wxSVGRectElement* bgRect = new wxSVGRectElement;
		bgRect->SetWidth(400);
		bgRect->SetHeight(400);
		bgRect->SetFill(wxSVGPaint(*wxBLACK));
		m_svg->GetRootElement()->InsertChild(bgRect, m_svg->GetRootElement()->GetChildren());
	} else {
		m_deleteSVG = false;
		m_svg = menu->GetSVG();
	}
	
	m_fileName = fileName;
	m_previewHighlighted = false;

	m_button = false;
	m_autoExecute = false;
	m_displayVideoFrame = true;
	m_customVideoFrame = false;
	m_displayVobId = DVD::MakeId(0, 0, 0);

	m_keepAspectRatio = false;
	m_aspectRatioElem = NULL;

	if (fileName.length())
		Init(fileName, x, y, param);
}

MenuObject::~MenuObject() {
	VECTOR_CLEAR(m_params, MenuObjectParam)
	if (m_use)
		m_use->GetParent()->RemoveChild(m_use);
	if (GetButtonSVG())
		GetButtonSVG()->GetParent()->RemoveChild(GetButtonSVG());
}

bool MenuObject::Init(wxString fileName, int x, int y, wxString param) {
	m_fileName = fileName;
	wxSvgXmlDocument xml;
	xml.Load(fileName);
	if (!xml.GetRoot())
		return false;
	wxSvgXmlNode* root = xml.GetRoot();

	m_button = root->GetName() == wxT("button");
	m_displayVideoFrame = m_button; // display video frame is default for button but not for objects
	m_previewHighlighted = root->GetAttribute(wxT("previewHighlighted")) == wxT("true");
	m_title = XmlReadValue(root, wxT("title"));
	
	if (m_id.length()) { // load button
		m_buttonSVG = (wxSVGSVGElement*) m_svg->GetElementById(wxT("s_") + m_id);
		m_use = (wxSVGUseElement*) m_svg->GetElementById(m_id);
	} else {
		// create new button
		m_id = GenerateId(m_button ? wxT("button") : wxT("obj"));
		wxSvgXmlNode* node = XmlFindNode(root, wxT("default-size"));
		wxString val;
		if (node && node->GetPropVal(wxT("keepAspectRatio"), &val) && (val == wxT("true") || val == wxT("1")))
			m_keepAspectRatio = true;
	}

	wxSvgXmlNode* svgNode = XmlFindNode(root, wxT("svg"));
	if (svgNode) {
		wxSVGDocument svg;
		LoadSVG(svg, svgNode);
		wxSVGSVGElement* root = svg.GetRootElement();
		if (root) {
			m_defaultWidth.value = root->GetWidth().GetBaseVal();
			m_defaultHeight.value = root->GetHeight().GetBaseVal();
			if (!GetButtonSVG())
				AddButtonSVG(m_id, root);
			if (!m_use)
				AddUseElement(m_id, x, y, m_defaultWidth.value, m_defaultHeight.value);
			wxSvgXmlNode* imageNode = XmlFindNode(svgNode, wxT("image"));
			if (imageNode != NULL) {
				wxSVGElement* elem = GetElementById(imageNode->GetAttribute(wxT("id")));
				if (elem && elem->GetDtd() == wxSVG_IMAGE_ELEMENT)
					m_aspectRatioElem = (wxSVGImageElement*) elem;
			}
		}
	}

	// load parameters 
	wxSvgXmlNode* paramsNode = XmlFindNode(root, wxT("parameters"));
	if (paramsNode) {
		wxSvgXmlNode* child = paramsNode->GetChildren();
		while (child) {
			if (child->GetType() == wxSVGXML_ELEMENT_NODE && child->GetName() == wxT("parameter")) {
				MenuObjectParam* param = new MenuObjectParam;
				param->PutXML(child);
				m_params.push_back(param);
				if (param->changeable) {
					param->changeable = false;
					param->normalColour = GetParamColour(param->name);
					param->changeable = true;
				}
			}
			child = child->GetNext();
		}
	}

	m_initParameter = XmlReadValue(root, wxT("init-parameter"));
	// set initial parameter value
	if (m_initParameter.length() && param.length())
		SetParam(m_initParameter, param);
	
	m_textOffset.SetValueAsString(XmlReadValue(root, wxT("text-offset")));

	// load default size
	wxSvgXmlNode* defaultSizeNode = XmlFindNode(root, wxT("default-size"));
	if (defaultSizeNode && defaultSizeNode->GetFirstChild()) {
		InitSize(defaultSizeNode, m_defaultWidth, m_defaultHeight);
	}

	// load min size
	wxSvgXmlNode* minSizeNode = XmlFindNode(root, wxT("min-size"));
	if (minSizeNode && minSizeNode->GetFirstChild()) {
		InitSize(minSizeNode, m_minWidth, m_minHeight);
	}

	UpdateSize();

	return true;
}

bool MenuObject::LoadSVG(wxSVGDocument& svg, wxSvgXmlNode* node) {
	bool res;
	wxSvgXmlDocument xml;
	xml.SetRoot(node->CloneNode());
	wxMemoryOutputStream output;
	xml.Save(output);
	wxMemoryInputStream input(output);
	res = svg.Load(input);
	return res;
}

wxString MenuObject::GenerateId(wxString prefix) {
	int i = 1;
	while (1) {
		wxString id = prefix + wxString::Format(wxT("%02d"), i);
		if (m_svg->GetElementById(id) == NULL)
			return id;
		i++;
	}
	return wxT("");
}

int MenuObject::GetChangebaleColourCount(bool drawButtonsOnBackground) {
	int count = 0;
	for (int i = 0; i < (int) m_params.size(); i++) {
		if (m_params[i]->isChangeable() && ((m_params[i]->normalColour.IsOk() && !drawButtonsOnBackground)
				|| m_params[i]->highlightedColour.IsOk() || m_params[i]->selectedColour.IsOk()))
			count++;
	}
	return count;
}

wxSVGSVGElement* MenuObject::AddButtonSVG(wxString id, wxSVGElement* content) {
	m_buttonSVG = new wxSVGSVGElement;
	m_buttonSVG->SetId(wxT("s_") + id);
	m_svg->GetElementById(wxT("defs"))->AppendChild(m_buttonSVG);
	if (content->GetDtd() == wxSVG_SVG_ELEMENT) {
		m_buttonSVG->SetViewBox(((wxSVGSVGElement*) content)->GetViewBox());
		m_buttonSVG->SetPreserveAspectRatio(((wxSVGSVGElement*) content)->GetPreserveAspectRatio());
	} else if (content->GetDtd() == wxSVG_SYMBOL_ELEMENT) {
		m_buttonSVG->SetViewBox(((wxSVGSymbolElement*) content)->GetViewBox());
		m_buttonSVG->SetPreserveAspectRatio(((wxSVGSymbolElement*) content)->GetPreserveAspectRatio());
	}
	wxSvgXmlElement* child = content->GetChildren();
	while (child) {
		m_buttonSVG->AppendChild(((wxSVGSVGElement*) child)->CloneNode());
		child = child->GetNext();
	}
	return m_buttonSVG;
}

void MenuObject::AddUseElement(wxString id, int x, int y, int width, int height, const wxSVGTransformList* transforms) {
	m_use = new wxSVGUseElement;
	m_use->SetId(id);
	m_use->SetHref(wxT("#s_") + id);
	m_use->SetX(x);
	m_use->SetY(y);
	m_use->SetWidth(width);
	m_use->SetHeight(height);
	if (transforms)
		m_use->SetTransform(*transforms);
	if (IsButton())
		m_svg->GetElementById(wxT("buttons"))->AppendChild(m_use);
	else
		m_svg->GetElementById(wxT("objects"))->AppendChild(m_use);
}

wxSVGSVGElement* MenuObject::GetButtonSVG() const {
	return m_buttonSVG;
}

void MenuObject::SetScale(double scaleX, double scaleY) {
	wxSVGGElement* transElem = (wxSVGGElement*) GetButtonSVG()->GetElementById(TRANS_ELEM_ID);
	if (transElem == NULL) {
		if (scaleX == 1 && scaleY == 1)
			return;
		// add transElem
		transElem = new wxSVGGElement();
		transElem->SetId(TRANS_ELEM_ID);
		GetButtonSVG()->AddChild(transElem);
		// move all children elements in gElem
		while (GetButtonSVG()->GetFirstChild() != transElem) {
			wxSvgXmlNode* elem = GetButtonSVG()->GetFirstChild();
			GetButtonSVG()->RemoveChild(elem);
			transElem->AppendChild(elem);
		}
	}
	wxSVGTransformList transList;
	if (scaleX != 1 || scaleY != 1) {
		wxSVGTransform transform;
		transform.SetScale(scaleX, scaleY);
		transList.Add(transform);
	}
	// copy old values
	const wxSVGTransformList& oldTransList = transElem->GetTransform().GetBaseVal();
	for (unsigned int i = 0; i < oldTransList.size(); i++)
		if (i > 0 || oldTransList[i].GetType() != wxSVG_TRANSFORM_SCALE)
			transList.Add(oldTransList[i]);
	transElem->SetTransform(transList);
}

wxString MenuObject::GetId(bool translate) {
	if (!translate)
		return m_id;
	long l = 0;
	m_id.Mid(IsButton() ? 6 : 3).ToLong(&l);
	return (IsButton() ? _("button") : _("object")) + wxString::Format(wxT(" %d"), (int) l);
}

int MenuObject::GetX() const {
	return m_use->GetX().GetBaseVal();
}
void MenuObject::SetX(int value) {
	m_use->SetX(value);
	UpdateTransform();
}

int MenuObject::GetY() const {
	return m_use->GetY().GetBaseVal();
}
void MenuObject::SetY(int value) {
	m_use->SetY(value);
	UpdateTransform();
}

int MenuObject::GetWidth() const {
	return m_use->GetWidth().GetBaseVal();
}
void MenuObject::SetWidth(int value) {
	m_use->SetWidth(value);
	UpdateTransform();
}

int MenuObject::GetHeight() const {
	return m_use->GetHeight().GetBaseVal();
}
void MenuObject::SetHeight(int value) {
	m_use->SetHeight(value);
	UpdateTransform();
}

void MenuObject::SetRect(wxRect rect) {
	m_use->SetX(rect.x);
	m_use->SetY(rect.y);
	m_use->SetWidth(rect.width);
	m_use->SetHeight(rect.height);
	UpdateTransform();
}

void MenuObject::UpdateTransform() {
	if (m_use->GetTransform().GetBaseVal().size() > 0
			&& m_use->GetTransform().GetBaseVal()[0].GetType() == wxSVG_TRANSFORM_ROTATE) {
		wxSVGTransform& transform = m_use->GetTransform().GetBaseVal()[0];
		transform.SetRotate(transform.GetAngle(), GetX() + GetWidth()/2, GetY() + GetHeight()/2);
	}
}

double MenuObject::GetAngle() const {
	MenuObjectParam* param = GetParamByName(wxT("rotation"));
	if (param != NULL) {
		return GetParamDouble(wxT("rotation"));
	}
	return m_use->GetTransform().GetBaseVal().size() > 0
			&& m_use->GetTransform().GetBaseVal()[0].GetType() == wxSVG_TRANSFORM_ROTATE
			? m_use->GetTransform().GetBaseVal()[0].GetAngle() : 0;
}

void MenuObject::SetAngle(double angle) {
	if (angle > 360)
		angle -= 360;
	if (angle < 0)
		angle += 360;
	MenuObjectParam* param = GetParamByName(wxT("rotation"));
	if (param != NULL) {
		SetParamDouble(wxT("rotation"), angle);
		return;
	}
	if (m_use->GetTransform().GetBaseVal().size() > 0
			&& m_use->GetTransform().GetBaseVal()[0].GetType() == wxSVG_TRANSFORM_ROTATE) {
		if (angle != 0) {
			m_use->GetTransform().GetBaseVal()[0].SetRotate(angle, GetX() + GetWidth()/2, GetY() + GetHeight()/2);
		} else {
			m_use->SetTransform(wxSVGTransformList());
		}
	} else if (angle != 0) {
		m_use->SetTransform(wxSVGTransformList());
		m_use->Rotate(angle, GetX() + GetWidth()/2, GetY() + GetHeight()/2);
	}
}

void MenuObject::UpdateMatrix(wxSVGMatrix& matrix) const {
	m_use->UpdateMatrix(matrix);
}

wxRect MenuObject::GetBBox() const {
	return wxRect(GetX(), GetY(), GetWidth(), GetHeight());
}

wxRect MenuObject::GetResultBBox() const {
	wxRect bbox = wxRect(GetX(), GetY(), GetWidth(), GetHeight());
//	wxSVGElement* cElem = GetElementById(wxT("circle"));
//	if (cElem != NULL) {
//		wxSVGSVGElement* viewportElem = new wxSVGSVGElement();
//		viewportElem->SetWidth(m_use->GetWidth().GetBaseVal());
//		viewportElem->SetHeight(m_use->GetHeight().GetBaseVal());
//		cElem->SetViewportElement(viewportElem);
//		wxSVGRect elemBox = wxSVGLocatable::GetElementResultBBox(cElem);
//		cElem->SetViewportElement(NULL);
//		delete viewportElem;
//		bbox = wxRect(GetX() + elemBox.GetX(), GetY() + elemBox.GetY(), elemBox.GetWidth(), elemBox.GetHeight());
//	}
	if (m_use->GetTransform().GetBaseVal().size() > 0
			&& m_use->GetTransform().GetBaseVal()[0].GetType() == wxSVG_TRANSFORM_ROTATE) {
		wxSVGTransform& transform = m_use->GetTransform().GetBaseVal()[0];
		double angle = transform.GetAngle();
		if (angle != 0) {
			wxSVGPoint point[4] = {
					wxSVGPoint(bbox.GetX(), bbox.GetY()).MatrixTransform(transform.GetMatrix()),
					wxSVGPoint(bbox.GetX() + bbox.GetWidth(), bbox.GetY()).MatrixTransform(transform.GetMatrix()),
					wxSVGPoint(bbox.GetX(), bbox.GetY() + bbox.GetHeight()).MatrixTransform(transform.GetMatrix()),
					wxSVGPoint(bbox.GetX() + bbox.GetWidth(), bbox.GetY() + bbox.GetHeight()).MatrixTransform(transform.GetMatrix())
			};
			wxSVGPoint p1 = point[0];
			wxSVGPoint p2 = point[0];
			for (int i = 1; i < 4; i++) {
				if (p1.GetX() > point[i].GetX())
					p1.SetX(point[i].GetX());
				if (p1.GetY() > point[i].GetY())
					p1.SetY(point[i].GetY());
				if (p2.GetX() < point[i].GetX())
					p2.SetX(point[i].GetX());
				if (p2.GetY() < point[i].GetY())
					p2.SetY(point[i].GetY());
			}
			return wxRect(p1.GetX(), p1.GetY(), p2.GetX() - p1.GetX(), p2.GetY() - p1.GetY());
		}
	}
	return bbox;
}

wxRect MenuObject::GetFrameBBox(SubStreamMode mode, bool ignorePadding) {
	wxRect bbox = GetResultBBox();
	double fx = mode == ssmPANSCAN ? 4.0/3 : 1.0;
	double fy = mode == ssmLETTERBOX ? 0.75 : 1.0;
	if (m_menu != NULL)
		fy *= (double) m_menu->GetFrameResolution().GetHeight() / m_menu->GetResolution().GetHeight();
	int width = round(bbox.GetWidth()*fx);
	int height = round(bbox.GetHeight()*fy);
	int padY = mode == ssmLETTERBOX && !ignorePadding ? m_menu->GetFrameResolution().GetHeight()*0.125: 0;
	int cropX = mode == ssmPANSCAN ? m_menu->GetFrameResolution().GetWidth()*0.125 : 0;
	int x = round((bbox.GetX() - cropX)*fx);
	int y = round(bbox.GetY()*fy) + padY;
	if (height % 2 == 1)
		height += y % 2 == 0 ? 1 : -1;
	if (y % 2 == 1)
		y++;
	return wxRect(x, y, width, height);
}

unsigned int MenuObject::CalcSize(MenuObjectSize& size, bool width) {
	unsigned int result = 0;
	for (unsigned int idx = 0; idx < size.elements.size(); idx++) {
		wxSVGElement* elem = GetElementById(size.elements[idx]);
		if (elem) {
			if (elem->GetDtd() == wxSVG_IMAGE_ELEMENT) {
				wxSVGImageElement* imgElem = (wxSVGImageElement*) elem;
				result += width ? imgElem->GetDefaultWidth() : imgElem->GetDefaultHeight();
			} else {
				wxSVGRect bbox = wxSVGLocatable::GetElementResultBBox(elem);
				result += width ? (unsigned int) bbox.GetWidth() : (unsigned int) bbox.GetHeight();
			}
		}
	}
	if (size.valuePercent > 0)
		result += lround(((double) result * size.valuePercent) / 100);
	result += size.valueInc;
	return wxMax(result, size.value);
}

bool MenuObject::IsAlignRight(MenuObjectSize& size) {
	if (size.elements.size() == 0) {
		return false;
	}
	wxSVGElement* elem = GetElementById(size.elements[0]);
	if (!elem || elem->GetDtd() != wxSVG_TEXT_ELEMENT) {
		return false;
	}
	return ((wxSVGTextElement*) elem)->GetTextAnchor() == wxCSS_VALUE_END;
}

#include <wxSVG/SVGCanvasItem.h>

void MenuObject::FixSize(int& width, int& height) {
	// calculate aspect ratio
	double aspectRatio = -1;
	if (m_keepAspectRatio && m_aspectRatioElem != NULL && m_aspectRatioElem->GetDefaultWidth() > 0)
		aspectRatio = ((double) m_aspectRatioElem->GetDefaultWidth()) / m_aspectRatioElem->GetDefaultHeight();
	
	if (m_defaultSize) {
		width = CalcSize(m_defaultWidth, true);
		if (aspectRatio > 0)
			height = width / aspectRatio;
		else
			height = CalcSize(m_defaultHeight, false);
	} else {
		width = wxMax(width, (int) CalcSize(m_minWidth, true));
		if (m_keepAspectRatio) {
			if (aspectRatio > 0)
				height = width / aspectRatio;
			else
				height = width * CalcSize(m_defaultHeight, false) / CalcSize(m_defaultWidth, true);
		} else
			height = wxMax(height, (int) CalcSize(m_minHeight, false));
	}
}

void MenuObject::UpdateSize() {
	int width = GetWidth();
	int height = GetHeight();
	FixSize(width, height);
	if (IsAlignRight(m_defaultSize ? m_defaultWidth : m_minWidth) && width != GetWidth()) {
		SetX(GetX() + GetWidth() - width);
	}
	SetWidth(width);
	SetHeight(height);
	if (GetX() + GetWidth() <= 0)
		SetX(0);
	if (GetY() + GetHeight() <= 0)
		SetY(0);
}

void GetAnimationElements(wxSVGElement* child, vector<wxSVGAnimateElement*>& animations) {
	while (child) {
		if (child->GetType() == wxSVGXML_ELEMENT_NODE && child->GetDtd() == wxSVG_ANIMATE_ELEMENT) {
			animations.push_back((wxSVGAnimateElement*) child);
		}
		child = (wxSVGElement*) child->GetNextSibling();
	}
}

/** Returns animation elements */
vector<wxSVGAnimateElement*> MenuObject::GetAnimations() {
	vector<wxSVGAnimateElement*> animations;
	GetAnimationElements((wxSVGElement*) m_use->GetFirstChild(), animations);
	GetAnimationElements((wxSVGElement*) GetButtonSVG()->GetFirstChild(), animations);
	return animations;
}

void RemoveAnimationElements(wxSVGElement* child) {
	while (child != NULL) {
		wxSVGElement* nextChild = (wxSVGElement*) child->GetNextSibling();
		if (child->GetType() == wxSVGXML_ELEMENT_NODE && child->GetDtd() == wxSVG_ANIMATE_ELEMENT) {
			child->GetParent()->RemoveChild(child);
			delete child;
		}
		child = nextChild;
	}
}

/** Sets animation elements */
void MenuObject::SetAnimations(vector<wxSVGAnimateElement*>& animations) {
	RemoveAnimationElements((wxSVGElement*) GetButtonSVG()->GetFirstChild());
	RemoveAnimationElements((wxSVGElement*) m_use->GetFirstChild());
	for (auto anim : animations) {
		if (anim->GetHref().length() >0)
			GetButtonSVG()->AddChild(anim);
		else
			m_use->AddChild(anim);
	}
}

void MenuObject::ToFront() {
	wxSVGElement* parent = (wxSVGElement*) m_use->GetParent();
	parent->RemoveChild(m_use);
	parent->AppendChild(m_use);
}

void MenuObject::Forward() {
	wxSVGElement* parent = (wxSVGElement*) m_use->GetParent();
	wxSVGElement* next = (wxSVGElement*) m_use->GetNextSibling();
	if (next && next->GetType() == wxSVGXML_ELEMENT_NODE && next->GetDtd() == wxSVG_USE_ELEMENT) {
		parent->RemoveChild(m_use);
		parent->InsertChild(m_use, next->GetNextSibling());
	}
}

void MenuObject::Backward() {
	wxSVGElement* parent = (wxSVGElement*) m_use->GetParent();
	wxSVGElement* prev = (wxSVGElement*) m_use->GetPreviousSibling();
	if (prev && prev->GetType() == wxSVGXML_ELEMENT_NODE && prev->GetDtd() == wxSVG_USE_ELEMENT) {
		parent->RemoveChild(m_use);
		parent->InsertChild(m_use, prev);
	}
}

void MenuObject::ToBack() {
	wxSVGElement* parent = (wxSVGElement*) m_use->GetParent();
	wxSVGElement* first = (wxSVGElement*) parent->GetFirstChild();
	while (first && (first->GetType() != wxSVGXML_ELEMENT_NODE
			|| first->GetDtd() != wxSVG_USE_ELEMENT))
		first = (wxSVGElement*) first->GetNextSibling();
	if (first && first != m_use) {
		parent->RemoveChild(m_use);
		parent->InsertChild(m_use, first);
	}
}

bool MenuObject::IsFirst() {
	wxSVGElement* prev = (wxSVGElement*) m_use->GetPreviousSibling();
	return !prev || prev->GetType() != wxSVGXML_ELEMENT_NODE;
}

bool MenuObject::IsLast() {
	wxSVGElement* next = (wxSVGElement*) m_use->GetNextSibling();
	return !next || next->GetType() != wxSVGXML_ELEMENT_NODE;
}

wxString MenuObject::GetDefaultFocusDest(NavigationButton navButton) {
	map<int, wxArrayString> btMap; // dist -> buttons
	map<int, wxArrayString> btMapAll; // dist -> buttons
	int minDist = 99999;
	int minDistAll = 99999;
	wxRect rect = GetBBox();
	for (unsigned int i = 0; i < m_menu->GetObjectsCount(); i++) {
		MenuObject* obj = m_menu->GetObject(i);
		if (!obj->IsButton())
			continue;
		wxRect objRect = obj->GetBBox();
		int dist = -1;
		if (navButton == nbLEFT)
			dist = rect.GetX() - objRect.GetRight() - 1;
		else if (navButton == nbRIGHT)
			dist = objRect.GetX() - rect.GetRight() - 1;
		else if (navButton == nbUP)
			dist = rect.GetY() - objRect.GetBottom() - 1;
		else // nbDOWN
			dist = objRect.GetY() - rect.GetBottom() - 1;
		if (dist < 0)
			continue;
		if (navButton == nbLEFT || navButton == nbRIGHT)
			objRect.SetX(rect.GetX());
		else // nbUP/nbDOWN
			objRect.SetY(rect.GetY());
		if (objRect.Intersects(rect)) {
			btMap[dist].Add(obj->GetId());
			if (dist < minDist)
				minDist = dist;
		}
		btMapAll[dist].Add(obj->GetId());
		if (dist < minDistAll)
			minDistAll = dist;
	}
	// filter by minDist
	wxArrayString buttons;
	for (map<int, wxArrayString>::iterator it = btMap.begin(); it != btMap.end(); it++)
		if (it->first <= minDist + 4)
			buttons.insert(buttons.begin(), it->second.begin(), it->second.end());
	if (buttons.IsEmpty()) {
		if (btMapAll.empty())
			return GetId(); // 'none'
		buttons = btMapAll[minDistAll];
	}
	// filter by distance to left/up
	wxString result;
	int resultDist = 0;
	for (unsigned int i = 0; i < buttons.size(); i++) {
		MenuObject* obj = m_menu->GetObject(buttons[i]);
		wxRect objRect = obj->GetBBox();
		int dist = navButton == nbLEFT || navButton == nbRIGHT
				? objRect.GetY() - rect.GetY() : objRect.GetX() - rect.GetX();
		if (result.length() == 0) {
			result = obj->GetId();
			resultDist = dist;
		} else if (dist < resultDist) {
			result = obj->GetId();
			resultDist = dist;
		}
	}
	return result.length() ? result : GetId(); // GetID() -> 'none'
}

wxImage MenuObject::GetImage(int maxWidth, int maxHeight) {
	if (!m_use)
		return wxImage();
	if (m_previewHighlighted) {
		for (unsigned int i = 0; i < m_params.size(); i++) {
			MenuObjectParam* param = m_params[i];
			if (param->changeable) {
				wxSVGElement* elem = GetElementById(param->element.front());
				if (elem && param->attribute.length()) {
					wxSVGPaint paint(param->highlightedColour);
					elem->SetAttribute(param->attribute, paint.GetCSSText());
				}
			}
		}
	}
	m_svg->GetRootElement()->SetWidth(GetWidth());
	m_svg->GetRootElement()->SetHeight(GetHeight());
	return m_svg->Render(maxWidth, maxHeight);
}

wxSvgXmlNode* MenuObject::GetXML(DVDFileType type, DVD* dvd, SubStreamMode mode, bool withSVG) {
	wxString rootName = wxT("object");
	if (IsButton())
		rootName = IsAutoExecute() && type == SPUMUX_XML ? wxT("action") : wxT("button");
	wxSvgXmlNode* rootNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, rootName);
	switch (type) {
	case DVDSTYLER_XML:
		if (IsButton())
			rootNode->AddChild(m_action.GetXML(type, dvd));
		if (GetFocusDest(nbLEFT).length() || GetFocusDest(nbRIGHT).length()
				|| GetFocusDest(nbUP).length() || GetFocusDest(nbDOWN).length()) {
			wxSvgXmlNode* directionNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, _T("direction"));
			if (GetFocusDest(nbLEFT).length())
				directionNode->AddProperty(_T("left"), GetFocusDest(nbLEFT));
			if (GetFocusDest(nbRIGHT).length())
				directionNode->AddProperty(_T("right"), GetFocusDest(nbRIGHT));
			if (GetFocusDest(nbUP).length())
				directionNode->AddProperty(_T("up"), GetFocusDest(nbUP));
			if (GetFocusDest(nbDOWN).length())
				directionNode->AddProperty(_T("down"), GetFocusDest(nbDOWN));
			rootNode->AddChild(directionNode);
		}
		XmlWriteValue(rootNode, _T("filename"), GetFileName().AfterLast(wxFILE_SEP_PATH));
		for (unsigned int i = 0; i < m_params.size(); i++) {
			MenuObjectParam* param = m_params[i];
			if (param->changeable) {
				wxSvgXmlNode* paramNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, _T("parameter"));
				paramNode->AddProperty(_T("name"), param->name);
				paramNode->AddProperty(_T("normal"), wxSVGPaint(param->normalColour).GetCSSText());
				paramNode->AddProperty(_T("highlighted"), wxSVGPaint(param->highlightedColour).GetCSSText());
				paramNode->AddProperty(_T("selected"), wxSVGPaint(param->selectedColour).GetCSSText());
				rootNode->AddChild(paramNode);
			}
		}
		rootNode->AddProperty(wxT("id"), GetId());
		if (m_defaultSize)
			rootNode->AddProperty(wxT("defSize"), wxT("true"));
		if (IsAutoExecute())
			rootNode->AddProperty(wxT("autoExecute"), wxT("true"));
		if (m_keepAspectRatio)
			rootNode->AddProperty(wxT("keepAspectRatio"), wxT("true"));
		if (!m_displayVideoFrame)
			rootNode->AddProperty(wxT("displayVideoFrame"), wxT("false"));
		if (m_customVideoFrame)
			rootNode->AddProperty(wxT("customVideoFrame"), wxT("true"));
		if (m_displayVobId != DVD::MakeId(0, 0, 0))
			rootNode->AddProperty(wxT("vobId"), wxString::Format(_T("%d"), m_displayVobId));
		break;
	case SPUMUX_XML: {
		int mwidth = (int) m_menu->GetFrameResolution().GetWidth() - 1;
		int mheight = (int) m_menu->GetFrameResolution().GetHeight() - 1;
		rootNode->AddProperty(_T("name"), GetId());
		wxRect rect = GetFrameBBox(mode);
		rootNode->AddProperty(_T("x0"), wxString::Format(_T("%d"), rect.GetX() > 0 ? rect.GetX() : 0));
		rootNode->AddProperty(_T("y0"), wxString::Format(_T("%d"), rect.GetY() > 0 ? rect.GetY() : 0));
		rootNode->AddProperty(_T("x1"), wxString::Format(_T("%d"),
				rect.GetX() + rect.GetWidth() < mwidth ? rect.GetX() + rect.GetWidth() : mwidth));
		rootNode->AddProperty(_T("y1"), wxString::Format(_T("%d"),
				rect.GetY() + rect.GetHeight() < mheight ? rect.GetY() + rect.GetHeight() : mheight));
		wxString focusDest = GetFocusDest(nbLEFT).length() ? GetFocusDest(nbLEFT) : GetDefaultFocusDest(nbLEFT);
		if (focusDest.length())
			rootNode->AddProperty(_T("left"), focusDest);
		focusDest = GetFocusDest(nbRIGHT).length() ? GetFocusDest(nbRIGHT) : GetDefaultFocusDest(nbRIGHT);
		if (focusDest.length())
			rootNode->AddProperty(_T("right"), focusDest);
		focusDest = GetFocusDest(nbUP).length() ? GetFocusDest(nbUP) : GetDefaultFocusDest(nbUP);
		if (focusDest.length())
			rootNode->AddProperty(_T("up"), focusDest);
		focusDest = GetFocusDest(nbDOWN).length() ? GetFocusDest(nbDOWN) : GetDefaultFocusDest(nbDOWN);
		if (focusDest.length())
			rootNode->AddProperty(_T("down"), focusDest);
		break;
	}
	case DVDAUTHOR_XML: {
		rootNode->AddProperty(_T("name"), GetId());
		wxString action = GetAction().AsString(dvd);
		action.Replace(wxT("vmMenu"), wxT("vmgm menu"));
		if (dvd->GetPlayAllRegister() != -1)
			action = wxString::Format(wxT("g%d=%d;"), dvd->GetPlayAllRegister(),
					GetAction().IsPlayAll() ? (GetAction().IsPlayAllTitlesets() ? 2 : 1): 0) + action;
		if (m_menu->GetRememberLastButton()) {
			int reg = m_menu->GetRememberLastButtonRegister();
			if (reg == -1)
				reg = dvd->GetRememberLastButtonRegister();
			action = wxString::Format(wxT("g%d=button;"), reg) + action;
		}
		rootNode->AddChild(new wxSvgXmlNode(wxSVGXML_TEXT_NODE, wxEmptyString, action));
		break;
	}
	default:
		break;
	}
	if (withSVG) {  // used by copy & paste
		rootNode->DeleteProperty(wxT("id"));
		wxSvgXmlNode* svgNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("svg"));
		wxSvgXmlNode* defsNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("defs"));
		wxSVGElement* symbol = (wxSVGElement*) GetButtonSVG()->CloneNode();
		symbol->SetId(wxT(""));
		defsNode->AddChild(symbol);
		svgNode->AddChild(defsNode);
		svgNode->AddChild(m_use->CloneNode());
		rootNode->AddChild(svgNode);
	}
	return rootNode;
}

bool MenuObject::PutXML(wxSvgXmlNode* node) {
	m_button = node->GetName() == wxT("button");

	long lval;
	wxString val;

	if (IsButton()) {
		wxSvgXmlNode* actionNode = XmlFindNodeSimple(node, _T("action"));
		if (actionNode)
			m_action.PutXML(actionNode);

		wxSvgXmlNode* dirNode = XmlFindNodeSimple(node, _T("direction"));
		if (dirNode) {
			if (dirNode->GetPropVal(_T("left"), &val))
				SetFocusDest(nbLEFT, val);
			if (dirNode->GetPropVal(_T("right"), &val))
				SetFocusDest(nbRIGHT, val);
			if (dirNode->GetPropVal(_T("up"), &val))
				SetFocusDest(nbUP, val);
			if (dirNode->GetPropVal(_T("down"), &val))
				SetFocusDest(nbDOWN, val);
		}
	}

	// deprecated
	int x = 0, y = 0;
	if (node->GetPropVal(_T("x"), &val) && val.ToLong(&lval))
		x = (int) (lval / m_svg->GetScale());
	if (node->GetPropVal(_T("y"), &val) && val.ToLong(&lval))
		y = (int) (lval / m_svg->GetScale());
	wxString fileName = XmlReadValue(node, _T("type"));
	if (fileName.length()) {
		wxString text = XmlReadValue(node, _T("text"));
		return Init(BUTTONS_DIR + wxT("text.xml"), x, y, text);
	}

	fileName = XmlReadValue(node, _T("filename"));
	if (!fileName.length())
		return false;

	wxFileName fn(fileName);
	wxString dir = IsButton() ? BUTTONS_DIR : OBJECTS_DIR;
	if (fn.IsRelative()) {
		if (wxFileName::FileExists(dir + fileName))
			fileName = dir + fileName;
		else if (wxFileName::FileExists(dir + wxT("deprecated") + wxFILE_SEP_PATH + fileName))
			fileName = dir + wxT("deprecated") + wxFILE_SEP_PATH + fileName;
		else if (wxFileName::FileExists(dir + fn.GetFullName()))
			fileName = dir + fn.GetFullName();
		else {
			wxLogError(_("can't open file '%s'"), fileName.c_str());
			return false;
		}
	} else if (!wxFileExists(fileName)) {
		if (wxFileName::FileExists(dir + fn.GetFullName()))
			fileName = dir + fn.GetFullName();
		else if (wxFileName::FileExists(dir + wxT("deprecated") + wxFILE_SEP_PATH + fn.GetFullName()))
			fileName = dir + wxT("deprecated") + wxFILE_SEP_PATH + fn.GetFullName();
		else {
			wxLogError(_("can't open file '%s'"), fileName.c_str());
			return false;
		}
	}

	node->GetPropVal(wxT("id"), &m_id);
	m_autoExecute = node->GetPropVal(wxT("autoExecute"), &val) && (val == wxT("true") || val == wxT("1"));
	if (!node->GetPropVal(wxT("defSize"), &val) || (val != wxT("true") && val != wxT("1")))
		m_defaultSize = false;

	// paste button
	wxSvgXmlNode* svgNode = XmlFindNode(node, wxT("svg"));
	if (svgNode) {
		m_id = GenerateId(m_button ? wxT("button") : wxT("obj"));
		wxSVGDocument svg;
		LoadSVG(svg, svgNode);
		wxSVGSVGElement* root = svg.GetRootElement();
		if (root && XmlFindNode(root, wxT("defs")) && XmlFindNode(root, wxT("use"))) {
			wxSVGElement* defsNode = (wxSVGElement*) XmlFindNode(root, wxT("defs"));
			wxSvgXmlElement* child = defsNode->GetChildren();
			while (child) {
				AddButtonSVG(m_id, (wxSVGElement*) child);
				child = child->GetNext();
			}
			wxSVGUseElement* useElem = (wxSVGUseElement*) XmlFindNode(root, wxT("use"));
			AddUseElement(m_id, useElem->GetX().GetBaseVal(), useElem->GetY().GetBaseVal(), useElem->GetWidth().GetBaseVal(),
					useElem->GetHeight().GetBaseVal(), &useElem->GetTransform().GetBaseVal());
		}
	}

	if (!Init(fileName, x, y))
		return false;
	
	m_keepAspectRatio = node->GetPropVal(wxT("keepAspectRatio"), &val) && (val == wxT("true") || val == wxT("1"));
	m_displayVideoFrame = !node->GetPropVal(wxT("displayVideoFrame"), &val) || (val != wxT("false") && val != wxT("0"));
	m_customVideoFrame = node->GetPropVal(wxT("customVideoFrame"), &val) && (val == wxT("true") || val == wxT("1"));
	if (node->GetPropVal(wxT("vobId"), &val) && val.ToLong(&lval))
		m_displayVobId = lval;
	
	// read changeable parameters
	wxSvgXmlNode* child = node->GetChildren();
	while (child) {
		if (child->GetName() == wxT("parameter")) {
			wxString name;
			wxString normal;
			wxString selected;
			wxString highlighted;
			if (child->GetPropVal(wxT("name"), &name)
					&& child->GetPropVal(wxT("normal"), &normal)
					&& child->GetPropVal(wxT("highlighted"), &highlighted)
					&& child->GetPropVal(wxT("selected"), &selected)) {
				wxCSSStyleDeclaration style;
				style.SetProperty(wxT("fill"), normal);
				SetParamColour(name, style.GetFill().GetRGBColor(), mbsNORMAL);
				style.SetProperty(wxT("fill"), highlighted);
				SetParamColour(name, style.GetFill().GetRGBColor(), mbsHIGHLIGHTED);
				style.SetProperty(wxT("fill"), selected);
				SetParamColour(name, style.GetFill().GetRGBColor(), mbsSELECTED);
			}
		}
		child = child->GetNext();
	}
	
	return true;
}
