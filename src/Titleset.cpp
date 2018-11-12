/////////////////////////////////////////////////////////////////////////////
// Name:        Titleset.cpp
// Purpose:     The class to store a DVD Titleset
// Author:      Alex Thuering
// Created:     29.01.2003
// RCS-ID:      $Id: Titleset.cpp,v 1.16 2016/03/12 17:48:16 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "Titleset.h"
#include "DVD.h"
#include "Menu.h"
#include "Utils.h"
#include "Config.h"
#include <wxVillaLib/utils.h>
#include <wxSVGXML/svgxmlhelpr.h>
#include <wx/sstream.h>
#include <wx/log.h>

////////////////////////////// Video ///////////////////////////////////
wxArrayString Video::GetAspectStrings(wxString def) {
	wxArrayString ret;
	ret.Add(def);
	ret.Add(wxT("4:3"));
	ret.Add(wxT("16:9"));
	return ret;
}

wxArrayString Video::GetWidescreenStrings(wxString def) {
	wxArrayString ret;
	ret.Add(def);
	ret.Add(wxT("nopanscan"));
	ret.Add(wxT("noletterbox"));
	return ret;
}

wxSvgXmlNode* Video::GetXML(DVDFileType type, bool menu) {
	wxSvgXmlNode* node = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("video"));
	
	if (m_format != vfNONE && m_format != vfCOPY)
		node->AddProperty(wxT("format"), isNTSC(m_format) ? wxT("ntsc") : wxT("pal"));
	if (m_aspect != arAUTO)
		node->AddProperty(wxT("aspect"), GetAspectStrings()[m_aspect]);
	if (m_widescreen != wtAUTO && (m_aspect == ar16_9 || type == DVDSTYLER_XML))
		node->AddProperty(wxT("widescreen"), GetWidescreenStrings()[m_widescreen]);
	if (m_resolution.length())
		node->AddProperty(wxT("resolution"), m_resolution);
	if (m_caption.length())
		node->AddProperty(wxT("caption"), m_caption);

	if (node->GetProperties() == NULL) {
		delete node;
		node = NULL;
	}
	return node;
}

bool Video::PutXML(wxSvgXmlNode* node, bool menu) {
	if (node == NULL || node->GetName() != wxT("video"))
		return false;
	
	wxString val;
	if (node->GetPropVal(wxT("format"), &val))
		m_format = val == wxT("ntsc") ? vfNTSC : (val == wxT("pal") ? vfPAL : vfCOPY);
	if (node->GetPropVal(wxT("aspect"), &val) && GetAspectStrings().Index(val) > 0)
		m_aspect = (AspectRatio) GetAspectStrings().Index(val);
	if (node->GetPropVal(wxT("widescreen"), &val) && GetWidescreenStrings().Index(val) > 0)
		m_widescreen = (WidescreenType) GetWidescreenStrings().Index(val);
	node->GetPropVal(wxT("resolution"), &m_resolution);
	node->GetPropVal(wxT("caption"), &m_caption);

	return true;
}

////////////////////////////////// SubPicture //////////////////////////////////////
/** Destructor */
SubPicture::~SubPicture() {
	VECTOR_CLEAR(m_streams, SubStream)
}

wxSvgXmlNode* SubPicture::GetXML(DVDFileType type, Video& video, bool menu) {
	wxSvgXmlNode* node = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("subpicture"));
	
	if (m_langCode.length())
		node->AddProperty(wxT("lang"), m_langCode);
	for (vector<SubStream*>::iterator it = m_streams.begin(); it != m_streams.end(); it++)
		node->AddChild((*it)->GetXML(type));
	if (type == DVDAUTHOR_XML && menu && m_streams.size() == 0 && video.GetAspect() == ar16_9) {
		node->AddChild(SubStream(wxT("0"), ssmWIDESCREEN).GetXML(type));
		if (video.GetWidescreen() == wtNOLETTERBOX)
			node->AddChild(SubStream(wxT("1"), ssmPANSCAN).GetXML(type));
		else
			node->AddChild(SubStream(wxT("1"), ssmLETTERBOX).GetXML(type));
		if (video.GetWidescreen() == wtAUTO)
			node->AddChild(SubStream(wxT("2"), ssmPANSCAN).GetXML(type));
	}
	
	return node;
}

bool SubPicture::PutXML(wxSvgXmlNode* node) {
	if (node == NULL || node->GetName() != wxT("subpicture"))
		return false;
	
	wxString val;
	if (node->GetPropVal(wxT("lang"), &val))
		m_langCode = val;
	
	for (wxSvgXmlNode* child = node->GetChildren(); child != NULL; child = child->GetNext()) {
		SubStream* subStream = new SubStream();
		if (subStream->PutXML(child))
			m_streams.push_back(subStream);
		else
			delete subStream;
	}
	
	return true;
}

