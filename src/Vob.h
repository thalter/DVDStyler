/////////////////////////////////////////////////////////////////////////////
// Name:        Vob.h
// Purpose:     The class to store a VOB data
// Author:      Alex Thuering
// Created:	    29.01.2003
// RCS-ID:      $Id: Vob.h,v 1.7 2016/12/15 20:46:17 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef VOB_H
#define VOB_H

#include "Stream.h"
#include "Cell.h"
#include "TextSub.h"
#include "Slideshow.h"
#include <vector>

using namespace std;

class wxSvgXmlNode;
class DVD;
class Menu;
class Cache;

class Vob {
public:
	Vob();
	Vob(const wxString& filename);
	Vob(Menu* menu);
	Vob(Slideshow* slideshow);
	Vob(const Vob& vob);
	~Vob();
	
	/** Sets name of video file for this vob*/
	bool SetFilename(const wxString& value);
	/** Returns name of video file */
	wxString GetFilename() { return m_filename; }
	/** Returns name of file to display */
	wxString GetFilenameDisplay();
	
	/** Returns true if there are audio streams in input file(s) */
	bool HasAudio();
	/** Returns count of audio streams */
	unsigned int GetAudioStreamCount();
	
    /** Returns array with audio tracks */
	const wxArrayString& GetAudioFilenames() { return m_audioFilenames; }
	/** Adds the given audio file to the vob */
	bool AddAudioFile(wxString filename);
	/** Removes audio file with given index from the vob */
	void RemoveAudioFile(int index);
	
    /** Returns array with subtitles tracks */
    TextSubArray& GetSubtitles() { return m_subtitles; }
	/** Adds the given subtitles file to the vob */
	bool AddSubtitlesFile(wxString filename);
	/** Returns count of subtitle streams */
	unsigned int GetSubtitleStreamsCount();
	
    /** Sets file name of temporary vob file */
	void SetTmpFilename(const wxString& value) { m_tmpFilename = value; }
    /** Returns file name of temporary vob file */
	const wxString& GetTmpFilename() { return m_tmpFilename; }
	
    /** Sets chapter list */
	void SetChapters(const wxString& value, bool firstVob);
    /** Returns chapter list */
	wxString GetChapters();
    /** Returns count of chapters */
	int GetChapterCount();
	
    /** Sets pause */
	void SetPause(int value) { m_pause = value; }
    /** Returns pause */
	int GetPause() { return m_pause; }
	
	/** Sets start time (sec) */
	void SetStartTime(double value) { m_startTime = value; }
	/** Returns start time (sec) */
	double GetStartTime() { return m_startTime; }
	
	/** Sets recording time (sec) */
	void SetRecordingTime(double value) { m_recordingTime = value; }
	/** Returns recording time (sec) */
	double GetRecordingTime() { return m_recordingTime; }
	
    /** Returns cells */
	vector<Cell*>& GetCells() { return m_cells; }
	
	/** Returns true if aspect ratio must be retained */
	bool GetKeepAspectRatio() { return m_keepAspectRatio; }
	/** Sets if aspect ratio must be retained */
	void SetKeepAspectRatio(bool keepAspectRatio) { m_keepAspectRatio = keepAspectRatio; }
	
	/** Returns true if video will be cropped to keep aspect ratio */
	bool GetKeepAspectCrop() { return m_keepAspectCrop; }
	/** Sets if video will be cropped to keep aspect ratio */
	void SetKeepAspectCrop(bool keepAspectCrop) { m_keepAspectCrop = keepAspectCrop; }
	
	/** Returns pad values: left, right, top, bottom */
	vector<int>& GetPad() { return m_pad; }
	/** Updates pad/crop value to keep aspect ratio*/
	void UpdatePadCrop(AspectRatio aspectRatio);
	/** Calculates pad value to keep aspect ratio*/
	bool CalcPad(int& padx, int& pady, VideoFormat videoFormat, AspectRatio aspectRatio, const vector<int>& crop);
	/** Calculates crop value to keep aspect ratio*/
	bool CalcCrop(int& cropx, int& cropy, VideoFormat videoFormat, AspectRatio aspectRatio, const vector<int>& pad);
	/** Returns crop values: left, right, top, bottom */
	vector<int>& GetCrop() { return m_crop; }
	
	/** Returns encode interlaced flag */
	bool GetInterlaced() { return m_interlaced; }
	/** Sets encode interlaced flag */
	void SetInterlaced(bool interlaced) { m_interlaced = interlaced; }
	
