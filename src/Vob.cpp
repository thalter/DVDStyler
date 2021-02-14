/////////////////////////////////////////////////////////////////////////////
// Name:        Vob.cpp
// Purpose:     The class to store a VOB data
// Author:      Alex Thuering
// Created:     29.01.2003
// RCS-ID:      $Id: Vob.cpp,v 1.15 2016/12/17 17:27:38 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "Vob.h"
#include "Menu.h"
#include "Cache.h"
#include "Config.h"
#include "Utils.h"
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>
#include <wx/sstream.h>
#include <wx/convauto.h>
#include <wx/progdlg.h>
#include <wxSVGXML/svgxmlhelpr.h>
#include <wxVillaLib/utils.h>
#include <wxSVG/mediadec_ffmpeg.h>
#include <wxSVG/SVGDocument.h>

#define DATA_FILE(fname) wxFindDataFile(_T("data") + wxString(wxFILE_SEP_PATH) + fname)

/** Constructor */
Vob::Vob() {
	Init();
}

/** Constructor */
Vob::Vob(const wxString& filename) {
	Init();
	SetFilename(filename);
}

/** Constructor */
Vob::Vob(Menu* menu) {
	Init(menu);
}

/** Constructor */
Vob::Vob(Slideshow* slideshow) {
	Init(NULL, slideshow);
}

void Vob::Init(Menu* menu, Slideshow* slideshow) {
	m_pause = 0;
	m_duration = 0;
	m_startTime = 0;
	m_recordingTime = -1;
	m_menu = menu;
	m_slideshow = slideshow;
	m_doNotTranscode = false;
	m_interlaced = false;
	m_firstField = ffAUTO;
	m_keepAspectRatio = true;
	m_keepAspectCrop = false;
	for (int i = 0; i < 4; i++) {
		m_pad.push_back(0);
		m_crop.push_back(0);
	}
	m_fadeIn = 0;
	m_fadeOut = 0;
}

/** Constructor */
Vob::Vob(const Vob& vob): m_pad(vob.m_pad), m_crop(vob.m_crop) {
	m_filename = vob.m_filename;
	m_audioFilenames = vob.m_audioFilenames;
	for (unsigned int i = 0; i<vob.m_subtitles.size(); i++)
		m_subtitles.Add(new TextSub(*vob.m_subtitles[i]));
	m_tmpFilename = vob.m_tmpFilename;
	m_pause = vob.m_pause;
	m_menu = vob.m_menu;
	m_slideshow = vob.m_slideshow != NULL ? new Slideshow(*vob.m_slideshow) : NULL;
	m_duration = vob.m_duration;
	m_startTime = vob.m_startTime;
	m_recordingTime = vob.m_recordingTime;
	for (unsigned int i = 0; i<vob.m_streams.size(); i++)
		m_streams.push_back(new Stream(*vob.m_streams[i]));
	VECTOR_COPY(vob.m_cells, m_cells, Cell);
	m_interlaced = vob.m_interlaced;
	m_firstField = vob.m_firstField;
	m_keepAspectRatio = vob.m_keepAspectRatio;
	m_keepAspectCrop = vob.m_keepAspectCrop;
	m_doNotTranscode = vob.m_doNotTranscode;
	m_fadeIn = vob.m_fadeIn;
	m_fadeOut = vob.m_fadeOut;
	m_videoBeforeFilters = vob.m_videoBeforeFilters;
	m_videoAfterFilters = vob.m_videoAfterFilters;
}

Vob::~Vob() {
	if (m_menu)
		delete m_menu;
	if (m_slideshow)
		delete m_slideshow;
	WX_CLEAR_ARRAY(m_subtitles);
	WX_CLEAR_ARRAY(m_streams);
	VECTOR_CLEAR(m_cells, Cell);
}

