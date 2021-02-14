/////////////////////////////////////////////////////////////////////////////
// Name:        ThumbnailFactory.cpp
// Purpose:     wxThumbnailFactory class
// Author:      Alex Thuering
// Created:		15.02.2003
// RCS-ID:      $Id: ThumbnailFactory.cpp,v 1.39 2016/10/23 19:02:41 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "ThumbnailFactory.h"
#include "Thumbnails.h"
#include "imagjpg.h"
#include <wx/dir.h>
#include <wx/app.h>
#include <wx/wx.h>
#include <wx/filename.h>
#include <sys/stat.h>
#include "utils.h"

#ifdef GNOME2
#include <glib.h>
#include <libgnomevfs/gnome-vfs.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdkdrawable.h>
#include <libgnomeui/gnome-ui-init.h>
#include <libgnomeui/gnome-thumbnail.h>
static GnomeThumbnailFactory* thumbnail_factory = NULL;
#elif defined(GNOME3)
  #define GNOME_DESKTOP_USE_UNSTABLE_API
  #include <libgnome-desktop/gnome-desktop-thumbnail.h>
  static GnomeDesktopThumbnailFactory* thumbnail_factory = NULL;
  #define gnome_thumbnail_factory_lookup gnome_desktop_thumbnail_factory_lookup
  #define gnome_thumbnail_path_for_uri gnome_desktop_thumbnail_path_for_uri
  #define gnome_thumbnail_is_valid gnome_desktop_thumbnail_is_valid
  #define gnome_thumbnail_factory_can_thumbnail gnome_desktop_thumbnail_factory_can_thumbnail
  #define gnome_thumbnail_factory_generate_thumbnail gnome_desktop_thumbnail_factory_generate_thumbnail
  #define gnome_thumbnail_factory_save_thumbnail gnome_desktop_thumbnail_factory_save_thumbnail
  #define gnome_thumbnail_factory_create_failed_thumbnail gnome_desktop_thumbnail_factory_create_failed_thumbnail
  #define GNOME_THUMBNAIL_SIZE_NORMAL GNOME_DESKTOP_THUMBNAIL_SIZE_NORMAL
#endif
#include <wxSVG/mediadec_ffmpeg.h>
#include <wxSVG/ExifHandler.h>
#define THUMBNAILS_DIR wxGetHomeDir() + wxFILE_SEP_PATH + _T(".thumb")

#include "rc/loading.png.h"
#include "rc/regular.png.h"
#include "rc/video.png.h"
#include "rc/audio.png.h"
#include "rc/subtitle.png.h"

class ThumbInfo {
public:
	ThumbInfo(wxString filename, wxWindow* parent, int width, int height) {
		this->filename = filename;
		this->parent = parent;
		this->width = width;
		this->height = height;
#if defined(GNOME2) || defined(GNOME3)
		pixbuf = NULL;
#endif
	}

	~ThumbInfo() {
#if defined(GNOME2) || defined(GNOME3)
		if (pixbuf)
			g_object_unref(pixbuf);
#endif
	}

	wxString filename;
	wxWindow* parent;
	int width;
	int height;
	wxString mimeType;
	wxString uri;
	wxDateTime mtime;
#if defined(GNOME2) || defined(GNOME3)
	GdkPixbuf* pixbuf;
#endif
};

ThumbInfoArray wxThumbnailFactory::m_queue;
wxMutex wxThumbnailFactory::m_queueMutex;
wxThumbnailFactory* wxThumbnailFactory::thread = NULL;
int wxThumbnailFactory::maxFileSize = 102400; // immediately render files with size < 100K

void wxThumbnailFactory::Init() {
#if defined(GNOME2) || defined(GNOME3)
	wxLogNull log;
	wxMkdir(THUMBNAILS_DIR);
	wxMkdir(THUMBNAILS_DIR + wxFILE_SEP_PATH + wxT("normal"));
	wxMkdir(THUMBNAILS_DIR + wxFILE_SEP_PATH + wxT("fail"));
#endif
}

