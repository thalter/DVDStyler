/////////////////////////////////////////////////////////////////////////////
// Name:        MenuObjectDef.cpp
// Purpose:     The class to store a DVD menu object definition
// Author:      Alex Thuering
// Created:     5.03.2018
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "MenuObjectDef.h"
#include <wxVillaLib/utils.h>
#include <wxSVG/svg.h>
#include <wxSVGXML/svgxmlhelpr.h>
#include <wx/mstream.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>
#include <wxSVG/svgctrl.h>

/////////////////////// MenuObjectSize //////////////////////////////
wxString MenuObjectSize::GetValueAsString() const {
	wxString result;
	if (value > 0)
		result = wxString::Format("%d", value);
	for (unsigned int idx = 0; idx < elements.size(); idx++) {
		if (result.length() > 0)
			result += idx == 0 ? "|" : "+";
	}
	if (valuePercent > 0)
		result += "+" + wxString::Format("%d", valuePercent) + "%";
	if (valueInc > 0)
		result += "+" + wxString::Format("%d", valueInc);
	return result;
}

void MenuObjectSize::SetValueAsString(wxString strValue) {
	long lval = 0;
	if (strValue.Index(wxT('|'))) {
		if (strValue.BeforeFirst(wxT('|')).ToLong(&lval))
			this->value = lval;
		strValue = strValue.AfterFirst(wxT('|'));
	}
	while (strValue.Length() > 0) {
		wxString val = strValue.BeforeFirst(wxT('+'));
		if (val.Last() == wxT('%') && val.SubString(0, val.length()-2).ToLong(&lval))
			this->valuePercent += lval;
		else if (val.ToLong(&lval))
			this->valueInc += lval;
		else
			this->elements.Add(val);
		strValue = strValue.AfterFirst(wxT('+'));
	}
}

/////////////////////// MenuObjectParam //////////////////////////////

/** Initializes object with XML data */
bool MenuObjectParam::PutXML(wxSvgXmlNode* node) {
	title = XmlReadValue(node, wxT("title"));
	name = XmlReadValue(node, wxT("name"));
	type = XmlReadValue(node, wxT("type"));
	wxStringTokenizer tokenizer(XmlReadValue(node, wxT("element")), wxT(","), wxTOKEN_RET_EMPTY_ALL);
	while (tokenizer.HasMoreTokens()) {
		element.push_back(tokenizer.GetNextToken());
	}
	attribute = XmlReadValue(node, wxT("attribute"));
	changeable = XmlFindNode(node, wxT("changeable")) != NULL && type == wxT("colour");
	if (changeable) {
		wxCSSStyleDeclaration style;
		style.SetProperty(wxCSS_PROPERTY_STROKE, XmlReadValue(node, wxT("default-value/highlighted")));
		highlightedColour = style.GetStroke().GetRGBColor();
		style.SetProperty(wxCSS_PROPERTY_STROKE, XmlReadValue(node, wxT("default-value/selected")));
		selectedColour = style.GetStroke().GetRGBColor();
	}
	return true;
}

/** Returns object data as XML */
wxSvgXmlNode* MenuObjectParam::GetXML() {
	wxSvgXmlNode* paramNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, "parameter");
	XmlWriteValue(paramNode, "title", this->title);
	XmlWriteValue(paramNode, "name", this->name);
	XmlWriteValue(paramNode, "type", this->type);
	wxString elements;
	for (wxString elem : this->element) {
		if (elements.length())
			elements += ",";
		elements += elem;
	}
	XmlWriteValue(paramNode, "element", elements);
	XmlWriteValue(paramNode, "attribute", this->attribute);
	if (this->changeable) {
		XmlWriteValue(paramNode, "changeable", "");
		wxSvgXmlNode* defValueNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, "default-value");
		XmlWriteValue(defValueNode, "highlighted", wxSVGColor(this->highlightedColour).GetCSSText());
		XmlWriteValue(defValueNode, "selected", wxSVGColor(this->selectedColour).GetCSSText());
		paramNode->AddChild(defValueNode);
	}
	return paramNode;
}

