/////////////////////////////////////////////////////////////////////////////
// Name:        Thumbnails.cpp
// Purpose:     wxThumbnails class
// Author:      Alex Thuering
// Created:		2.02.2003
// RCS-ID:      $Id: Thumbnails.cpp,v 1.44 2017/02/25 10:24:30 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "Thumbnails.h"
#include "ThumbnailFactory.h"
#include "ImageProc.h"
#include "utils.h"
#include <wx/filename.h>
#include <wx/dnd.h>
#include <wx/settings.h>

#include "rc/noread.png.h"
#include "rc/video.png.h"

/////////////////////////////// wxThumb //////////////////////////////////////

wxBitmap wxThumb::GetBitmap(wxThumbnails* parent, int width, int height) {
	if (!m_bitmap.Ok()) {
		wxImage img;
		if (!m_image.Ok()) {
			m_noOutline = false;
			img = wxThumbnailFactory::GetThumbnail(parent, m_filename, 128, 128, &m_noOutline);
		} else
			img = m_image;
		float scale = (float) width / img.GetWidth();
		if (scale > (float) height / img.GetHeight())
			scale = (float) height / img.GetHeight();
		m_bitmap = wxBitmap(img.Scale((int) (img.GetWidth() * scale), (int) (img.GetHeight() * scale)));
	}
	return m_bitmap;
}

void wxThumb::SetFilename(wxString fname) {
	m_filename = fname;
	m_bitmap = wxBitmap();
}

void wxThumb::SetCaption(wxString caption) {
	m_caption = caption;
	m_captionBreaks.Clear();
}

wxString wxThumb::GetCaption(unsigned int line) {
	if (line + 1 >= m_captionBreaks.GetCount())
		return wxEmptyString;
	return m_caption.Mid(m_captionBreaks[line], m_captionBreaks[line + 1] - m_captionBreaks[line]);
}

int wxThumb::GetCaptionLinesCount(int width) {
	BreakCaption(width);
	return m_captionBreaks.Count() - 1;
}

void wxThumb::BreakCaption(int width) {
	if (m_captionBreaks.Count() || width < 16)
		return;
	m_captionBreaks.Add(0);
	if (!m_caption.length())
		return;
	wxMemoryDC dc;
	wxBitmap bmp(10, 10);
	dc.SelectObject(bmp);
	dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	int sw, sh;
	unsigned int pos = width / 16;
	unsigned int beg = 0;
	unsigned int end = 0;
	while (1) {
		if (pos >= m_caption.length()) {
			m_captionBreaks.Add(m_caption.length());
			break;
		}
		dc.GetTextExtent(m_caption.Mid(beg, pos - beg), &sw, &sh);
		if (sw > width) {
			if (end > 0) {
				m_captionBreaks.Add(end);
				beg = end;
			} else {
				m_captionBreaks.Add(pos);
				beg = pos;
			}
			pos = beg + width / 16;
			end = 0;
		}
		if (pos < m_caption.length() && (m_caption[pos] == ' '
				|| m_caption[pos] == '-' || m_caption[pos] == ','
				|| m_caption[pos] == '.' || m_caption[pos] == '_'))
			end = pos + 1;
		pos++;
	}
}
///////////////////////////// wxThumbnails ///////////////////////////////////
BEGIN_EVENT_TABLE(wxThumbnails, wxScrolledWindow)
	EVT_PAINT(wxThumbnails::OnPaint) EVT_ERASE_BACKGROUND(wxThumbnails::OnEraseBackground)
	EVT_SIZE(wxThumbnails::OnResize)
	EVT_LEFT_DOWN(wxThumbnails::OnMouseDown)
	EVT_RIGHT_DOWN(wxThumbnails::OnMouseDown)
	EVT_LEFT_UP(wxThumbnails::OnMouseUp)
	EVT_RIGHT_UP(wxThumbnails::OnMouseUp)
	EVT_LEFT_DCLICK(wxThumbnails::OnMouseDClick)
	EVT_MOTION(wxThumbnails::OnMouseMove)
	EVT_LEAVE_WINDOW(wxThumbnails::OnMouseLeave)
	EVT_THUMBNAILS_THUMB_CHANGED(-1, wxThumbnails::OnThumbChanged)
	EVT_KEY_DOWN(wxThumbnails::OnKeyDown)