////////////////////////////////// PgcArray //////////////////////////////////////
/** Destructor */
PgcArray::~PgcArray() {
	VECTOR_CLEAR(m_subPictures, SubPicture)
}

unsigned int PgcArray::GetAudioStreamCount() {
	unsigned int count = 1; // at least one audio stream
	for (iterator it = begin(); it != end(); it++)
		for (VobArray::iterator it2 = (*it)->GetVobs().begin(); it2 != (*it)->GetVobs().end(); it2++)
			if (count < (*it2)->GetAudioStreamCount())
				count = (*it2)->GetAudioStreamCount();
	return count;
}

unsigned int PgcArray::GetSubtitleStreamsCount() {
	unsigned int count = 0;
	for (iterator it = begin(); it != end(); it++)
		for (VobArray::iterator it2 = (*it)->GetVobs().begin(); it2 != (*it)->GetVobs().end(); it2++)
			if (count < (*it2)->GetSubtitleStreamsCount())
				count = (*it2)->GetSubtitleStreamsCount();
	return count;
}

void PgcArray::UpdateAudioLangCodes() {
	unsigned int audioCnt = GetAudioStreamCount();
	while (m_audioLangCodes.size() < audioCnt)
		m_audioLangCodes.push_back(s_config.GetDefAudioLanguage());
	while (m_audioLangCodes.size() > audioCnt)
			m_audioLangCodes.pop_back();
}

void PgcArray::UpdateSubtitleLangCodes() {
	unsigned int subtitleCnt = GetSubtitleStreamsCount();
	while (m_subPictures.size() < subtitleCnt)
		m_subPictures.push_back(new SubPicture(s_config.GetDefSubtitleLanguage()));
	while (m_subPictures.size() > subtitleCnt)
		m_subPictures.pop_back();
}

/** Sets video format */
void PgcArray::SetVideoFormat(VideoFormat videoFormat) {
	m_video.SetFormat(videoFormat);
	for (unsigned int pgci=0; pgci<GetCount(); pgci++) {
		Menu* menu = (*this)[pgci]->GetMenu();
		if (menu != NULL)
			menu->SetVideoFormat(videoFormat);
	}
}

/** Sets aspect ratio */
void PgcArray::SetAspectRatio(AspectRatio aspectRatio) {
	m_video.SetAspect(aspectRatio);
	for (unsigned int pgci=0; pgci<GetCount(); pgci++) {
		Menu* menu = (*this)[pgci]->GetMenu();
		if (menu != NULL)
			menu->SetAspectRatio(aspectRatio);
	}
}

/** Sets post commands */
void PgcArray::SetPostCommands(wxString postCommands) {
	for (unsigned int pgci=0; pgci<GetCount(); pgci++) {
		Pgc* pgc = (*this)[pgci];
		pgc->SetPostCommands(postCommands);
	}
}

/** Returns true if at least one title use "call last menu" command */
bool PgcArray::HasCallLastMenu() {
	for (int pgci = 0; pgci<(int)Count(); pgci++) {
		Pgc* pgc = (*this)[pgci];
		if (pgc->GetPreCommands().Find(wxT("call last menu")) >= 0
				|| pgc->GetPostCommands().Find(wxT("call last menu")) >= 0)
			return true;
		for (int vobi = 0; vobi<(int)pgc->GetVobs().Count(); vobi++) {
			Vob* vob = pgc->GetVobs()[vobi];
			for (vector<Cell*>::iterator it = vob->GetCells().begin(); it != vob->GetCells().end(); it++)
				if ((*it)->GetCommands().Find(wxT("call last menu")) >= 0)
					return true;
		}
	}
	return false;
}

////////////////////////////////// Menus //////////////////////////////////////
StringSet Menus::GetEntries() {
	StringSet entries;
	for (unsigned int pgci = 0; pgci < Count(); pgci++) {
		const StringSet& pgcEntries = Item(pgci)->GetEntries();
		for (StringSet::const_iterator entry = pgcEntries.begin(); entry != pgcEntries.end(); entry++) {
			entries.insert(*entry);
		}
	}
	return entries;
}

int Menus::GetPgciByEntry(wxString entry) {
	for (unsigned int pgci = 0; pgci < Count(); pgci++) {
		const StringSet& pgcEntries = Item(pgci)->GetEntries();
		for (StringSet::const_iterator e = pgcEntries.begin(); e != pgcEntries.end(); e++) {
			if (*e == entry)
				return pgci;
		}
	}
	return entry == wxT("root") && Count() > 0 ? 0 : -1;
}

