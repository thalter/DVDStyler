/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessMenuTransitions.h
// Author:      Alex Thuering
// Created:     27.09.2014
// RCS-ID:      $Id: ProcessMenuTransitions.cpp,v 1.5 2016/12/11 10:06:31 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "ProcessMenuTransitions.h"
#include "mediaenc_ffmpeg.h"
#include "mediatrc_ffmpeg.h"
#include "DVD.h"
#include "Menu.h"
#include "Config.h"
#include <wxSVG/SVGDocument.h>
#include <wxSVG/SVGImageElement.h>
#include <wxSVG/SVGAnimateElement.h>
#include <wxSVG/SVGAnimateTransformElement.h>
#include <wxSVG/mediadec_ffmpeg.h>
#include <wx/textfile.h>

/** Constructor */
ProcessMenuTransitions::ProcessMenuTransitions(ProgressDlg* progressDlg, DVD* dvd, wxString dvdTmpDir): ProcessTranscode(progressDlg) {
	this->dvd = dvd;
	this->dvdTmpDir = dvdTmpDir;
	this->menu = NULL;
	// transition
	if (dvd->GetVmgm().size() > 0 && dvd->GetTitlesets()[0]->GetTitles()[0]->GetVobs().size() > 0) {
		Menu* menu = dvd->GetVmgm()[0]->GetMenu();
		if (menu->GetSVG()->GetElementById(wxT("main"))) {
			this->menu = menu;
		}
	}
}

/** Returns true, if process need be executed */
bool ProcessMenuTransitions::IsNeedExecute() {
	return menu != NULL;
}
/** Executes process */
bool ProcessMenuTransitions::Execute() {
	// transition
	Vob* vob = dvd->GetTitlesets()[0]->GetTitles()[0]->GetVobs()[0];
	if (!GenerateMenuTransition(menu, vob, dvd, dvdTmpDir))
		return false;
	return true;
}