void wxThumbnailFactory::InitGnome(const char* appName, const char* appVersion, int argc, char** argv) {
#if defined(GNOME2)
	gnome_init(appName, appVersion, argc, argv);
	gnome_vfs_init();
	thumbnail_factory
			= gnome_thumbnail_factory_new(GNOME_THUMBNAIL_SIZE_NORMAL);
#elif defined(GNOME3)
	thumbnail_factory = gnome_desktop_thumbnail_factory_new(GNOME_THUMBNAIL_SIZE_NORMAL);
#endif
}


/**
 * Removes all thumbnails files from cache directory
 */
void wxThumbnailFactory::ClearCacheDir() {
#ifndef GNOME2
	if (wxDirExists(THUMBNAILS_DIR + wxFILE_SEP_PATH + wxT("normal"))) {
		wxArrayString files;
		wxDir::GetAllFiles(THUMBNAILS_DIR + wxFILE_SEP_PATH + wxT("normal"), &files, wxEmptyString,
				wxDIR_FILES | wxDIR_HIDDEN);
		while (files.size() > 0) {
			wxRemoveFile(files.back());
			files.pop_back();
		}
		wxRmdir(THUMBNAILS_DIR + wxFILE_SEP_PATH + wxT("normal"));
	}
	if (wxDirExists(THUMBNAILS_DIR + wxFILE_SEP_PATH + wxT("fail"))) {
		wxArrayString files;
		wxDir::GetAllFiles(THUMBNAILS_DIR + wxFILE_SEP_PATH + wxT("fail"), &files, wxEmptyString,
				wxDIR_FILES | wxDIR_HIDDEN);
		while (files.size() > 0) {
			wxRemoveFile(files.back());
			files.pop_back();
		}
		wxRmdir(THUMBNAILS_DIR + wxFILE_SEP_PATH + wxT("fail"));
	}
	if (wxDirExists(THUMBNAILS_DIR))
		wxRmdir(THUMBNAILS_DIR);
#endif
}

wxImage wxThumbnailFactory::GetThumbnail(wxWindow* parent, wxString filename, int width, int height, bool* noOutline) {
	wxImage img;

	if (!filename.length())
		return wxBITMAP_FROM_MEMORY(regular).ConvertToImage();

	ThumbInfo* info = GetThumbInfo(filename, parent, width, height);
	img = LoadThumbnail(*info); // load thumbnail, if it was already generated

	if (!img.Ok()) { // add thumbnail to queue => generate
		if (wxThumbnails::IsAudio(filename))
			img = wxBITMAP_FROM_MEMORY(audio).ConvertToImage();
		else if (wxThumbnails::IsSubtitle(filename))
			img = wxBITMAP_FROM_MEMORY(subtitle).ConvertToImage();
		else if (CanThumbnail(*info))
			img = AddToQueue(info);
		else {
			if (wxThumbnails::IsVideo(filename))
				img = wxBITMAP_FROM_MEMORY(video).ConvertToImage();
			else
				img = wxBITMAP_FROM_MEMORY(regular).ConvertToImage();
		}
		if (noOutline != NULL)
			*noOutline = true;
	} else {
		delete info;
		if (noOutline != NULL)
			*noOutline = false;
	}
	return img;
}

wxThread::ExitCode wxThumbnailFactory::Entry() {
	while (1) {
		m_queueMutex.Lock();
		if (!m_queue.Count())
			break;
		ThumbInfo* info = m_queue.Item(0);
		m_queue.RemoveAt(0);
		m_queueMutex.Unlock();

		wxImage img = LoadThumbnail(*info);
		if (!img.Ok())
			img = GenerateThumbnail(*info);
		wxCommandEvent evt(EVT_COMMAND_THUMBNAILS_THUMB_CHANGED);
		evt.SetString(info->filename);
		evt.SetClientData(new wxImage(img));
		wxPostEvent(info->parent, evt);
		delete info;
	}
	thread = NULL;
	m_queueMutex.Unlock();
	return 0;
}