bool Vob::SetFilename(const wxString& filename) {
	wxFfmpegMediaDecoder ffmpeg;
	if (filename.length() == 0 || !ffmpeg.Load(filename))
		return false;
	m_filename = filename;
	// read streams information
	if (m_streams.size() > m_audioFilenames.size())
		m_streams.erase(m_streams.begin(), m_streams.end() - m_audioFilenames.size());
	for (unsigned int stIdx = 0; stIdx < ffmpeg.GetStreamCount(); stIdx++) {
		Stream* stream = new Stream(ffmpeg.GetStreamType(stIdx), ffmpeg.GetCodecName(stIdx));
		if (stream->GetType() == stAUDIO) {
			stream->SetSourceChannelNumber(ffmpeg.GetChannelNumber(stIdx));
			stream->SetSourceSampleRate(ffmpeg.GetSampleRate(stIdx));
			stream->SetSourceBitrate(ffmpeg.GetBitrate(stIdx));
		} else if (stream->GetType() == stVIDEO) {
			stream->SetSourceVideoSize(ffmpeg.GetVideoSize());
			stream->SetSourceBitrate(ffmpeg.GetBitrate(stIdx));
			stream->SetSourceAspectRatio(ffmpeg.GetFrameAspectRatio());
			stream->SetSourceFps(ffmpeg.GetFps());
		}
		map<wxString, wxString> metadata = ffmpeg.GetMetadata(stIdx);
		if (metadata.find(wxT("language")) != metadata.end())
			stream->SetLanguage(metadata[wxT("language")]);
		m_streams.insert(m_streams.begin() + stIdx, stream);
	}
	// get video duration
	m_duration = ffmpeg.GetDuration();
	if (m_duration < 0) {
		if (ffmpeg.SetPosition(360000, false))
			m_duration = ffmpeg.GetPosition();
	}
	// set chapters
	for (double t : ffmpeg.GetChapters()) {
		m_cells.push_back(new Cell(lround(t*1000)));
	}
	return true;
}

bool Vob::HasAudio() {
	if (GetAudioFilenames().size() > 0)
		return true;
	for (int i = 0; i < (int)GetStreams().size(); i++) {
		if (GetStreams()[i]->GetType() == stAUDIO)
			return true;
	}
	return false;
}

/** Returns count of audio streams */
unsigned int Vob::GetAudioStreamCount() {
	unsigned int cnt = 0;
	for (Stream* st : m_streams)
		if (st->GetType() == stAUDIO && st->GetAudioFormat() != afNONE)
			cnt++;
	return cnt;
}

/** Returns count of subtitle streams */
unsigned int Vob::GetSubtitleStreamsCount() {
	unsigned int cnt = 0;
	if (!m_menu) {
		for (Stream* st : m_streams)
			if (st->GetType() == stSUBTITLE && st->GetSubtitleFormat() != sfNONE)
				cnt++;
		cnt += GetSubtitles().size();
	} else
		cnt = 1;
	return cnt;
}

/** Adds the given audio file to the vob */
bool Vob::AddAudioFile(wxString filename) {
	if (filename.EndsWith(wxT("txt"))) {
		// concat format
		m_audioFilenames.Add(filename);
		m_streams.push_back(new Stream(stAUDIO, wxT("concat")));
		return true;
	}
	if (filename.EndsWith(wxT(".pcm")) || filename.EndsWith(wxT(".lpcm"))) {
		// PCM Raw format
		m_audioFilenames.Add(filename);
		Stream* stream = new Stream(stAUDIO, filename.EndsWith(wxT(".lpcm")) ? wxT("lpcm") : wxT("pcm"));
		stream->SetSourceChannelNumber(filename.EndsWith(wxT(".lpcm")) ? 6 : 2);
		stream->SetSourceSampleRate(48000);
		m_streams.push_back(stream);
		return true;
	}
	wxFfmpegMediaDecoder ffmpeg;
	if (!ffmpeg.Load(filename))
		return false;
	m_audioFilenames.Add(filename);
	if (ffmpeg.GetStreamCount() > 0) {
		for (unsigned int i = 0; i < ffmpeg.GetStreamCount(); i++) {
			if (ffmpeg.GetStreamType(i) == stAUDIO) {
				Stream* stream = new Stream(ffmpeg.GetStreamType(i), ffmpeg.GetCodecName(i));
				stream->SetSourceChannelNumber(ffmpeg.GetChannelNumber(i));
				stream->SetSourceSampleRate(ffmpeg.GetSampleRate(i));
				m_streams.push_back(stream);
				break;
			}
		}
	} else
		m_streams.push_back(new Stream(stAUDIO, wxT("unknown")));
	if (GetFilename().length() == 0) // menu or slideshow
		m_duration = ffmpeg.GetDuration();
	return true;
}

/** Adds the given subtitles file to the vob */
bool Vob::AddSubtitlesFile(wxString filename) {
	TextSub* textSub = new TextSub(filename);
	// detect encoding
	wxFile file;
	if (!file.Open(filename))
		return false;
	char buf[1024];
	int size = file.Read(buf, 1024);
	wxBOM bom = wxConvAuto::DetectBOM(buf, size);
	if (bom == wxBOM_UTF8) {
		textSub->SetCharacterSet(wxT("UTF-8"));
	} else if (bom == wxBOM_UTF16BE) {
		textSub->SetCharacterSet(wxT("UTF-16BE"));
	} else if (bom == wxBOM_UTF16LE) {
		textSub->SetCharacterSet(wxT("UTF-16LE"));
	} else if (bom == wxBOM_UTF32BE) {
		textSub->SetCharacterSet(wxT("UTF-32BE"));
	} else if (bom == wxBOM_UTF32LE) {
		textSub->SetCharacterSet(wxT("UTF-32LE"));
	} else if (!s_config.GetSubtitlesCharacterSet().StartsWith(wxT("UTF"))) {
		textSub->SetCharacterSet(s_config.GetSubtitlesCharacterSet());
	} else {
		wxString encoding = wxLocale::GetSystemEncodingName();
		if (encoding.StartsWith(wxT("windows-"))) {
			encoding = wxT("CP") + encoding.substr(8);
		}
		if (encoding.length()) {
			textSub->SetCharacterSet(encoding);
		}
	}
	m_subtitles.Add(textSub);
	return true;
}