END_EVENT_TABLE()

DEFINE_EVENT_TYPE(EVT_COMMAND_THUMBNAILS_SEL_CHANGED)
DEFINE_EVENT_TYPE(EVT_COMMAND_THUMBNAILS_POINTED)
DEFINE_EVENT_TYPE(EVT_COMMAND_THUMBNAILS_DCLICK)
DEFINE_EVENT_TYPE(EVT_COMMAND_THUMBNAILS_CAPTION_CHANGED)
DEFINE_EVENT_TYPE(EVT_COMMAND_THUMBNAILS_THUMB_CHANGED)

wxThumbnails::wxThumbnails(wxWindow* parent, int id, long style):
		wxScrolledWindow(parent, id, wxDefaultPosition, wxDefaultSize, style) {
	SetThumbSize(96, 80, 8);
	m_tTextHeight = 16;
	m_tCaptionBorder = 8;
	m_tOutline = wxTHUMB_OUTLINE_IMAGE;
	m_tOutlineColour = *wxBLACK;
	m_tOutlineSelectedColour = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	m_tImageSelectedColour = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	m_filter = -1;
	m_selected = -1;
	m_pointed = -1;
	m_labelControl = NULL;
	m_pmenu = m_gpmenu = NULL;
	m_allowDragging = true;
	m_beginDrag = false;
	m_orient = wxTHUMB_VERTICAL;
	ShowFileNames(true);
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
	wxThumbnailFactory::Init();
}

wxThumbnails::~wxThumbnails() {
	WX_CLEAR_ARRAY(m_items);
}

void wxThumbnails::SetThumbSize(int width, int height, int border) {
	m_tWidth = width;
	m_tHeight = height;
	m_tBorder = border;
	SetScrollRate((m_tWidth + m_tBorder) / 4, (m_tHeight + m_tBorder) / 4);
	SetSizeHints(m_tWidth + m_tBorder * 2 + 16, m_tHeight + m_tBorder * 2);
}

void wxThumbnails::SetThumbImageSize(int width, int height, int border) {
	bool drawShadow = m_tOutline == wxTHUMB_OUTLINE_IMAGE;
	SetThumbSize(width + (m_tOutline != wxTHUMB_OUTLINE_NONE ? 4 : 0),
			height + (m_tOutline != wxTHUMB_OUTLINE_NONE ? 4 : 0) + (drawShadow ? 4 : 0) + 1, border);
}

void wxThumbnails::GetThumbSize(int& width, int& height, int& border) {
	width = m_tWidth;
	height = m_tHeight;
	border = m_tBorder;
}

int wxThumbnails::GetThumbImageWidth() {
	return m_tWidth - (m_tOutline != wxTHUMB_OUTLINE_NONE ? 4 : 0);
}

int wxThumbnails::GetThumbImageHeight() {
	bool drawShadow = m_tOutline == wxTHUMB_OUTLINE_IMAGE;
	return m_tHeight - (m_tOutline != wxTHUMB_OUTLINE_NONE ? 4 : 0) - (drawShadow ? 4 : 0) - 1;
}

void wxThumbnails::Clear() {
	WX_CLEAR_ARRAY(m_items);
	m_selectedArray.Clear();
	wxThumbnailFactory::ClearQueue(this);
	UpdateProp();
	Refresh();
}

void wxThumbnails::ShowDir(wxString dir, int filter) {
	m_dir = dir;
	if (filter >= 0)
		m_filter = filter;
	SetCaption(m_dir);
	// update items
	m_items.Clear();
	wxThumbnailFactory::ClearQueue(this);
	wxString fname = wxFindFirstFile(m_dir + wxFILE_SEP_PATH + _T("*"));
	while (!fname.IsEmpty()) {
		wxString caption;
		if (m_showFileNames)
			caption = fname.AfterLast(wxFILE_SEP_PATH);
		if (m_filter == -1 || ((m_filter & wxTHUMB_FILTER_VIDEOS) && IsVideo(fname))
				|| ((m_filter & wxTHUMB_FILTER_AUDIOS) && IsAudio(fname))
				|| ((m_filter & wxTHUMB_FILTER_AUDIOS) && IsSubtitle(fname))
				|| ((m_filter & wxTHUMB_FILTER_IMAGES)
						&& wxImage::FindHandler(fname.AfterLast('.').Lower(), BITMAP_TYPE_ANY))
				|| ((m_filter & wxTHUMB_FILTER_CUSTOM) && IsCustomFile(fname)))
			m_items.Add(new wxThumb(fname, caption));
		fname = wxFindNextFile();
	}
	SortItems();
	m_selectedArray.Clear();
	UpdateProp();
	Refresh();
}

