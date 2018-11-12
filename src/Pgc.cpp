/////////////////////////////////////////////////////////////////////////////
// Name:        Pgc.cpp
// Purpose:     The class to store a PGC
// Author:      Alex Thuering
// Created:     29.01.2003
// RCS-ID:      $Id: Pgc.cpp,v 1.59 2015/09/25 13:36:50 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "Pgc.h"
#include "Menu.h"
#include "DVD.h"
#include "Config.h"
#include <wx/tokenzr.h>
#include <wx/progdlg.h>
#include <wxSVGXML/svgxmlhelpr.h>
#include <wxVillaLib/utils.h>

void ReplaceCallLastMenu(wxString& commands, DVD* dvd) {
	int reg = dvd->GetLastMenuRegister();
	if (reg == -1)
		return;
	wxString cmd = wxT("call last menu");
	int stPos = commands.Find(cmd);
	if (stPos >= 0)
		commands.replace(stPos, cmd.length(), wxString::Format(wxT("g%d|=0x8000; call menu entry root"), reg));
}

wxString Pgc::GetEntriesAsString(bool vmgm) {
	wxString result;
	for (StringSet::const_iterator entry = m_entries.begin(); entry != m_entries.end(); entry++) {
		if (vmgm && *entry != wxT("title")) // only entry "title" is allowed for vmgm menu
			continue;
		if (result.length())
			result += wxT(",");
		result += *entry;
	}
	return result;
}

void Pgc::RemoveEntries(const StringSet& entries) {
	for (StringSet::const_iterator entry = entries.begin(); entry != entries.end(); entry++) {
		m_entries.erase(*entry);
	}
}

/** Returns menu */
Menu* Pgc::GetMenu() {
	for (int i = 0; i < (int) GetVobs().size(); i++)
		if (GetVobs()[i]->GetMenu())
			return GetVobs()[i]->GetMenu();
	return NULL;
}

/** Returns slideshow */
Slideshow* Pgc::GetSlideshow() {
	for (int i = 0; i < (int) GetVobs().size(); i++)
		if (GetVobs()[i]->GetSlideshow())
			return GetVobs()[i]->GetSlideshow();
	return NULL;
}

unsigned int Pgc::GetChapterCount(int lastVobi) {
	if (lastVobi == -1)
		lastVobi = (int) GetVobs().Count();
	int result = 0;
	for (int vi = 0; vi < lastVobi; vi++) {
		Vob& vob = *GetVobs()[vi];
		if (vob.GetSlideshow()) {
			result += 1 + vob.GetSlideshow()->size() / 5;
		} else {
			result += vob.GetChapterCount();
		}
	}
	return result;
}

/** Returns URI of video frame */
wxString Pgc::GetVideoFrameUri(int chapter, long position, bool fileNameOnly) {
	if (GetVobs().size() == 0)
		return wxT("");
	wxString uri;
	if (position == -1 && chapter > 0) {
		for (unsigned int vobi = 0; vobi < m_vobs.size() && chapter >= 0; vobi++) {
			Vob* vob = GetVobs()[vobi];
			uri = vob->GetFilename();
			for (vector<Cell*>::iterator it = vob->GetCells().begin();
					it != vob->GetCells().end() && chapter >= 0; it++) {
				if ((*it)->IsChapter()) {
					position = lround(vob->GetStartTime() * 1000) + (*it)->GetStart();
					chapter--;
				}
			}
		}
		if (chapter >= 0)
			return wxT("");
	} else {
		Vob* vob = GetVobs()[0];
		uri = vob->GetFilename();
		if (position == -1 && chapter == 0) {
			double dpos = vob->GetDuration() < 3600 ? vob->GetDuration() * 0.05 : 180;
			position = lround((vob->GetStartTime() + dpos) * 1000);
		}
	}
	if (!fileNameOnly && position >= 0 && uri.length() > 0)
		uri += wxT('#') + wxString::Format(wxT("%ld"), position);
	return uri;
}

