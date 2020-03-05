/////////////////////////////////////////////////////////////////////////////
// Name:        VobListBox.cpp
// Purpose:     The list box to display information about streams in given VOB
// Author:      Alex Thuering
// Created:     03.05.2009
// RCS-ID:      $Id: VobListBox.cpp,v 1.32 2016/12/17 17:27:38 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "VobListBox.h"
#include "Config.h"
#include "VideoPropDlg.h"
#include "AudioPropDlg.h"
#include "SubtitlePropDlg.h"
#include "TitlePropDlg.h"
#include <wx/filedlg.h>
#include <wx/progdlg.h>
#include <wx/arrimpl.cpp>
#include <wxSVG/mediadec_ffmpeg.h>
#include <wxVillaLib/utils.h>
#include <wxVillaLib/rc/video.png.h>
#include <wxVillaLib/rc/audio.png.h>
#include <wxVillaLib/rc/subtitle.png.h>

WX_DEFINE_OBJARRAY(RectList);
WX_DEFINE_OBJARRAY(RectListOfList);
WX_DEFINE_OBJARRAY(StringListOfList);

BEGIN_EVENT_TABLE(VobListBox, wxVListBox)
	EVT_LEFT_DCLICK(VobListBox::OnDoubleClick)
	EVT_BUTTON(wxID_ANY, VobListBox::OnButton)
	EVT_CHOICE(wxID_ANY, VobListBox::OnFormatChange)
END_EVENT_TABLE()

const int ITEM_HEIGHT = 50;
#define ITEM_FONT wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)

VobListBox::VobListBox(wxScrolledWindow* parent, wxWindowID id, Vob* vob, PgcArray* pgcs, DVD* dvd,
		TitlePropDlg* titlePropDlg): wxVListBox(parent, id) {
	m_titlePropDlg = titlePropDlg;
	m_vob = vob;
	m_pgcs = pgcs;
	m_dvd = dvd;
	m_aspectRatio = pgcs->GetVideo().GetAspect();
	m_videoImg = Scale(LoadFrame(m_vob->GetFilename()));
	m_audioImg = Scale(wxBITMAP_FROM_MEMORY(audio).ConvertToImage());
	m_subtitleImg = Scale(wxBITMAP_FROM_MEMORY(subtitle).ConvertToImage());
	RefreshInfo();
	int maxh = 300;
	for (unsigned int i = 0; i < m_infoRect.size(); i++) {
		int h = OnMeasureItem(i);
		if (h > maxh)
			maxh = h;
	}
	SetMinSize(wxSize(-1, maxh));
}

wxCoord VobListBox::OnMeasureItem(size_t n) const {
	if (n >= m_infoRect.size())
		return ITEM_HEIGHT;
	int h = m_infoRect[n][m_infoRect[n].size()-1].GetBottom();
	return h + 4 > ITEM_HEIGHT ? h + 4 : ITEM_HEIGHT;
}

void VobListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const {
	// image
	wxBitmap image = n == 0 ? m_videoImg : n > m_vob->GetAudioFilenames().size() ? m_subtitleImg : m_audioImg;
	dc.DrawBitmap(image, rect.x + 2, rect.y + 2);
	
	// info
	dc.SetFont(ITEM_FONT);
	dc.SetTextForeground((int)n == GetSelection() ? wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT)
			: wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT));
	dc.SetTextForeground(*wxBLACK);
	wxArrayString& info = m_info[n];
	RectList& infoRect = m_infoRect[n];
	for (unsigned int i=0; i<info.GetCount(); i++) {
		dc.DrawText(info[i], rect.x + infoRect[i].GetX(), rect.y + infoRect[i].GetY());
	}
}