void wxThumbnails::SetSelected(int value) {
	m_selected = value;
	m_selectedArray.Clear();
	if (value != -1)
		AddToSelection(value);
}

wxString wxThumbnails::GetAudioVideoExtWildcard() {
	return wxT("*.asf;*.avi;*.avs;*.dv;*.flv;*.m2p;*.m2ts;*.mkv;*.mov;*.mp4;*.mpeg;*.mpg;*.mts;*.ogg;*.ogm;*.vob;*.vro;*.webm;*.wmv;");
}

wxString wxThumbnails::GetVideoExtWildcard() {
	return GetAudioVideoExtWildcard() + wxT("*.m1v;*.m2v;*.mpv;*.m4v;");
}

wxString wxThumbnails::GetAudioExtWildcard() {
	return wxT("*.mpa;*.mp2;*.mp3;*.m4a;*.ac3;*.dts;*.pcm;*.lpcm;*.wav;");
}

wxString wxThumbnails::GetSubtitleExtWildcard() {
	return wxT("*.sub;*.srt;*.ssa;*.smi;*.rt;*.txt;*.aqt;*.jss;*.js;*.ass;");
}

bool wxThumbnails::IsAudioVideo(const wxString& fname) {
	return GetAudioVideoExtWildcard().Find(wxT(".") + fname.AfterLast('.').Lower() + wxT(";")) != wxNOT_FOUND;
}

bool wxThumbnails::IsVideo(const wxString& fname) {
	return GetVideoExtWildcard().Find(wxT(".") + fname.AfterLast('.').Lower() + wxT(";")) != wxNOT_FOUND;
}

bool wxThumbnails::IsAudio(const wxString& fname) {
	return GetAudioExtWildcard().Find(wxT(".") + fname.AfterLast('.').Lower() + wxT(";")) != wxNOT_FOUND;
}

bool wxThumbnails::IsSubtitle(const wxString& fname) {
	return GetSubtitleExtWildcard().Find(wxT(".") + fname.AfterLast('.').Lower() + wxT(";")) != wxNOT_FOUND;
}

void wxThumbnails::UpdateItems() {
	wxArrayInt selected = m_selectedArray;
	wxArrayString selectedFname;
	wxArrayLong selectedItemId;
	wxThumb thumb(_T(""));
	for (int i = 0; i < (int) m_selectedArray.Count(); i++) {
		selectedFname.Add(GetSelectedItem(i)->GetFilename());
		selectedItemId.Add(GetSelectedItem(i)->GetId());
	}
	UpdateItemsInt();
	if (selected.Count() > 0) {
		m_selectedArray.Clear();
		for (int i = 0; i < (int) m_items.GetCount(); i++) {
			for (int selIndex = 0; selIndex < (int) selected.Count(); selIndex++) {
				if (m_items[i]->GetFilename() == selectedFname[selIndex]
						&& m_items[i]->GetId() == selectedItemId[selIndex]) {
					m_selectedArray.Add(i);
					if (m_selectedArray.Count() == 1)
						ScrollToSelected();
				}
			}
		}
		if (m_selectedArray.Count() > 0) {
			Refresh();
			wxCommandEvent evt(EVT_COMMAND_THUMBNAILS_SEL_CHANGED, this->GetId());
			GetEventHandler()->ProcessEvent(evt);
		}
	}
}

void wxThumbnails::SetCaption(wxString caption) {
	m_caption = caption;
	if (m_labelControl) {
		int maxWidth = m_labelControl->GetSize().GetWidth() / 8;
		if ((int) caption.length() > maxWidth)
			caption = _T("...") + caption.Mid(caption.length() + 3 - maxWidth);
		m_labelControl->SetLabel(caption);
	}
	wxCommandEvent evt(EVT_COMMAND_THUMBNAILS_CAPTION_CHANGED, this->GetId());
	GetEventHandler()->ProcessEvent(evt);
}