wxSvgXmlNode* Menus::GetXML(DVDFileType type, DVD* dvd, bool vmgm) {
	wxSvgXmlNode* node = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("menus"));
	wxSvgXmlNode* videoNode = GetVideo().GetXML(type, true);
	if (videoNode)
		node->AddChild(videoNode);
	UpdateAudioLangCodes();
	for (wxArrayString::iterator it = GetAudioLangCodes().begin(); it != GetAudioLangCodes().end(); it++) {
		wxSvgXmlNode* audio = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("audio"));
		audio->AddProperty(wxT("lang"), *it);
		node->AddChild(audio);
	}
	UpdateSubtitleLangCodes();
	for (vector<SubPicture*>::iterator it = GetSubPictures().begin(); it != GetSubPictures().end(); it++) {
		node->AddChild((*it)->GetXML(type, m_video, true));
	}
	for (int i = 0; i < (int) GetCount(); i++) {
		node->AddChild(new wxSvgXmlNode(wxSVGXML_COMMENT_NODE, wxT(""),
				wxString::Format(wxT("************** MENU %d **************"), i + 1)));
		node->AddChild((*this)[i]->GetXML(type, dvd, *this, vmgm));
	}
	return node;
}

bool Menus::PutXML(wxSvgXmlNode* node, DVD* dvd, int tsi, wxProgressDialog* progressDlg) {
	if (node == NULL || node->GetName() != wxT("menus"))
		return false;
	WX_CLEAR_ARRAY(*this);
	wxSvgXmlNode* child = node->GetChildren();
	while (child) {
		if (child->GetName() == wxT("audio") && child->HasAttribute(wxT("lang")))
			GetAudioLangCodes().Add(child->GetAttribute(wxT("lang")));
		else if (child->GetName() == wxT("video"))
			m_video.PutXML(child, true);
		else if (child->GetName() == wxT("pgc")) {
			Pgc* pgc = new Pgc;
			if (pgc->PutXML(child, dvd, tsi, GetCount(), true, progressDlg))
				Add(pgc);
			else {
				delete pgc;
				wxLogError(_("Can't load pgc"));
				return false;
			}
		}
		child = child->GetNext();
	}
	if (m_video.GetAspect() == arAUTO && GetCount() > 0) {
		for (int i = 0; i < (int) GetCount(); i++) {
			Menu* menu = (*this)[i]->GetMenu();
			if (menu != NULL) {
				m_video.SetAspect(menu->GetAspectRatio());
				break;
			}
		}
	}
	return true;
}

////////////////////////////////// Titles ////////////////////////////////////

wxSvgXmlNode* Titles::GetXML(DVDFileType type, DVD* dvd, int nextTitleset) {
	wxSvgXmlNode* node = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("titles"));
	wxSvgXmlNode* videoNode = GetVideo().GetXML(type, false);
	if (videoNode)
		node->AddChild(videoNode);
	UpdateAudioLangCodes();
	for (wxArrayString::iterator it = GetAudioLangCodes().begin(); it != GetAudioLangCodes().end(); it++) {
		wxSvgXmlNode* audio = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("audio"));
		audio->AddProperty(wxT("lang"), *it);
		node->AddChild(audio);
	}
	UpdateSubtitleLangCodes();
	for (vector<SubPicture*>::iterator it = GetSubPictures().begin(); it != GetSubPictures().end(); it++) {
		node->AddChild((*it)->GetXML(type, m_video, false));
	}
	for (int i = 0; i < (int) GetCount(); i++) {
		node->AddChild(new wxSvgXmlNode(wxSVGXML_COMMENT_NODE, wxT(""),
				wxString::Format(wxT("************** TITLE %d **************"), i + 1)));
		wxString nextTitle;
		node->AddChild((*this)[i]->GetXML(type, dvd, *this, false, i != (int) GetCount() - 1 ? i + 2: -1, nextTitleset));
	}
	return node;
}