/** Returns name of file to display */
wxString Vob::GetFilenameDisplay() {
	if (m_filename.StartsWith(wxT("concat:"))) {
		wxString fname = m_filename.Mid(7).BeforeFirst(wxT('|'));
		int idx = fname.Find(wxT("VIDEO_TS"));
		return idx > 0 ? fname.Mid(0, idx - 1) : fname;
	}
	return m_filename;
}

/** Removes audio file with given index from the vob */
void Vob::RemoveAudioFile(int index) {
	m_streams.erase(m_streams.end() - m_audioFilenames.size() + index);
	m_audioFilenames.RemoveAt(index);
}

/** Sets chapter list */
void Vob::SetChapters(const wxString& value, bool firstVob) {
	Stream* stream = GetVideoStream();
	float fps = stream != NULL ? stream->GetSourceFps() : 25;
	VECTOR_CLEAR(m_cells, Cell);
	wxStringTokenizer tkz(value, _T(","));
	while (tkz.HasMoreTokens()) {
		wxString token = tkz.GetNextToken().Strip(wxString::both);
		long startTime = String2Time(token, fps);
		if (m_cells.size() == 0 && startTime > 0)
			m_cells.push_back(new Cell(0, firstVob));
		m_cells.push_back(new Cell(startTime));
	}
	if (firstVob && m_cells.size() == 0)
		m_cells.push_back(new Cell(0));
}

/** Returns chapter list */
wxString Vob::GetChapters() {
	wxString result;
	for (Cell* cell : m_cells)
		if (cell->IsChapter()) {
			if (result.length())
				result += wxT(",");
			result += cell->GetStartStr();
		}
	return result;
}

/** Returns count of chapters */
int Vob::GetChapterCount() {
	int result = 0;
	for (Cell* cell : m_cells)
		if (cell->IsChapter())
			result++;
	return result;
}

/** Updates pad/crop values to keep aspect ratio*/
void Vob::UpdatePadCrop(AspectRatio aspectRatio) {
	Stream* videoSt = GetVideoStream();
	VideoFormat videoFormat = videoSt != NULL ? videoSt->GetVideoFormat() : vfCOPY;
	if (GetKeepAspectRatio()) {
		if (GetKeepAspectCrop()) {
			int cropx = 0;
			int cropy = 0;
			if (CalcCrop(cropx, cropy, videoFormat, aspectRatio, GetPad())) {
				m_crop[0] = m_crop[1] = cropx;
				m_crop[2] = m_crop[3] = cropy;
			}
		} else {
			int padx = 0;
			int pady = 0;
			if (CalcPad(padx, pady, videoFormat, aspectRatio, GetCrop())) {
				m_pad[0] = m_pad[1] = padx;
				m_pad[2] = m_pad[3] = pady;
			}
		}
	}
}

/** Calculates pad values to keep aspect ratio*/
bool Vob::CalcPad(int& padx, int& pady, VideoFormat videoFormat, AspectRatio aspectRatio, const vector<int>& crop) {
	Stream* videoSt = GetVideoStream();
	if (videoSt != NULL && videoSt->GetSourceAspectRatio() <= 0)
		return false;
	padx = 0;
	pady = 0;
	if (videoFormat != vfCOPY && aspectRatio != arAUTO) {
		float oldAspect = videoSt->GetSourceAspectRatio();
		wxSize oldFrameSize = videoSt->GetSourceVideoSize();
		int cropX = crop[0] + crop[1];
		int cropY = crop[2] + crop[3];
		if (cropX + cropY > 0 && oldFrameSize.x > cropX && oldFrameSize.y > cropY)
			oldAspect *= ((double) oldFrameSize.y)/oldFrameSize.x*(oldFrameSize.x - cropX)/(oldFrameSize.y - cropY);
		float aspect = GetFrameAspectRatio(aspectRatio);
		wxSize frameSize = GetFrameSize(videoFormat);
		if (aspect > oldAspect)
			padx = lround(((double)frameSize.x)*(1.0 - oldAspect/aspect)/2);
		else if (aspect < oldAspect)
			pady = lround(((double)frameSize.y)*(1.0 - aspect/oldAspect)/2);
	}
	return true;
}