void wxThumbnails::UpdateItemsInt() {
	if (m_dir.Length())
		ShowDir(m_dir);
}

int wxThumbnails::GetRowHeight(unsigned int begRow, unsigned int count) {
	// border
	bool drawShadow = m_tOutline == wxTHUMB_OUTLINE_IMAGE;
	int border = m_tBorder + (m_tOutline != wxTHUMB_OUTLINE_NONE ? 4 : 0) + (drawShadow ? 4 : 0) + 1;
	// image height
	int imgHeight = 0;
	for (unsigned int i = begRow; i < begRow + count; i++)
		if (i < m_tImageHeight.GetCount())
			imgHeight += m_tImageHeight[i];
	// caption height
	int capHeight = 0;
	for (unsigned int i = begRow; i < begRow + count; i++)
		if (i < m_tCaptionHeight.GetCount())
			capHeight += m_tCaptionHeight[i];
	return imgHeight + border*count + capHeight*m_tTextHeight;
}

int wxThumbnails::GetItemIndex(int x, int y) {
	int col = (x - m_tBorder) / (m_tWidth + m_tBorder);
	if (col >= m_cols)
		col = m_cols - 1;
	int row = -1;
	y -= m_tBorder;
	while (y > 0) {
		row++;
		y -= GetRowHeight(row);
	}
	if (row < 0)
		row = 0;
	int index = row * m_cols + col;
	if (index >= (int) m_items.GetCount())
		index = -1;
	return index;
}

void wxThumbnails::UpdateProp(bool checkSize) {
	int width = GetClientSize().GetWidth();
	m_cols = (width - m_tBorder) / (m_tWidth + m_tBorder);
	if (m_cols == 0)
		m_cols = 1;
	m_rows = m_items.GetCount() / m_cols + (m_items.GetCount() % m_cols ? 1 : 0);
	// calculate maximal height of images in each row
	m_tImageHeight.Clear();
	for (int row = 0; row < m_rows; row++) {
		int maxHeight = 0;
		for (int col = 0; col < m_cols; col++) {
			unsigned int i = row * m_cols + col;
			if (m_items.GetCount() > i && GetThumbImage(*m_items[i]).GetHeight() > maxHeight)
				maxHeight = GetThumbImage(*m_items[i]).GetHeight();
		}
		m_tImageHeight.Add(maxHeight);
	}
	// calculate maximal height of captions in each row
	m_tCaptionHeight.Clear();
	for (int row = 0; row < m_rows; row++) {
		int maxHeight = 0;
		for (int col = 0; col < m_cols; col++) {
			unsigned int i = row * m_cols + col;
			if (m_items.GetCount() > i && m_items[i]->GetCaptionLinesCount(m_tWidth - m_tCaptionBorder) > maxHeight)
				maxHeight = m_items[i]->GetCaptionLinesCount(m_tWidth - m_tCaptionBorder);
		}
		m_tCaptionHeight.Add(maxHeight);
	}
	SetVirtualSize(m_cols * (m_tWidth + m_tBorder) + m_tBorder, GetRowHeight(0, m_rows) + m_tBorder);
	SetSizeHints(m_tWidth + m_tBorder * 2 + 16, m_tBorder + 8 + (m_rows ? GetRowHeight(0) : m_tHeight + m_tBorder));
	if (checkSize && width != GetClientSize().GetWidth())
		UpdateProp(false);
}

void wxThumbnails::InsertItem(wxThumb* thumb, int pos) {
	if (pos < 0 || pos > (int) m_items.GetCount())
		m_items.Add(thumb);
	else
		m_items.Insert(thumb, pos);
	UpdateProp();
}

void wxThumbnails::RemoveItemAt(int pos, int count) {
	for (int i = 0; i < count; i++)
		if (i < (int) m_items.GetCount()) {
			delete m_items[pos + i];
		}
	m_items.RemoveAt(pos, count);
	UpdateProp();
}

int cmpthumb(wxThumb** first, wxThumb** second) {
  return (*first)->GetFilename().CmpNoCase((*second)->GetFilename());
}