void VobListBox::RefreshInfo() {
	SetItemCount(1 + m_vob->GetAudioFilenames().size() + m_vob->GetSubtitles().size());
	
	m_info.Clear();
	m_infoRect.Clear();
	
	int choiceIdx = 0;
	int buttonIdx = 0;
	int itemY = 0;
	unsigned int audioIdx = 0;
	unsigned int subtitleIdx = 0;
	for (unsigned int n = 0; n < 1 + m_vob->GetAudioFilenames().size(); n++) {
		m_info.Add(wxArrayString());
		m_infoRect.Add(RectList());
		
		wxMemoryDC dc;
		dc.SetFont(ITEM_FONT);
		wxBitmap image = n == 0 ? m_videoImg : m_audioImg;
		int x = image.GetWidth() + 8;
		int y = 2;
		
		// filename
		y = AddInfo(n == 0 ? m_vob->GetFilenameDisplay() : m_vob->GetAudioFilenames()[n-1], n, dc, x, y);
			
		// duration
		if (n == 0) {
			wxString s = _("Duration:") + wxString(wxT(" "));
			if (m_vob->GetDuration()>0) {
				int secs = (int) m_vob->GetDuration();
				int mins = secs / 60;
				secs %= 60;
				int hours = mins / 60;
				mins %= 60;
				s += wxString::Format(wxT("%02d:%02d:%02d"), hours, mins, secs);
			} else
				s += wxT("N/A");
			y = AddInfo(s, n, dc, x, y);
		}
		
		// stream info
		int stIdx = n == 0 ? 0 : (int)m_vob->GetStreams().size() - m_vob->GetAudioFilenames().size() + n - 1;
		int streamsCount = n == 0 ? (int)m_vob->GetStreams().size() - m_vob->GetAudioFilenames().size() : 1;
		for (int stN = 0; stN < streamsCount; stN++) {
			Stream* stream = m_vob->GetStreams()[stIdx + stN];
			wxString srcFormat = stream->GetSourceFormat();
			switch (stream->GetType()) {
			case stVIDEO: {
				y = AddInfo(_("Video:") + wxString(wxT(" ")) + srcFormat, n, dc, x, y);
				wxRect& rect = m_infoRect[n][m_infoRect[n].size()-1];
				int top = rect.GetTop();
				int x = rect.GetRight() + 4;
				bool copyEnabled = stream->IsCopyPossible();
				AddChoiceCtrl(DVD::GetVideoFormatLabels(copyEnabled, true, false, m_dvd->IsHD()),
						GetVideoFormatIdx(stream, stream->GetVideoFormat()),
						x, itemY + top, choiceIdx, true);
				y += UpdateRect(rect, m_choiceList[choiceIdx-1]);
				x += m_choiceList[choiceIdx-1]->GetSize().GetWidth() + 2;
				// button
				AddButton(x, itemY + top, buttonIdx, stream->GetVideoFormat() != vfNONE, stIdx + stN);
				break;
			}
			case stAUDIO: {
				y = AddInfo(_("Audio:") + wxString(wxT(" ")) + srcFormat, n, dc, x, y);
				wxRect& rect = m_infoRect[n][m_infoRect[n].size()-1];
				int top = rect.GetTop();
				int x = rect.GetRight() + 4;
				bool copyEnabled = stream->IsCopyPossible();
				int afIdx = stream->GetAudioFormat();
				if (!copyEnabled && afIdx > 0)
					afIdx -= 1;
				AddChoiceCtrl(DVD::GetAudioFormatLabels(copyEnabled, true), afIdx, x, itemY + top, choiceIdx, true);
				y += UpdateRect(rect, m_choiceList[choiceIdx-1]);
				x += m_choiceList[choiceIdx-1]->GetSize().GetWidth() + 2;
				// langCode
				wxString langCode = stream->GetLanguageCode();
				if (stream->GetAudioFormat() != afNONE) {
					if (audioIdx < m_pgcs->GetAudioLangCodes().size())
						langCode = m_pgcs->GetAudioLangCodes()[audioIdx];
					audioIdx++;
				} 
				if (langCode.length() == 0) {
					langCode = s_config.GetDefAudioLanguage();
				}
				int langIdx = DVD::GetAudioLanguageCodes().Index(langCode);
				AddChoiceCtrl(DVD::GetAudioLanguageCodes(), langIdx, x, itemY + top, choiceIdx, true);
				x += m_choiceList[choiceIdx-1]->GetSize().GetWidth() + 2;
				// button
				AddButton(x, itemY + top, buttonIdx, true, stIdx + stN);
				break;
			}
			case stSUBTITLE: {
				y = AddInfo(_("Subtitle:") + wxString(wxT(" ")) + srcFormat, n, dc, x, y);
				wxRect& rect = m_infoRect[n][m_infoRect[n].size()-1];
				int top = rect.GetTop();
				int x = rect.GetRight() + 4;
				bool copyEnabled = stream->IsCopyPossible();
				int sfIdx = stream->GetSubtitleFormat();
				if (!copyEnabled && sfIdx > 0)
					sfIdx -= 1;
				AddChoiceCtrl(DVD::GetSubtitleFormatLabels(copyEnabled, true), sfIdx, rect.GetRight() + 4,
						itemY + top, choiceIdx, true);
				y += UpdateRect(rect, m_choiceList[choiceIdx-1]);
				x += m_choiceList[choiceIdx-1]->GetSize().GetWidth() + 2;
				// lang code
				wxString langCode = stream->GetLanguageCode();
				if (stream->GetSubtitleFormat() != sfNONE) {
					if (subtitleIdx < m_pgcs->GetSubPictures().size())
						langCode = m_pgcs->GetSubPictures()[subtitleIdx]->GetLangCode();
					subtitleIdx++;
				} 
				if (langCode.length() == 0) {
					langCode = s_config.GetDefSubtitleLanguage();
				}
				int langIdx = DVD::GetAudioLanguageCodes().Index(langCode);
				AddChoiceCtrl(DVD::GetAudioLanguageCodes(), langIdx, x, itemY + top, choiceIdx, true);
				x += m_choiceList[choiceIdx-1]->GetSize().GetWidth() + 2;
				break;
			}
			default:
				break;
			}
		}
		itemY += y > ITEM_HEIGHT ? y + 1 : ITEM_HEIGHT + 1;
		
		if (n == 0)
			m_audioChoiceIdx = choiceIdx;
	}

	m_subtitleChoiceIdx = choiceIdx;
	for (unsigned int si = 0; si < m_vob->GetSubtitles().size(); si++) {
		m_info.Add(wxArrayString());
		m_infoRect.Add(RectList());
		int n = m_info.GetCount() - 1;
		
		wxMemoryDC dc;
		dc.SetFont(ITEM_FONT);
		int x = m_subtitleImg.GetWidth() + 8;
		int y = 2;
		
		// filename
		y = AddInfo(m_vob->GetSubtitles()[si]->GetFilename(), n, dc, x, y);
		y = AddInfo(_("Subtitle:") + wxString(wxT(" ")) + m_vob->GetSubtitles()[si]->GetFilename().AfterLast(wxT('.')),
				n, dc, x, y);
		
		wxRect& rect = m_infoRect[n][m_infoRect[n].size()-1];
		int top = rect.GetTop();
		x = rect.GetRight() + 4;
		int langIdx = DVD::GetAudioLanguageCodes().Index(subtitleIdx < m_pgcs->GetSubPictures().size()
				? m_pgcs->GetSubPictures()[subtitleIdx]->GetLangCode() : s_config.GetDefSubtitleLanguage());
		subtitleIdx++;
		AddChoiceCtrl(DVD::GetAudioLanguageCodes(), langIdx, x, itemY + top, choiceIdx, true);
		y += UpdateRect(rect, m_choiceList[choiceIdx-1]);
		x += m_choiceList[choiceIdx-1]->GetSize().GetWidth() + 2;
		// button
		AddButton(x, itemY + top, buttonIdx, true, m_vob->GetStreams().size() + si);
		
		itemY += y > ITEM_HEIGHT ? y + 1 : ITEM_HEIGHT + 1;
	}
}