bool Pgc::IsCellsUseCallLastMenu() {
	for (unsigned int vobi = 0; vobi < m_vobs.size(); vobi++) {
		Vob* vob = GetVobs()[vobi];
		for (vector<Cell*>::iterator it = vob->GetCells().begin(); it != vob->GetCells().end(); it++) {
			if ((*it)->GetCommands().Find(wxT("call last menu")) >= 0)
				return true;
		}
	}
	return false;
}

wxSvgXmlNode* Pgc::GetXML(DVDFileType type, DVD* dvd, PgcArray& pgcs, bool vmgm, int nextTitle, int nextTitleset) {
	wxSvgXmlNode* node = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("pgc"));
	if (m_entries.size() > 0)
		node->AddProperty(wxT("entry"), GetEntriesAsString(vmgm));
	
	if (m_palette.length())
		node->AddProperty(wxT("palette"), m_palette);
	
	if (GetMenu() && type == DVDAUTHOR_XML)
		GetMenu()->GetXML(type, node);
	
	if (GetSlideshow() && type == DVDAUTHOR_XML)
		GetSlideshow()->GetXML(type, node);
	
	for (int i = 0; i < (int) GetVobs().size(); i++)
		node->AddChild(GetVobs()[i]->GetXML(type, dvd, nextTitle));
	
	wxString preCommands = GetPreCommands();
	if (type == DVDAUTHOR_XML) {
		if (GetMenu()) {
			unsigned int pgci = 0;
			for (unsigned int i = 0; i < pgcs.size(); i++) {
				if (this == pgcs[i]) {
					pgci = i;
					break;
				}
			}
			// Play all titlesets: jump to next titleset, if playAllRegister > 2
			int paReg = dvd->GetPlayAllRegister();
			bool paTitlesets = dvd->HasPlayAllTitlesetButton();
			if (paReg != -1 && GetEntries().find(wxT("title")) != GetEntries().end() && paTitlesets) {
				int n = 1;
				wxString s;
				for (unsigned int i = 1; i < dvd->GetTitlesets().size(); i++) {
					n += dvd->GetTitlesets()[i-1]->GetTitles().size();
					s += wxString::Format(wxT("if (g%d == %d) { g%d = 2; jump title %d; }"), paReg, i + 2, paReg, n);
				}
				preCommands = s + preCommands;
			}
			// jump to last menu if 15th bit of lastMenuReg is set else set lastMenuReg to menu index 
			int lastMenuReg = dvd->GetLastMenuRegister();
			if (lastMenuReg != -1) {
				preCommands = wxString::Format(vmgm ? wxT("g%d=1%02d;") : wxT("g%d=%d;"), lastMenuReg, pgci + 1)
						+ preCommands;
				if (GetEntries().find(wxT("root")) != GetEntries().end() && dvd->HasCallLastMenu()) {
					wxString s;
					for (unsigned int i = 0; i < dvd->GetVmgm().size(); i++)
						s += wxString::Format(wxT("if (g%d==1%02d) jump vmgm menu %d;"), lastMenuReg, i + 1, i + 1);
					for (unsigned int i = 0; i < pgcs.size(); i++)
						if (i != pgci)
							s += wxString::Format(wxT("if (g%d==%d) jump menu %d;"), lastMenuReg, i + 1, i + 1);
					preCommands = wxString::Format(wxT("if (g%d & 0x8000 !=0) {g%d^=0x8000;"), lastMenuReg, lastMenuReg)
							+ s + wxT("}") + preCommands;
				}
			}
			// select last button
			if (GetMenu()->GetRememberLastButton()) {
				int lastBtReg = GetMenu()->GetRememberLastButtonRegister();
				bool checkLastMenu = false;
				if (lastBtReg == -1) {
					lastBtReg = dvd->GetRememberLastButtonRegister();
					checkLastMenu = true;
				} else {
					for (int tsi = -1; tsi < (int) dvd->GetTitlesets().Count(); tsi++) {
						PgcArray& pgcs = dvd->GetPgcArray(tsi, true);
						for (int pgci = 0; pgci<(int)pgcs.Count(); pgci++) {
							Menu* menu2 = pgcs[pgci]->GetMenu();
							if (menu2 && menu2 != GetMenu() && menu2->GetRememberLastButtonRegister() == lastBtReg) {
								checkLastMenu = true;
								break;
							}
						}
						if (checkLastMenu)
							break;
					}
				}
				wxString expr1;
				if (checkLastMenu && lastMenuReg != -1)
					expr1 = wxString::Format(wxT("g%d & 0xFFF == %d and "), lastMenuReg, pgci + (vmgm ? 101 : 1));
				preCommands = wxString::Format(wxT("if (%sg%d gt 0) button = g%d; else button = 1024;"),
						expr1.c_str(), lastBtReg, lastBtReg) + preCommands;
			}
		} else {
			ReplaceCallLastMenu(preCommands, dvd);
			ReplaceJumpNextTitle(preCommands, dvd, nextTitle);
			// check if "call last menu;" is used in chapter commands
			if (IsCellsUseCallLastMenu()) {
				int reg = dvd->GetLastMenuRegister();
				if (reg > 0) {
					preCommands = wxString::Format(wxT("g%d|=0x8000;"), reg) + preCommands;
				}
			}
		}
	}
	if (preCommands.length())
		XmlWriteValue(node, wxT("pre"), preCommands);
	
	wxString postCommands = GetPostCommands();
	if (type == DVDAUTHOR_XML) {
		ReplaceCallLastMenu(postCommands, dvd);
		ReplaceJumpNextTitle(postCommands, dvd, nextTitle);
		int paReg = dvd->GetPlayAllRegister();
		bool paTitlesets = dvd->HasPlayAllTitlesetButton();
		if (paReg != -1) {
			if (nextTitle > 0) {
				postCommands = wxString::Format(wxT("if (g%d gt 0) jump title %d;"), paReg, nextTitle) + postCommands;
			} else if (nextTitleset > 0 && paTitlesets) {
				postCommands = wxString::Format(wxT("if (g%d gt 1) { g%d = %d; call vmgm menu entry title; }"),
						paReg, paReg, nextTitleset + 1, nextTitle) + postCommands;
			}
		}
	}
	if (postCommands.length())
		XmlWriteValue(node, wxT("post"), postCommands);
	
	return node;
}