void wxThumbnailFactory::ClearQueue(wxWindow* parent) {
	wxMutexLocker locker(m_queueMutex);
	for (int i = 0; i < (int) m_queue.Count();) {
		if (m_queue[i]->parent == parent) {
			delete m_queue[i];
			m_queue.RemoveAt(i);
		} else
			i++;
	}
}

//////////////////////////////////////////////////////////////////////////////

wxImage wxThumbnailFactory::AddToQueue(ThumbInfo* info) {
	wxImage img = wxBITMAP_FROM_MEMORY(loading).ConvertToImage();
	wxMutexLocker locker(m_queueMutex);
	for (int i = 0; i < (int) m_queue.Count(); i++)
		if (m_queue[i]->filename == info->filename && m_queue[i]->parent == info->parent)
			return img;
	m_queue.Add(info);
	if (!thread) { // start thread
		thread = new wxThumbnailFactory;
		thread->Create();
		thread->Run();
	}
	return img;
}

#if defined(GNOME2) || defined(GNOME3)
/*wxImage pixbuf2image(GdkPixbuf* pixbuf) {
	 wxBitmap bitmap(gdk_pixbuf_get_width(pixbuf), gdk_pixbuf_get_height(pixbuf));
	 GdkBitmap* gbitmap = bitmap.GetPixmap();
	 gdk_draw_pixbuf(gbitmap, NULL, pixbuf, 0,0,0,0, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
	 return bitmap.ConvertToImage();
}*/
#else
#include "thumb_md5.cpp"
#endif

ThumbInfo* wxThumbnailFactory::GetThumbInfo(wxString filename, wxWindow* parent, int width, int height) {
	ThumbInfo* info = new ThumbInfo(filename, parent, width, height);
	if (filename.StartsWith(wxT("concat:")))
		info->mimeType = _T("concat");
	else if (wxImage::FindHandler(filename.AfterLast('.').Lower(), wxBITMAP_TYPE_ANY))
		info->mimeType = _T("image");
	else if (wxThumbnails::IsVideo(filename))
		info->mimeType = _T("video/mpeg");
	else
		info->mimeType = _T("unknown");
	info->uri = info->mimeType == _T("concat") ? _T("concat://") : _T("file://");
	info->uri += filename;
	wxString fname = filename;
	if (info->mimeType == _T("concat"))
		fname = filename.Mid(7).BeforeFirst(wxT('|'));
	info->mtime = wxFileName(fname).GetModificationTime();
	return info;
}

wxString wxThumbnailFactory::GetThumbPath(ThumbInfo& info, ThumbType type) {
#if defined(GNOME2) || defined(GNOME3)
	char* thumbnail_path = NULL;
	if (info.uri.length()) {
		thumbnail_path = gnome_thumbnail_factory_lookup(thumbnail_factory, info.uri.c_str(), info.mtime.GetTicks());
		if (thumbnail_path == NULL)
			thumbnail_path = gnome_thumbnail_path_for_uri(info.uri, GNOME_THUMBNAIL_SIZE_NORMAL);
	}
	if (!thumbnail_path || thumbnail_path[0] != '/')
		return wxEmptyString;
	wxString thumbPath = wxString(thumbnail_path, *wxConvCurrent);
	if (type == THUMBNAIL_FAILED) {
		int idx = thumbPath.Find(wxString(wxFILE_SEP_PATH) + wxT("normal") + wxString(wxFILE_SEP_PATH));
		if (idx > 0)
			thumbPath = thumbPath.Mid(0, idx + 1) + wxT("fail") + thumbPath.Mid(idx + 7);
		else
			thumbPath = wxEmptyString;
	}
	return thumbPath;
#else
	unsigned char digest[16];
	thumb_md5(info.uri.utf8_str(), digest);
	wxString md5 = wxString(thumb_digest_to_ascii(digest), *wxConvCurrent);
	wxString file = md5 + _T(".png");
	wxString dir = THUMBNAILS_DIR;
	dir += wxFILE_SEP_PATH;
	dir += type == THUMBNAIL_FAILED ? _T("fail") : _T("normal");
	return dir + wxFILE_SEP_PATH + file;
#endif
}