void wxThumbnails::SortItems() {
	m_items.Sort(cmpthumb);
}

wxRect wxThumbnails::GetPaintRect() {
	wxSize size = GetClientSize();
	wxRect paintRect(0, 0, size.GetWidth(), size.GetHeight());
	GetViewStart(&paintRect.x, &paintRect.y);
	int xu, yu;
	GetScrollPixelsPerUnit(&xu, &yu);
	paintRect.x *= xu;
	paintRect.y *= yu;
	return paintRect;
}

void wxThumbnails::ScrollToSelected() {
	if (GetSelected() == -1)
		return;
	// get row
	int row = GetSelected() / m_cols;
	// calc position to scroll view
	int sy; // scroll to y
	wxRect paintRect = GetPaintRect();
	int y1 = GetRowHeight(0, row);
	int y2 = y1 + m_tBorder + GetRowHeight(row);
	if (y1 < paintRect.GetTop())
		sy = y1; // scroll top
	else if (y2 > paintRect.GetBottom())
		sy = y2 - paintRect.height; // scroll bottom
	else
		return;
	// scroll view
	int xu, yu;
	GetScrollPixelsPerUnit(&xu, &yu);
	sy = sy / yu + (sy % yu ? 1 : 0); // convert sy to scroll units
	int x, y;
	GetViewStart(&x, &y);
	Scroll(x, sy);
}

wxBitmap wxThumbnails::GetThumbImage(wxThumb& thumb) {
	return thumb.GetBitmap(this, GetThumbImageWidth(), GetThumbImageHeight());
}