/////////////////////// MenuObjectDef //////////////////////////////
MenuObjectDef::MenuObjectDef(wxSVGDocument* svg) {
	m_previewHighlighted = false;
	if (svg) {
		m_svg = svg;
		m_deleteSVG = false;
	} else {
		m_svg = new wxSVGDocument;
		m_deleteSVG = true;
	}
	m_buttonState = mbsNORMAL;
	m_defaultSize = true;
	m_defaultWidth.value = m_defaultHeight.value = 0;
	m_defaultWidth.valueInc = m_defaultHeight.valueInc = 0;
	m_defaultWidth.valuePercent = m_defaultHeight.valuePercent = 0;
	m_minWidth.value = m_minHeight.value = 0;
	m_minWidth.valueInc = m_minHeight.valueInc = 0;
	m_minWidth.valuePercent = m_minHeight.valuePercent = 0;
}

MenuObjectDef::~MenuObjectDef() {
	if (m_deleteSVG)
		delete m_svg;
}

void RecurseElementId(wxSVGElement* root, set<wxString>& idSet) {
	if (root->GetId().length())
		idSet.insert(root->GetId());
	// check childs
	wxSVGElement* child = (wxSVGElement*) root->GetChildren();
	while (child) {
		if (child->GetType() == wxSVGXML_ELEMENT_NODE) {
			if (child->GetDtd() == wxSVG_SVG_ELEMENT) {
				if (root->GetId().length())
					idSet.insert(root->GetId());
			} else {
				RecurseElementId(child, idSet);
			}
		}
		child = (wxSVGElement*) child->GetNext();
	}
}

void MenuObjectDef::UpdateIdList() {
	set<wxString> idSet;
	RecurseElementId(m_svg->GetRootElement(), idSet);
	for (wxString id : idSet) {
		if (id != "DVDStyler_bt_bg")
			m_idList.Add(id);
	}
}

/** Loads SVG file */
bool MenuObjectDef::LoadSVG(const wxString& filename) {
	if (!m_svg->Load(filename))
		return false;
	wxSVGSVGElement* root = m_svg->GetRootElement();
	if (!root)
		return false;
	if (root->GetViewBox().GetBaseVal().IsEmpty()) {
		root->SetViewBox(wxSVGRect(0, 0, root->GetWidth().GetBaseVal().GetValue(),
				root->GetHeight().GetBaseVal().GetValue()));
	}
	root->SetWidth(200);
	root->SetHeight(200);
	m_defaultWidth.value = 64;
	m_defaultHeight.value = 64;
	UpdateIdList();
	return true;
}

wxString MenuObjectDef::ReadNodeContent(wxSvgXmlNode* node) {
	if (node->GetChildren()
			&& (node->GetChildren()->GetType() == wxSVGXML_TEXT_NODE
					|| node->GetChildren()->GetType() == wxSVGXML_CDATA_SECTION_NODE))
		return node->GetChildren()->GetContent().Strip(wxString::both);
	return "";
}

void MenuObjectDef::InitSize(wxSvgXmlNode* node, MenuObjectSize& width, MenuObjectSize& height) {
	wxString sizeElem = XmlReadValue(node, wxT("element"));
	if (sizeElem.length() > 0) {
		width.elements.Add(sizeElem);
		height.elements.Add(sizeElem);
	}
	width.SetValueAsString(XmlReadValue(node, "width"));
	height.SetValueAsString(XmlReadValue(node, "height"));
}