int VobListBox::AddInfo(const wxString& s, int n, wxDC& dc, int x, int y) {
	m_info[n].Add(s);
	wxCoord w;
	wxCoord h;
	dc.GetTextExtent(s, &w, &h);
	m_infoRect[n].Add(wxRect(x, y, w, h));
	return y + h + 2;
}

/** Creates choice control and adds it to the list m_choiceList */
void VobListBox::AddChoiceCtrl(wxArrayString formats, int selection, int x, int y, int& choiceIdx,
		bool enabled) {
	wxChoice* ctrl = NULL;
	if (choiceIdx >= (int) m_choiceList.size() || m_choiceList[choiceIdx] == NULL) {
		ctrl = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, formats);
		ctrl->SetSelection(selection);
		if (choiceIdx >= (int) m_choiceList.size())
			m_choiceList.push_back(ctrl);
		else
			m_choiceList[choiceIdx] = ctrl;
	} else
		ctrl = m_choiceList[choiceIdx];
	choiceIdx++;
	ctrl->Enable(enabled);
	ctrl->SetPosition(wxPoint(x, y));
}

/** Increase y and height if height of control is greater that actual height */
int VobListBox::UpdateRect(wxRect& rect, wxControl* ctrl) {
	if (ctrl->GetSize().GetHeight() > rect.GetHeight()) {
		int p = (ctrl->GetSize().GetHeight() - rect.GetHeight() + 1)/2;
		rect.SetY(rect.GetY() + p);
		rect.SetHeight(rect.GetHeight() + p);
		return 2*p;
	}
	return 0;
	
}