void wxThumbnails::DrawThumbnail(wxBitmap& bmp, wxThumb& thumb, int index) {
	wxMemoryDC dc;
	dc.SelectObject(bmp);
	dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	int x = m_tBorder / 2;
	int y = m_tBorder / 2;
	// background
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
	dc.DrawRectangle(0, 0, bmp.GetWidth(), bmp.GetHeight());
	// size
	bool drawShadow = m_tOutline == wxTHUMB_OUTLINE_IMAGE;
	int imgWidth = GetThumbImageWidth();
	int imgHeight = m_tImageHeight[index / m_cols];
	int tHeight = imgHeight + (m_tOutline != wxTHUMB_OUTLINE_NONE ? 4 : 0) + (drawShadow ? 4 : 0) + 1;
	// image
	wxBitmap img = GetThumbImage(thumb);
	if (IsSelected(index) && m_tImageSelectedColour.IsOk() && (m_tOutline == wxTHUMB_OUTLINE_NONE
			|| (m_tOutline == wxTHUMB_OUTLINE_IMAGE && thumb.IsNoOutline()))) {
		wxImage imgTmp = img.ConvertToImage();
		wxAdjustToColour(imgTmp, m_tImageSelectedColour);
		img = wxBitmap(imgTmp);
	}
	if (index == m_pointed) {
		wxImage imgTmp = img.ConvertToImage();
		wxAdjustBrightness(imgTmp, POINTED_BRIGHTNESS);
		img = wxBitmap(imgTmp);
	}
	wxRect imgRect(x + (imgWidth - img.GetWidth()) / 2, y + (imgHeight - img.GetHeight()) / 2,
			img.GetWidth(), img.GetHeight());
	dc.DrawBitmap(img, imgRect.x, imgRect.y, true);
	// outline
	if (m_tOutline != wxTHUMB_OUTLINE_NONE && (m_tOutline != wxTHUMB_OUTLINE_IMAGE || !thumb.IsNoOutline())) {
		if (m_tOutline == wxTHUMB_OUTLINE_FULL || m_tOutline == wxTHUMB_OUTLINE_RECT) {
			imgRect.x = x;
			imgRect.y = y;
			imgRect.width = bmp.GetWidth() - m_tBorder;
			imgRect.height = bmp.GetHeight() - m_tBorder;
			if (m_tOutline == wxTHUMB_OUTLINE_RECT)
				imgRect.height = tHeight - (drawShadow ? 4 : 0);
		}
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		bool selOutline = IsSelected(index) && m_tOutlineSelectedColour.IsOk();
		dc.SetPen(selOutline ? wxPen(m_tOutlineSelectedColour, 0, wxPENSTYLE_SOLID) : *wxLIGHT_GREY_PEN);
		dc.DrawRectangle(imgRect.x - 1, imgRect.y - 1, imgRect.width + 2, imgRect.height + 2);
		dc.SetPen(wxPen(selOutline ? m_tOutlineSelectedColour : m_tOutlineColour, 0, wxPENSTYLE_SOLID));
		dc.DrawRectangle(imgRect.x - 2, imgRect.y - 2, imgRect.width + 4, imgRect.height + 4);
		
		if (drawShadow) {
			int y1 = imgRect.y + imgRect.height + 2;
			dc.SetPen(wxPen(wxColour(0xf2, 0xf2, 0xf2)));
			dc.DrawRectangle(imgRect.x - 3, y1, imgRect.width + 6, 4);
			dc.SetPen(wxPen(wxColour(0xfb, 0xfb, 0xfb)));
			dc.DrawPoint(imgRect.x - 3, y1 + 3); dc.DrawPoint(imgRect.x + imgRect.width + 2, y1 + 3);
			dc.SetPen(wxPen(wxColour(0xf6, 0xf6, 0xf6)));
			dc.DrawPoint(imgRect.x - 2, y1 + 3); dc.DrawPoint(imgRect.x + imgRect.width + 1, y1 + 3);
			
			dc.SetPen(wxPen(wxColour(0xe3, 0xe3, 0xe3)));
			dc.DrawRectangle(imgRect.x - 2, y1, imgRect.width + 4, 3);
			dc.SetPen(wxPen(wxColour(0xf6, 0xf6, 0xf6)));
			dc.DrawPoint(imgRect.x - 3, y1 + 2); dc.DrawPoint(imgRect.x + imgRect.width + 2, y1 + 2);
			dc.SetPen(wxPen(wxColour(0xeb, 0xeb, 0xeb)));
			dc.DrawPoint(imgRect.x - 2, y1 + 2); dc.DrawPoint(imgRect.x + imgRect.width + 1, y1 + 2);
			
			dc.SetPen(wxPen(wxColour(0xba, 0xba, 0xba)));
			dc.DrawLine(imgRect.x - 1, y1, imgRect.x + imgRect.width + 1, y1);
			
			dc.SetPen(wxPen(wxColour(0xcd, 0xcd, 0xcd)));
			dc.DrawLine(imgRect.x - 1, y1 + 1, imgRect.x + imgRect.width + 1, y1 + 1);
		}
	}
	// draw caption
	int textWidth = 0;
	for (int i = 0; i < thumb.GetCaptionLinesCount(m_tWidth - m_tCaptionBorder); i++) {
		wxString caption = thumb.GetCaption(i);
		int sw, sh;
		dc.GetTextExtent(caption, &sw, &sh);
		if (sw > textWidth)
			textWidth = sw;
	}
	textWidth += 8;
	int tx = x + (m_tWidth - textWidth) / 2;
	int ty = y + tHeight;
	if (IsSelected(index) && thumb.GetCaption().length()) {
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT), wxBRUSHSTYLE_SOLID));
		dc.DrawRoundedRectangle(tx, ty, textWidth,
				thumb.GetCaptionLinesCount(m_tWidth - m_tCaptionBorder) * m_tTextHeight, 4);
		dc.SetTextForeground(*wxWHITE);
	}
	for (int i = 0; i < thumb.GetCaptionLinesCount(m_tWidth - m_tCaptionBorder); i++) {
		wxString caption = thumb.GetCaption(i);
		int sw, sh;
		dc.GetTextExtent(caption, &sw, &sh);
		int tx = x + (m_tWidth - sw) / 2;
		int ty = y + tHeight + i * m_tTextHeight + (m_tTextHeight - sh) / 2;
		dc.DrawText(caption, tx, ty);
	}
	dc.SelectObject(wxNullBitmap);
}

void wxThumbnails::OnPaint(wxPaintEvent& WXUNUSED(event)) {
	wxPaintDC dc(this);
	PrepareDC(dc);
	Paint(dc);
}