/** Loads object definition */
bool MenuObjectDef::Load(const wxString& filename) {
	wxSvgXmlDocument xml;
	if (!xml.Load(filename)) {
		wxLogError(_("Cannot open file '%s'."), filename.c_str());
		return false;
	}

	wxSvgXmlNode* root = xml.GetRoot();
	if (root == NULL || root->GetName() != "button") {
		wxLogError("'%s' is not a DVDStyler button file", filename.c_str());
		return false;
	}
	
	SetPreviewHighlighted(root->GetAttribute("previewHighlighted") == "true");
	
	wxSvgXmlNode* child = root->GetChildren();
	while (child) {
		if (child->GetName() == "title") {
			SetTitle(ReadNodeContent(child));
		} else if (child->GetName() == "svg") {
			wxSvgXmlDocument svgXml;
			svgXml.SetRoot(child->CloneNode());
			svgXml.GetRoot()->SetAttribute("xmlns", "http://www.w3.org/2000/svg");
			svgXml.GetRoot()->SetAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
			wxMemoryOutputStream output;
			svgXml.Save(output);
			wxMemoryInputStream input(output);
			if (!m_svg->Load(input))
				return false;
			wxSVGSVGElement* root = m_svg->GetRootElement();
			if (!root)
				return false;
			m_defaultWidth.value = root->GetWidth().GetBaseVal();
			m_defaultHeight.value = root->GetHeight().GetBaseVal();
			root->SetWidth(200);
			root->SetHeight(200);
			// black background
			wxSVGRectElement* bgRect = new wxSVGRectElement;
			bgRect->SetId("DVDStyler_bt_bg");
			if (!root->GetViewBox().GetBaseVal().IsEmpty()) {
				bgRect->SetWidth(root->GetViewBox().GetBaseVal().GetWidth());
				bgRect->SetHeight(root->GetViewBox().GetBaseVal().GetHeight());
			} else {
				bgRect->SetWidth(root->GetWidth());
				bgRect->SetHeight(root->GetHeight());
			}
			bgRect->SetFill(wxSVGPaint(*wxBLACK));
			root->InsertChild(bgRect, root->GetChildren());
			
			UpdateIdList();
		} else if (child->GetName() == "parameters") {
			wxSvgXmlNode* paramNode = child->GetChildren();
			while (paramNode) {
				MenuObjectParam* param = new MenuObjectParam;
				param->PutXML(paramNode);
				m_params.push_back(param);
				if (param->changeable) {
					param->changeable = false;
					param->normalColour = GetParamColour(param->name);
					param->changeable = true;
				}
				paramNode = paramNode->GetNext();
			}
		} else if (child->GetName() == "init-parameter") {
			SetInitParameter(ReadNodeContent(child));
		} else if (child->GetName() == "text-offset") {
			wxSVGLength l;
			l.SetValueAsString(ReadNodeContent(child));
			SetTextOffset(l);
		} else if (child->GetName() == "default-size" && child->GetFirstChild()) {
			InitSize(child, m_defaultWidth, m_defaultHeight);
		} else if (child->GetName() == "min-size" && child->GetFirstChild()) {
			InitSize(child, m_minWidth, m_minHeight);
		}
		child = child->GetNext();
	}
	return true;
}

/** Saves object definition */
bool MenuObjectDef::Save(const wxString& fileName) {
	wxSvgXmlDocument xml;
	wxSvgXmlNode* rootNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, "button");
	if (m_previewHighlighted)
		rootNode->SetAttribute("previewHighlighted", "true");
	XmlWriteValue(rootNode, "title", GetTitle());
	wxSVGSVGElement* svgCopy = (wxSVGSVGElement*) m_svg->GetRootElement()->CloneNode();
	svgCopy->SetWidth(m_defaultWidth.value);
	svgCopy->SetHeight(m_defaultHeight.value);
	wxSVGElement* bgElement = (wxSVGElement*) (svgCopy)->GetElementById("DVDStyler_bt_bg");
	if (bgElement)
		svgCopy->RemoveChild(bgElement);
	rootNode->AddChild(svgCopy);
	wxSvgXmlNode* paramsNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, "parameters");
	for (MenuObjectParam* param : GetParams()) {
		paramsNode->AddChild(param->GetXML());
	}
	rootNode->AddChild(paramsNode);
	if (GetInitParameter().length())
		XmlWriteValue(rootNode, "init-parameter", GetInitParameter());
	if (GetTextOffset().GetValue() != 0)
		XmlWriteValue(rootNode, "text-offset", GetTextOffset().GetValueAsString());
	if (m_defaultWidth.elements.size() || m_defaultHeight.elements.size()) {
		wxSvgXmlNode* defSizeNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, "default-size");
		XmlWriteValue(defSizeNode, "width", m_defaultWidth.GetValueAsString());
		XmlWriteValue(defSizeNode, "height", m_defaultHeight.GetValueAsString());
	}
	if (m_minWidth.value > 0 || m_minHeight.value > 0
			|| m_minWidth.elements.size() || m_minHeight.elements.size()) {
		wxSvgXmlNode* defSizeNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, "min-size");
		XmlWriteValue(defSizeNode, "width", m_minWidth.GetValueAsString());
		XmlWriteValue(defSizeNode, "height", m_minHeight.GetValueAsString());
	}
	xml.SetRoot(rootNode);
	return xml.Save(fileName);
}