/** Creates button and adds it to the list m_buttonList */
void VobListBox::AddButton(int x, int y, int& buttonIdx, bool enabled, int streamIdx) {
	wxButton* ctrl = NULL;
	if (buttonIdx >= (int) m_buttonList.size() || m_buttonList[buttonIdx] == NULL) {
		int h = (*m_choiceList.begin())->GetSize().GetHeight();
		ctrl = new wxButton(this, wxID_ANY, wxT("..."), wxDefaultPosition, wxSize(h, h));
		if (buttonIdx >= (int) m_buttonList.size()) {
			m_buttonList.push_back(ctrl);
			m_buttonStreamList.push_back(streamIdx);
		} else {
			m_buttonList[buttonIdx] = ctrl;
			m_buttonStreamList[buttonIdx] = streamIdx;
		}
	} else {
		ctrl = m_buttonList[buttonIdx];
		m_buttonStreamList[buttonIdx] = streamIdx;
	}
	buttonIdx++;
	ctrl->Enable(enabled);
	ctrl->SetPosition(wxPoint(x, y));
}

wxImage VobListBox::LoadFrame(const wxString& filename) const {
	wxImage image;
	wxFfmpegMediaDecoder decoder;
	if (decoder.Load(filename)) {
		double duration = decoder.GetDuration();
		if (duration > 0) {
			image = decoder.GetNextFrame();
			double dpos = duration < 6000 ? duration * 0.05 : 300;
			decoder.SetPosition(dpos - 1.0);
			for (int i = 0; i < 60; i++) {
				image = decoder.GetNextFrame();
				if (decoder.GetPosition() >= dpos)
					break;
			}
		} else {
			for (int i = 0; i < 30; i++)
				image = decoder.GetNextFrame();
		}
		decoder.Close();
	}
	return image.Ok() ? image : wxBITMAP_FROM_MEMORY(video).ConvertToImage();
}

wxBitmap VobListBox::Scale(wxImage image) {
	int h = ITEM_HEIGHT - 4;
	int w = image.GetWidth()*h/image.GetHeight();
	return wxBitmap(image.Scale(w, h));
}