bool Pgc::PutXML(wxSvgXmlNode* node, DVD* dvd, int tsi, int pgci, bool menu, wxProgressDialog* progressDlg) {
	if (progressDlg)
		progressDlg->Pulse();
	WX_CLEAR_ARRAY(GetVobs());

	wxString val;
	m_entries.clear();
	if (node->GetPropVal(wxT("entry"), &val)) {
		wxArrayString entryArray = wxStringTokenize(val, wxT(","));
		for (unsigned int entryIdx = 0; entryIdx < entryArray.Count(); entryIdx++)
			m_entries.insert(entryArray[entryIdx]);
	}

	node->GetPropVal(wxT("palette"), &m_palette);

	wxSvgXmlNode* child = node->GetChildren();
	Menu* menuObj = NULL; // old

	while (child) {
		if (child->GetName() == wxT("spumux")) {
			// get spunode (old)
			menuObj = new Menu(dvd, tsi, pgci);
			menuObj->PutXML(child);
		} else if (child->GetName() == wxT("vob")) {
			// get vob
			Vob* vob;
			if (menuObj)
				vob = new Vob(menuObj);
			else
				vob = new Vob(wxT(""));
			if (vob->PutXML(child, dvd, tsi, pgci, menu, GetVobs().size() == 0))
				GetVobs().Add(vob);
			else {
				delete vob;
				wxLogError(_("Can't load vob"));
				return false;
			}
		} else if (child->GetName() == wxT("pre")) // get pre commands
			SetPreCommands(child->GetChildren()->GetContent());
		else if (child->GetName() == wxT("post")) // get post commands
			SetPostCommands(child->GetChildren()->GetContent());
		child = child->GetNext();
	}
	return true;
}