void wxThumbnails::Paint(wxPaintDC& dc) {
	wxRect paintRect = GetPaintRect();
	
	// items
	int row = -1;
	for (int i = 0; i < (int) m_items.GetCount(); i++) {
		int col = i % m_cols;
		if (col == 0)
			row++;
		int tx = m_tBorder / 2 + col * (m_tWidth + m_tBorder);
		int ty = m_tBorder / 2 + GetRowHeight(0, row);
		int tw = m_tWidth + m_tBorder;
		int th = GetRowHeight(row);
		// visible?
		if (!paintRect.Intersects(wxRect(tx, ty, tw, th)))
			continue;
		wxBitmap thmb(tw, th);
		DrawThumbnail(thmb, *m_items[i], i);
		dc.DrawBitmap(thmb, tx, ty);
	}

	// background
	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
	wxRect rect(m_tBorder / 2, m_tBorder / 2, m_cols * (m_tWidth + m_tBorder), GetRowHeight(0, m_rows));
	int w = wxMax(GetClientSize().GetWidth(), rect.width);
	int h = wxMax(GetClientSize().GetHeight(), rect.height);
	dc.DrawRectangle(0, 0, w, rect.y);
	dc.DrawRectangle(0, 0, rect.x, h);
	dc.DrawRectangle(rect.GetRight(), 0, w - rect.GetRight(), h + 50);
	dc.DrawRectangle(0, rect.GetBottom(), w, h - rect.GetBottom() + 50);
	int col = m_items.GetCount() % m_cols;
	if (col > 0) {
		rect.x += col * (m_tWidth + m_tBorder);
		rect.y += GetRowHeight(0, m_rows - 1);
		dc.DrawRectangle(rect);
	}
}

void wxThumbnails::OnResize(wxSizeEvent &event) {
	UpdateProp();
	ScrollToSelected();
	Refresh();
}

void wxThumbnails::OnMouseDown(wxMouseEvent &event) {
	// get item number to select
	int x = event.GetX();
	int y = event.GetY();
	CalcUnscrolledPosition(x, y, &x, &y);
	int lastSelected = m_selected;
	m_selected = GetItemIndex(x, y);

	// set new selection
	m_mouseEventHandled = false;
	bool update = false;
	if (event.ControlDown()) {
		if (m_selected == -1)
			m_mouseEventHandled = true;
		else if (!IsSelected(m_selected)) {
			m_selectedArray.Add(m_selected);
			update = true;
			m_mouseEventHandled = true;
		}
	} else if (event.ShiftDown()) {
		if (m_selected != -1) {
			int begIndex = m_selected;
			int endIndex = lastSelected;
			if (lastSelected < m_selected) {
				begIndex = lastSelected;
				endIndex = m_selected;
			}
			m_selectedArray.Clear();
			for (int i = begIndex; i <= endIndex; i++)
				m_selectedArray.Add(i);
			update = true;
		}
		m_selected = lastSelected;
		m_mouseEventHandled = true;
	} else {
		if (m_selected == -1) {
			update = m_selectedArray.Count() > 0;
			m_selectedArray.Clear();
			m_mouseEventHandled = true;
		} else if (m_selectedArray.Count() <= 1) {
			update = m_selectedArray.Count() == 0 || m_selectedArray[0] != m_selected;
			m_selectedArray.Clear();
			m_selectedArray.Add(m_selected);
			m_mouseEventHandled = true;
		}
	}

	if (update) {
		ScrollToSelected();
		Refresh();
		wxCommandEvent evt(EVT_COMMAND_THUMBNAILS_SEL_CHANGED, this->GetId());
		GetEventHandler()->ProcessEvent(evt);
	}

	m_beginDrag = m_selected >= 0;

	SetFocus();
}

void wxThumbnails::OnMouseUp(wxMouseEvent &event) {
	m_beginDrag = false;
	// get item number to select
	int x = event.GetX();
	int y = event.GetY();
	CalcUnscrolledPosition(x, y, &x, &y);
	int lastSelected = m_selected;
	m_selected = GetItemIndex(x, y);

	if (!m_mouseEventHandled) {
		// set new selection
		if (event.ControlDown()) {
			m_selectedArray.Remove(m_selected);
			m_selected = -1;
		} else {
			m_selectedArray.Clear();
			m_selectedArray.Add(m_selected);
		}

		ScrollToSelected();
		Refresh();
		wxCommandEvent evt(EVT_COMMAND_THUMBNAILS_SEL_CHANGED, this->GetId());
		GetEventHandler()->ProcessEvent(evt);
	}

	// Popup menu
	if (event.RightUp()) {
		if (m_selected >= 0 && m_pmenu)
			PopupMenu(m_pmenu, event.GetPosition());
		else if (m_selected == -1 && m_gpmenu)
			PopupMenu(m_gpmenu, event.GetPosition());
	}

	if (event.ShiftDown())
		m_selected = lastSelected;
}

