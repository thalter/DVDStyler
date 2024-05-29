/////////////////////////////////////////////////////////////////////////////
// Name:        Config.h
// Purpose:     Configuration
// Author:      Alex Thuering
// Created:     27.03.2003
// RCS-ID:      $Id: Config.cpp,v 1.8 2015/02/09 18:33:59 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "Config.h"
#include <wxVillaLib/utils.h>
#include <wx/fileconf.h>
#include <wx/stdpaths.h>
#include <wx/dir.h>

Config s_config;

void Config::Init() {
#ifdef __WXMSW__
	// check if INI file exists
	wxString fileName = wxGetAppPath() + wxT("..") + wxFILE_SEP_PATH + wxT("DVDStyler.ini");
	if (wxFileExists(fileName)) {
		if (fileName.Lower().StartsWith(wxT("c:\\program files"))) {
			wxConfig::Set(new wxFileConfig(wxT(""), wxT(""),
				wxGetHomeDir() + wxFILE_SEP_PATH + wxT("DVDStyler.ini"), fileName));
		} else
			wxConfig::Set(new wxFileConfig(wxT(""), wxT(""), fileName));
	}
	cfg = wxConfig::Get();
#elif defined(__WXMAC__)
	cfg = wxConfig::Get();
#else
	// check if .dvdstyler exist and move it
	wxString dataDir = wxStandardPaths::Get().GetUserLocalDataDir();
	if (wxFileExists(dataDir)) {
		wxRenameFile(dataDir, dataDir + ".tmp");
		wxMkdir(dataDir);
		wxRenameFile(dataDir + ".tmp", dataDir + wxFILE_SEP_PATH + "dvdstyler");
	}
	if (!wxDir::Exists(dataDir)) {
		wxMkdir(dataDir);
	}

	cfg = new wxFileConfig("", "", dataDir + wxFILE_SEP_PATH + "dvdstyler");
	wxConfig::Set(cfg);
#endif
}

bool Config::IsMainWinMaximized() {
	bool ret = false;
	cfg->Read(wxT("MainWin/maximized"), &ret);
	return ret;
}

wxRect Config::GetMainWinLocation() {
	wxRect rect;
	rect.x = cfg->Read(wxT("MainWin/x"), -1);
	rect.y = cfg->Read(wxT("MainWin/y"), -1);
	rect.width = cfg->Read(wxT("MainWin/width"), -1);
	rect.height = cfg->Read(wxT("MainWin/height"), -1);
	return rect;
}

void Config::SetMainWinLocation(wxRect rect, bool isMaximized) {
	cfg->Write(wxT("MainWin/maximized"), isMaximized);
	if (rect.width > 50 && rect.height > 50) {
		cfg->Write(wxT("MainWin/x"), rect.x);
		cfg->Write(wxT("MainWin/y"), rect.y);
		cfg->Write(wxT("MainWin/width"), rect.width);
		cfg->Write(wxT("MainWin/height"), rect.height);
	}
}
