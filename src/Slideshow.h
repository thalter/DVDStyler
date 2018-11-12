/////////////////////////////////////////////////////////////////////////////
// Name:		Slideshow.h
// Purpose:		The class to store information for slideshow
// Author:		Alex Thuering
// Created:		26.06.2005
// RCS-ID:		$Id: Slideshow.h,v 1.13 2015/10/03 13:29:57 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:		GPL
/////////////////////////////////////////////////////////////////////////////

#ifndef SLIDESHOW_H
#define SLIDESHOW_H

#include "DVDAction.h"
#include "mediaenc.h"
#include <vector>
using namespace std;

class Slide {
public:
	Slide() {}
	Slide(const wxString& filename): m_filename(filename) {}
	Slide(wxSvgXmlNode* node) { PutXML(node); }
	
	inline wxString GetFilename() { return m_filename; }
	inline void SetFilename(const wxString& filename) { m_filename = filename; }
	
	inline const wxString& GetTransition() const { return m_transition; }
	inline void SetTransition(const wxString& transition) { m_transition = transition; }
	
	wxSvgXmlNode* GetXML();
	bool PutXML(wxSvgXmlNode* node);
  
protected:
	wxString m_filename;
	wxString m_transition;
};

class Slideshow {
public:
	Slideshow(VideoFormat videoFormat = vfPAL, AspectRatio aspectRatio = ar4_3);
	Slideshow(wxSvgXmlNode* node);
	Slideshow(const Slideshow& slideshow);
	~Slideshow();
	
	inline VideoFormat GetVideoFormat() { return m_videoFormat; }
	inline void SetVideoFormat(VideoFormat format) { m_videoFormat = format; }
	wxSize GetResolution();
	inline AspectRatio GetAspectRatio() { return m_aspectRatio; }
	inline void SetAspectRatio(AspectRatio aspectRatio) { m_aspectRatio = aspectRatio; }
	float GetFPS();
	
	inline int GetDuration() { return m_duration; }
	inline void SetDuration(int duration) { m_duration = duration; }
	
	inline const wxString& GetTransition() const { return m_transition; }
	inline void SetTransition(const wxString& transition) { m_transition = transition; }
	
	inline unsigned int GetCount() { return m_slides.size(); }
	inline unsigned int size() { return m_slides.size(); }
	inline Slide* GetSlide(unsigned int idx) { return m_slides[idx]; }
	inline void AddSlide(Slide* slide) { m_slides.push_back(slide); }
	inline void RemoveSlide(unsigned int idx) { m_slides.erase(m_slides.begin() + idx); }
	void Clear();
	
	wxImage GetThumbImage(int width, int height);
	wxImage GetImage(int index);
	/** Returns total duration of slideshow (sec) */
	int GetResultDuration();
	
	wxSvgXmlNode* GetXML(DVDFileType type, wxSvgXmlNode* node = NULL);
	bool PutXML(wxSvgXmlNode* node);
	
	static void GetTransitions(wxArrayString& transitions, wxArrayString& labels);
  
protected:
	VideoFormat m_videoFormat;
	AspectRatio m_aspectRatio;
	int m_duration;
	wxString m_transition;
	vector<Slide*> m_slides;
};

#endif //SLIDESHOW_H
