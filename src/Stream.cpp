/////////////////////////////////////////////////////////////////////////////
// Name:        Stream.cpp
// Purpose:     The class to store stream information
// Author:      Alex Thuering
// Created:     25.10.2013
// RCS-ID:      $Id: Stream.cpp,v 1.11 2016/12/17 17:27:38 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "Stream.h"
#include "Vob.h"
#include "DVD.h"
#include <wx/intl.h>
#include <wxSVGXML/svgxmlhelpr.h>

Stream::Stream(StreamType type, wxString codecName) {
	m_type = type;
	m_sourceCodecName = codecName;
	m_sourceBitrate = 0;
	m_sourceSampleRate = 0;
	m_sourceChannelNumber = -1;
	m_sourceAspectRatio = -1;
	m_sourceFps = -1;
	m_destinationFormat = 1; // vfCOPY/afCOPY/sfCOPY
	m_audioAdjustType = aatChangePercent;
	m_audioVolume = 1.0;
	m_replayGainCalculated = false;
	m_trackGain = 0;
	m_channelNumber = -1;
	m_tsOffset = 0;
}

VideoFormat Stream::GetSourceVideoFormat() {
	int fps = lround(m_sourceFps);
	int w = m_sourceVideoSize.GetWidth();
	int h = m_sourceVideoSize.GetHeight();
	if (fps == 25) {
		if (w == 1920 && h == 1080)
			return vfPAL_FULL_HD;
		else if (w == 1440 && h == 1080)
			return vfPAL_HALF_HD;
		else if (w == 1280 && h == 720)
			return vfPAL_HALF_HD;
		else if (w == 720 && h == 576)
			return vfPAL;
		else if (w == 704 && h == 576)
			return vfPAL_CROPPED;
		else if (w == 352 && h == 576)
			return vfPAL_HALF_D1;
		else if (w == 352 && h == 288)
			return vfPAL_VCD;
	} else if (fps == 24 || fps == 30) {
		if (w == 1920 && h == 1080)
			return vfNTSC_FULL_HD;
		else if (w == 1440 && h == 1080)
			return vfNTSC_HALF_HD;
		else if (w == 1280 && h == 720)
			return vfNTSC_HALF_HD;
		else if (w == 720 && h == 480)
			return vfNTSC;
		else if (w == 704 && h == 480)
			return vfNTSC_CROPPED;
		else if (w == 352 && h == 480)
			return vfNTSC_HALF_D1;
		else if (w == 352 && h == 240)
			return vfNTSC_VCD;
	}
	return vfNONE; // unknown
}

AudioFormat Stream::GetSourceAudioFormat() {
	if (m_sourceCodecName == wxT("mp2"))
		return afMP2;
	else if (m_sourceCodecName == wxT("liba52") || m_sourceCodecName == wxT("ac3")
			|| m_sourceCodecName == wxT("ac-3"))
		return afAC3;
	else if (m_sourceCodecName.StartsWith("pcm") || m_sourceCodecName.StartsWith("lpcm"))
		return afPCM;
	return afNONE;
}

wxString Stream::GetSourceFormat() {
	wxString result = m_sourceCodecName;
	if (result == wxT("liba52") || m_sourceCodecName == wxT("ac-3"))
		result = wxT("ac3");
	else if (result.StartsWith(wxT("mpeg2video")))
		result = wxT("mpeg2");
	switch (m_type) {
	case stVIDEO:
		if (m_sourceVideoSize.IsFullySpecified()) {
			result += wxString::Format(wxT(", %dx%d"), m_sourceVideoSize.GetWidth(), m_sourceVideoSize.GetHeight());
			int fps = lround(m_sourceFps);
			if (fps == 25)
				result += wxT(" (PAL)");
			else if (fps == 30)
				result += wxT(" (NTSC)");
			else if (fps == 24)
				result += wxT(" (NTSC film)");
			else if (fps > 0)
				result += wxString::Format(wxT(" (%d fps)"), fps);
			if (round(GetSourceAspectRatio()*100) == 133 || round(GetSourceAspectRatio()*100) == 136)
				result += wxT(", 4:3");
			else if (round(GetSourceAspectRatio()*100) == 178)
				result += wxT(", 16:9");
			else
				result += wxString::Format(wxT(", 1:%0.2f"), GetSourceAspectRatio());
		}
		break;
	case stAUDIO:
		if (m_sourceChannelNumber > 0) {
			result += wxT(", ");
			if (m_sourceChannelNumber == 1)
				result += _("mono");
			else if (m_sourceChannelNumber == 2)
				result += _("stereo");
			else if (m_sourceChannelNumber == 6)
				result += wxT("5.1");
			else
				result += wxString::Format(_("%d channels"), m_sourceChannelNumber);
		}
		if (m_sourceSampleRate)
			result += wxT(", ") + wxString::Format(_("%d Hz"), m_sourceSampleRate);
		break;
	default:
		break;
	}
	return result;
}

