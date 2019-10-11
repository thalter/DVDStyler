///////////////////////////////////////////////////////////////////////////////
// Name:        Palettes3D.h
// Purpose:     The class to create and store palette with 3 colours per entry 
// Author:      Alex Thuering
// Created:	05.11.2006
// RCS-ID:      $Id: Palette3D.cpp,v 1.3 2011/07/16 18:56:33 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
///////////////////////////////////////////////////////////////////////////////

#include "Palette3D.h"
#include <wx/log.h>

inline unsigned char getR(unsigned int c) {
	return (unsigned char) (c >> 16);
}
inline unsigned char getG(unsigned int c) {
	return (unsigned char) (c >> 8);
}
inline unsigned char getB(unsigned int c) {
	return (unsigned char) c;
}
inline unsigned char getA(unsigned int c) {
	unsigned char a = c >> 24;
	return a < 127 ? a*2 : 255;
}

inline int Colour2Int(const wxColour& colour) {
	return colour.Ok() ? (colour.Alpha()/2 << 24) + (colour.Red() << 16) + (colour.Green() << 8) + colour.Blue() : -1;
}

inline wxColour Int2Colour(int colour) {
	return colour != -1 ? wxColour(getR(colour), getG(colour), getB(colour), getA(colour)) : wxColour();
}

inline double getDinstance(int c1, int c2) {
	if (c1 == -1 && c2 == -1)
		return 0;
	else if (c1 == -1 || c2 == -1)
		return 768;
	return 0.299 * abs(getR(c1) - getR(c2)) + 0.587 * abs(getG(c1) - getG(c2)) + 0.114 * abs(getB(c1) - getB(c2)) +
			abs(getA(c1) - getA(c2));
}

void Palette3D::Add(const wxColour& colour1, const wxColour& colour2, const wxColour& colour3) {
	int c1 = Colour2Int(colour1);
	int c2 = Colour2Int(colour2);
	int c3 = Colour2Int(colour3);
	for (int i = 0; i < (int) m_colours1.GetCount(); i++) {
		if (m_colours1[i] == c1 && m_colours2[i] == c2 && m_colours3[i] == c3)
			return; // duplicated entry
	}
	m_colours1.Add(c1);
	m_colours2.Add(c2);
	m_colours3.Add(c3);
}

bool Palette3D::Apply(MenuObjectParam* param, bool drawButtonsOnBackground) {
	if (m_colours1.GetCount() == 0)
		return false;

	int c1 = !drawButtonsOnBackground ? Colour2Int(param->normalColour) : -1;
	int c2 = Colour2Int(param->highlightedColour);
	int c3 = Colour2Int(param->selectedColour);
	
	int closest = 0;
	double d, dinstance = 7000; // max 7*768
	for (int i = 0; i < (int) m_colours1.GetCount(); i++) {
		d = 4*getDinstance(c1, m_colours1[i]) + 2*getDinstance(c2, m_colours2[i]) + getDinstance(c3, m_colours3[i]);
		if (d < dinstance) {
			dinstance = d;
			closest = i;
		}
	}
	if (c1 != m_colours1[closest] || c2 != m_colours2[closest] || c3 != m_colours3[closest]) {
		if (!drawButtonsOnBackground)
			param->normalColour = Int2Colour(m_colours1[closest]);
		param->highlightedColour = Int2Colour(m_colours2[closest]);
		param->selectedColour = Int2Colour(m_colours3[closest]);
		return true;
	}
	return false;
}

void Palette3D::ReduceColours(int maxColours) {
	while ((int) m_colours1.GetCount() > maxColours) {
		int closest = 0;
		double d, dinstance = 7000; // max 7*768
		for (int i = 0; i < (int) m_colours1.GetCount(); i++)
			for (int j = 0; j < (int) m_colours1.GetCount(); j++)
				if (i != j) {
					d = 4*getDinstance(m_colours1[i], m_colours1[j]) + 2*getDinstance(m_colours2[i], m_colours2[j])
							+ getDinstance(m_colours3[i], m_colours3[j]);
					if (d < dinstance) {
						dinstance = d;
						closest = j;
					}
				}
		m_colours1.RemoveAt(closest);
		m_colours2.RemoveAt(closest);
		m_colours3.RemoveAt(closest);
	}
}