void wxThumbnails::OnMouseDClick(wxMouseEvent &event) {
	wxCommandEvent evt(EVT_COMMAND_THUMBNAILS_DCLICK, this->GetId());
	GetEventHandler()->ProcessEvent(evt);
}

void wxThumbnails::OnMouseMove(wxMouseEvent &event) {
	// -- drag & drop --
	if (m_allowDragging && event.Dragging() && m_selectedArray.Count() > 0 && m_beginDrag) {
		wxFileDataObject files;
		for (int i = 0; i < (int) m_selectedArray.Count(); i++)
			files.AddFile(GetSelectedItem(i)->GetFilename());
		wxDropSource source(files, this);
		source.DoDragDrop(wxDrag_DefaultMove);
		m_beginDrag = false;
	}

	// -- light-effect --
	int x = event.GetX();
	int y = event.GetY();
	CalcUnscrolledPosition(x, y, &x, &y);
	// get item number
	int sel = GetItemIndex(x, y);
	if (sel == m_pointed)
		return;
	// update thumbnail
	m_pointed = sel;
	Refresh();
	wxCommandEvent evt(EVT_COMMAND_THUMBNAILS_POINTED, this->GetId());
	GetEventHandler()->ProcessEvent(evt);
}

void wxThumbnails::OnMouseLeave(wxMouseEvent &event) {
	if (m_pointed != -1) {
		m_pointed = -1;
		Refresh();
		wxCommandEvent evt(EVT_COMMAND_THUMBNAILS_POINTED, this->GetId());
		GetEventHandler()->ProcessEvent(evt);
	}
}

void wxThumbnails::OnThumbChanged(wxCommandEvent &event) {
	wxImage* img = NULL;
	for (int i = 0; i < (int) m_items.GetCount(); i++) {
		if (m_items[i]->GetFilename() == event.GetString()) {
			m_items[i]->SetFilename(m_items[i]->GetFilename());
			if (event.GetClientData()) {
				wxImage* img = (wxImage*) event.GetClientData();
				m_items[i]->SetImage(*img);
				m_items[i]->SetNoOutline(false);
			}
		}
	}
	if (img)
		delete img;
	UpdateProp();
	Refresh();
}

void wxThumbnails::OnKeyDown(wxKeyEvent& event) {
	int oldSelected = m_selected;
	switch (event.GetKeyCode()) {
	case WXK_LEFT:
		if (m_selected > 0)
			m_selected--;
		break;
	case WXK_RIGHT:
		if (m_selected < (int) m_items.GetCount() - 1)
			m_selected++;
		break;
	case WXK_UP:
		if (m_selected >= m_cols)
			m_selected -= m_cols;
		break;
	case WXK_DOWN:
		if (m_selected + m_cols <= (int) m_items.GetCount() - 1)
			m_selected += m_cols;
		break;
	case WXK_F5:
		UpdateItems();
		break;
	case 'A':
		if (event.ControlDown()) {
			// select all
			m_selectedArray.Clear();
			for (unsigned int i = 0; i < m_items.GetCount() - 1; i++) {
				m_selectedArray.Add(i);
			}
			Refresh();
			wxCommandEvent evt(EVT_COMMAND_THUMBNAILS_SEL_CHANGED, this->GetId());
			GetEventHandler()->ProcessEvent(evt);
			return;
		}
		break;
	default:
		event.Skip();
		break;
	}
	if (m_selected != oldSelected) {
		m_selectedArray.Clear();
		m_selectedArray.Add(m_selected);
		ScrollToSelected();
		Refresh();
		wxCommandEvent evt(EVT_COMMAND_THUMBNAILS_SEL_CHANGED, this->GetId());
		GetEventHandler()->ProcessEvent(evt);
	}
}
