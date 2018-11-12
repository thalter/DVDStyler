/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessProjectInfo.h
// Author:      Alex Thuering
// Created:     4.10.2014 (refactored)
// RCS-ID:      $Id: ProcessProjectInfo.cpp,v 1.5 2017/11/25 15:56:34 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "ProcessProjectInfo.h"
#include "DVD.h"
#include "Menu.h"
#include "Utils.h"
#include <wxSVG/SVGDocument.h>

/** Constructor */
ProcessProjectInfo::ProcessProjectInfo(ProgressDlg* progressDlg, DVD* dvd): Process(progressDlg) {
	this->dvd = dvd;
}

/** Returns true, if process need be executed */
bool ProcessProjectInfo::IsNeedExecute() {
	return true;
}

/** Returns true, if gauge need be updated */
bool ProcessProjectInfo::IsUpdateGauge() {
	return false;
}

void ProcessProjectInfo::PrintInfoLine(const wxString& info, unsigned int inputSize, unsigned int outputSize,
		double outputDuration, int bitrate) {
	wxString text = info.substr(0, 12);
	text.Append(wxT(' '), 12 - info.length()).Append(wxT("| "));
	// input size
	wxString str = inputSize > 0 ? wxString::Format(wxT("%.1f MB"), ((float) inputSize) / 1024) : wxT("");
	str.Append(wxT(' '), 11 - str.length());
	text.Append(str).Append(wxT("| "));
	// output duration
	str = outputDuration > 0 ? Time2String(outputDuration*1000) + wxT(" sec") : wxT("");
	str.Append(wxT(' '), 16 - str.length());
	text.Append(str).Append(wxT("| "));
	// output bitrate
	if (bitrate >= 1000)
		str = wxString::Format(wxT("%.1f Mb/s"), ((float) bitrate) / 1000);
	else
		str = bitrate > 0 ? wxString::Format(wxT("%d Kb/s"), bitrate) : wxT("");
	str.Append(wxT(' '), 9 - str.length());
	text.Append(str).Append(wxT("| "));
	// output size
	str = wxString::Format(wxT("%.1f MB"), ((float) outputSize) / 1024);
	text.Append(str);
	progressDlg->AddDetailMsg(text);
}

/** Executes process */
bool ProcessProjectInfo::Execute() {
	progressDlg->AddDetailMsg(wxT("============================================================================="));
	progressDlg->AddDetailMsg(wxT("            | Input size | Output duration | Bitrate  | Estimated output size"));
	double totalDuration = 0;
	for (int tsi = -1; tsi < (int) dvd->GetTitlesets().Count(); tsi++) {
		PgcArray& pgcArray = dvd->GetPgcArray(tsi, true);
		for (int pgci = 0; pgci < (int) pgcArray.Count(); pgci++) {
			Pgc* pgc = pgcArray[pgci];
			for (int vobi = 0; vobi < (int) pgc->GetVobs().Count(); vobi++) {
				Vob* vob = pgc->GetVobs()[vobi];
				double dur = vob->GetMenu() ? vob->GetMenu()->GetSVG()->GetDuration() : 0;
				PrintInfoLine(wxString::Format(tsi == -1 ? wxT("VMGM menu %d") : wxT("Menu %d"), pgci + 1), 0,
						vob->GetOutputFileSize(dvd, 0), dur, vob->GetBitrate(dvd, 0));
				totalDuration += dur;
				for (unsigned int fileIdx = 0; fileIdx < vob->GetAudioFilenames().size(); fileIdx++) {
					PrintInfoLine(wxT("  ") + vob->GetAudioFilenames()[fileIdx].AfterLast(wxT('.')),
							vob->GetFileSize(vob->GetAudioFilenames()[fileIdx]),
							vob->GetOutputFileSize(dvd, fileIdx + 1), 0, vob->GetBitrate(dvd, fileIdx + 1));
				}
			}
		}
		if (tsi == -1)
			continue;
		Titles& titles = dvd->GetTitlesets()[tsi]->GetTitles();
		for (int pgci = 0; pgci < (int) titles.Count(); pgci++) {
			Pgc* pgc = titles[pgci];
			progressDlg->AddDetailMsg(wxString::Format(wxT("Title %d"), pgci + 1));
			for (int vobi = 0; vobi < (int) pgc->GetVobs().Count(); vobi++) {
				Vob* vob = pgc->GetVobs()[vobi];
				if (vob->GetSlideshow()) {
					double dur = vob->GetSlideshow()->GetResultDuration();
					PrintInfoLine(wxT("  slideshow"), 0, vob->GetOutputFileSize(dvd, 0), dur,
							vob->GetBitrate(dvd, 0));
					totalDuration += dur;
				} else if (vob->GetFilename().length() > 0) {
					double dur = vob->GetResultDuration();
					PrintInfoLine(wxT("  ") + vob->GetFilename().AfterLast(wxT('.')),
							vob->GetFileSize(vob->GetFilename()), vob->GetOutputFileSize(dvd, 0), dur,
							vob->GetBitrate(dvd, 0));
					totalDuration += dur;
				}
				for (unsigned int fileIdx = 0; fileIdx < vob->GetAudioFilenames().size(); fileIdx++) {
					PrintInfoLine(wxT("  ") + vob->GetAudioFilenames()[fileIdx].AfterLast(wxT('.')),
							vob->GetFileSize(vob->GetAudioFilenames()[fileIdx]),
							vob->GetOutputFileSize(dvd, fileIdx + 1), 0, vob->GetBitrate(dvd, fileIdx + 1));
				}
			}
		}
	}
	PrintInfoLine(wxT("Total"), 0, dvd->GetSize(), totalDuration, 0);
	progressDlg->AddDetailMsg(wxT("============================================================================="));
	return true;
}