const wxString paramTypesArray[] = { "colour", "decimal", "image", "integer", "integer(0,360)",
		"integer(-180,180)", "integer(-999,999)", "percent", "shadow", "string", "text" };
static wxArrayString s_paramTypes(11, paramTypesArray);

const wxArrayString& MenuObjectDef::GetParamTypes() {
	return s_paramTypes;
}

wxSVGSVGElement* MenuObjectDef::GetButtonSVG() const {
	return GetSVG()->GetRootElement();
}

/** Returns object SVG element by element id */
wxSVGElement* MenuObjectDef::GetElementById(wxString id) const {
	return (wxSVGElement*) GetButtonSVG()->GetElementById(id);
}

MenuObjectParam* MenuObjectDef::GetParamByName(wxString name) const {
	for (unsigned int i = 0; i < m_params.size(); i++) {
		if (m_params[i]->name == name)
			return m_params[i];
	}
	return NULL;
}

MenuObjectParam* MenuObjectDef::GetInitParam() {
	if (!m_initParameter.length())
		return NULL;
	return GetParamByName(m_initParameter);
}

MenuObjectParam* MenuObjectDef::GetImageParam() {
	for (unsigned int i = 0; i < m_params.size(); i++) {
		if (m_params[i]->type == wxT("image"))
			return m_params[i];
	}
	return NULL;
}

wxString MenuObjectDef::GetParam(wxString name, wxString attribute) const {
	MenuObjectParam* param = GetParamByName(name);
	if (!param || param->element.size() == 0)
		return wxT("");
	wxSVGElement* elem = GetElementById(param->element.front());
	if (!elem)
		return wxT("");
	if (param->attribute.length()) {
		// return attribute value
		if (param->attribute.Find(wxT('#')) > 0) {
			wxString value = elem->GetAttribute(param->attribute.BeforeFirst(wxT('#')));
			long n = 0;
			param->attribute.AfterFirst(wxT('#')).ToLong(&n);
			if (n > 0 && value.length() > 0 && value.Index(wxT('(')) > 0) {
				// return n-parameter of the function (e.g. if you want to get rotation angle of transform)
				value = value.AfterFirst(wxT('(')).BeforeFirst(wxT(')'));
				for (int i = 1; i < n; i++)
					value = value.AfterFirst(wxT(','));
				value = value.BeforeFirst(wxT(','));
				return value.Trim().Trim(false);
			}
		}
		return elem->GetAttribute(attribute.length() && attribute[0] != wxT('-') ?
				attribute : param->attribute + attribute);
	} else if (elem->GetDtd() == wxSVG_TEXT_ELEMENT) {
		if (attribute.length())
			return elem->GetAttribute(attribute);
		wxString value;
		wxSvgXmlNode* child = elem->GetChildren();
		while (child != NULL) {
			if (child->GetType() == wxSVGXML_TEXT_NODE)
				value += child->GetContent().Strip(wxString::both);
			else if (child->GetType() == wxSVGXML_ELEMENT_NODE
					&& ((wxSVGElement*) child)->GetDtd() == wxSVG_TBREAK_ELEMENT)
				value += wxT("\n");
			child = child->GetNextSibling();
		}
		return value;
	}
	return wxT("");
}