wxImage wxThumbnailFactory::LoadThumbnail(ThumbInfo& info, ThumbType type) {
	wxImage img;

	if (info.mimeType == _T("image")) {
		wxStructStat fInfo;
		if (wxStat(info.filename, &fInfo) == 0 && fInfo.st_size < maxFileSize) {
			img = GenerateThumbnail(info, false);
			return img;
		}
	}

	// load thumbnail for video
	wxString thumbPath = GetThumbPath(info, type);
#if defined(GNOME2) || defined(GNOME3)
	GdkPixbuf* pixbuf = NULL;
	if (thumbPath.Length())
		pixbuf = gdk_pixbuf_new_from_file(thumbPath.mb_str(), NULL);
	if (pixbuf) {
		if (gnome_thumbnail_is_valid(pixbuf, info.uri.c_str(), info.mtime.GetTicks())) {
			wxLogNull log;
			if (wxFileExists(thumbPath))
				img.LoadFile(thumbPath); // img = pixbuf2image(pixbuf); failed on some systems
		}
		g_object_unref(pixbuf);
	}
#else
	if (wxFileExists(thumbPath) && wxFileName(thumbPath).GetModificationTime().IsLaterThan(info.mtime))
		img.LoadFile(thumbPath);
#endif
	return img;
}

bool wxThumbnailFactory::CanThumbnail(ThumbInfo& info) {
	if (info.mimeType == _T("unknown"))
		return false;
#if defined(GNOME2) || defined(GNOME3)
	if (info.mimeType != _T("image") && info.mimeType != _T("concat") && !info.mimeType.StartsWith(wxT("video/")))
		return info.uri.length() > 0
			&& gnome_thumbnail_factory_can_thumbnail(thumbnail_factory,
					info.uri.c_str(), info.mimeType.c_str(), info.mtime.GetTicks());
#endif
	// check if we already tried to generate this thumbnail and it fails
	wxString thumbPath = GetThumbPath(info, THUMBNAIL_FAILED);
	if (wxFileExists(thumbPath)) {
		if (wxFileName(thumbPath).GetModificationTime().IsLaterThan(info.mtime))
			return false;
		else
			wxRemoveFile(thumbPath);
	}
	return true;
}

unsigned char GetBarColor(int x, int y) {
	y = y % 7;
	if (x == 0 || x == 6 || y <= 1 || y >= 5)
		return 0x00;
	if (x == 1 || x == 5 || y == 2 || y == 4)
		return (x == 1 || x == 5) && (y == 2 || y == 4) ? 0xA0 : 0xD0;
	return 0xFF;
}

wxImage wxThumbnailFactory::GenerateThumbnailFromVideo(const wxString& filename) {
	wxString fname = filename;
	if (fname.StartsWith(wxT("concat:")))
		fname = fname.Mid(7).BeforeFirst(wxT('|'));
	wxImage img;
	wxFfmpegMediaDecoder decoder;
	if (wxFileExists(fname) && decoder.Load(fname) && decoder.BeginDecode(96, 80)) {
		if (decoder.GetDuration() > 0) {
			decoder.GetNextFrame();
			double dpos = decoder.GetDuration() * 0.05;
			decoder.SetPosition(dpos > 0.5 ? dpos - 0.5 : dpos);
			for (int i = 0; i < 100; i++) {
				img = decoder.GetNextFrame();
				double dpos1 = decoder.GetPosition();
				if (dpos1 >= dpos || dpos1 < 0)
					break;
			}
		} else {
			for (int i = 0; i < 30; i++)
				img = decoder.GetNextFrame();
		}
		if (img.Ok() && decoder.GetFrameAspectRatio() > 0 
				&& labs(decoder.GetFrameAspectRatio()*100 - img.GetWidth()*100/img.GetHeight()) >= 5) {
			if (lround(img.GetWidth() / decoder.GetFrameAspectRatio()) > img.GetHeight())
				img.Rescale(img.GetWidth(), lround(img.GetWidth() / decoder.GetFrameAspectRatio()));
			else
				img.Rescale(lround(img.GetHeight() * decoder.GetFrameAspectRatio()), img.GetHeight());
		}
	}
	if (img.Ok()) {
		unsigned char* data = img.GetData();
		for (int y = 0; y<img.GetHeight(); y++) {
			for (int x = 0; x<7; x++) {
				data[0] = data[1] = data[2] = GetBarColor(x,y);
				data += 3;
			}
			data += (img.GetWidth()-14)*3;
			for (int x = 0; x<7; x++) {
				data[0] = data[1] = data[2] = GetBarColor(x,y);
				data += 3;
			}
		}
	}
	return img;
}

