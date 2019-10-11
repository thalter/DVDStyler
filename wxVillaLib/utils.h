/////////////////////////////////////////////////////////////////////////////
// Name:        utils.h
// Purpose:     Miscellaneous utilities
// Author:      Alex Thuering
// Created:	23.10.2003
// RCS-ID:      $Id: utils.h,v 1.4 2012/04/01 23:25:43 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef WXVILLALIB_UTILS_H
#define WXVILLALIB_UTILS_H

#include <wx/wx.h>
#include <wx/log.h>
#include <wx/mstream.h>

wxString wxGetAppPath();
void wxSetAppPath(wxString value);

wxString wxFindDataDirectory(wxString dir);
wxString wxFindDataFile(wxString filename);

#define wxBITMAP_FROM_MEMORY(name) wxGetBitmapFromMemory(name##_png, sizeof(name##_png))

inline wxBitmap wxGetBitmapFromMemory(const unsigned char *data, int length) {
   wxMemoryInputStream is(data, length);
   return wxBitmap(wxImage(is, wxBITMAP_TYPE_ANY, -1), -1);
}

#define wxICON_FROM_MEMORY(name) wxGetIconFromMemory(name##_png, sizeof(name##_png))

inline wxIcon wxGetIconFromMemory(const unsigned char *data, int length) {
   wxIcon icon;
   icon.CopyFromBitmap(wxGetBitmapFromMemory(data, length));
   return icon;
}

#endif // WXVILLALIB_UTILS_H