void VobListBox::RemoveItem(int index) {
	if (index > (int)m_vob->GetAudioFilenames().size()) {
		// remove subtitle
		int subtitleIdx = index - 1 - m_vob->GetAudioFilenames().size();
		m_vob->GetSubtitles().RemoveAt(subtitleIdx);
		// choice index = count of video stream + count of audio streams * 2 + count of audio subtitle streams
		int choiceIdx = m_subtitleChoiceIdx + subtitleIdx;
		m_choiceList[choiceIdx]->Hide();
		m_choiceList.erase(m_choiceList.begin() + choiceIdx);
		int buttonIdx = GetButtonIdx(m_vob->GetStreams().size() + subtitleIdx);
		if (buttonIdx >= 0) {
			m_buttonList[buttonIdx]->Hide();
			m_buttonList.erase(m_buttonList.begin() + buttonIdx);
			m_buttonStreamList.erase(m_buttonStreamList.begin() + buttonIdx);
		}
	} else {
		// removed audio file
		int audioIdx = index - 1;
		m_vob->RemoveAudioFile(audioIdx);
		// choice index = count of video stream + count of audio streams * 2
		int choiceIdx = m_audioChoiceIdx + audioIdx*2;
		m_choiceList[choiceIdx]->Hide();
		m_choiceList[choiceIdx + 1]->Hide();
		m_choiceList.erase(m_choiceList.begin() + choiceIdx);
		m_choiceList.erase(m_choiceList.begin() + choiceIdx);
		int buttonIdx = GetButtonIdx(m_vob->GetStreams().size() - m_vob->GetAudioFilenames().size() + audioIdx);
		if (buttonIdx >= 0) {
			m_buttonList[buttonIdx]->Hide();
			m_buttonList.erase(m_buttonList.begin() + buttonIdx);
			m_buttonStreamList.erase(m_buttonStreamList.begin() + buttonIdx);
		}
	}
	RefreshInfo();
	RefreshAll();
}

void VobListBox::AddAudio(wxString filename) {
	m_vob->AddAudioFile(filename);
	if (filename.EndsWith(wxT(".pcm")) || filename.EndsWith(wxT(".lpcm"))) {
		wxLogWarning("Only s16be 2-channel PCM and 6-channel LPCM audio files are supported now (experimental)");
	}
	m_vob->SetDoNotTranscode(false);
	// check if reencoding is needed
	Stream* stream = m_vob->GetStreams()[m_vob->GetStreams().size() - 1];
	if (m_vob->GetStreams().size() == 2 && stream->GetSourceAudioFormat() != m_dvd->GetAudioFormat())
		stream->SetDestinationFormat(m_dvd->GetAudioFormat());
	else if (stream->GetSourceAudioFormat() != afMP2 && stream->GetSourceAudioFormat() != afAC3)
		stream->SetDestinationFormat(m_dvd->GetAudioFormat());
	else if (stream->GetSourceSampleRate() != 48000)
		stream->SetDestinationFormat(stream->GetSourceAudioFormat());
	// add choice controls
	m_choiceList.insert(m_choiceList.begin() + m_subtitleChoiceIdx, NULL);
	m_choiceList.insert(m_choiceList.begin() + m_subtitleChoiceIdx, NULL);
	// update list box
	RefreshInfo();
	RefreshAll();
}

void VobListBox::AddSubtitle(wxString filename) {
	m_vob->AddSubtitlesFile(filename);
	// update list box
	RefreshInfo();
	RefreshAll();
}

void VobListBox::SetDoNotTranscode(bool value) {
	m_vob->SetDoNotTranscode(value);
	int choiceIdx = 0;
	for (unsigned int stIdx = 0; stIdx < m_vob->GetStreams().size(); stIdx++) {
		Stream* stream = m_vob->GetStreams()[stIdx];
		switch (stream->GetType()) {
		case stVIDEO:
			if (value && stream->GetVideoFormat() != vfNONE)
				m_choiceList[choiceIdx]->SetSelection(vfCOPY);
			choiceIdx++;
			break;
		case stAUDIO:
			if (value)
				m_choiceList[choiceIdx]->SetSelection(afCOPY);
			choiceIdx += 2;
			break;
		default:
			break;
		}
	}
}

/**
 * Returns true if "do not transcode active ist"
 */
bool VobListBox::GetDoNotTranscode() {
	return m_vob->GetDoNotTranscode();
}

bool VobListBox::HasAudioFiles() {
	return m_vob->GetAudioFilenames().size() > 0;
}

