/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessTranscode.h
// Author:      Alex Thuering
// Created:     26.09.2014 (refactored)
// RCS-ID:      $Id: ProcessTranscode.cpp,v 1.15 2016/12/11 10:06:31 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "ProcessTranscode.h"
#include "ProcessExecute.h"
#include "DVD.h"
#include "Config.h"
#include "mediaenc_ffmpeg.h"
#include "mediatrc_ffmpeg.h"
#include <wx/progdlg.h>

/** Returns audio extenstion */
wxString ProcessTranscode::GetAudioExt(AudioFormat srcFormat, AudioFormat dstFormat) {
	AudioFormat format = dstFormat != afCOPY ? dstFormat : srcFormat;
	if (format == afMP2)
		return ".mp2";
	else if (format == afAC3)
		return ".ac3";
	else if (format == afPCM)
		return ".lpcm";
	return ".audio";
}

bool ProcessTranscode::Transcode(Vob* vob, AspectRatio aspect, int videoBitrate, int audioBitrate, bool useMplex,
				AudioFormat defAudioFormat) {
	if (progressDlg->WasCanceled())
		return false;
	int subStep = progressDlg->GetSubStep();
	progressDlg->AddDetailMsg(_("Transcode video file: ") + vob->GetFilename());
	// set output formats
	double startTime = vob->GetStartTime();
	double recordingTime = vob->GetRecordingTime();
	bool needEncode = vob->GetAudioFilenames().size() != vob->GetAudioStreamCount()
			|| startTime != 0 || recordingTime != -1; // some files are not elementary streams
	VideoFormat videoFormat = vfCOPY;
	bool ntscFilm = false;
	wxArrayInt audioFormats;
	wxArrayInt audioSrcFormat;
	wxArrayInt subtitleFormats;
	wxString lpcmParams;
	for (unsigned int stIdx = 0; stIdx < vob->GetStreams().size(); stIdx++) {
		Stream* stream = vob->GetStreams()[stIdx];
		switch (stream->GetType()) {
		case stVIDEO:
			if (stream->GetVideoFormat() != vfNONE) {
				videoFormat = stream->GetVideoFormat();
				if (!s_config.GetDefRencodeNtscFilm())
					ntscFilm = lround(stream->GetSourceFps()) == 24;
			}
			needEncode = needEncode || stream->GetVideoFormat() != vfCOPY;
			break;
		case stAUDIO:
			audioFormats.Add(stream->GetAudioFormat());
			audioSrcFormat.Add(stream->GetSourceAudioFormat());
//			if (stream->GetDestinationFormat() == afPCM) {
//				lpcmParams = wxString::Format("48000:%d:16",
//						stream->GetChannelNumber() < 0 ? stream->GetSourceChannelNumber() : stream->GetChannelNumber());
//			} else if (stream->GetDestinationFormat() == afCOPY) { 
//				if (stream->GetSourceCodecName().StartsWith("pcm_s")) {
//					lpcmParams = wxString::Format("48000:%d:", stream->GetSourceChannelNumber())
//							+ stream->GetSourceCodecName().substr(5, 2);//"48000:6:24";
//				} else if (stream->GetSourceCodecName().StartsWith("pcm") || stream->GetSourceCodecName().StartsWith("lpcm")) {
//					lpcmParams = wxString::Format("48000:%d:16", stream->GetSourceChannelNumber());
//				}
//			}
			needEncode = needEncode || stream->GetAudioFormat() != afCOPY;
			break;
		case stSUBTITLE:
			subtitleFormats.Add(stream->GetSubtitleFormat());
			break;
		default:
			break;
		}
	}
	
	progressDlg->AddDetailMsg(wxString(wxT("Need encode: ")) + (needEncode ? wxT("true") : wxT("false"))
			+ wxString(wxT(", use mplex: ")) + (useMplex ? wxT("true") : wxT("false")));
	
	// transcode
	if (useMplex && !needEncode) {
		if (!Multiplex(vob->GetFilename(), vob->GetAudioFilenames(), vob->GetTmpFilename()))
			return false;
	} else {
		// add input files
		wxFfmpegMediaTranscoder transcoder;
		if (!transcoder.AddInputFile(vob->GetFilename())) {
			progressDlg->Failed(wxT("Error by transcoding of ") + vob->GetFilename());
			return false;
		}
		for (int i = 0; i < (int)vob->GetAudioFilenames().size(); i++) {
			int stIdx = vob->GetStreams().size() - vob->GetAudioFilenames().size() + i;
			long tsOffset = vob->GetStreams()[stIdx]->GetTsOffset();
			wxString audioFilename = vob->GetAudioFilenames()[i];
			wxString audioFormatStr;
			if (audioFilename.EndsWith(wxT(".pcm"))) {
				audioFormatStr = "s16be:2";
			} else if (audioFilename.EndsWith(wxT(".lpcm"))) {
				audioFormatStr = "s16be:6";
			}
			if (!transcoder.AddInputFile(audioFilename, audioFormatStr, tsOffset)) {
				progressDlg->Failed(wxT("Error by transcoding of ") + vob->GetAudioFilenames()[i]);
				return false;
			}
		}
		if (vob && vob->GetFilename().length() && !vob->HasAudio()) {
#ifdef __WXMSW__
			wxString zero = wxT("aevalsrc=0");
			wxString format = wxT("lavfi");
#else
			wxString zero = wxT("/dev/zero");
			wxString format = wxT("");
#endif
			if (!transcoder.AddInputFile(zero, format)) {
				progressDlg->Failed(wxT("Error by transcoding of ") + zero);
				return false;
			}
			audioFormats.Add(defAudioFormat);
			audioSrcFormat.Add(afNONE);
			if (recordingTime <= 0)
				recordingTime = vob->GetDuration() > 0 ? vob->GetDuration() : 1.0;
		}
		
		// set output options
		transcoder.SetInterlaced(vob->GetInterlaced());
		transcoder.SetFirstField(vob->GetFirstField());
		if (vob->GetKeepAspectRatio()) {
			vob->UpdatePadCrop(aspect);
		}
		
		// set list of chapters to force key frames
		wxString chapters;
		for (vector<Cell*>::iterator it = vob->GetCells().begin(); it != vob->GetCells().end(); it++) {
			if ((*it)->GetStart() == 0)
				continue;
			if (chapters.length())
				chapters += wxT(",");
			chapters += (*it)->GetStartStr();
		}
		transcoder.SetChapterList(chapters);
		
		// set video filters
		transcoder.SetVideoFilters(vob->GetAllVideoFilters());
		
		// set audio filters
		int audioIdx = 0;
		for (unsigned int stIdx = 0; stIdx < vob->GetStreams().size(); stIdx++) {
			Stream* stream = vob->GetStreams()[stIdx];
			if (stream->GetType() == stAUDIO) {
				if (stream->GetAudioAdjustType() == aatNormalize && !stream->IsReplayGainCalculated()) {
					// calculate replay gain
					wxString audioFile;
					if (stIdx >= vob->GetStreams().size() - vob->GetAudioFilenames().size()) {
						int audioFileIdx = stIdx - vob->GetStreams().size() + vob->GetAudioFilenames().size();
						audioFile = vob->GetAudioFilenames()[audioFileIdx];
					}
					CalculateReplayGain(audioFile.size() ? audioFile : vob->GetFilename(),
							audioFile.size() ? 0 : audioIdx, stream);
				}
				wxString audioFilters = stream->GetAllAudioFilters();
				if (audioFilters.length())
					transcoder.SetAudioFilters(audioIdx, audioFilters);
				audioIdx++;
			}
		}
		
		if (startTime != 0) {
			wxLogMessage(wxT("startTime: %f"), startTime);
			transcoder.SetStartTime(startTime);
		}
		if (recordingTime > 0) {
			wxLogMessage(wxT("recordingTime: %f"), recordingTime);
			transcoder.SetRecordingTime(recordingTime);
		}
		
		if (!useMplex) {
			double fps = GetFps(videoFormat, ntscFilm);
			AVConvExecute exec(progressDlg, lround(vob->GetDuration() * fps));
			if (!transcoder.SetOutputFile(vob->GetTmpFilename(), videoFormat, ntscFilm, aspect, audioFormats,
					subtitleFormats, videoBitrate, s_config.GetVbr(), audioBitrate)
					|| !exec.Execute(transcoder.GetCmd())) {
				if (wxFileExists(vob->GetTmpFilename()))
					wxRemoveFile(vob->GetTmpFilename());
				progressDlg->Failed(_("Error transcoding of ") + vob->GetFilename());
				return false;
			}
		} else {
			wxString videoFile = vob->GetTmpFilename() + wxT(".m2v");
			if (wxFileExists(videoFile) && !wxRemoveFile(videoFile)) {
				wxLogError(wxString::Format(_("Can't remove file '%s'"), videoFile.c_str()));
				return false;
			}
			if (!transcoder.SetOutputFile(videoFile, videoFormat, ntscFilm, aspect, afNONE, sfNONE,
					videoBitrate, s_config.GetVbr(), audioBitrate, 0, 0, stVIDEO)) {
				progressDlg->Failed(_("Error transcoding of ") + vob->GetFilename());
				return false;
			}
			wxArrayString audioFiles;
			for (unsigned int audioIdx = 0; audioIdx < audioFormats.size(); audioIdx++) {
				if (audioFormats[audioIdx] == afNONE)
					continue;
				wxString audioFile = vob->GetTmpFilename() + wxString::Format(wxT("-%u"), audioIdx) +
						GetAudioExt((AudioFormat) audioSrcFormat[audioIdx], (AudioFormat) audioFormats[audioIdx]);;
				audioFiles.Add(audioFile);
				if (wxFileExists(audioFile) && !wxRemoveFile(audioFile)) {
					wxLogError(wxString::Format(_("Can't remove file '%s'"), audioFile.c_str()));
					return false;
				}
				if (startTime != 0)
					transcoder.SetStartTime(startTime);
				if (recordingTime > 0)
					transcoder.SetRecordingTime(recordingTime);
				transcoder.SetAudioFilters(0, transcoder.GetAudioFilters()[audioIdx]);
				int audioFileIdx = audioIdx + 1 - audioFormats.size() + vob->GetAudioFilenames().size();
				int audioStreamIdx = audioFileIdx > 0 ? 0 : audioIdx;
				if (audioFileIdx <= 0) {
					audioFileIdx = 0;
				}
				if (!transcoder.SetOutputFile(audioFile, vfNONE, false, aspect, (AudioFormat) audioFormats[audioIdx],
						sfNONE, videoBitrate, s_config.GetVbr(), audioBitrate, audioFileIdx, audioStreamIdx, stAUDIO)) {
					progressDlg->Failed(_("Error transcoding of ") + vob->GetFilename());
					return false;
				}
			}
			double fps = GetFps(videoFormat, ntscFilm);
			AVConvExecute exec(progressDlg, lround(vob->GetDuration() * fps));
			if (!exec.Execute(transcoder.GetCmd())) {
				if (wxFileExists(videoFile))
					wxRemoveFile(videoFile);
				for (unsigned int audioIdx = 0; audioIdx < audioFiles.size(); audioIdx++)
					if (wxFileExists(audioFiles[audioIdx]))
						wxRemoveFile(audioFiles[audioIdx]);
				progressDlg->Failed(_("Error transcoding of ") + vob->GetFilename());
				return false;
			}
			progressDlg->SetSubStep(subStep+150);
			if (!Multiplex(videoFile, audioFiles, vob->GetTmpFilename(), lpcmParams)) {
				return false;
			}
			// remove temp files
			if (s_config.GetRemoveTempFiles()) {
				DeleteFile(videoFile);
				for (unsigned int audioIdx = 0; audioIdx < audioFiles.size(); audioIdx++)
					DeleteFile(audioFiles[audioIdx]);
			}
		}
	}
	progressDlg->SetSubStep(subStep+200);
	return true;
}