/** Returns the stream language code (ISO 639) */
wxString Stream::GetLanguageCode() {
	wxString lang = GetLanguage().Upper().substr(0, 2);
	return DVD::GetAudioLanguageCodes().Index(lang) > 0 ? lang : wxT("");
}

/** Returns all audio filters (incl. volume, up- and downmix) */
wxString Stream::GetAllAudioFilters(bool forCacheEntry) {
	if (GetAudioFormat() == afCOPY)
		return wxT("");
	wxString result;
	switch (GetAudioAdjustType()) {
	case aatChangePercent:
		if (GetAudioVolume() != 1.0)
			result = wxString::Format(wxT("volume=%0.2f"), GetAudioVolume());
		break;
	case aatChangeDB:
		result = wxString::Format(wxT("volume=%+0.2f"), GetAudioVolume()) + wxT("dB");
		break;
	case aatNormalize:
		if (forCacheEntry)
			result = wxT("normalize");
		else if (IsReplayGainCalculated() && fabs(GetTrackGain()) > 1.0)
			result = wxString::Format(wxT("volume=%+0.2f"), GetTrackGain()) + wxT("dB");
		break;
	}
	if (GetDestinationFormat() >= afAC3 && GetChannelNumber() > 0) {
		if (result.length())
			result += wxT(',');
#ifdef USE_FFMPEG
		if (GetChannelNumber() == 6)
			result += wxT("aformat=channel_layouts=5.1,pan=5.1|c0=c0|c1=c1|c2=0.5*c0+0.5*c1|c3=c3|c4=c4|c5=c5");
		else
#endif
		if (GetChannelNumber() == 2)
			result += wxString::Format(wxT("aformat=channel_layouts=stereo"), GetChannelNumber());
		else
			result += wxString::Format(wxT("aformat=channel_layouts=%d"), GetChannelNumber());
	}
	if (GetFilters().length()) {
		if (result.length())
			result += wxT(',');
		result += GetFilters();
	}
	return result;
}