void VobListBox::SetValues() {
	unsigned int choiceIdx = 0;
	unsigned int audioIdx = 0;
	unsigned int subtitleIdx = 0;
	for (unsigned int stIdx = 0; stIdx < m_vob->GetStreams().size(); stIdx++) {
		Stream* stream = m_vob->GetStreams()[stIdx];
		switch (stream->GetType()) {
		case stVIDEO: {
			int vf = m_choiceList[choiceIdx++]->GetSelection();
			if (m_dvd->IsHD()) {
				if (stream->IsCopyPossible()) {
					if (vf > 1) {
						if (vf <= 7) {
							vf += vfPAL_HALF_HD - 2;
						} else
							vf -= 6;
					}
				} else {
					if (vf > 0) {
						if (vf <= 6) {
							vf += vfPAL_HALF_HD - 1;
						} else
							vf -= 5;
					}
				}
			} else if (vf != 0 && !stream->IsCopyPossible()) {
				vf += 1;
			}
			stream->SetDestinationFormat(vf);
			if (stream->GetVideoFormat() != vfCOPY)
				m_vob->SetDoNotTranscode(false);
			break;
		}
		case stAUDIO: {
			int af = m_choiceList[choiceIdx++]->GetSelection();
			if (af != 0 && !stream->IsCopyPossible())
				af += 1;
			stream->SetDestinationFormat(af);
			if (stream->GetAudioFormat() != afCOPY)
				m_vob->SetDoNotTranscode(false);
			if (stream->GetAudioFormat() != afNONE) {
				if (audioIdx >= m_pgcs->GetAudioLangCodes().size())
					m_pgcs->GetAudioLangCodes().push_back(m_choiceList[choiceIdx]->GetStringSelection());
				else
					m_pgcs->GetAudioLangCodes()[audioIdx] = m_choiceList[choiceIdx]->GetStringSelection();
				audioIdx++;
			}
			choiceIdx++;
			break;
		}
		case stSUBTITLE: {
			int sf = m_choiceList[choiceIdx++]->GetSelection();
			if (sf != 0 && !stream->IsCopyPossible())
				sf += 1;
			stream->SetDestinationFormat(sf);
			if (stream->GetSubtitleFormat() != sfCOPY)
				m_vob->SetDoNotTranscode(false);
			if (stream->GetSubtitleFormat() != sfNONE) {
				if (m_pgcs->GetSubPictures().size() <= subtitleIdx)
					m_pgcs->GetSubPictures().push_back(new SubPicture(m_choiceList[choiceIdx]->GetStringSelection()));
				else
					m_pgcs->GetSubPictures()[subtitleIdx]->SetLangCode(m_choiceList[choiceIdx]->GetStringSelection());
				subtitleIdx++;
			}
			choiceIdx++;
			break;
		}
		default:
			break;
		}
	}
	for (unsigned int si = 0; si < m_vob->GetSubtitles().size(); si++) {
		if (m_pgcs->GetSubPictures().size() <= subtitleIdx)
			m_pgcs->GetSubPictures().push_back(new SubPicture(m_choiceList[choiceIdx++]->GetStringSelection()));
		else
			m_pgcs->GetSubPictures()[subtitleIdx]->SetLangCode(m_choiceList[choiceIdx++]->GetStringSelection());
		subtitleIdx++;
	}
}

/** Returns index of choice control for given stream */
int VobListBox::GetChoiceIdx(unsigned int streamIdx) {
	int choiceIdx = 0;
	for (unsigned int stIdx = 0; stIdx < streamIdx; stIdx++) {
		Stream* stream = m_vob->GetStreams()[stIdx];
		switch (stream->GetType()) {
		case stVIDEO:
			choiceIdx++;
			break;
		case stAUDIO:
		case stSUBTITLE:
			choiceIdx += 2;
			break;
		default:
			break;
		}
	}
	return choiceIdx;
}