void MenuObjectDef::SetParam(wxString name, wxString value, wxString attribute) {
	MenuObjectParam* param = GetParamByName(name);
	if (!param)
		return;
	for (vector<wxString>::const_iterator elemIt = param->element.begin(); elemIt != param->element.end(); elemIt++) {
		wxSVGElement* elem = GetElementById(*elemIt);
		if (!elem)
			continue;
		if (param->attribute.length()) {
			if (param->attribute.Find(wxT('#')) > 0) {
				// sets n-parameter of the function (e.g. if you want to set rotation angle of transform)
				wxString oldValue = elem->GetAttribute(param->attribute.BeforeFirst(wxT('#')));
				long n = 0;
				param->attribute.AfterFirst(wxT('#')).ToLong(&n);
				if (n > 0 && oldValue.length() > 0 && oldValue.Index(wxT('(')) > 0) {
					wxString newValue = oldValue.BeforeFirst(wxT('(')) + wxT('(');
					for (int i = 1; i < n; i++)
						newValue += oldValue.BeforeFirst(wxT(',')) + wxT(',');
					newValue += value;
					if (oldValue.Index(wxT(')')) < oldValue.Index(wxT(',')) && oldValue.Index(wxT(')')) > 0)
						newValue += wxT(')') + oldValue.AfterFirst(wxT(')'));
					else
						newValue += wxT(',') + oldValue.AfterFirst(wxT(','));
					value = newValue;
				}
				elem->SetAttribute(param->attribute.BeforeFirst(wxT('#')), value);
			} else {
				elem->SetAttribute(attribute.length() && attribute[0] != wxT('-') ?
						attribute : param->attribute + attribute, value);
			}
		} else if (elem->GetDtd() == wxSVG_TEXT_ELEMENT) {
			if (attribute.length()) {
				elem->SetAttribute(attribute, value);
				return;
			}
			wxStringTokenizer tokenizer(value, wxT("\r\n"), wxTOKEN_RET_EMPTY_ALL);
			wxSvgXmlNode* child = elem->GetChildren();
			while (tokenizer.HasMoreTokens()) {
				wxString token = tokenizer.GetNextToken();
				// insert text node (token)
				if (child != NULL && child->GetType() == wxSVGXML_TEXT_NODE) {
					child->SetContent(token);
					child = child->GetNextSibling();
				} else
					elem->InsertChild(new wxSvgXmlNode(wxSVGXML_TEXT_NODE, wxEmptyString, token), child);
				// insert t-break
				if (tokenizer.HasMoreTokens()) {
					if (child != NULL && child->GetType() == wxSVGXML_ELEMENT_NODE
							&& ((wxSVGElement*) child)->GetDtd() != wxSVG_TBREAK_ELEMENT)
						child = child->GetNextSibling();
					else
						elem->InsertChild(new wxSVGTBreakElement(), child);
				}
				((wxSVGTextElement*) elem)->SetXmlspace(wxT("preserve"));
			}
			while (child != NULL) {
				wxSvgXmlNode* nextChild = child->GetNextSibling();
				elem->RemoveChild(child);
				child = nextChild;
			}
		}
	}
}

int MenuObjectDef::GetParamInt(wxString name, wxString attribute) const {
	long lval = 0;
	GetParam(name, attribute).ToLong(&lval);
	return lval;
}

void MenuObjectDef::SetParamInt(wxString name, int value, wxString attribute) {
	SetParam(name, wxString::Format(wxT("%d"), value), attribute);
}

double MenuObjectDef::GetParamDouble(wxString name) const {
	double dval = 0;
	GetParam(name).ToDouble(&dval);
	return dval;
}

void MenuObjectDef::SetParamDouble(wxString name, double value) {
	SetParam(name, wxString::Format(wxT("%g"), value));
}

wxFont MenuObjectDef::GetParamFont(wxString name) const {
	MenuObjectParam* param = GetParamByName(name);
	if (!param || param->element.size() == 0)
		return wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false);
	wxSVGElement* elem = GetElementById(param->element.front());
	if (!elem) {
		return wxFont(20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false);
	}

	int size = 20;
	double dval;
	if (elem->GetAttribute(wxT("font-size")).ToDouble(&dval))
		size = (int) dval;

	wxFontStyle style = wxFONTSTYLE_NORMAL;
	wxString styleStr = elem->GetAttribute(wxT("font-style"));
	if (styleStr == wxT("italic"))
		style = wxFONTSTYLE_ITALIC;
	else if (styleStr == wxT("oblique"))
		style = wxFONTSTYLE_SLANT;

	wxFontWeight weight = wxFONTWEIGHT_NORMAL;
	wxString weightStr = elem->GetAttribute(wxT("font-weight"));
	if (weightStr == wxT("bold"))
		weight = wxFONTWEIGHT_BOLD;
	if (weightStr == wxT("bolder"))
		weight = wxFONTWEIGHT_MAX;
	else if (weightStr == wxT("lighter"))
		weight = wxFONTWEIGHT_LIGHT;

	wxString faceName = elem->GetAttribute(wxT("font-family"));

	return wxFont(size, wxFONTFAMILY_DEFAULT, style, weight, false, faceName);
}