/** Calculates crop value to keep aspect ratio*/
bool Vob::CalcCrop(int& cropx, int& cropy, VideoFormat videoFormat, AspectRatio aspectRatio, const vector<int>& pad) {
	Stream* videoSt = GetVideoStream();
	if (videoSt != NULL && videoSt->GetSourceAspectRatio() <= 0)
		return false;
	cropx = 0;
	cropy = 0;
	if (videoFormat != vfCOPY && aspectRatio != arAUTO) {
		float oldAspect = videoSt->GetSourceAspectRatio();
		wxSize oldFrameSize = videoSt->GetSourceVideoSize();
		float aspect = GetFrameAspectRatio(aspectRatio);
		wxSize frameSize = GetFrameSize(videoFormat);
		int padX = pad[0] + pad[1];
		int padY = pad[2] + pad[3];
		if (padX + padY > 0 && frameSize.x > padX && frameSize.y > padY)
			aspect *= ((double) frameSize.y)/frameSize.x*(frameSize.x - padX)/(frameSize.y - padY);
		if (aspect > oldAspect)
			cropy = lround(((double)oldFrameSize.y)*(1.0 - oldAspect/aspect)/2);
		else if (aspect < oldAspect)
			cropx = lround(((double)oldFrameSize.x)*(1.0 - aspect/oldAspect)/2);
	}
	return true;
}

/** Returns all video filters (incl. crop, pad, fade-in and fade-out) */
wxString Vob::GetAllVideoFilters() {
	wxString result;
	
	// add crop & pad filters
	VideoFormat videoFormat = vfCOPY;
	for (unsigned int stIdx = 0; stIdx < GetStreams().size(); stIdx++) {
		Stream* stream = GetStreams()[stIdx];
		if (stream->GetType() == stVIDEO && stream->GetVideoFormat() != vfNONE) {
			videoFormat = stream->GetVideoFormat();
			break;
		}
	}
	if (videoFormat != vfCOPY) {
		wxSize frameSize = GetFrameSize(videoFormat);
		bool doCrop = m_crop.size() == 4 && m_crop[0] + m_crop[1] + m_crop[2] + m_crop[3] > 0;
		bool doPad = m_pad.size() == 4 && m_pad[0] + m_pad[1] + m_pad[2] + m_pad[3] > 0;
		if (doCrop) {
			if (result.length())
				result += wxT(',');
			result += wxString::Format(wxT("crop=iw-%d:ih-%d:%d:%d"),
					m_crop[0] + m_crop[1], m_crop[2] + m_crop[3], m_crop[0], m_crop[2]);
		}
		if (doCrop || doPad) {
			if (result.length())
				result += wxT(',');
			result += wxString::Format(wxT("scale=%d:%d"),
					frameSize.GetWidth() - m_pad[0] - m_pad[1], frameSize.GetHeight() - m_pad[2] - m_pad[3]);
		}
		if (doPad) {
			result += wxString::Format(wxT(",pad=%d:%d:%d:%d"),
					frameSize.GetWidth(), frameSize.GetHeight(), m_pad[0], m_pad[2]);
		}
	}
	
	// add fade-in and fade-out filters
	if (GetFadeIn() > 0) {
		if (result.length())
			result += wxT(',');
		result += wxString::Format(wxT("fade=in:st=%g:d=%g"), GetStartTime(), GetFadeIn());
	}
	if (GetFadeOut() > 0) {
		if (result.length())
			result += wxT(',');
		double endTime = GetStartTime() + GetResultDuration();
		result += wxString::Format(wxT("fade=out:st=%g:d=%g"), endTime - GetFadeOut(), GetFadeOut());
	}
	
	// add custom filters 
	if (GetVideoBeforeFilters().length()) {
		if (result.length())
			result = wxT(',') + result;
		result = GetVideoBeforeFilters() + result;
	}
	if (GetVideoAfterFilters().length()) {
		if (result.length())
			result += wxT(',');
		result += GetVideoAfterFilters();
	}
	return result;
}

/** Returns duration of result video (sec) */
double Vob::GetResultDuration() {
	if (GetSlideshow()) {
		return GetSlideshow()->GetResultDuration();
	}
	return GetRecordingTime() >= 0 ? GetRecordingTime() : GetDuration() - GetStartTime();
}	

/** Returns video stream parameters */
Stream* Vob::GetVideoStream() {
	for (int i = 0; i < (int)GetStreams().size(); i++)
		if (GetStreams()[i]->GetType() == stVIDEO)
			return GetStreams()[i];
	return NULL;
}

/** Returns video stream index */
int Vob::GetVideoStreamIndex() {
	for (int i = 0; i < (int)GetStreams().size(); i++)
		if (GetStreams()[i]->GetType() == stVIDEO)
			return i;
	return -1;
}