bool ProcessTranscode::Multiplex(const wxString& videoFile, const wxArrayString& audioFiles,
		const wxString& vobFile, const wxString& lpcmParams) {
	if (progressDlg->WasCanceled())
		return false;
	progressDlg->AddDetailMsg(_("Multiplexing video and audio streams"));
	wxString cmd = s_config.GetMplexCmd();
	cmd.Replace("$PARAMS", lpcmParams.length() ? "-L " + lpcmParams : lpcmParams);
	cmd.Replace("$FILE_VIDEO", videoFile);
	wxString audio;
	for (unsigned int i = 0; i < audioFiles.Count(); i++)
		audio += (i > 0 ? wxT("\" \"") : wxT("")) + audioFiles[i];
	cmd.Replace("$FILE_AUDIO", audio);
	cmd.Replace("$FILE_OUT", vobFile);
	if (!Exec(cmd)) {
		progressDlg->Failed();
		return false;
	}
	return true;
}

/** Calculate replay gain values */
bool ProcessTranscode::CalculateReplayGain(const wxString& fileName, int audioStreamIdx, Stream* stream) {
	wxFfmpegMediaTranscoder transcoder;
	transcoder.AddInputFile(fileName);
	transcoder.ReplayGain(audioStreamIdx);
	wxString msg = wxString::Format(_("Analysis of audio track %d"), audioStreamIdx + 1);
	progressDlg->AddDetailMsg(msg);
	AVConvTimeExecute exec(progressDlg);
	if (!exec.Execute(transcoder.GetCmd(true))) {
		wxLogError(_("Execution of '%s' failed."), transcoder.GetCmd().c_str());
		return false;
	}
	if (!exec.IsOk()) {
		wxLogError(wxT("Failed calculation of replay gain"));
		return false;
	}
	double trackGain = exec.GetTrackGain();
	if (trackGain > 0) {
		wxFfmpegMediaTranscoder transcoder;
		transcoder.AddInputFile(fileName);
		transcoder.VolumeDetect(audioStreamIdx);AVConvTimeExecute exec(progressDlg);
		AVConvTimeExecute exec2(progressDlg);
		if (!exec2.Execute(transcoder.GetCmd(true))) {
			wxLogError(_("Execution of '%s' failed."), transcoder.GetCmd().c_str());
			return false;
		}
		if (!exec2.IsOk()) {
			wxLogError(wxT("Failed calculation of replay gain"));
			return false;
		}
		if (trackGain > -exec2.GetMaxVolume())
			trackGain = -exec2.GetMaxVolume();
	}
	stream->SetTrackGain(trackGain);
	stream->SetReplayGainCalculated(true);
	return true;
}

bool ProcessTranscode::Exec(wxString command, wxString inputFile, wxString outputFile) {
	ProcessExecute exec(progressDlg);
	return exec.Execute(command, inputFile, outputFile);
}
