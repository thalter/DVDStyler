/////////////////////////////////////////////////////////////////////////////
// Name:        SConverter.cpp
// Purpose:     String converter
// Author:      Alex Thuering
// Created:	21.07.2003
// RCS-ID:      $Id: SConv.cpp,v 1.2 2006/12/06 14:08:03 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#include "SConv.h"

wxString SConv::ToString(wxColour value, bool withNumberSign)
{
  if (value.Ok())
	return wxString::Format(
	  withNumberSign ? _T("#%02x%02x%02x") : _T("%02x%02x%02x"),
	  value.Red(), value.Green(), value.Blue());
  return wxEmptyString;
}

wxColour SConv::ToColour(wxString value)
{
  wxColour res;
  if (value.Mid(0,1) != _T("#"))
	return res;
  long r,g,b;
  value.Mid(1,2).ToLong(&r,16);
  value.Mid(3,2).ToLong(&g,16);
  value.Mid(5,2).ToLong(&b,16);
  res.Set(r,g,b);
  return res;
}

wxString SConv::ToString(wxFont value)
{
  return wxString::Format(_T("%d;%d;%d;%d;%d;%s"),
	value.GetPointSize(), value.GetFamily(), value.GetStyle(),
	value.GetWeight(), value.GetUnderlined(), value.GetFaceName().c_str());
}

wxFont SConv::ToFont(wxString value)
{
  wxString v;
  long params[5];
  for (int i=0; i<5; i++)
  {
	v = value.BeforeFirst(wxT(';'));
	if (v.length() == 0 || !v.ToLong(&params[i]))
	  return wxFont();
	value.Remove(0, v.length()+1);
  }
  return wxFont(params[0], (wxFontFamily)params[1], (wxFontStyle)params[2], (wxFontWeight)params[3], params[4], value);
}