wxSvgXmlNode* Vob::GetXML(DVDFileType type, DVD* dvd, int nextTitle) {
	wxSvgXmlNode* node = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("vob"));
	wxString fname = GetFilename();
	if (type == DVDAUTHOR_XML) {
		if (GetTmpFilename().length())
			fname = GetTmpFilename();
	} else {
		wxFileName filename(fname);
		if (filename.GetPath() == dvd->GetPath(false))
			fname = filename.GetFullName();
		else if (dvd->GetPath(false).length() > 0 && filename.GetPath().StartsWith(dvd->GetPath(false)))
			fname = filename.GetPath().substr(dvd->GetPath(false).length() + 1) + wxFILE_SEP_PATH
					+  filename.GetFullName();
	}
	if (fname.length())
		node->AddProperty(wxT("file"), fname);
	
	if (type == DVDSTYLER_XML) {
		int stIdx = 0;
		for (; stIdx < (int)(GetStreams().size() - GetAudioFilenames().size()); stIdx++) {
			if (GetStreams()[stIdx]->GetType() == stUNKNOWN)
				continue;
			node->AddChild(GetStreams()[stIdx]->GetXML(type, this));
		}
		for (int i = 0; i < (int)GetAudioFilenames().size(); i++) {
			wxSvgXmlNode* audioNode = GetStreams()[stIdx++]->GetXML(type, this);
			audioNode->AddChild(new wxSvgXmlNode(wxSVGXML_TEXT_NODE, wxEmptyString, GetAudioFilenames()[i]));
			node->AddChild(audioNode);
		}
		for (int i = 0;  i < (int) GetSubtitles().size(); i++) {
			node->AddChild(GetSubtitles()[i]->GetXML(type));
		}
		if (GetDoNotTranscode())
			node->AddProperty(wxT("doNotTranscode"), wxT("1"));
		if (m_pad[0] + m_pad[1] + m_pad[2] + m_pad[3] > 0) {
			wxSvgXmlNode* padNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("pad"));
			if (m_pad[0] > 0)
				padNode->AddProperty(wxT("left"), wxString::Format(wxT("%d"), m_pad[0]));
			if (m_pad[1] > 0)
				padNode->AddProperty(wxT("right"), wxString::Format(wxT("%d"), m_pad[1]));
			if (m_pad[2] > 0)
				padNode->AddProperty(wxT("top"), wxString::Format(wxT("%d"), m_pad[2]));
			if (m_pad[3] > 0)
				padNode->AddProperty(wxT("bottom"), wxString::Format(wxT("%d"), m_pad[3]));
			node->AddChild(padNode);
		}
		if (m_crop[0] + m_crop[1] + m_crop[2] + m_crop[3] > 0) {
			wxSvgXmlNode* cropNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, wxT("crop"));
			if (m_crop[0] > 0)
				cropNode->AddProperty(wxT("left"), wxString::Format(wxT("%d"), m_crop[0]));
			if (m_crop[1] > 0)
				cropNode->AddProperty(wxT("right"), wxString::Format(wxT("%d"), m_crop[1]));
			if (m_crop[2] > 0)
				cropNode->AddProperty(wxT("top"), wxString::Format(wxT("%d"), m_crop[2]));
			if (m_crop[3] > 0)
				cropNode->AddProperty(wxT("bottom"), wxString::Format(wxT("%d"), m_crop[3]));
			node->AddChild(cropNode);
		}
		// start & recording time
		if (GetStartTime() > 0)
			node->AddProperty(wxT("startTime"), wxString::Format(wxT("%f"), GetStartTime()));
		if (GetRecordingTime() > 0)
			node->AddProperty(wxT("recordingTime"), wxString::Format(wxT("%f"), GetRecordingTime()));
		// video filters
		if (GetFadeIn() > 0)
			node->AddProperty(wxT("fadeIn"), wxString::Format(wxT("%g"), GetFadeIn()));
		if (GetFadeOut() > 0)
			node->AddProperty(wxT("fadeOut"), wxString::Format(wxT("%g"), GetFadeOut()));
		if (GetVideoBeforeFilters().length() > 0)
			node->AddProperty(wxT("videoBeforeFilters"), GetVideoBeforeFilters());
		if (GetVideoAfterFilters().length() > 0)
			node->AddProperty(wxT("videoFilters"), GetVideoAfterFilters());
	}
	
	// chapters/cells
	bool cells = false;
	if (m_cells.size()) {
		if ((*m_cells.begin())->GetStart() > 0)
			cells = true;
		else if ((*(m_cells.end()-1))->GetEnd() != -1)
			cells = true;
	}
	for (Cell* cell : m_cells) {
		if ((!cell->IsChapter() && m_cells.size() > 0) || cell->GetEnd() != -1
				|| cell->GetPause() != 0 || cell->GetCommands().length() > 0) {
			cells = true;
			break;
		}
	}
	if (cells) {
		for (vector<Cell*>::iterator it = m_cells.begin(); it != m_cells.end(); it++) {
			Cell* cell = *it;
			long nextTime = it + 1 != m_cells.end() ? (*(it + 1))->GetStart() : -1;
			long endTime = type == DVDAUTHOR_XML && cell->GetEnd() == -1 ? nextTime : cell->GetEnd();
			node->AddChild(cell->GetXML(type, endTime, dvd, nextTitle));
		}
	} else {
		wxString chapters = GetChapters();
		if (chapters.length())
			node->AddProperty(wxT("chapters"), chapters);
		
		if (GetPause() != 0) {
			wxString pauseStr = GetPause() > 0 ? wxString::Format(wxT("%d"), GetPause()) : wxT("inf");
			node->AddProperty(wxT("pause"), pauseStr);
		}
	}
	
	if (GetMenu() && type == DVDSTYLER_XML)
		node->AddChild(GetMenu()->GetXML(type));
	
	if (GetSlideshow()) {
		if (type == DVDSTYLER_XML)
			node->AddChild(GetSlideshow()->GetXML(type));
		else if (type == DVDAUTHOR_XML && m_cells.size() == 0) {
			wxString chapters;
			int t = 1;
			for (unsigned i=1; i<GetSlideshow()->size()/5; i++) {
				t += GetSlideshow()->GetResultDuration();
				int h = t/3600;
				int m = (t%3600)/60;
				int s = t%60;
				if (chapters.length())
					chapters += wxT(",");
				chapters += wxString::Format(wxT("%d:%2d:%2d.1"), h, m, s);
			}
			node->AddProperty(wxT("chapters"), chapters);
		}
	}
	
	return node;
}