wxImage wxThumbnailFactory::GenerateThumbnail(ThumbInfo& info, bool save) {
	wxImage img;
	wxString thumbPath = GetThumbPath(info);
	if (info.mimeType == _T("image")) {
		img.Create(1, 1);
		img.SetOption(wxIMAGE_OPTION_MAX_WIDTH, info.width);
		img.SetOption(wxIMAGE_OPTION_MAX_HEIGHT, info.height);
		if (wxFileExists(info.filename)) {
			wxLogNull l;
#if wxCHECK_VERSION(2,9,0)
			img.LoadFile(info.filename);
#else
			LoadImageFile(img, info.filename);
#endif
		}
#if defined(WX_SVG) || defined(GNOME2)
		ExifHandler::rotateImage(info.filename, img);
#endif
	} else if (info.mimeType == _T("concat")) {
		img = GenerateThumbnailFromVideo(info.filename);
	} else {
#if defined(GNOME2) || defined(GNOME3)
		info.pixbuf = gnome_thumbnail_factory_generate_thumbnail(thumbnail_factory, info.uri.c_str(), info.mimeType.c_str());
		if (info.pixbuf) {
			gnome_thumbnail_factory_save_thumbnail(thumbnail_factory, info.pixbuf, info.uri.c_str(), info.mtime.GetTicks());
			wxLogNull log;
			if (wxFileExists(thumbPath))
				img.LoadFile(thumbPath);
			return img;
		} else {
			if (info.mimeType.StartsWith(wxT("video/"))) {
				img = GenerateThumbnailFromVideo(info.filename);
			}
			if (!img.Ok()) {
				gnome_thumbnail_factory_create_failed_thumbnail(thumbnail_factory, info.uri.c_str(), info.mtime.GetTicks());
				return img;
			}
		}
#elif defined(WX_SVG)
		img = GenerateThumbnailFromVideo(info.filename);
#endif
	}
	if (!save)
		return img;
	// save thumbnail
	if (!img.Ok()) {
		img.Create(1, 1);
		img.SaveFile(GetThumbPath(info, THUMBNAIL_FAILED), wxBITMAP_TYPE_PNG);
		if (wxFileExists(thumbPath))
			wxRemoveFile(thumbPath);
	} else {
		if (img.GetWidth() > info.width || img.GetHeight() > info.height) {
			float scale = (float) info.width / img.GetWidth();
			if (scale > (float) info.height / img.GetHeight())
				scale = (float) info.height / img.GetHeight();
			img.Rescale((int) (img.GetWidth() * scale), (int) (img.GetHeight() * scale));
		}
#if defined(GNOME2) || defined(GNOME3)
		if (info.uri.length() > 0) {
			// create gdk pixbuf
			int width = img.GetWidth();
			int height = img.GetHeight();
			GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8, width, height);
			// copy data from image
			const unsigned char* in = img.GetData();
			unsigned char *out = gdk_pixbuf_get_pixels(pixbuf);
			int rowpad = gdk_pixbuf_get_rowstride(pixbuf) - 3 * width;
			for (int y = 0; y < height; y++, out += rowpad) {
				for (int x = 0; x < width; x++, out += 3, in += 3) {
					out[0] = in[0];
					out[1] = in[1];
					out[2] = in[2];
				}
			}
			// save
			gnome_thumbnail_factory_save_thumbnail(thumbnail_factory, pixbuf, info.uri.c_str(), info.mtime.GetTicks());
		}
#else
		wxLogNull log;
		img.SaveFile(thumbPath, wxBITMAP_TYPE_PNG);
#endif
	}
	return img;
}