/** Generates a transition from menu to title */
bool ProcessMenuTransitions::GenerateMenuTransition(Menu* menu, Vob* vob, DVD* dvd, const wxString& dvdTmpDir) {
	wxString videoFile = vob->GetTmpFilename().length() ? vob->GetTmpFilename() : vob->GetFilename();
	wxString transitionFile = dvdTmpDir + wxT("title0_t.m2v");
	wxString video2File = dvdTmpDir + wxT("title0.m2v");
	wxString resultFile = dvdTmpDir + wxT("title0.vob");
	double transitionDur = 7;
	
	// transition object
	MenuObject* menuObj = menu->GetObjectByTitle(0, 0, dvd);
	if (!menuObj) {
		for (unsigned int obji = 0; obji < menu->GetObjectsCount(); obji++) {
			MenuObject* obj = menu->GetObject(obji);
			if (obj->GetImageParam() != NULL) {
				menuObj = obj;
				break;
			}
		}
	}
	if (!menuObj)
		return true;
	
	// generate transition video
	if (!GenerateMenuTransitionVideo(menu, menuObj, videoFile, transitionDur, transitionFile))
		return false;
	
	// join the transition
	//ffmpeg.exe -i transition.mpg -ss 7 -i video.mpg -i video.mpg
	//-filter_complex "[0:v] [1:v] concat=n=2:v=1:a=0 [v]" -c:a copy -map "[v]" -map "2:a" result.vob
//	wxFfmpegMediaTranscoder transcoder(s_config.GetThreadCount());
//	transcoder.AddInputFile(transitionFile);
//	transcoder.SetStartTime(transitionDur);
//	transcoder.AddInputFile(videoFile);
//	transcoder.AddInputFile(videoFile);
//	transcoder.ConcatVideo(resultFile);
//	
//	AVConvExecute exec(this, lround(vob->GetDuration() * GetFps(menu->GetVideoFormat(), false)));
//	if (!exec.Execute(transcoder.GetCmd())) {
//		if (wxFileExists(resultFile))
//			wxRemoveFile(resultFile);
//		progressDlg->Failed(_("Error creation of transition"));
//		return false;
//	}
	// extract video
	progressDlg->AddDetailMsg(wxT("Extracting video stream"));
	wxFfmpegMediaTranscoder transcoder;
	transcoder.AddInputFile(videoFile);
	transcoder.SetStartTime(transitionDur);
	transcoder.SetOutputFile(video2File, vfCOPY, false, ar4_3, afNONE, sfNONE);
	
	AVConvExecute execv(progressDlg, lround(vob->GetDuration() * GetFps(menu->GetVideoFormat(), false)));
	if (!execv.Execute(transcoder.GetCmd())) {
		if (wxFileExists(video2File))
			wxRemoveFile(video2File);
		progressDlg->Failed(_("Error creation of transition"));
		return false;
	}
	// extract audio
	progressDlg->AddDetailMsg(wxT("Extracting audio streams"));
	transcoder.Init();
	transcoder.AddInputFile(videoFile);
	wxArrayString audioFiles;
	wxFfmpegMediaDecoder decoder;
	if (!decoder.Load(videoFile))
		return false;
	for (unsigned int stIdx = 0; stIdx < decoder.GetStreamCount(); stIdx++) {
		if (decoder.GetStreamType(stIdx) != stAUDIO)
			continue;
		wxString audioFile = resultFile + wxString::Format(wxT(".audio%u"), audioFiles.size());
		audioFiles.Add(audioFile);
		if (wxFileExists(audioFile) && !wxRemoveFile(audioFile)) {
			wxLogError(wxString::Format(_("Can't remove file '%s'"), audioFile.c_str()));
			return false;
		}
		if (!transcoder.SetOutputFile(audioFile, vfNONE, false, ar4_3, afCOPY, sfNONE, s_config.GetVideoBitrate(),
				s_config.GetVbr(), s_config.GetAudioBitrate(), 0, stIdx)) {
			progressDlg->Failed(_("Error transcoding of ") + vob->GetFilename());
			return false;
		}
	}
	double fps = GetFps(menu->GetVideoFormat(), false);
	AVConvExecute execa(progressDlg, lround(vob->GetDuration() * fps));
	if (!execa.Execute(transcoder.GetCmd())) {
		for (unsigned int audioIdx = 0; audioIdx < audioFiles.size(); audioIdx++)
			if (wxFileExists(audioFiles[audioIdx]))
				wxRemoveFile(audioFiles[audioIdx]);
		progressDlg->Failed(_("Error transcoding of ") + vob->GetFilename());
		return false;
	}
	// concat video
	progressDlg->AddDetailMsg(wxT("Concating video"));
	wxString resultVideoFile = resultFile + wxT("_video.m2v");
	wxString concatFile = resultFile + wxT("_video.txt");
	wxTextFile file(concatFile);
	file.Create();
	file.AddLine(wxT("ffconcat version 1.0"));
	file.AddLine(wxT("file ") + transitionFile.AfterLast(wxFILE_SEP_PATH));
	file.AddLine(wxT("file ") + video2File.AfterLast(wxFILE_SEP_PATH));
	file.Write();
	transcoder.Init();
	transcoder.AddInputFile(concatFile, wxT("concat"));
	transcoder.SetOutputFile(resultVideoFile, vfCOPY, false, ar4_3, afNONE, sfNONE);
	wxString cdir = wxGetCwd();
	wxSetWorkingDirectory(dvdTmpDir);
	
	AVConvExecute execConcat(progressDlg, lround(vob->GetDuration() * GetFps(menu->GetVideoFormat(), false)));
	if (!execConcat.Execute(transcoder.GetCmd())) {
		if (wxFileExists(resultVideoFile))
			wxRemoveFile(resultVideoFile);
		if (wxFileExists(concatFile))
			wxRemoveFile(concatFile);
		progressDlg->Failed(_("Error creation of transition"));
		wxSetWorkingDirectory(cdir);
		return false;
	}
	wxSetWorkingDirectory(cdir);
	
	// multiplex video with audio
	if (!Multiplex(resultVideoFile, audioFiles, resultFile))
		return false;
	
	vob->SetTmpFilename(resultFile);
	return true;
}