void MenuObjectDef::SetParamFont(wxString name, wxFont value) {
	MenuObjectParam* param = GetParamByName(name);
	if (!param)
		return;
	for (vector<wxString>::const_iterator elemIt = param->element.begin(); elemIt != param->element.end(); elemIt++) {
		wxSVGElement* elem = GetElementById(*elemIt);
		if (!elem)
			continue;
	
		elem->SetAttribute(wxT("font-size"), wxString::Format(wxT("%d"), value.GetPointSize()));
	
		wxString styleStr = wxT("normal");
		if (value.GetStyle() == wxFONTSTYLE_ITALIC)
			styleStr = wxT("italic");
		else if (value.GetStyle() == wxFONTSTYLE_SLANT)
			styleStr = wxT("oblique");
		elem->SetAttribute(wxT("font-style"), styleStr);
	
		wxString weightStr = wxT("normal");
		if (value.GetWeight() == wxFONTWEIGHT_BOLD)
			weightStr = wxT("bold");
		if (value.GetWeight() == wxFONTWEIGHT_MAX)
			weightStr = wxT("bolder");
		else if (value.GetWeight() == wxFONTWEIGHT_LIGHT)
			weightStr = wxT("lighter");
		elem->SetAttribute(wxT("font-weight"), weightStr);
	
		elem->SetAttribute(wxT("font-family"), value.GetFaceName());
		
		if (elem->GetDtd() == wxSVG_TEXT_ELEMENT)
			((wxSVGTextElement*) elem)->SetCanvasItem(NULL);
	}
}

wxColour MenuObjectDef::GetParamColour(wxString name, MenuButtonState state) const {
	MenuObjectParam* param = GetParamByName(name);
	if (!param)
		return wxColour();
	if (param->changeable) {
		switch (state) {
		case mbsNORMAL:
			return param->normalColour;
		case mbsHIGHLIGHTED:
			return param->highlightedColour;
		case mbsSELECTED:
			return param->selectedColour;
		}
	}
	if (!param->element.size())
		return wxColour();
	wxSVGElement* elem = GetElementById(param->element.front());
	if (!elem)
		return wxColour();
	if (param->attribute.length()) {
		const wxCSSStyleDeclaration& style = wxSVGStylable::GetElementStyle(*elem);
		const wxCSSValue& value = style.GetPropertyCSSValue(param->attribute);
		switch (value.GetCSSValueType()) {
		case wxCSS_PRIMITIVE_VALUE:
			return ((wxCSSPrimitiveValue&) value).GetRGBColorValue();
		case wxCSS_SVG_COLOR:
		case wxCSS_SVG_PAINT:
			return ((wxSVGColor&) value).GetRGBColor();
		default:
			break;
		}
	}
	return wxColour();
}

void MenuObjectDef::SetParamColour(wxString name, wxColour value, MenuButtonState state) {
	MenuObjectParam* param = GetParamByName(name);
	if (!param)
		return;
	if (param->changeable) {
		switch (state) {
		case mbsNORMAL:
			param->normalColour = value;
			break;
		case mbsHIGHLIGHTED:
			param->highlightedColour = value;
			break;
		case mbsSELECTED:
			param->selectedColour = value;
			break;
		}
	}
	if (state != mbsNORMAL || param->element.size() == 0)
		return;
	wxSVGElement* elem = GetElementById(param->element.front());
	if (!elem)
		return;
	if (param->attribute.length()) {
		wxSVGPaint paint(value);
		elem->SetAttribute(param->attribute, paint.GetCSSText());
	}
}

double MenuObjectDef::GetParamVideoClipBegin(const wxString& name) {
	MenuObjectParam* param = GetParamByName(name);
	if (!param || param->element.size() == 0)
		return 0;
	wxSVGElement* elem = GetElementById(param->element.front());
	if (!elem || elem->GetDtd() != wxSVG_VIDEO_ELEMENT)
		return 0;
	return ((wxSVGVideoElement*) elem)->GetClipBegin();
}

double MenuObjectDef::GetParamVideoDuration(const wxString& name) {
	MenuObjectParam* param = GetParamByName(name);
	if (!param || param->element.size() == 0)
		return 0;
	wxSVGElement* elem = GetElementById(param->element.front());
	if (!elem || elem->GetDtd() != wxSVG_VIDEO_ELEMENT)
		return 0;
	return ((wxSVGVideoElement*) elem)->GetDur();
}