bool Vob::PutXML(wxSvgXmlNode* node, DVD* dvd, int tsi, int pgci, bool menu, bool firstVob) {
	wxString val;
	long lval;
	double dval;

	node->GetPropVal(wxT("file"), &val);
	if (val.length() > 0) {
		if (!val.StartsWith(wxT("concat:"))) {
			wxFileName fname(val);
			if (fname.IsRelative())
				val = dvd->GetPath() + fname.GetFullPath();
			else if (!wxFileExists(val) && wxFileExists(dvd->GetPath() + fname.GetFullName()))
				val = dvd->GetPath() + fname.GetFullName();
		}
		SetFilename(val);
	}
	int stIdx = 0;
	wxSvgXmlNode* child = node->GetChildren();
	while (child) {
		if (child->GetName() == wxT("video")) {
			if (child->GetChildren() != NULL && child->GetChildren()->GetContent().length() > 0) {
				val = child->GetChildren()->GetContent();
				wxFileName fname(val);
				if (fname.IsRelative())
					val = dvd->GetPath() + fname.GetFullPath();
				else if (!wxFileExists(val) && wxFileExists(dvd->GetPath() + fname.GetFullName()))
					val = dvd->GetPath() + fname.GetFullName();
				SetFilename(val);
			}

			if (child->GetPropVal(wxT("format"), &val) && val.length() > 0 && val.ToLong(&lval)) {
				if ((int)m_streams.size() <= stIdx || m_streams[stIdx]->GetType() != stVIDEO)
					stIdx = 0;
				while ((int)m_streams.size() > stIdx) {
					if (m_streams[stIdx]->GetType() == stVIDEO) {
						m_streams[stIdx]->PutXML(child, this);
						stIdx++;
						break;
					}
					stIdx++;
				}
			}
		} else if (child->GetName() == wxT("audio")) {
			if (child->GetChildren() != NULL && child->GetChildren()->GetContent().length() > 0) {
				val = child->GetChildren()->GetContent();
				wxFileName fname(val);
				if (fname.IsRelative())
					val = dvd->GetPath() + fname.GetFullPath();
				else if (!wxFileExists(val) && wxFileExists(dvd->GetPath() + fname.GetFullName()))
					val = dvd->GetPath() + fname.GetFullName();
				if (AddAudioFile(val)) {
					stIdx = m_streams.size() - 1;
					m_streams[stIdx]->PutXML(child, this);
					stIdx++;
				}
			} else if (child->GetPropVal(wxT("format"), &val) && val.length() > 0 && val.ToLong(&lval)) {
				if ((int)m_streams.size() <= stIdx || m_streams[stIdx]->GetType() != stAUDIO)
					stIdx = 0;
				while ((int)m_streams.size() > stIdx) {
					if (m_streams[stIdx]->GetType() == stAUDIO) {
						m_streams[stIdx]->PutXML(child, this);
						stIdx++;
						break;
					}
					stIdx++;
				}
			}
		} else if (child->GetName() == wxT("subtitle")) {
			if (child->GetPropVal(wxT("format"), &val) && val.length() > 0 && val.ToLong(&lval) && lval != 1) {
				if ((int)m_streams.size() <= stIdx || m_streams[stIdx]->GetType() != stSUBTITLE)
					stIdx = 0;
				while ((int)m_streams.size() > stIdx) {
					if (m_streams[stIdx]->GetType() == stSUBTITLE) {
						m_streams[stIdx]->PutXML(child, this);
						stIdx++;
						break;
					}
					stIdx++;
				}
			}
		} else if (child->GetName() == wxT("cell")) {
			Cell* cell = new Cell;
			cell->PutXML(child);
			m_cells.push_back(cell);
		} else if (child->GetName() == wxT("pad")) {
			m_pad.clear();
			m_pad.push_back(child->GetPropVal(wxT("left"), &val) && val.ToLong(&lval) ? lval : 0);
			m_pad.push_back(child->GetPropVal(wxT("right"), &val) && val.ToLong(&lval) ? lval : 0);
			m_pad.push_back(child->GetPropVal(wxT("top"), &val) && val.ToLong(&lval) ? lval : 0);
			m_pad.push_back(child->GetPropVal(wxT("bottom"), &val) && val.ToLong(&lval) ? lval : 0);
		} else if (child->GetName() == wxT("crop")) {
			m_crop.clear();
			m_crop.push_back(child->GetPropVal(wxT("left"), &val) && val.ToLong(&lval) ? lval : 0);
			m_crop.push_back(child->GetPropVal(wxT("right"), &val) && val.ToLong(&lval) ? lval : 0);
			m_crop.push_back(child->GetPropVal(wxT("top"), &val) && val.ToLong(&lval) ? lval : 0);
			m_crop.push_back(child->GetPropVal(wxT("bottom"), &val) && val.ToLong(&lval) ? lval : 0);
		} else if (child->GetName() == wxT("textsub")) {
			TextSub* textsub = new TextSub;
			textsub->PutXML(child);
			GetSubtitles().Add(textsub);
		} else if (child->GetName() == wxT("menu")) {
			m_menu = new Menu(dvd, tsi, pgci);
			if (!m_menu->PutXML(child))
				return false;
		} else if (child->GetName() == wxT("slideshow"))
			m_slideshow = new Slideshow(child);
		child = child->GetNext();
	}
	if (node->GetPropVal(wxT("pause"), &val)) {
		if (val == wxT("inf"))
			m_pause = -1;
		else if (val.ToLong(&lval))
			m_pause = int(lval);
	}
	m_startTime = node->GetPropVal(wxT("startTime"), &val) && val.ToDouble(&dval) ? dval : 0;
	m_recordingTime = node->GetPropVal(wxT("recordingTime"), &val) && val.ToDouble(&dval) ? dval : -1;
	m_fadeIn = node->GetPropVal(wxT("fadeIn"), &val) && val.ToDouble(&dval) ? dval : 0;
	m_fadeOut = node->GetPropVal(wxT("fadeOut"), &val) && val.ToDouble(&dval) ? dval : 0;
	m_videoBeforeFilters = node->GetPropVal(wxT("videoBeforeFilters"), wxT(""));
	m_videoAfterFilters = node->GetPropVal(wxT("videoFilters"), wxT(""));
			
	if (node->GetPropVal(wxT("doNotTranscode"), &val) && val == wxT("1"))
		SetDoNotTranscode(true);
	if (node->GetPropVal(wxT("chapters"), &val))
		SetChapters(val, firstVob);
	
	return true;
}