/** Returns index of stream for given choice control */
int VobListBox::GetStreamIdx(wxChoice* choice) {
	int choiceIdx = 0;
	for (unsigned int stIdx = 0; stIdx < m_vob->GetStreams().size(); stIdx++) {
		if ((int) m_choiceList.size() > choiceIdx && m_choiceList[choiceIdx] == choice)
			return stIdx;
		Stream* stream = m_vob->GetStreams()[stIdx];
		switch (stream->GetType()) {
		case stVIDEO:
			choiceIdx++;
			break;
		case stAUDIO:
		case stSUBTITLE:
			choiceIdx += 2;
			break;
		default:
			break;
		}
	}
	return -1;
}

/** Returns index of button for given stream */
int VobListBox::GetButtonIdx(unsigned int streamIdx) {
	for (unsigned int btIdx = 0; btIdx < m_buttonStreamList.size(); btIdx++) {
		if (m_buttonStreamList[btIdx] == (int) streamIdx) {
			return btIdx;
		}
	}
	return -1;
}

/** Get video format */
int VobListBox::GetVideoFormat(unsigned int streamIdx) {
	return m_choiceList[streamIdx]->GetSelection();
}

/** Gets video format index */
int VobListBox::GetVideoFormatIdx(Stream* stream, VideoFormat videoFormat) {
	bool copyEnabled = stream->IsCopyPossible();
	int vf = videoFormat;
	if (m_dvd->IsHD()) {
		if (vf >= vfPAL_HALF_HD) {
			vf = vf - vfPAL_HALF_HD + (copyEnabled ? 2 : 1);
		} else
			vf = vf >= vfPAL && vf < vfPAL_HALF_HD ? vf + (copyEnabled ? 6 : 5) : 1;
	} else if (!copyEnabled && vf > 0) {
		vf -= 1;
	}
	return vf;
}

/** Sets audio format */
void VobListBox::SetVideoFormat(unsigned int streamIdx, int videoFormat) {
	m_choiceList[GetChoiceIdx(streamIdx)]->SetSelection(videoFormat);
}

/** Get audio format */
int VobListBox::GetAudioFormat(unsigned int streamIdx) {
	return m_choiceList[GetChoiceIdx(streamIdx)]->GetSelection();
}

/** Sets audio format */
void VobListBox::SetAudioFormat(unsigned int streamIdx, int audioFormat) {
	m_choiceList[GetChoiceIdx(streamIdx)]->SetSelection(audioFormat);
}

/** Get audio language code */
wxString VobListBox::GetAudioLangCode(unsigned int streamIdx) {
	return m_choiceList[GetChoiceIdx(streamIdx) + 1]->GetStringSelection();
}

/** Sets audio language code */
void VobListBox::SetAudioLangCode(unsigned int streamIdx, wxString langCode) {
	m_choiceList[GetChoiceIdx(streamIdx) + 1]->SetSelection(DVD::GetAudioLanguageCodes().Index(langCode));
}

/**
 * Get subtitle language code (for property dialog, ignores muxed subtitles)
 */
wxString VobListBox::GetSubtitleLangCode(int subtitleIndex) {
	return m_choiceList[m_subtitleChoiceIdx + subtitleIndex]->GetStringSelection();
}

/**
 * Sets subtitle language code (for property dialog, ignores muxed subtitles)
 */
void VobListBox::SetSubtitleLangCode(int subtitleIndex, wxString langCode) {
	m_choiceList[m_subtitleChoiceIdx + subtitleIndex]->SetSelection(DVD::GetAudioLanguageCodes().Index(langCode));
}

/**
 * Shows subtitle properties dialog  
 */
void VobListBox::ShowPropDialog() {
	if (GetSelection() == 0) {
		SetValues(); // update destination format
		for (unsigned int i = 0; i < m_vob->GetStreams().size(); i++)
			if (m_vob->GetStreams()[i]->GetType() == stVIDEO
					&& m_vob->GetStreams()[i]->GetDestinationFormat() != vfNONE)
				ShowPropDialog(i);
	} else if (GetSelection() < 1 + (int) m_vob->GetAudioFilenames().GetCount()) {
		// audio
		int audioIdx = GetSelection() - 1;
		int streamIdx = m_vob->GetStreams().size() - m_vob->GetAudioFilenames().GetCount() + audioIdx;
		ShowPropDialog(streamIdx);
	} else {
		// subtitle
		int idx = GetSelection() - 1 - m_vob->GetAudioFilenames().GetCount();
		ShowPropDialog(m_vob->GetStreams().size() + idx);
	}
}