bool Titles::PutXML(wxSvgXmlNode* node, DVD* dvd, int tsi, wxProgressDialog* progressDlg) {
	if (node == NULL || node->GetName() != wxT("titles"))
		return false;
	wxSvgXmlNode* child = node->GetChildren();
	while (child) {
		if (child->GetName() == wxT("audio") && child->HasAttribute(wxT("lang"))) {
			GetAudioLangCodes().Add(child->GetAttribute(wxT("lang")));
		} else if (child->GetName() == wxT("subpicture")) {
			SubPicture* subPicture = new SubPicture();
			if (subPicture->PutXML(child))
				GetSubPictures().push_back(subPicture);
		} else if (child->GetName() == wxT("video")) {
			GetVideo().PutXML(child, false);
		} else if (child->GetName() == wxT("pgc")) {
			Pgc* pgc = new Pgc;
			if (pgc->PutXML(child, dvd, tsi, GetCount(), false, progressDlg)) {
				Add(pgc);
				if (GetCount() == 1 && pgc->GetVobs().GetCount() > 0 && pgc->GetVobs()[0]->GetVideoStream())
					GetVideo().SetFormat(pgc->GetVobs()[0]->GetVideoStream()->GetVideoFormat());;
			} else {
				delete pgc;
				wxLogError(_("Can't load pgc"));
				return false;
			}
		}
		child = child->GetNext();
	}
	// set aspect ratio if it isn't set
	if (GetVideo().GetAspect() == arAUTO && GetCount() > 0 && Item(0)->GetVobs().GetCount() > 0) {
		if (Item(0)->GetVobs()[0]->GetSlideshow() != NULL) {
			GetVideo().SetAspect(Item(0)->GetVobs()[0]->GetSlideshow()->GetAspectRatio());
		} else {
			Stream* videoSt = Item(0)->GetVobs()[0]->GetVideoStream();
			float aspect = videoSt != NULL ? videoSt->GetSourceAspectRatio() : 0;
			if (aspect > 0)
				GetVideo().SetAspect(aspect < 1.4 ? ar4_3 : ar16_9);
			else
				GetVideo().SetAspect(dvd->GetAspectRatio());
		}
	}
	return true;
}

////////////////////////////////// Titleset //////////////////////////////////

wxSvgXmlNode* Titleset::GetXML(DVDFileType type, DVD* dvd, int nextTitleset) {
	wxSvgXmlNode* node = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("titleset"));
	if (m_menus.Count() || type != DVDAUTHOR_XML)
		node->AddChild(m_menus.GetXML(type, dvd));
	else if (dvd->IsEmptyMenuEnabled() && dvd->HasMenus()) {
		// create empty menu with post command
		Menus menus;
		Pgc* pgc = new Pgc;
		pgc->GetEntries().insert(wxT("root"));
		pgc->SetPreCommands(wxT("jump vmgm menu;"));
		pgc->GetVobs().Add(Vob::CreateEmptyVob(dvd->GetVideoFormat(), dvd->GetAudioFormat()));
		Menu* menu = new Menu(dvd, -1, 0);
		menu->SetRememberLastButton(false);
		pgc->GetVobs()[0]->SetMenu(menu);
		menus.Add(pgc);
		node->AddChild(menus.GetXML(type, dvd));
	}
	node->AddChild(m_titles.GetXML(type, dvd, nextTitleset));
	return node;
}

bool Titleset::PutXML(wxSvgXmlNode* node, DVD* dvd, int tsi, wxProgressDialog* progressDlg) {
	if (node == NULL || node->GetName() != wxT("titleset"))
		return false;
	wxSvgXmlNode* child = node->GetChildren();
	while (child) {
		if (child->GetName() == wxT("menus")) {
			if (!GetMenus().PutXML(child, dvd, tsi, progressDlg)) {
				wxLogError(_("Can't load menus"));
				return false;
			}
		} else if (child->GetName() == wxT("titles")) {
			if (!GetTitles().PutXML(child, dvd, tsi, progressDlg)) {
				wxLogError(_("Can't load titles"));
				return false;
			}
		}
		child = child->GetNext();
	}
	return true;
}

////////////////////////////////// VMGM //////////////////////////////////////

wxSvgXmlNode* Vmgm::GetXML(DVDFileType type, DVD* dvd) {
	wxSvgXmlNode* node = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("vmgm"));
	// add fgc
	if (GetFpc().length())
		XmlWriteValue(node, wxT("fpc"), GetFpc());

	// add menus
	if (type == DVDSTYLER_XML || Count())
		node->AddChild(Menus::GetXML(type, dvd, true));
	else {
		// create empty vmgm with post command
		Menus menus;
		menus.GetVideo().SetFormat(dvd->GetVideoFormat());
		menus.GetVideo().SetAspect(dvd->GetAspectRatio());
		if (dvd->IsEmptyMenuEnabled() && dvd->HasMenus()) {
			Pgc* pgc = new Pgc;
			pgc->SetPreCommands(wxT("jump titleset 1 menu;"));
			pgc->GetVobs().Add(Vob::CreateEmptyVob(dvd->GetVideoFormat(), dvd->GetAudioFormat()));
			menus.Add(pgc);
		}
		node->AddChild(menus.GetXML(DVDAUTHOR_XML, dvd, true));
	}
	return node;
}

bool Vmgm::PutXML(wxSvgXmlNode* node, DVD* dvd, wxProgressDialog* progressDlg) {
	wxSvgXmlNode* child = node->GetChildren();
	while (child) {
		if (child->GetName() == wxT("menus"))
			Menus::PutXML(child, dvd, -1, progressDlg);
		else if (child->GetName() == wxT("fpc"))
			SetFpc(child->GetChildren()->GetContent());
		child = child->GetNext();
	}
	return true;
}
