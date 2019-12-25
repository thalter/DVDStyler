/////////////////////////////////////////////////////////////////////////////
// Name:        ProcessTranscode.h
// Author:      Alex Thuering
// Created:     26.09.2014 (refactored)
// RCS-ID:      $Id: ProcessTranscode.h,v 1.6 2016/12/11 10:06:31 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef DS_PROCESS_TRANSCODE_H
#define DS_PROCESS_TRANSCODE_H

#include "Process.h"
#include "ProcessExecute.h"
class Vob;
class Stream;

/**
 * Provides transcoding and multiplex methods
 */
class ProcessTranscode: public Process {
protected:
	/** Constructor */
	ProcessTranscode(ProgressDlg* progressDlg): Process(progressDlg) { }
	/** Process transcoding for VOB */
	bool Transcode(Vob* vob, AspectRatio aspect, int videoBitrate, int audioBitrate, bool useMplex,
				AudioFormat defAudioFormat = afAC3);
	/** Multiplex video with audio */
	bool Multiplex(const wxString& videoFile, const wxArrayString& audioFiles, const wxString& vobFile,
			const wxString& lpcmParams = "");
	/** Calculate replay gain values */
	bool CalculateReplayGain(const wxString& fileName, int audioStreamIdx, Stream* stream);
	/** Executes given command */
	bool Exec(wxString command, wxString inputFile = wxEmptyString, wxString outputFile = wxEmptyString);

private:
	/** Returns audio extenstion */
	wxString GetAudioExt(AudioFormat srcFormat, AudioFormat dstFormat);
};

#endif // DS_PROCESS_TRANSCODE_H