void MenuObjectDef::SetParamImageVideo(const wxString& name, const wxString& filename, long pos, int duration) {
	MenuObjectParam* param = GetParamByName(name);
	if (!param || param->element.size() == 0)
		return;
	for (vector<wxString>::const_iterator elemIt = param->element.begin(); elemIt != param->element.end(); elemIt++) {
		MenuObjectDef::SetImageVideoParams(GetButtonSVG(), *elemIt, filename, pos, duration);
	}
}

void MenuObjectDef::SetImageVideoParams(wxSVGSVGElement* svgElem, const wxString& id, const wxString& filename,
		long pos, int duration) {
	wxSVGElement* elem = (wxSVGElement*) svgElem->GetElementById(id);
	if (!elem || (elem->GetDtd() != wxSVG_IMAGE_ELEMENT && elem->GetDtd() != wxSVG_VIDEO_ELEMENT))
		return;
	if (duration <= 0) {
		if (elem->GetDtd() != wxSVG_IMAGE_ELEMENT) {
			wxSVGVideoElement* oldElem = (wxSVGVideoElement*) elem;
			wxSVGImageElement* newElem = new wxSVGImageElement;
			newElem->SetId(oldElem->GetId());
			newElem->SetX(oldElem->GetX().GetBaseVal());
			newElem->SetY(oldElem->GetY().GetBaseVal());
			newElem->SetWidth(oldElem->GetWidth().GetBaseVal());
			newElem->SetHeight(oldElem->GetHeight().GetBaseVal());
			newElem->SetStyle(oldElem->GetStyle());
			newElem->SetPreserveAspectRatio(oldElem->GetPreserveAspectRatio());
			oldElem->GetParent()->InsertBefore(newElem, oldElem);
			oldElem->GetParent()->RemoveChild(oldElem);
			elem = newElem;
		}
		wxString href = filename + (filename.length() && pos >= 0 ? wxString::Format(wxT("#%ld"), pos) : wxT(""));
		if (href != ((wxSVGImageElement*) elem)->GetHref()) {
			((wxSVGImageElement*) elem)->SetCanvasItem(NULL);
			((wxSVGImageElement*) elem)->SetHref(href);
		}
	} else {
		if (elem->GetDtd() != wxSVG_VIDEO_ELEMENT) {
			wxSVGImageElement* oldElem = (wxSVGImageElement*) elem;
			wxSVGVideoElement* newElem = new wxSVGVideoElement;
			newElem->SetId(oldElem->GetId());
			newElem->SetX(oldElem->GetX().GetBaseVal());
			newElem->SetY(oldElem->GetY().GetBaseVal());
			newElem->SetWidth(oldElem->GetWidth().GetBaseVal());
			newElem->SetHeight(oldElem->GetHeight().GetBaseVal());
			newElem->SetStyle(oldElem->GetStyle());
			newElem->SetPreserveAspectRatio(oldElem->GetPreserveAspectRatio());
			oldElem->GetParent()->InsertBefore(newElem, oldElem);
			oldElem->GetParent()->RemoveChild(oldElem);
			elem = newElem;
		}
		if (filename != ((wxSVGVideoElement*) elem)->GetHref()) {
			((wxSVGVideoElement*) elem)->SetCanvasItem(NULL);
			((wxSVGVideoElement*) elem)->SetHref(filename);
		}
		((wxSVGVideoElement*) elem)->SetClipBegin(((double)pos)/1000);
		((wxSVGVideoElement*) elem)->SetDur(duration);
	}
}

void MenuObjectDef::SetButtonState(MenuButtonState buttonState) {
	if (m_buttonState == buttonState)
		return;
	m_buttonState = buttonState;
	for (MenuObjectParam* p : GetParams()) {
		if (p->isChangeable()) {
			p->changeable = false;
			switch (buttonState) {
			case mbsHIGHLIGHTED:
				SetParamColour(p->name, p->highlightedColour);
				break;
			case mbsSELECTED:
				SetParamColour(p->name, p->selectedColour);
				break;
			default:
				SetParamColour(p->name, p->normalColour);
				break;
			}
			p->changeable = true;
		}
	}
}