wxSvgXmlNode* Stream::GetXML(DVDFileType type, Vob* vob) {
	wxString streamName;
	switch (GetType()) {
	case stVIDEO:
		streamName = wxT("video");
		break;
	case stAUDIO:
		streamName = wxT("audio");
		break;
	case stSUBTITLE:
		streamName = wxT("subtitle");
		break;
	default:
		streamName = wxT("unknown");
		wxLogError(wxT("Unknown node"));
		break;
	}
	wxSvgXmlNode* streamNode = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, streamName);
	streamNode->AddProperty(wxT("format"), wxString::Format(wxT("%d"), GetDestinationFormat()));
	if (GetType() == stVIDEO) {
		if (vob->GetInterlaced()) {
			streamNode->AddProperty(wxT("interlaced"), wxT("1"));
			streamNode->AddProperty(wxT("firstField"), wxString::Format(wxT("%d"), vob->GetFirstField()));
		}
		if (!vob->GetKeepAspectRatio() && GetVideoFormat() > vfCOPY) {
			streamNode->AddProperty(wxT("keepAspect"), wxT("0"));
		} else if (vob->GetKeepAspectCrop()) {
			streamNode->AddProperty(wxT("keepAspectCrop"), wxT("1"));
		}
	} else if (GetType() == stAUDIO) {
		if (GetAudioAdjustType() != aatChangePercent)
			streamNode->AddProperty(wxT("adjustType"), wxString::Format(wxT("%d"), GetAudioAdjustType()));
		if (GetAudioAdjustType() != aatNormalize && (GetAudioAdjustType() != aatChangePercent || GetAudioVolume() != 1.0))
			streamNode->AddProperty(wxT("volume"), wxString::Format(wxT("%0.1f"), GetAudioVolume()));
		if (GetAudioAdjustType() == aatNormalize && IsReplayGainCalculated())
			streamNode->AddProperty(wxT("trackGain"), wxString::Format(wxT("%0.1f"), GetTrackGain()));
		if (GetChannelNumber() != -1)
			streamNode->AddProperty(wxT("channelNumber"), wxString::Format(wxT("%d"), GetChannelNumber()));
		if (GetTsOffset() != 0)
			streamNode->AddProperty(wxT("tsOffset"), wxString::Format(wxT("%ld"), GetTsOffset()));
		if (GetFilters().length())
			streamNode->AddProperty(wxT("filters"), GetFilters());
	}
	return streamNode;
}

/** Returns true if copy of video stream is possible */
bool Stream::IsCopyPossible() {
	if (GetType() != stVIDEO)
		return true;
	return m_sourceCodecName.StartsWith(wxT("mpeg2video"));
}

/** Returns true if video stream is DVD compliant */
bool Stream::IsDvdCompliant(bool hd) {
	if (GetType() != stVIDEO)
		return true;
	if (!IsCopyPossible() || GetSourceVideoFormat() == vfNONE)
		return false;
	if (!hd && GetSourceVideoFormat() >= vfPAL_HALF_HD)
		return false;
	return fabs(GetSourceAspectRatio() - GetFrameAspectRatio(ar4_3)) < 0.1 ||
			fabs(GetSourceAspectRatio() - GetFrameAspectRatio(ar16_9)) < 0.1;
}

bool Stream::PutXML(wxSvgXmlNode* node, Vob* vob) {
	wxString val;
	long lval;
	double dval;
	if (node->GetPropVal(wxT("format"), &val) && val.length() > 0 && val.ToLong(&lval))
		SetDestinationFormat(lval);
	if (GetType() == stVIDEO) {
		if (node->GetPropVal(wxT("interlaced"), &val) && val == wxT("1")) {
			vob->SetInterlaced(true);
			if (node->GetPropVal(wxT("firstField"), &val) && val.length() > 0 && val.ToLong(&lval))
				vob->SetFirstField((FirstField) lval);
		}
		if (node->GetPropVal(wxT("keepAspect"), &val) && val == wxT("0"))
			vob->SetKeepAspectRatio(false);
		if (node->GetPropVal(wxT("keepAspectCrop"), &val) && val == wxT("1"))
			vob->SetKeepAspectCrop(true);
	} else if (GetType() == stAUDIO) {
		if (node->GetPropVal(wxT("adjustType"), &val) && val.length() > 0 && val.ToLong(&lval))
			SetAudioAdjustType((AudioAdjustmentType) lval);
		if (node->GetPropVal(wxT("volume"), &val) && val.length() > 0 && val.ToDouble(&dval))
			SetAudioVolume(dval > 10 && GetAudioAdjustType() == aatChangePercent ? dval/256 : dval);
		if (node->GetPropVal(wxT("trackGain"), &val) && val.length() > 0 && val.ToDouble(&dval)) {
			SetTrackGain(dval);
			SetReplayGainCalculated(true);
		}
		if (node->GetPropVal(wxT("channelNumber"), &val) && val.length() > 0 && val.ToLong(&lval))
			SetChannelNumber(lval);
		if (node->GetPropVal(wxT("tsOffset"), &val) && val.length() > 0 && val.ToLong(&lval))
			SetTsOffset(lval);
		if (node->GetPropVal(wxT("filters"), &val))
			SetFilters(val);
	}
	return true;
}