/** Shows subtitle properties dialog */
void VobListBox::ShowPropDialog(unsigned int streamIdx) {
	if (streamIdx >= m_vob->GetStreams().size()) {
		// subtitle
		unsigned int idx = streamIdx - m_vob->GetStreams().size();
		if (idx >= m_vob->GetSubtitles().size())
			return;
		TextSub* textsub = m_vob->GetSubtitles()[idx];
		
		if (!TextSub::IsFontMapInitialized()) {
			wxProgressDialog pDlg(wxT("DVDStyler"), _("Please wait..."), 99, this,
					wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_SMOOTH);
			pDlg.Show();
			pDlg.SetFocus();
			pDlg.Pulse();
			TextSub::GetFontMap();
			pDlg.Hide();
		}
		
		SubtitlePropDlg dialog(this, textsub, GetSubtitleLangCode(idx));
		if (dialog.ShowModal() == wxID_OK) {
			SetSubtitleLangCode(idx, dialog.GetLangCode());
		}
	} else {
		Stream* stream = m_vob->GetStreams()[streamIdx];
		if (stream->GetType() == stVIDEO) {
			// video
			SetValues(); // update destination format
			if (!m_titlePropDlg->ApplyChaptersCtrl())
				return;
			VideoPropDlg dialog(this, m_dvd, m_vob, stream, m_aspectRatio);
			if (dialog.ShowModal() == wxID_OK) {
				SetVideoFormat(streamIdx, GetVideoFormatIdx(stream, dialog.GetVideoFormat()));
				m_aspectRatio = dialog.GetAspectRatio();
				UpdateDoNotTranscodeCheck();
				m_titlePropDlg->UpdateChaptersCtrl();
			}
		} else if (stream->GetType() == stAUDIO) {
			// audio
			SetValues(); // update destination format
			int audioIdx = streamIdx - (m_vob->GetStreams().size() - m_vob->GetAudioFilenames().GetCount());
			wxString audioFile = audioIdx >= 0 ? m_vob->GetAudioFilenames()[audioIdx] : wxT("");
			AudioPropDlg dialog(this, m_vob, audioFile, GetAudioLangCode(streamIdx), streamIdx);
			if (dialog.ShowModal() == wxID_OK) {
				SetAudioFormat(streamIdx, dialog.GetAudioFormat());
				SetAudioLangCode(streamIdx, dialog.GetLangCode());
				UpdateDoNotTranscodeCheck();
			}
		}
	}
}

/** Processes a double click event */
void VobListBox::OnDoubleClick(wxMouseEvent& evt) {
	ShowPropDialog();
}

/** Processes a button event */
void VobListBox::OnButton(wxCommandEvent& event) {
	int buttonIdx = 0;
	for (vector<wxButton*>::iterator btIt = m_buttonList.begin(); btIt < m_buttonList.end(); btIt++) {
		if (*btIt == event.GetEventObject())
			break;
		buttonIdx++;
	}
	ShowPropDialog(m_buttonStreamList[buttonIdx]);
}

void VobListBox::UpdateDoNotTranscodeCheck() {
	if (m_vob->GetDoNotTranscode()) {
		SetValues(); // update destination format
		if (!m_vob->GetDoNotTranscode())
			m_titlePropDlg->UpdateDoNotTranscodeCheck();
	}
}

/** Processes a format change event */
void VobListBox::OnFormatChange(wxCommandEvent& event) {
	int stIdx = GetStreamIdx((wxChoice*) event.GetEventObject());
	if (stIdx >= 0 && m_vob->GetStreams()[stIdx]->GetType() == stVIDEO) {
		int buttonIdx = GetButtonIdx(stIdx);
		if (buttonIdx >= 0) {
			m_buttonList[buttonIdx]->Enable(event.GetInt() != 0);
		}
	}
	UpdateDoNotTranscodeCheck();
}
