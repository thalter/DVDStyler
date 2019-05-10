/////////////////////////////////////////////////////////////////////////////
// Name:        dvdstyler.cpp
// Purpose:     DVD Authoring Application
// Author:      Alex Thuering
// Created:	10.10.2003
// RCS-ID:      $Id: dvdstyler.cpp,v 1.72 2016/08/21 15:52:01 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "WelcomeDlg.h"
#include "TemplateDlg.h"
#include "MainWin.h"
#include "About.h"
#include "Config.h"
#include "Languages.h"
#include "MPEG.h"
#include "mediatrc_ffmpeg.h"
#include <wx/config.h>
#include <wx/filesys.h>
#include <wx/fs_zip.h>
#include <wxVillaLib/imagjpg.h>
#include <wxVillaLib/utils.h>
#include <wxVillaLib/ThumbnailFactory.h>
#include <wxSVG/SVGCanvas.h>
#include <wxSVG/imagsvg.h>
#include <wxSVG/mediadec_ffmpeg.h>
#include <wx/cmdline.h>

#ifndef __WXWINCE__
#include <locale.h>
#endif

#include "rc/dvdstyler.png.h"

#define LOCALE_DIR wxFindDataDirectory(_T("..") + wxString(wxFILE_SEP_PATH) + _T("locale"))

#ifdef __WXMAC__
#include <fontconfig/fontconfig.h>
#define FONTS_CONF wxFindDataFile(wxT("Resources/fonts.conf"))
#endif

class DVDStyler : public wxApp {
public:
	bool OnInit();
	int OnExit();

protected:
	wxLocale m_locale;
};

IMPLEMENT_APP(DVDStyler)