/** Generates a transition from menu to title */
bool ProcessMenuTransitions::GenerateMenuTransitionVideo(Menu* menu, MenuObject* menuObj, const wxString& videoFile,
		double transitionDur, const wxString& transitionFile) {
	wxString menuLabel = wxString::Format(_("VMGM menu %d"), 1);
	progressDlg->AddSummaryMsg(wxString::Format(_("Generate transition from %s"), menuLabel.c_str()));
//	wxSVGDocument* svgDoc = new wxSVGDocument();
//	svgDoc->SetRootElement((wxSVGSVGElement*) menu->GetSVG()->GetRootElement()->CloneNode());
	wxSVGDocument* svgDoc = menu->GetBackgroundSVG();
	// 1. set video and animate opacity
	wxSVGElement* menuObjSvg = svgDoc->GetElementById(wxT("s_") + menuObj->GetId());
	if (menuObjSvg && menuObjSvg->GetDtd() == wxSVG_SVG_ELEMENT) {
		MenuObject::SetImageVideoParams((wxSVGSVGElement*) menuObjSvg, wxT("image"), videoFile, 0, transitionDur);
		wxSVGElement* frameSvgElem = (wxSVGElement*) ((wxSVGSVGElement*) menuObjSvg)->GetElementById(wxT("frame"));
		if (frameSvgElem && transitionDur >= 4) {
			wxSVGAnimateElement* animElem = new wxSVGAnimateElement();
			animElem->SetAttributeName(wxT("opacity"));
			animElem->SetFrom(wxSVGAnimatedType(wxSVGLength(1)));
			animElem->SetTo(wxSVGAnimatedType(wxSVGLength(0)));
			animElem->SetBegin(transitionDur - 2);
			animElem->SetDur(2);
			animElem->SetFill(wxSVG_ANIMATION_FILL_FREEZE);
			frameSvgElem->AppendChild(animElem);
		}
	}
	// 2. set animate transfrom
	wxRect objBbox = menuObj->GetBBox();
	for (int i = 0; i < 3; i++) {
		wxSVGAnimateTransformElement* animElem = new wxSVGAnimateTransformElement();
		animElem->SetDur(5);
		animElem->SetFill(wxSVG_ANIMATION_FILL_FREEZE);
		animElem->SetAdditive(wxSVG_ANIMATION_ADDITIVE_SUM);
		switch (i) {
		case 0: {
			animElem->SetType(wxSVG_ANIMATETRANSFORM_ROTATE);
			wxSVGLengthList list;
			list.Add(wxSVGLength(0));
			list.Add(wxSVGLength(objBbox.GetX() + 0.5*objBbox.GetWidth()));
			list.Add(wxSVGLength(objBbox.GetY() + 0.5*objBbox.GetHeight()));
			animElem->SetFrom(wxSVGAnimatedType(list));
			list[0] = wxSVGLength(9);
			animElem->SetTo(wxSVGAnimatedType(list));
			break;
		}
		case 1: {
			animElem->SetType(wxSVG_ANIMATETRANSFORM_TRANSLATE);
			wxSVGLengthList from;
			from.Add(wxSVGLength(0));
			from.Add(wxSVGLength(0));
			animElem->SetFrom(wxSVGAnimatedType(from));
			wxSVGLengthList to;
			to.Add(wxSVGLength(-3*(objBbox.GetX() + 0.05*objBbox.GetWidth())));
			to.Add(wxSVGLength(-3*(objBbox.GetY() + 0.05*objBbox.GetHeight())));
			animElem->SetTo(wxSVGAnimatedType(to));
			break;
		}
		case 2:
			animElem->SetType(wxSVG_ANIMATETRANSFORM_SCALE);
			animElem->SetFrom(wxSVGAnimatedType(wxSVGLength(1)));
			animElem->SetTo(wxSVGAnimatedType(wxSVGLength(3)));
			break;
		}
		svgDoc->GetElementById(wxT("main"))->AppendChild(animElem);
	}
	
	wxFfmpegMediaEncoder ffmpeg(s_config.GetThreadCount());
	if (!ffmpeg.BeginEncode(transitionFile, menu->GetVideoFormat(), afNONE,
			menu->GetAspectRatio(), s_config.GetMenuVideoBitrate(), s_config.GetMenuVideoCBR())) {
		progressDlg->Failed(_("Error creation of transition"));
		return false;
	}
	
	int width = GetFrameSize(menu->GetVideoFormat()).GetWidth();
	int height = GetFrameSize(menu->GetVideoFormat()).GetHeight();
	int frameCount = lround(GetFps(menu->GetVideoFormat(), false) * transitionDur);
	for (int f = 0; f < frameCount; f++) {
		if (f % 100 == 0)
			progressDlg->AddDetailMsg(wxString::Format(_("Encoding frame %d of %d"), f + 1, frameCount));
		wxImage img = svgDoc->Render(width, height, NULL, false);
		if (!ffmpeg.EncodeImage(img, 1, progressDlg)) {
			progressDlg->Failed(_("Error creation of transition"));
			return false;
		}
		svgDoc->SetCurrentTime(((double) f) / GetFps(menu->GetVideoFormat(), false));
	}
	delete svgDoc;
	ffmpeg.EndEncode();
	return true;
}