unsigned int Vob::GetFileSize(const wxString& filename) {
	if (filename.StartsWith(wxT("concat:"))) {
		unsigned int size = 0;
		wxString files = filename.substr(7);
		while (files.length() > 0) {
			wxString fname = files.BeforeFirst(wxT('|'));
			files = files.AfterFirst(wxT('|'));
			size += wxFile(fname).Length()/1024;
		}
		return size;
	}
	return wxFile(filename).Length()/1024;
}

/**
 * Returns size in KB
 * @return Size of VOB file in KB
 */
int Vob::GetSize(DVD* dvd) {
	long size = GetOutputFileSize(dvd, 0);
	for (unsigned int i = 0; i<GetAudioFilenames().Count(); i++) {
		size += GetOutputFileSize(dvd, i + 1);
	}
	for (unsigned int i=0; i<GetSubtitles().Count(); i++)
		size += GetFileSize(GetSubtitles()[i]->GetFilename());
	return size;
}

/**
 * Returns size of output file in KB
 */
int Vob::GetOutputFileSize(DVD* dvd, int fileIdx) {
	if (fileIdx > 0) {
		int stIdx = (int)m_streams.size() - m_audioFilenames.size() + fileIdx - 1;
		Stream* stream = m_streams[stIdx++];
		if (stream->GetAudioFormat() == afCOPY)
			return GetFileSize(GetAudioFilenames()[fileIdx - 1]);
		return (int) (GetResultDuration()*dvd->GetAudioBitrate()/8);
	}
	long size = 0;
	int stIdx = 0;
	if (GetMenu()) {
		double duration = GetMenu()->GetSVG()->GetDuration();
		if (duration < 1)
			duration = s_config.GetMenuFrameCount()/(GetMenu()->GetVideoFormat() == vfPAL ? 25 : 30);
		long menuSize = (long) (duration*s_config.GetMenuVideoBitrate()/8);
		if (!s_config.GetMenuVideoCBR())
			menuSize = menuSize / 3;
		if (menuSize < 100)
			menuSize = 100;
		size += menuSize;
	} else if (GetSlideshow()) {
		long slideshowSize = GetSlideshow()->GetResultDuration()*s_config.GetSlideshowVideoBitrate()/8;
		if (!s_config.GetSlideshowVideoCBR())
			slideshowSize = slideshowSize / 3;
		size += slideshowSize;
	} else {
		if (m_doNotTranscode) {
			size += GetFileSize(GetFilename());
		} else {
			if (GetFilename().length()) {
				int streamsCount = (int)m_streams.size() - m_audioFilenames.size();
				bool copyVideo = false;
				long audioSize = 0;
				for (; stIdx < streamsCount; stIdx++) {
					Stream* stream = m_streams[stIdx];
					switch (stream->GetType()) {
					case stVIDEO:
						if (stream->GetVideoFormat() == vfCOPY)
							copyVideo = true;
						else if (stream->GetVideoFormat() != vfNONE)
							size += (long) (GetResultDuration()*dvd->GetVideoBitrate()/8);
						break;
					case stAUDIO: {
						int srcBitrate = stream->GetSourceBitrate() > 0 ? stream->GetSourceBitrate() : 64000;
						audioSize += (long) (GetResultDuration()*srcBitrate/8/1024);
						if (stream->GetAudioFormat() == afCOPY)
							size += (long) (GetResultDuration()*srcBitrate/8/1024);
						else if (stream->GetAudioFormat() != afNONE)
							size += (long) (GetResultDuration()*dvd->GetAudioBitrate()/8);
						break;
					}
					case stSUBTITLE:
					case stUNKNOWN:
						break;
					}
				}
				if (copyVideo) {
					size += (GetFileSize(GetFilename()) - audioSize)*GetResultDuration()/GetDuration();
				}
			}
		}
	}
	return size;
}

