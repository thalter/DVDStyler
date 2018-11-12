/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessMenu.cpp
// Author:      Alex Thuering
// Created:     26.09.2014 (refactored)
// RCS-ID:      $Id: ProcessMenu.cpp,v 1.11 2016/12/11 10:06:31 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "ProcessMenu.h"
#include "DVD.h"
#include "Menu.h"
#include "Config.h"
#include "mediaenc_ffmpeg.h"
#include "mediatrc_ffmpeg.h"
#include <wxSVG/SVGDocument.h>
#include <wxSVG/SVGImageElement.h>
#include <wxSVG/SVGAnimateElement.h>
#include <wxSVG/SVGAnimateTransformElement.h>
#include <wxSVG/mediadec_ffmpeg.h>

#define WARNING_MPLEX_MSG wxT("Warning: using of mplex (mjpegs-tool) for generation of menus with audio is disabled. This can produce not compliant DVD.")

/** Constructor */
ProcessMenu::ProcessMenu(ProgressDlg* progressDlg, DVD* dvd, wxString dvdTmpDir): ProcessTranscode(progressDlg) {
	this->dvd = dvd;
	menuSubSteps = 0;
	for (int tsi = -1; tsi < (int) dvd->GetTitlesets().Count(); tsi++) {
		PgcArray& pgcs = dvd->GetPgcArray(tsi, true);
		for (int pgci = 0; pgci < (int) pgcs.Count(); pgci++) {
			Pgc* pgc = pgcs[pgci];
			if (pgc->GetVobs().Count() == 0)
				continue;
			// set temp file name
			wxString menuFile = dvdTmpDir + wxString::Format(_T("menu%d-%d.mpg"), tsi + 1, pgci);
			Vob* vob = pgc->GetVobs()[0];
			vob->SetTmpFilename(menuFile);
			// calculate sub steps
			Menu* menu = vob->GetMenu();
			WidescreenType widescreenType = pgcs.GetVideo().GetWidescreen();
			bool videoMenu = menu->GetSVG()->GetDuration() > 0;
			if (videoMenu) {
				menuSubSteps += 400; // generate mpeg(200) + transcode(200)
			} else if (vob->GetAudioFilenames().size() > 0 || s_config.GetUseMplexForMenus())
				menuSubSteps += 300; // generate mpeg(25+75) + transcode(200)
			else
				menuSubSteps += 25; // generate mpeg
			menuSubSteps += (menu->GetAspectRatio() == ar4_3 ? 1 : widescreenType != wtAUTO ? 2 : 3) * 50; // spumux
			menuVobs.Add(vob);
			menuWSTypes.Add((int) widescreenType);
		}
	}
}

/** Returns true, if process need be executed */
bool ProcessMenu::IsNeedExecute() {
	return menuVobs.Count() > 0;
}

/** Executes process */
bool ProcessMenu::Execute() {
	if (menuVobs.Count() == 0)
		return true;
	if (progressDlg->WasCanceled())
		return false;
	progressDlg->SetSubSteps(menuSubSteps);
	for (unsigned int i = 0; i < menuVobs.Count(); i++) {
		wxString msg = wxString::Format(_("Generating menu %u of %lu"), i + 1, menuVobs.Count());
		if (i == 0) {
			progressDlg->AddSummaryMsg(msg);
		} else {
			progressDlg->ReplaceSummaryMsg(msg);
			progressDlg->AddDetailMsg(msg);
		}
		Vob* vob = (Vob*) menuVobs[i];
		WidescreenType wsType = (WidescreenType) menuWSTypes[i];
		
		wxString audioFile;
		AudioFormat audioFormat = dvd->GetAudioFormat();
		if (vob->GetAudioFilenames().size() > 0 && vob->GetStreams().size() > 0
				&& vob->GetStreams()[0]->GetType() == stAUDIO) {
			audioFile = vob->GetAudioFilenames()[0];
			Stream* stream = vob->GetStreams()[0];
			audioFormat = stream->GetAudioFormat();
			if (audioFormat == afCOPY) {
				// set destination format if it set to COPY and codec is not mp2/ac3 or sample rate != 48 kHz
				if (stream->GetSourceCodecName() != wxT("mp2")
						&& stream->GetSourceCodecName() != wxT("ac3")
						&& stream->GetSourceCodecName() != wxT("liba52")) {
					audioFormat = dvd->GetAudioFormat();
				} else if (stream->GetSourceSampleRate() != 48000) {
					audioFormat = stream->GetSourceCodecName() == wxT("mp2") ? afMP2 : afAC3;
				}
			}
		}
		
		if (!GenerateMenu(vob->GetMenu(), wsType, vob->GetTmpFilename(), audioFormat, audioFile, dvd->GetAudioBitrate()))
			return false;
	}
	progressDlg->IncStep();
	return true;
}

