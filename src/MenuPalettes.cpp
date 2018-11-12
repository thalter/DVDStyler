/////////////////////////////////////////////////////////////////////////////
// Name:        MenuPalettes.h
// Purpose:     The class to create and store menu palettes 
// Author:      Alex Thuering
// Created:	04.11.2006
// RCS-ID:      $Id: MenuPalettes.cpp,v 1.6 2014/02/25 11:08:27 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "MenuPalettes.h"
#include <wx/log.h>

bool Palette::Create(IntHashSet& colours) {
	m_hasTranspColour = false;
	if (colours.find(-1) != colours.end()) // => image can contain transparent pixels
	{
		m_hasTranspColour = true;
		colours.erase(-1);
	}
	coloursCount = colours.size();
	unsigned char r[colours.size()], g[colours.size()], b[colours.size()];
	int i = 0;
	for (IntHashSet::iterator it = colours.begin(); it != colours.end(); ++it) {
		unsigned int color = (unsigned int) *it;
		r[i] = (color >> 16) & 0xFF;
		g[i] = (color >> 8) & 0xFF;
		b[i] = color & 0xFF;
		i++;
	}
	return wxPalette::Create(colours.size(), r, g, b);
}

bool Palette::Apply(const unsigned char* pixel, unsigned char *dest, unsigned char *destAlpha) const {
	if (coloursCount == 0) { // there are no colours => colour of this pixel can be only transparent
		*destAlpha = 0;
		dest[0] = 0;
		dest[1] = 0;
		dest[2] = 0;
	} else if (!m_hasTranspColour || *destAlpha > 0) { // must be not transparent
		if (*destAlpha == 0)
			*destAlpha = 32;
		// find matched colour
		int n = GetPixel(pixel[0], pixel[1], pixel[2]);
		return wxPalette::GetRGB(n, dest, dest + 1, dest + 2);
	}
	return true;
}

WX_DECLARE_HASH_MAP(int, IntHashSet, wxIntegerHash, wxIntegerEqual, IntHashSetMap)
;
WX_DECLARE_HASH_MAP(int, IntHashSetMap, wxIntegerHash, wxIntegerEqual, IntHashSet2DMap)
;

///////////////////////////// Menu Palettes //////////////////////////////////

MenuPalettes::MenuPalettes(MenuObject& obj, bool drawButtonsOnBackground) {
	m_drawButtonsOnBackground = drawButtonsOnBackground;
	// palette 1 for buttons_normal => all colours + transport colour
	IntHashSet colours;
	if (!m_drawButtonsOnBackground) {
		for (unsigned int i = 0; i < obj.GetParams().size(); i++) {
			MenuObjectParam* param = obj.GetParams()[i];
			if (param->isChangeable()) {
				wxColour& colour = param->normalColour;
				int value = colour.Ok() ? (colour.Red() << 16) + (colour.Green() << 8) + colour.Blue() : -1;
				colours.insert(value);
			}
		}
		colours.insert(-1); // first image can always contain transparent pixels
	}
	m_palette1.Create(colours);
	
	// palette(s) 2 for buttons_highlighted => a palette per colour from palette 1
	IntHashSetMap coloursMap;
	for (unsigned int i = 0; i < obj.GetParams().size(); i++) {
		MenuObjectParam* param = obj.GetParams()[i];
		if (param->isChangeable()) {
			wxColour colour1 = m_drawButtonsOnBackground ? wxColour() : param->normalColour;
			int key1 = colour1.Ok() ? (colour1.Red() << 16) + (colour1.Green() << 8) + colour1.Blue() : -1;
			wxColour& colour = param->highlightedColour;
			int value = colour.Ok() ? (colour.Red() << 16) + (colour.Green() << 8) + colour.Blue() : -1;
			coloursMap[key1].insert(value);
		}
	}
	coloursMap[-1].insert(-1); // second image can contain transparent pixels if pixel in first image is transparent 
	for (IntHashSetMap::iterator it = coloursMap.begin(); it != coloursMap.end(); ++it)
		m_palette2[it->first].Create(it->second);
	
	// palette(s) 3 for buttons_selected => a palette per colour from palette 1 and 2
	IntHashSet2DMap colours2DMap;
	for (unsigned int i = 0; i < obj.GetParams().size(); i++) {
		MenuObjectParam* param = obj.GetParams()[i];
		if (param->isChangeable()) {
			wxColour colour1 = m_drawButtonsOnBackground ? wxColour() : param->normalColour;
			int key1 = colour1.Ok() ? (colour1.Red() << 16) + (colour1.Green() << 8) + colour1.Blue() : -1;
			wxColour& colour2 = param->highlightedColour;
			int key2 = colour2.Ok() ? (colour2.Red() << 16) + (colour2.Green() << 8) + colour2.Blue() : -1;
			wxColour& colour = param->selectedColour;
			int value = colour.Ok() ? (colour.Red() << 16) + (colour.Green() << 8) + colour.Blue() : -1;
			colours2DMap[key1][key2].insert(value);
		}
	}
	colours2DMap[-1][-1].insert(-1); // third image can contain transparent pixels if pixel in first and second image is transparent
	for (IntHashSet2DMap::iterator it = colours2DMap.begin(); it != colours2DMap.end(); ++it)
		for (IntHashSetMap::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			m_palette3[it->first][it2->first].Create(it2->second);
}

int MenuPalettes::GetKey(const unsigned char* pixel, unsigned char alpha) {
	if (alpha == 0)
		return -1; // it's transparent colour
	return (pixel[0] << 16) + (pixel[1] << 8) + pixel[2];
}

bool MenuPalettes::Apply(const unsigned char* pixel1, unsigned char *dest, unsigned char *destAlpha) {
	if (m_drawButtonsOnBackground)
		return false;
	return m_palette1.Apply(pixel1, dest, destAlpha);
}

bool MenuPalettes::Apply(const unsigned char* pixel1, unsigned char pixel1Alpha, const unsigned char* pixel2,
		unsigned char *dest, unsigned char *destAlpha) {
	int key1 = GetKey(pixel1, pixel1Alpha);
	return m_palette2[key1].Apply(pixel2, dest, destAlpha);
}

bool MenuPalettes::Apply(const unsigned char* pixel1, unsigned char pixel1Alpha, const unsigned char* pixel2,
		unsigned char pixel2Alpha, const unsigned char* pixel3, unsigned char *dest, unsigned char *destAlpha) {
	int key1 = GetKey(pixel1, pixel1Alpha);
	int key2 = GetKey(pixel2, pixel2Alpha);
	return m_palette3[key1][key2].Apply(pixel3, dest, destAlpha);
}