/**
 * Returns bitrate in KBit/s
 */
int Vob::GetBitrate(DVD* dvd, int fileIdx) {
	if (fileIdx > 0) {
		int stIdx = (int)m_streams.size() - m_audioFilenames.size() + fileIdx - 1;
		Stream* stream = m_streams[stIdx++];
		if (stream->GetAudioFormat() == afCOPY)
			return 0;
		return dvd->GetAudioBitrate();
	}
	if (GetMenu()) {
		return s_config.GetMenuVideoBitrate();
	} else if (GetSlideshow()) {
		return s_config.GetSlideshowVideoBitrate();
	} else if (m_doNotTranscode || GetVideoStream() == NULL || GetVideoStream()->GetDestinationFormat() == vfCOPY) {
		return 0;
	}
	return dvd->GetVideoBitrate();
}


/**
 * Returns size after transcoding in KB
 * @return Size of VOB file in KB
 */
int Vob::GetTranscodedSize(DVD* dvd) {
	if (GetTmpFilename().length())
		return GetFileSize(GetTmpFilename());
	else if (GetFilename().length())
		return GetFileSize(GetFilename());
	return 0;
}

int Vob::GetRequiredSize(DVD* dvd, Cache* cache) {
	int size = GetSize(dvd);
	if (GetMenu() || GetSlideshow() || GetSubtitles().Count() > 0
			 || ((!GetDoNotTranscode() || GetAudioFilenames().Count() > 0)
					 && cache->Find(this, dvd).length() == 0))
		size = size * 2;
	return size;
}

Vob* Vob::CreateEmptyVob(VideoFormat videoFormat, AudioFormat audioFormat) {
	wxString filename= wxT("empty_pal_mp2.mpg");
	if (videoFormat == vfNTSC) {
		filename = audioFormat == afMP2 ? wxT("empty_ntsc_mp2.mpg") : wxT("empty_ntsc_ac3.mpg");
	} else if (audioFormat != afMP2) {
		filename = wxT("empty_pal_ac3.mpg");
	}
	return new Vob(DATA_FILE(filename));
}