bool ProcessMenu::GenerateMenu(Menu* menu, WidescreenType widescreenType, const wxString& menuFile,
		AudioFormat audioFormat, wxString audioFile, int audioBitrate) {
	if (progressDlg->WasCanceled())
		return false;
	wxString mpegFile = menuFile + _T("_bg.mpg");
	wxString m2vFile = menuFile + _T("_bg.m2v");
	wxString audioFileTmp = menuFile + _T("_bg.audio");
	wxString btFile = menuFile + _T("_buttons.png");
	wxString hlFile = menuFile + _T("_highlight.png");
	wxString selFile = menuFile + _T("_select.png");
	wxString spuFile = menuFile + _T("_spumux.xml");

	bool videoMenu = menu->GetSVG()->GetDuration() > 0;
	wxYield();

	progressDlg->AddDetailMsg(_("Create menu MPEG"));
	if (s_config.GetMenuVideoBitrate() < 1000)
		s_config.SetMenuVideoBitrate(1000);
	
	if (videoMenu) {
		// get background audio format
		AudioFormat bgAudioFormat = afNONE;
		wxString bgAudioOutputFormat;
		if (menu->HasVideoBackground() && audioFile.length() == 0) {
			wxFfmpegMediaDecoder ffmpeg;
			if (!ffmpeg.Load(menu->GetBackground()))
				return false;
			for (unsigned int i = 0; i < ffmpeg.GetStreamCount(); i++) {
				if (ffmpeg.GetStreamType(i) == stAUDIO) {
					if (ffmpeg.GetCodecName(i) != wxT("mp2") && ffmpeg.GetCodecName(i) != wxT("ac3")) {
						bgAudioFormat = audioFormat;
					} else if (ffmpeg.GetSampleRate(i) != 48000) {
						bgAudioFormat = ffmpeg.GetCodecName(i) == wxT("mp2") ? afMP2 : afAC3;
					} else {
						bgAudioFormat = afCOPY;
						bgAudioOutputFormat = ffmpeg.GetCodecName(i);
					}
					break;
				}
			}
		}
		// encode video
		bool hasAudio = audioFile.length() || bgAudioFormat != afNONE || s_config.GetUseMplexForMenus();
		wxFfmpegMediaEncoder ffmpeg(s_config.GetThreadCount());
		if (!ffmpeg.BeginEncode(hasAudio ? m2vFile : mpegFile, menu->GetVideoFormat(), hasAudio ? afNONE : audioFormat,
				menu->GetAspectRatio(), s_config.GetMenuVideoBitrate(), s_config.GetMenuVideoCBR())) {
			progressDlg->Failed(_("Error creation of menu"));
			return false;
		}
		wxSVGDocument* svg = menu->GetBackgroundSVG();
		int width = menu->GetFrameResolution().GetWidth();
		int height = menu->GetFrameResolution().GetHeight();
		progressDlg->AddDetailMsg(wxString::Format(_("Video duration: %f sec"), svg->GetDuration()));
		int frameCount = (int) (svg->GetDuration() * GetFps(menu->GetVideoFormat(), false));
		if (frameCount == 0)
			frameCount = s_config.GetMenuFrameCount();
		for (int f = 0; f < frameCount; f++) {
			if (progressDlg->WasCanceled())
				return false;
			if (f % 10 == 0) {
				wxString msg = wxString::Format(_("Encoding frame %d of %d"), (f > 0 ? f : 1), frameCount);
				if (f == 0) {
					progressDlg->AddDetailMsg(msg);
				} else {
					progressDlg->ReplaceLastDetailMsg(msg);
				}
			}
			svg->SetCurrentTime(((double) f) / GetFps(menu->GetVideoFormat(), false));
			wxImage img = svg->Render(width, height, NULL, false);
			if (!ffmpeg.EncodeImage(img, 1, progressDlg)) {
				progressDlg->Failed(_("Error creation of menu"));
				return false;
			}
			progressDlg->SetSubStep(200 * (f + 1) / frameCount);
		}
		ffmpeg.EndEncode();
		if (hasAudio) {
			if (audioFile.length() == 0 && bgAudioFormat == afNONE) {
				// encode audio
				audioFileTmp = menuFile + wxString(wxT("_bg.")) + (audioFormat == afAC3 ? wxT("ac3") : wxT("mp2"));
				double duration = (double) frameCount / GetFps(menu->GetVideoFormat(), false);
				if (!ffmpeg.BeginEncode(audioFileTmp, vfNONE, audioFormat, menu->GetAspectRatio(),
						s_config.GetMenuVideoBitrate(), s_config.GetMenuVideoCBR())
						|| !ffmpeg.EncodeAudio(duration, progressDlg)) {
					progressDlg->Failed(_("Error creation of menu"));
					return false;
				}
				ffmpeg.EndEncode();
			}
			Vob menuVob;
			menuVob.SetFilename(m2vFile);
			menuVob.GetVideoStream()->SetDestinationFormat(vfCOPY);
			if (audioFile.length()) {
				menuVob.AddAudioFile(audioFile);
			} else if (bgAudioFormat == afNONE) {
				menuVob.AddAudioFile(audioFileTmp);
			} else {
				progressDlg->AddDetailMsg(_("Transcode audio from") + wxString(wxT(" ")) + menu->GetBackground());
				// transcode audio
				wxFfmpegMediaTranscoder transcoder;
				if (!transcoder.AddInputFile(menu->GetBackground())) {
					progressDlg->Failed(wxT("Error by transcoding of ") + menu->GetBackground());
					return false;
				}
				transcoder.SetOutputFormat(bgAudioOutputFormat);
				if (!transcoder.SetOutputFile(audioFileTmp, vfNONE, false, menu->GetAspectRatio(), bgAudioFormat,
						sfNONE, 0, false, audioBitrate)) {
					progressDlg->Failed(_("Error transcoding of ") + menu->GetBackground());
					return false;
				}
				AVConvExecute exec(progressDlg, -1);
				if (!exec.Execute(transcoder.GetCmd())) {
					DeleteFile(audioFileTmp);
					progressDlg->Failed(_("Error transcoding of ") + menu->GetBackground());
					return false;
				}
				menuVob.AddAudioFile(audioFileTmp);
			}
			progressDlg->AddDetailMsg(_("Multiplexing audio and video"));
			for (unsigned int i = 0; i < menuVob.GetStreams().size(); i++) {
				Stream* stream = menuVob.GetStreams()[i];
				if (stream->GetType() == stAUDIO && stream->GetAudioFormat() == afCOPY) {
					// change destination format if it set to COPY and codec is not mp2/ac3 or sample rate != 48 kHz
					if (stream->GetSourceCodecName() != wxT("mp2") && stream->GetSourceCodecName() != wxT("ac3")) {
						stream->SetDestinationFormat(audioFormat);
					} else if (stream->GetSourceSampleRate() != 48000) {
						stream->SetDestinationFormat(stream->GetSourceCodecName() == wxT("mp2") ? afMP2 : afAC3);
					}
				}
			}
			menuVob.SetKeepAspectRatio(false);
			menuVob.SetTmpFilename(mpegFile);
			if (!s_config.GetUseMplexForMenus())
				progressDlg->AddDetailMsg(WARNING_MPLEX_MSG, *wxRED);
			if (!Transcode(&menuVob, menu->GetAspectRatio(), s_config.GetMenuVideoBitrate(), audioBitrate,
					s_config.GetUseMplexForMenus()))
				return false;
			if (s_config.GetRemoveTempFiles()) {
				DeleteFile(m2vFile);
				DeleteFile(audioFileTmp);
			}
		}
	} else { // menu with still image
		wxImage bgImage = menu->GetBackgroundImage();
		if (!bgImage.Ok()) {
			progressDlg->Failed(_("Error creation of menu"));
			return false;
		}
		
		bool hasAudio = audioFile.length(); // || s_config.GetUseMplexForMenus(); disable mplex for still image
		int frameCount = s_config.GetMenuFrameCount();
		if (audioFile.length() > 0) {
			// get audio duration
			wxFfmpegMediaDecoder decoder;
			if (decoder.Load(audioFile)) {
				double duration = decoder.GetDuration();
				if (duration > 0) {
					progressDlg->AddDetailMsg(wxString::Format(_("Audio duration: %f sec"), duration));
					if (menu->GetVideoFormat() == vfPAL)
						frameCount = (int) (duration * 25);
					else
						frameCount = (int) (duration * 30000 / 1001);
				}
			}
		}

		// encode video
		wxFfmpegMediaEncoder ffmpeg(s_config.GetThreadCount());
		if (frameCount < (isNTSC(menu->GetVideoFormat()) ? 15 : 12)) {
			frameCount = (isNTSC(menu->GetVideoFormat()) ? 15 : 12); // set minimum of frames to default GOP size
		}
		progressDlg->AddDetailMsg(_("Frame count of menu:") + wxString::Format(wxT(" %d"), frameCount));
		if (!ffmpeg.BeginEncode(hasAudio ? m2vFile : mpegFile, menu->GetVideoFormat(), hasAudio ? afNONE : audioFormat,
					menu->GetAspectRatio(), s_config.GetMenuVideoBitrate(), s_config.GetMenuVideoCBR())
				|| !ffmpeg.EncodeImage(bgImage, frameCount, progressDlg)) {
			progressDlg->Failed(_("Error creation of menu"));
			return false;
		}
		ffmpeg.EndEncode();
		progressDlg->IncSubStep(25);
		
		if (hasAudio) { // audio file or mplex is used
			if (audioFile.length() == 0) {
				// encode audio
				audioFileTmp = menuFile + wxString(wxT("_bg.")) + (audioFormat == afAC3 ? wxT("ac3") : wxT("mp2"));
				double duration = (double) frameCount / GetFps(menu->GetVideoFormat(), false);
				if (!ffmpeg.BeginEncode(audioFileTmp, vfNONE, audioFormat, menu->GetAspectRatio(),
						s_config.GetMenuVideoBitrate(), s_config.GetMenuVideoCBR())
						|| !ffmpeg.EncodeAudio(duration, progressDlg)) {
					progressDlg->Failed(_("Error creation of menu"));
					return false;
				}
				ffmpeg.EndEncode();
			}
			progressDlg->IncSubStep(75);
			// mplex (and optionally transcode audio)
			if (progressDlg->WasCanceled())
				return false;
			progressDlg->AddDetailMsg(_("Multiplexing audio and video"));
			Vob menuVob;
			menuVob.SetFilename(m2vFile);
			menuVob.AddAudioFile(audioFile.length() ? audioFile : audioFileTmp);
			for (unsigned int i = 0; i < menuVob.GetStreams().size(); i++) {
				Stream* stream = menuVob.GetStreams()[i];
				if (stream->GetType() == stAUDIO)
					stream->SetDestinationFormat(audioFile.length() ? audioFormat : afCOPY);
			}
			menuVob.SetTmpFilename(mpegFile);
			if (!s_config.GetUseMplexForMenus())
				progressDlg->AddDetailMsg(WARNING_MPLEX_MSG, *wxRED);
			if (!Transcode(&menuVob, menu->GetAspectRatio(), s_config.GetMenuVideoBitrate(), audioBitrate,
					s_config.GetUseMplexForMenus()))
				return false;
			if (s_config.GetRemoveTempFiles()) {
				DeleteFile(m2vFile);
				DeleteFile(audioFileTmp);
			}
		}
	}

	//spumux
	if (progressDlg->WasCanceled())
		return false;
	progressDlg->AddDetailMsg(_("Multiplexing subpictures into mpeg"));
	int stCount = menu->GetAspectRatio() == ar4_3 ? 1 : widescreenType != wtAUTO ? 2 : 3;
	for (int stIdx = 0; stIdx < stCount; stIdx++) {
		if (stIdx > 0) {
			if (mpegFile == menu->GetBackground())
				mpegFile = menuFile + _T("_bg.mpg");
			if (!wxRenameFile(menuFile, mpegFile, false)) {
				progressDlg->Failed(wxString::Format(_("Can't rename file '%s' in '%s'"), menuFile.c_str(), mpegFile.c_str()));
				return false;
			}
		}
		// save subpictures
		SubStreamMode mode = menu->GetAspectRatio() == ar4_3 ? ssmNORMAL : ssmWIDESCREEN;
		if (stIdx == 1)
			mode = widescreenType == wtNOLETTERBOX ? ssmPANSCAN : ssmLETTERBOX;
		else if (stIdx == 2)
			mode = ssmPANSCAN;
		wxImage* images = menu->GetSubPictures(mode);
		images[0].SaveFile(btFile);
		images[1].SaveFile(hlFile);
		images[2].SaveFile(selFile);
		delete[] images;
		// save spumux
		menu->SaveSpumux(spuFile, mode, btFile, hlFile, selFile);
		wxString cmd = s_config.GetSpumuxCmd();
		cmd.Replace(wxT("$FILE_CONF"), spuFile);
		cmd.Replace(wxT("$STREAM"), wxString::Format(wxT("%d"), stIdx));
		wxULongLong fileSize = wxFileName::GetSize(mpegFile);
		SpumuxExecute exec(progressDlg, fileSize != wxInvalidSize ? fileSize.ToDouble() : -1);
		if (!exec.Execute(cmd, mpegFile, menuFile)) {
			progressDlg->Failed();
			return false;
		}
		if (s_config.GetRemoveTempFiles() || stIdx + 1 < stCount) {
			if ((!videoMenu || mpegFile != menu->GetBackground()))
				DeleteFile(mpegFile);
			DeleteFile(btFile);
			DeleteFile(hlFile);
			DeleteFile(selFile);
			DeleteFile(spuFile);
		}
	}
	
	wxYield();
	return true;
}