	/** Returns first field (interlacing) flag */
	FirstField GetFirstField() { return m_firstField; }
	/** Sets first field (interlacing) flag */
	void SetFirstField(FirstField firstField) { m_firstField = firstField; }
	
    /** Sets fade-in duration (sec) */
	void SetFadeIn(double value) { m_fadeIn = value; }
    /** Returns fade-in duration (sec) */
	double GetFadeIn() { return m_fadeIn; }
	
    /** Sets fade-out duration (sec) */
	void SetFadeOut(double value) { m_fadeOut = value; }
    /** Returns fade-out duration (sec) */
	double GetFadeOut() { return m_fadeOut; }
	
    /** Sets custom video filters (before pad/scale) */
	void SetVideoBeforeFilters(const wxString& value) { m_videoBeforeFilters = value; }
    /** Returns custom video filters (before pad/scale) */
	const wxString& GetVideoBeforeFilters() { return m_videoBeforeFilters; }
	/** Sets custom video filters (after pad/scale) */
	void SetVideoAfterFilters(const wxString& value) { m_videoAfterFilters = value; }
	/** Returns custom video filters (after pad/scale) */
	const wxString& GetVideoAfterFilters() { return m_videoAfterFilters; }
	/** Returns all video filters (incl. crop, pad, fade-in and fade-out) */
	wxString GetAllVideoFilters();
	
	void SetMenu(Menu* menu) { m_menu = menu; }
	Menu* GetMenu() { return m_menu; }
	
    void SetSlideshow(Slideshow* slideshow) { m_slideshow = slideshow; }
	Slideshow* GetSlideshow() { return m_slideshow; }
	
	/** Returns duration of original video (sec) */
	inline double GetDuration() { return m_duration; }
	/** Returns duration of result video (sec) */
	double GetResultDuration();
	/** Returns array with stream parameters */
	vector<Stream*>& GetStreams() { return m_streams; }
    /** Returns video stream parameters */
    Stream* GetVideoStream();
    /** Returns video stream index */
    int GetVideoStreamIndex();
	
    /** Returns true if the source video file must not be remultiplexed/transcoded */
   	inline bool GetDoNotTranscode() { return m_doNotTranscode; }
  	/** Sets if the source video file must not be remultiplexed/transcoded */
   	inline void SetDoNotTranscode(bool doNotTranscode) { m_doNotTranscode = doNotTranscode; }
	
	wxSvgXmlNode* GetXML(DVDFileType type, DVD* dvd, int nextTitle);
	bool PutXML(wxSvgXmlNode* node, DVD* dvd, int tsi, int pgci, bool menu, bool firstVob);
	
	/**
	 * Returns size in KB
     * @return Size of VOB file in KB
     */
	int GetSize(DVD* dvd);
	
	/**
	 * Returns size of output file in KB
     */
	int GetOutputFileSize(DVD* dvd, int fileIdx);
	
	/**
	 * Returns bitrate in KB/s
     */
	int GetBitrate(DVD* dvd, int fileIdx);
	
	/**
	 * Returns size after transcoding in KB
     * @return Size of VOB file in KB
     */
	int GetTranscodedSize(DVD* dvd);
	
	/**
	 * Returns size in KB required for generation
	 * @return Size in KB
	 */
	int GetRequiredSize(DVD* dvd, Cache* cache);
	
	/** Returns file size in KB */
	static unsigned int GetFileSize(const wxString& filename);
	
	/** Creates a VOB with blank video and silence audio */
	static Vob* CreateEmptyVob(VideoFormat videoFormat, AudioFormat audioFormat);
	
private:
	wxString m_filename;
	wxArrayString m_audioFilenames;
    TextSubArray m_subtitles;
	wxString m_tmpFilename;
	
	int m_pause;
	vector<Cell*> m_cells;
	
	Menu* m_menu;
    Slideshow* m_slideshow;
	
	double m_duration;
	double m_startTime;
	double m_recordingTime;
	vector<Stream*> m_streams;
	bool m_doNotTranscode;
	bool m_interlaced;
	FirstField m_firstField;
	bool m_keepAspectRatio;
	bool m_keepAspectCrop;
	vector<int> m_pad;
	vector<int> m_crop;
	double m_fadeIn; // fade-in duration (sec)
	double m_fadeOut; // fade-out duration (sec)
	wxString m_videoBeforeFilters; // filteres before pad/scale
	wxString m_videoAfterFilters; // filters after pad/scale
	void Init(Menu* menu = NULL, Slideshow* slideshow = NULL);
};

WX_DEFINE_ARRAY(Vob*, VobArray);

#endif // VOB_H