bool DVDStyler::OnInit() {
	wxGetApp();
	
	// fix config
	s_config.Init();
	wxString version = s_config.GetVersion();
	bool firstStart = version != APP_VERSION;
	if (version.length() < 3 || version.Mid(0, 3) != APP_VERSION.Mid(0, 3)) {
		wxLogNull log;
		s_config.SetVersion(APP_VERSION);
		if (version.Mid(0, 3) == wxT("1.7")) {
			s_config.DeleteGroup(_T("Generate"));
		} else if (version.length() < 3 || version.Mid(0, 3) == wxT("1.5") || version.Mid(0, 3) == wxT("1.6")) {
			s_config.DeleteGroup(_T("Generate"));
			s_config.DeleteGroup(_T("Preview"));
			s_config.DeleteGroup(_T("Iso"));
			s_config.DeleteGroup(_T("Burn"));
		} else if (version.length() > 0) {
			long ver;
			if (version.Mid(0, 3).ToLong(&ver) && ver < 2.3) {
				s_config.SetDvdReservedSpace(0);
			}
		}
		s_config.Flush();
	} else if (version != APP_VERSION) {
		s_config.SetVersion(APP_VERSION);
		s_config.Flush();
	}
	
#if wxDEBUG_LEVEL > 0
	wxRegEx reStableVersion(wxT("^[0-9\\.]+$"));
	if (reStableVersion.Matches(APP_VERSION)) {
		wxDisableAsserts();
	}
#endif
	
	wxInitAllImageHandlers();
#if !wxCHECK_VERSION(2,9,0)
	// load new jpeg handler (with scale option)
	if (!wxImage::RemoveHandler(_T("JPEG file")))
		wxMessageBox(_T("Error: Can't remove old JPEG handler"));
	wxImage::InsertHandler(new wxJPGHandler);
#endif
	wxImage::InsertHandler(new wxSVGHandler);
	
    wxFileSystem::AddHandler(new wxZipFSHandler);

	wxThumbnailFactory::InitGnome(APP_NAME.mb_str(), APP_VERSION.mb_str(), argc, (char**)argv);

	// locale
	int lang = wxLANGUAGE_ENGLISH;
	wxString languageCode = s_config.GetLanguageCode();
	if (languageCode.length() > 0 && wxLocale::FindLanguageInfo(languageCode)) {
		lang = wxLocale::FindLanguageInfo(languageCode)->Language;
	} else {
		lang = ChooseLanguage(wxLocale::GetSystemLanguage());
		if (lang == wxLANGUAGE_UNKNOWN)
			lang = wxLANGUAGE_ENGLISH;
		languageCode = wxLocale::GetLanguageInfo(lang)->CanonicalName;
	}
	if (s_config.GetDefAudioLanguage().length() == 0) {
		wxString langCode = languageCode.Upper().substr(0, 2);
		if (DVD::GetAudioLanguageCodes().Index(langCode) != wxNOT_FOUND) {
			s_config.SetDefAudioLanguage(langCode);
			s_config.SetDefSubtitleLanguage(langCode);
		} else {
			s_config.SetDefAudioLanguage(wxT("EN"));
			s_config.SetDefSubtitleLanguage(wxT("EN"));
		}
	}
	if (lang != wxLANGUAGE_ENGLISH) {
		m_locale.Init(lang);
		m_locale.AddCatalogLookupPathPrefix(LOCALE_DIR);
		m_locale.AddCatalog(wxT("wxstd"));
		m_locale.AddCatalog(wxT("dvdstyler"));
	}
	
	// set default video format
	if (firstStart) {
#ifdef __WXMSW__
		wxString ntscCountriesArr[4] = { wxT("Brazil"), wxT("Canada"), wxT("Japan"), wxT("USA") };
		wxArrayString ntscCountries(4, ntscCountriesArr);
		wxRegKey key(wxT("HKEY_CURRENT_USER\\Control Panel\\International"));
		wxString country;
		if (key.QueryValue(wxT("sCountry"), country) && ntscCountries.Index(country, false) >= 0)
			s_config.SetDefVideoFormat(vfNTSC);
#else
		if (wxDateTime::GetCountry() == wxDateTime::USA)
			s_config.SetDefVideoFormat(vfNTSC);
#endif
	}
	
#ifndef __WXWINCE__
	setlocale(LC_NUMERIC, "C");
#endif
	
	wxFfmpegMediaDecoder::Init();
	
#ifdef __WXMAC__
	// init font config
	wxString fontConfigFile;
	if (!wxGetEnv(wxT("FONTCONFIG_FILE"), &fontConfigFile)
			&& !wxFile::Exists(wxGetHomeDir() + wxFILE_SEP_PATH + wxT(".fonts.conf"))) {
		// load fonts.conf
		FcConfig* fc = FcConfigCreate();
		if (fc == NULL) {
			wxLogError(wxString(wxT("SubtitlePropDlg::UpdateFontList(): ")) + wxString(wxT("FcConfigCreate failed.")));
			return false;
		}
		FcConfigParseAndLoad(fc, (const FcChar8*) (const char*) FONTS_CONF.ToUTF8(), FcTrue);
		FcConfigBuildFonts(fc);
		FcConfigSetCurrent(fc);
	}
#endif
	
	// create main window
	MainWin* mainWin = new MainWin();
#ifndef __WXMSW__
	mainWin->SetIcon(wxICON_FROM_MEMORY(dvdstyler));
#else
	mainWin->SetIcon(wxICON(dvdstyler));
#endif
	
	// parse command line
	static const wxCmdLineEntryDesc cmdLineDesc[] = {
#if wxCHECK_VERSION(2,9,0)
		{ wxCMD_LINE_SWITCH, "s", "start", "automatically starts the generation and burning" },
		{ wxCMD_LINE_OPTION, "t", "tempDir", "temporary directory" },
		{ wxCMD_LINE_OPTION, "o", "outputDir", "output directory" },
		{ wxCMD_LINE_OPTION, "i", "isoFile", "output ISO file name" },
		{ wxCMD_LINE_OPTION, "d", "device", "DVD device to burn" },
		{ wxCMD_LINE_SWITCH, "e", "stderr", "write log messages to standard error" },
		{ wxCMD_LINE_SWITCH, "v", "version", "show version of program" },
		{ wxCMD_LINE_SWITCH, "h", "help", "show summary of options" },
		{ wxCMD_LINE_PARAM, NULL, NULL, "input file", wxCMD_LINE_VAL_STRING,
				wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
#else
		{ wxCMD_LINE_SWITCH, wxT("s"), wxT("start"), _("automatically starts the generation and burning") },
		{ wxCMD_LINE_OPTION, wxT("t"), wxT("tempDir"), _("temporary directory") },
		{ wxCMD_LINE_OPTION, wxT("o"), wxT("outputDir"), _("output directory") },
		{ wxCMD_LINE_OPTION, wxT("i"), wxT("isoFile"), _("output ISO file name") },
		{ wxCMD_LINE_OPTION, wxT("d"), wxT("device"), _("DVD device to burn") },
		{ wxCMD_LINE_SWITCH, wxT("e"), wxT("stderr"), _("write log messages to standard error") },
		{ wxCMD_LINE_SWITCH, wxT("v"), wxT("version"), _("show version of program") },
		{ wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), _("show summary of options") },
		{ wxCMD_LINE_PARAM, NULL, NULL, _("input file"), wxCMD_LINE_VAL_STRING,
				wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
#endif
		{ wxCMD_LINE_NONE }
	};
	
	wxCmdLineParser parser(cmdLineDesc, argc, argv);
	wxString logoText= wxT("DVDStyler, version ") + APP_VERSION + wxString::Format(
			wxString(wxT("\nCopyright © 2003 - %d Alex Thüring. All Rights Reserved.\n"), wxConvUTF8), wxDateTime::GetCurrentYear());
	parser.SetLogo(logoText);
	if (parser.Parse() != 0) {
		exit(1);
	} else if (parser.Found(wxT("v"))) {
		fprintf(stdout, "%s", (const char*) logoText.mb_str());
		exit(0);
	} else if (parser.Found(wxT("h"))) {
		parser.Usage();
		exit(0);
	}
	
	if (parser.Found(wxT("e"))) {
		wxLog::SetActiveTarget(new wxLogStderr);
		wxLog::GetActiveTarget()->SetVerbose(true);
	}

	for (int i=0; i<(int)parser.GetParamCount(); i++) {
		wxString filename = parser.GetParam(i);
		if (i == 0 && (filename.Lower().EndsWith(wxT(".xml")) || filename.Lower().EndsWith(wxT(".dvds")))) {
			mainWin->Open(filename);
		} else if (i == 0 && filename.Lower().EndsWith(wxT(".dvdt"))) {
			VideoFormat vf = (VideoFormat) s_config.GetDefVideoFormat();
			AudioFormat af = (AudioFormat) s_config.GetDefAudioFormat();
			AspectRatio ar = (AspectRatio) s_config.GetDefAspectRatio();
			mainWin->NewDVD(filename, wxT(""), s_config.GetDefDiscLabel(),
					(DvdResolution) s_config.GetDefDvdResolution(), (DiscCapacity) s_config.GetDefDiscCapacity(),
					s_config.GetVideoBitrate(), s_config.GetAudioBitrate(), vf <= vfCOPY ? vfPAL : vf,
					af <= afCOPY ? afMP2 : af, ar <= arAUTO ? ar4_3 : ar);
		} else {
			wxFfmpegMediaDecoder decoder;
			if (decoder.Load(filename))
				mainWin->AddTitle(filename);
		}
	}
	
	// temporary directory
	wxString val;
	if (parser.Found(wxT("t"), &val)) {
		s_config.SetTempDir(val);
	}

	// set output directory
	if (parser.Found(wxT("o"), &val)) {
		s_config.SetGenerateDo(true);
		s_config.SetIsoDo(false);
		s_config.SetBurnDo(false);
		mainWin->SetOutputDir(val);
	}

	// set output ISO file name
	if (parser.Found(wxT("i"), &val)) {
		s_config.SetGenerateDo(false);
		s_config.SetIsoDo(true);
		s_config.SetBurnDo(false);
		mainWin->SetIsoFile(val);
	}

	// set DVD device to burn
	if (parser.Found(wxT("d"), &val)) {
		s_config.SetGenerateDo(false);
		s_config.SetIsoDo(false);
		s_config.SetBurnDo(true);
		s_config.SetBurnDevice(val);
	}
	
	// show main window
	mainWin->Show();
	SetTopWindow(mainWin);
	if (parser.Found(wxT("s")))
		mainWin->Burn(true);
	
	if (parser.GetParamCount() == 0 && s_config.GetShowWelcomeDlg()) {
		// show welcome dialog
		WelcomeDlg dlg(mainWin);
		if (dlg.ShowModal() == wxID_OK) {
			if (dlg.IsNewProject()) {
				TemplateDlg templateDlg(mainWin, dlg.GetAspectRatio());
				wxString templateFile = templateDlg.ShowModal() == wxID_OK ? templateDlg.GetTemplate() : wxT("");
				mainWin->NewDVD(templateFile, templateDlg.GetTitle(), dlg.GetLabel(),
						dlg.GetDvdResolution(), dlg.GetCapacity(), dlg.GetVideoBitrate(), dlg.GetAudioBitrate(),
						dlg.GetVideoFormat(), dlg.GetAudioFormat(), dlg.GetAspectRatio(),
						dlg.GetDefPostCommand(), templateDlg.IsChapterMenu());
			} else if (dlg.GetOpenFilename().length() > 0) {
				mainWin->Open(dlg.GetOpenFilename());
			} else {
				mainWin->ShowOpenDlg();
			}
		}
	}
	
	s_config.SetLanguageCode(languageCode); // save choosed languages if all ok
	return true;
}

int DVDStyler::OnExit() {
	if (s_config.GetClearThumbnailCache())
		wxThumbnailFactory::ClearCacheDir();
	return wxApp::OnExit();
}
