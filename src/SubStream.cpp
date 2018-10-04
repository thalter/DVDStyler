/////////////////////////////////////////////////////////////////////////////
// Name:        SubPicture.cpp
// Author:      Alex Thuering
// Created:	    24.02.2018
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "SubStream.h"
#include <wxSVGXML/svgxmlhelpr.h>

wxString s_subModeArray[4] = { "normal", "widescreen", "letterbox", "panscan" };
wxArrayString s_subModes(4, s_subModeArray);

wxString s_subConentArray[10] = { "normal", "large", "children", "normal_cc", "large_cc",
		"children_cc", "forced", "director", "large_director", "children_director" };
wxArrayString s_subConents(10, s_subConentArray);

wxSvgXmlNode* SubStream::GetXML(DVDFileType type) {
	wxSvgXmlNode* node = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, "stream");
	if (m_id.length())
		node->AddProperty("id", m_id);
	if (m_mode != ssmNORMAL)
		node->AddProperty("mode", s_subModes[m_mode]);
	if (m_content != sscNORMAL)
		node->AddProperty("content", s_subConents[m_content]);
	return node;
}

bool SubStream::PutXML(wxSvgXmlNode* node) {
	if (node == NULL || node->GetName() != "stream")
		return false;

	wxString val;
	m_id = node->GetPropVal("id", "");
	int ival = node->GetPropVal("mode", &val) && s_subModes.Index(val) > 0 ? s_subModes.Index(val) : 0;
	m_mode = (SubStreamMode) ival;
	ival = node->GetPropVal("content", &val) && s_subConents.Index(val) > 0 ? s_subConents.Index(val) : 0;
	m_content = (SubStreamContent) ival;

	return true;
}
