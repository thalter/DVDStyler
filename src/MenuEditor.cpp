/////////////////////////////////////////////////////////////////////////////
// Name:        MenuEditor.cpp
// Purpose:     The widget to edit a DVD Menu
// Author:      Alex Thuering
// Created:     11.10.2003
// RCS-ID:      $Id: MenuEditor.cpp,v 1.81 2015/02/15 19:37:39 ntalex Exp $
// Copyright:   (c) Alex Thuering
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

#include "MenuEditor.h"
#include "MenuPropDlg.h"
#include "MenuObjectPropDlg.h"
#include "MPEG.h"
#include "TitlesetManager.h"
#include "Utils.h"
#include "Config.h"
#include <wxVillaLib/ImageProc.h>
#include <wxVillaLib/Thumbnails.h>
#include <wxVillaLib/utils.h>
#include <wxSVG/svg.h>
#include <wxSVGXML/svgxml.h>
#include "math.h"
#include <wx/dnd.h>
#include <wx/utils.h>
#include <wx/clipbrd.h>
#include <wx/artprov.h>
#include <wx/filename.h>
#include <wx/colordlg.h>

#define OBJECTS_DIR wxFindDataDirectory(_T("objects"))
#define BUTTONS_DIR wxFindDataDirectory(_T("buttons"))
#define CURSOR_FILE(fname) wxFindDataFile(_T("rc") + wxString(wxFILE_SEP_PATH) + fname)

//////////////////////////////////////////////////////////////////////////////
////////////////////////////// MenuDnDFile ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class MenuDnDFile : public wxDropTarget {
public:
	MenuDnDFile(MenuEditor* menuEditor) {
		m_menuEditor = menuEditor;
		wxDataFormat actionDataFormat(DATAFORMAT_ACTION);
		wxDataObjectComposite* dataObject = new wxDataObjectComposite();
		dataObject->Add(m_fileDataObject = new wxFileDataObject);
		dataObject->Add(m_actionDataObject = new wxCustomDataObject(actionDataFormat));
		SetDataObject(dataObject);
	}
	
	wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def) {
		if (!GetData())
			return wxDragError;
		
		wxDataObjectComposite* dobj = (wxDataObjectComposite*) m_dataObject;
		
		if (dobj->GetReceivedFormat() == wxDF_FILENAME) {
			return OnDropFiles( x, y, m_fileDataObject->GetFilenames() ) ? def : wxDragError;
		} else if (dobj->GetReceivedFormat().GetId() == DATAFORMAT_ACTION) {
			wxString actionStr((const char*) m_actionDataObject->GetData(), wxConvISO8859_1, m_actionDataObject->GetSize());
			DVDAction action;
			action.Deserialize(actionStr);
			AddButton(x, y, action.GetTsi(), action.GetPgci(), action.IsMenu(), action.GetChapter());
			return def;
		}
		
		return wxDragError;
	}

	bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) {
		if (!m_menuEditor->GetMenu())
			return false;
		bool res = false;
		for (int i = 0; i<(int)filenames.Count(); i++) {
			res = AddFile(x, y, filenames[i], i == 0) || res;
			if (x < m_menuEditor->GetMenu()->GetResolution().x-48
					&& y < m_menuEditor->GetMenu()->GetResolution().y-48) {
				x += 16;
				y += 16;
			}
		}
		return res;
	}

	bool AddFile(wxCoord x, wxCoord y, wxString filename, bool first) {
		wxString ext = filename.AfterLast('.').Lower();
		if (ext == _T("xml"))
			m_menuEditor->AddButton(filename, (int) (x/m_menuEditor->GetScaleX()), (int) (y/m_menuEditor->GetScaleY()));
		else if (wxImage::FindHandler(ext, BITMAP_TYPE_ANY))
			m_menuEditor->AddImage(filename, (int) (x/m_menuEditor->GetScaleX()), (int) (y/m_menuEditor->GetScaleY()));
		else if (wxThumbnails::IsVideo(filename)) {
			wxFfmpegMediaDecoder decoder;
			if (!decoder.Load(filename)) {
				wxLogError(filename + _(": unknown format"));
				return false;
			}
			// check video
			bool hasVideo = false;
			for (unsigned int i = 0; i < decoder.GetStreamCount(); i++) {
				if (decoder.GetStreamType(i) == stVIDEO) {
					hasVideo = true;
					break;
				}
			}
			if (!hasVideo) {
				return false;
			}
			// try to find this file
			bool found = false;
			for (unsigned int tsi = 0; tsi < m_menuEditor->GetDVD()->GetTitlesets().GetCount(); tsi++) {
				Titleset& ts = *m_menuEditor->GetDVD()->GetTitlesets()[tsi];
				for (unsigned int pgci = 0; pgci < ts.GetTitles().GetCount(); pgci++) {
					Pgc* pgc = ts.GetTitles()[pgci];
					for (unsigned int vobi = 0; vobi < pgc->GetVobs().GetCount(); vobi++) {
						Vob* vob = pgc->GetVobs()[vobi];
						if (vob->GetFilename() == filename) {
							found = true;
							int chapter = 0;
							if (vobi > 0)
								chapter = pgc->GetChapterCount(vobi) + (vob->GetChapters().length() ? 0 : -1);
							AddButton(x, y, tsi, pgci, false, chapter, !first);
							break;
						}
					}
				}
			}
			if (!found) {
				// add file to DVD
				int tsi = m_menuEditor->GetTsi();
				if (tsi == -1)
					tsi = m_menuEditor->GetDVD()->GetTitlesets().size() - 1;
				if (tsi == -1)
					tsi = 0;
				if (m_menuEditor->GetTitlesetManager()->AddVideo(filename, true, tsi, false)) {
					int pgci = m_menuEditor->GetDVD()->GetPgcArray(tsi, false).GetCount()-1;
					AddButton(x, y, tsi, pgci, false, 0, !first);
				}
			}
		}
		m_menuEditor->SetFocus();
		return true;
	}

	void AddButton(wxCoord x, wxCoord y, int tsi, int pgci, bool isMenu, int chapter, bool force = false) {
		wxString objId = m_menuEditor->HitTest(x, y);
		if (!force && objId.length() && m_menuEditor->GetMenu()->GetObject(objId)->IsButton()) {
			MenuObject* obj = m_menuEditor->GetMenu()->GetObject(objId);
			MenuObjectParam* initParam = obj->GetInitParam();
			if (initParam && initParam->type == wxT("image")) {
				Pgc* pgc = m_menuEditor->GetDVD()->GetPgc(tsi, false, pgci);
				if (pgc) {
					obj->SetParam(initParam->name, pgc->GetVideoFrameUri(0));
					m_menuEditor->Select(objId);
				}
			}
			obj->GetAction().SetTsi(tsi == m_menuEditor->GetTsi() ? -2 : tsi);
			obj->GetAction().SetPgci(pgci);
			obj->GetAction().SetMenu(isMenu);
			obj->GetAction().SetChapter(chapter);
		} else {
			x = (int) (x/m_menuEditor->GetScaleX());
			y = (int) (y/m_menuEditor->GetScaleY());
			if (isMenu)
				m_menuEditor->AddButton(BUTTONS_DIR + s_config.GetDefTextButton(), x, y, pgci, tsi, isMenu, chapter);
			else
				m_menuEditor->AddButton(BUTTONS_DIR + s_config.GetDefButton(), x, y, pgci, tsi, isMenu, chapter);
		}
	}

private:
	MenuEditor *m_menuEditor;
	wxFileDataObject* m_fileDataObject;
	wxCustomDataObject* m_actionDataObject;
};

//////////////////////////////////////////////////////////////////////////////
/////////////////////////// Help functions ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

wxRect ScaleRect(wxRect rect, double scaleX, double scaleY) {
	wxRect res;
	res.x = (int) (rect.x * scaleX);
	res.y = (int) (rect.y * scaleY);
	res.width = (int) (rect.width * scaleX);
	res.height = (int) (rect.height * scaleY);
	return res;
}

wxRect ScaleRect(wxSVGRect rect, double scaleX, double scaleY) {
	wxRect res;
	res.x = (int) (rect.GetX() * scaleX);
	res.y = (int) (rect.GetY() * scaleY);
	res.width = (int) (rect.GetWidth() * scaleX);
	res.height = (int) (rect.GetHeight() * scaleY);
	return res;
}

//////////////////////////////////////////////////////////////////////////////
/////////////////////////////// MenuEditor ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

enum {
	EVENT_TIMER_ID = 8500,
	MENUEDITOR_PROP_ID,
	MENUEDITOR_OBJMENU_CUT_ID,
	MENUEDITOR_OBJMENU_COPY_ID,
	MENUEDITOR_OBJMENU_PASTE_ID,
	MENUEDITOR_OBJMENU_DELETE_ID,
	MENUEDITOR_OBJMENU_FIRSTBUTTON_ID,
	MENUEDITOR_ARRANGEMENU_ID,
	MENUEDITOR_ARRANGEMENU_TOFRONT_ID,
	MENUEDITOR_ARRANGEMENU_FORWARD_ID,
	MENUEDITOR_ARRANGEMENU_BACKWARD_ID,
	MENUEDITOR_ARRANGEMENU_TOBACK_ID,
	MENUEDITOR_ALIGNMENU_ID,
	MENUEDITOR_ALIGNMENU_TOCENTERMH_ID,
	MENUEDITOR_ALIGNMENU_TOCENTERMV_ID,
	MENUEDITOR_ALIGNMENU_TOCENTERH_ID,
	MENUEDITOR_ALIGNMENU_TOCENTERV_ID,
	MENUEDITOR_ALIGNMENU_TOLEFT_ID,
	MENUEDITOR_ALIGNMENU_TORIGHT_ID,
	MENUEDITOR_ALIGNMENU_TOTOP_ID,
	MENUEDITOR_ALIGNMENU_TOBOTTOM_ID,
	MENUEDITOR_VIEWMENU_ID,
	MENUEDITOR_VIEWMENU_SAFETV_ID,
	MENUEDITOR_GRIDMENU_ID = 8600,
	MENUEDITOR_GRIDMENU_ITEM_ID, //8601..8605
	MENUEDITOR_GRIDMENU_SHOW_ID = 8610,
	MENUEDITOR_GRIDMENU_OVEROBJECTS_ID,
	MENUEDITOR_GRIDMENU_COLOUR_ID,
	MENUEDITOR_ADDMENU_ID = 9000,
	MENUEDITOR_ADDMENU_OBJECT_ID //9001..9999
};

BEGIN_EVENT_TABLE(MenuEditor, wxSVGCtrl)
  EVT_RIGHT_DOWN(MenuEditor::OnMouseRightButton)
  EVT_RIGHT_UP(MenuEditor::OnMouseRightButton)
  EVT_LEFT_DOWN(MenuEditor::OnMouseLeftButton)
  EVT_LEFT_UP(MenuEditor::OnMouseLeftButton)
  EVT_MOTION(MenuEditor::OnMouseMove)
  EVT_LEFT_DCLICK(MenuEditor::OnMouseDClick)
  EVT_KEY_DOWN(MenuEditor::OnKeyDown)
  EVT_MENU(MENUEDITOR_PROP_ID, MenuEditor::OnProperties)
  EVT_UPDATE_UI(MENUEDITOR_PROP_ID, MenuEditor::OnUpdateUIProperties)
  EVT_MENU(MENUEDITOR_OBJMENU_CUT_ID, MenuEditor::OnObjectCut)
  EVT_MENU(MENUEDITOR_OBJMENU_COPY_ID, MenuEditor::OnObjectCopy)
  EVT_MENU(MENUEDITOR_OBJMENU_PASTE_ID, MenuEditor::OnObjectPaste)
  EVT_UPDATE_UI(MENUEDITOR_OBJMENU_PASTE_ID, MenuEditor::OnUpdateUIObjectPaste)
  EVT_MENU(MENUEDITOR_OBJMENU_FIRSTBUTTON_ID, MenuEditor::OnFirstButton)
  EVT_UPDATE_UI(MENUEDITOR_OBJMENU_FIRSTBUTTON_ID, MenuEditor::OnUpdateUIFirstButton)
  EVT_MENU(MENUEDITOR_OBJMENU_DELETE_ID, MenuEditor::OnObjectDelete)
  EVT_MENU(MENUEDITOR_ARRANGEMENU_TOFRONT_ID, MenuEditor::OnObjectToFront)
  EVT_MENU(MENUEDITOR_ARRANGEMENU_FORWARD_ID, MenuEditor::OnObjectForward)
  EVT_MENU(MENUEDITOR_ARRANGEMENU_BACKWARD_ID, MenuEditor::OnObjectBackward)
  EVT_MENU(MENUEDITOR_ARRANGEMENU_TOBACK_ID, MenuEditor::OnObjectToBack)
  EVT_UPDATE_UI(MENUEDITOR_ARRANGEMENU_TOFRONT_ID, MenuEditor::OnUpdateUIObjectForward)
  EVT_UPDATE_UI(MENUEDITOR_ARRANGEMENU_FORWARD_ID, MenuEditor::OnUpdateUIObjectForward)
  EVT_UPDATE_UI(MENUEDITOR_ARRANGEMENU_BACKWARD_ID, MenuEditor::OnUpdateUIObjectBackward)
  EVT_UPDATE_UI(MENUEDITOR_ARRANGEMENU_TOBACK_ID, MenuEditor::OnUpdateUIObjectBackward)
  EVT_MENU(MENUEDITOR_ALIGNMENU_TOCENTERMH_ID, MenuEditor::OnAlignToCenterMH)
  EVT_MENU(MENUEDITOR_ALIGNMENU_TOCENTERMV_ID, MenuEditor::OnAlignToCenterMV)
  EVT_MENU(MENUEDITOR_ALIGNMENU_TOCENTERH_ID, MenuEditor::OnAlignToCenterH)
  EVT_MENU(MENUEDITOR_ALIGNMENU_TOCENTERV_ID, MenuEditor::OnAlignToCenterV)
  EVT_MENU(MENUEDITOR_ALIGNMENU_TOLEFT_ID, MenuEditor::OnAlignToLeft)
  EVT_MENU(MENUEDITOR_ALIGNMENU_TORIGHT_ID, MenuEditor::OnAlignToRight)
  EVT_MENU(MENUEDITOR_ALIGNMENU_TOTOP_ID, MenuEditor::OnAlignToTop)
  EVT_MENU(MENUEDITOR_ALIGNMENU_TOBOTTOM_ID, MenuEditor::OnAlignToBottom)
  EVT_UPDATE_UI(MENUEDITOR_ALIGNMENU_TOCENTERH_ID, MenuEditor::OnUpdateUIAlign)
  EVT_UPDATE_UI(MENUEDITOR_ALIGNMENU_TOCENTERV_ID, MenuEditor::OnUpdateUIAlign)
  EVT_UPDATE_UI(MENUEDITOR_ALIGNMENU_TOLEFT_ID, MenuEditor::OnUpdateUIAlign)
  EVT_UPDATE_UI(MENUEDITOR_ALIGNMENU_TORIGHT_ID, MenuEditor::OnUpdateUIAlign)
  EVT_UPDATE_UI(MENUEDITOR_ALIGNMENU_TOTOP_ID, MenuEditor::OnUpdateUIAlign)
  EVT_UPDATE_UI(MENUEDITOR_ALIGNMENU_TOBOTTOM_ID, MenuEditor::OnUpdateUIAlign)
  EVT_MENU(MENUEDITOR_VIEWMENU_SAFETV_ID, MenuEditor::OnSafeTV)
  EVT_MENU(MENUEDITOR_GRIDMENU_ITEM_ID, MenuEditor::OnGridMenu)
  EVT_MENU(MENUEDITOR_GRIDMENU_ITEM_ID+1, MenuEditor::OnGridMenu)
  EVT_MENU(MENUEDITOR_GRIDMENU_ITEM_ID+2, MenuEditor::OnGridMenu)
  EVT_MENU(MENUEDITOR_GRIDMENU_ITEM_ID+3, MenuEditor::OnGridMenu)
  EVT_MENU(MENUEDITOR_GRIDMENU_ITEM_ID+4, MenuEditor::OnGridMenu)
  EVT_MENU(MENUEDITOR_GRIDMENU_ITEM_ID+5, MenuEditor::OnGridMenu)
  EVT_MENU(MENUEDITOR_GRIDMENU_SHOW_ID, MenuEditor::OnShowGrid)
  EVT_MENU(MENUEDITOR_GRIDMENU_OVEROBJECTS_ID, MenuEditor::OnShowGridOverObjects)
  EVT_MENU(MENUEDITOR_GRIDMENU_COLOUR_ID, MenuEditor::OnGridColour)
  EVT_MENU_RANGE(MENUEDITOR_ADDMENU_OBJECT_ID, MENUEDITOR_ADDMENU_OBJECT_ID+999, MenuEditor::OnAddObject)
END_EVENT_TABLE()

DEFINE_EVENT_TYPE(EVT_COMMAND_MENUEDITOR_MENU_CHANGED)
DEFINE_EVENT_TYPE(EVT_COMMAND_MENUEDITOR_OBJECT_POINTED)

MenuEditor::MenuEditor(wxWindow *parent, wxWindowID id): wxSVGCtrl(parent, id, wxDefaultPosition, wxDefaultSize,
		wxWANTS_CHARS), m_gridColour(255, 255, 255) {
	m_dvd = NULL;
	m_menu = NULL;
	m_doAction = false;
	DoSelect();
	m_safeTV = true;
	m_showGrid = false;
	m_gridOverObejcts = false;
	m_grid = 2;
	m_doCopy = false;
	m_copied = false;
	SetDropTarget(new MenuDnDFile(this));
	SetFitToFrame(true);
	
	// sub menu "Arrange"
	wxMenu* arrangeMenu = new wxMenu;
	arrangeMenu->Append
	
	(MENUEDITOR_ARRANGEMENU_TOFRONT_ID, _("&Bring to Front"));
	arrangeMenu->Append(MENUEDITOR_ARRANGEMENU_FORWARD_ID, _("Bring &Forward"));
	arrangeMenu->Append(MENUEDITOR_ARRANGEMENU_BACKWARD_ID, _("Send Back&ward"));
	arrangeMenu->Append(MENUEDITOR_ARRANGEMENU_TOBACK_ID, _("&Send to Back"));

	// sub menu "Align"
	wxMenu* alignMenu = new wxMenu;
	alignMenu->Append(MENUEDITOR_ALIGNMENU_TOLEFT_ID, _("To &Left"));
	alignMenu->Append(MENUEDITOR_ALIGNMENU_TORIGHT_ID, _("To &Right"));
	alignMenu->Append(MENUEDITOR_ALIGNMENU_TOTOP_ID, _("To &Top"));
	alignMenu->Append(MENUEDITOR_ALIGNMENU_TOBOTTOM_ID, _("To &Bottom"));
	alignMenu->Append(MENUEDITOR_ALIGNMENU_TOCENTERH_ID, _("To Center &horizontally"));
	alignMenu->Append(MENUEDITOR_ALIGNMENU_TOCENTERV_ID, _("To Center &vertically"));
	alignMenu->Append(MENUEDITOR_ALIGNMENU_TOCENTERMH_ID, _("To &Center of Menu horizontally"));
	alignMenu->Append(MENUEDITOR_ALIGNMENU_TOCENTERMV_ID, _("To Center of &Menu vertically"));

	// sub menu "Add"
	wxMenu* addMenu = new wxMenu;
	wxMenu* addMenu2 = new wxMenu;
	wxString fname = wxFindFirstFile(OBJECTS_DIR + _T("*.xml"));
	int addId = MENUEDITOR_ADDMENU_OBJECT_ID;
	while (!fname.IsEmpty()) {
		MenuObject obj(NULL, false, fname);
		addMenu->Append(addId, wxGetTranslation(obj.GetTitle().c_str()));
		addMenu2->Append(addId, wxGetTranslation(obj.GetTitle().c_str()));
		m_addMenuObjects.Add(obj.GetFileName());
		addId++;
		fname = wxFindNextFile();
	}
	
	// sub menu "View"
	wxMenu* viewMenu = new wxMenu;
	viewMenu->Append(MENUEDITOR_VIEWMENU_SAFETV_ID, _("&Show Safe TV area"), wxEmptyString, wxITEM_CHECK);
	viewMenu->Check(MENUEDITOR_VIEWMENU_SAFETV_ID, m_safeTV);
	
	// sub menu "Grid"
	m_gridMenu = new wxMenu;
	m_gridMenu->Append(MENUEDITOR_GRIDMENU_ITEM_ID, _("&none"), wxEmptyString, wxITEM_RADIO);
	m_gridMenu->Append(MENUEDITOR_GRIDMENU_ITEM_ID+1, _T("4x4"), wxEmptyString, wxITEM_RADIO);
	m_gridMenu->Append(MENUEDITOR_GRIDMENU_ITEM_ID+2, _T("8x8"), wxEmptyString, wxITEM_RADIO);
	m_gridMenu->Append(MENUEDITOR_GRIDMENU_ITEM_ID+3, _T("16x16"), wxEmptyString, wxITEM_RADIO);
	m_gridMenu->Append(MENUEDITOR_GRIDMENU_ITEM_ID+4, _T("32x32"), wxEmptyString, wxITEM_RADIO);
	m_gridMenu->Append(MENUEDITOR_GRIDMENU_ITEM_ID+5, _T("64x64"), wxEmptyString, wxITEM_RADIO);
	m_gridMenu->Check(MENUEDITOR_GRIDMENU_ITEM_ID + m_grid - 1, true);
	m_gridMenu->Append(MENUEDITOR_GRIDMENU_SHOW_ID, _("&Show"), wxEmptyString, wxITEM_CHECK);
	m_gridMenu->Append(MENUEDITOR_GRIDMENU_OVEROBJECTS_ID, _("&Over objects"), wxEmptyString, wxITEM_CHECK);
	m_gridMenu->Append(MENUEDITOR_GRIDMENU_COLOUR_ID, _("&Colour..."));
	
	// object context menu
	m_objMenu = new wxMenu;
	m_objMenu->Append(MENUEDITOR_ARRANGEMENU_ID, _("A&rrange"), arrangeMenu);
	m_objMenu->Append(MENUEDITOR_ALIGNMENU_ID, _("A&lign"), alignMenu);
	m_objMenu->Append(MENUEDITOR_ADDMENU_ID, _("&Add"), addMenu);
//	m_objMenu->Append(MENUEDITOR_VIEWMENU_ID, _("&View"), viewMenu);
//	m_objMenu->Append(MENUEDITOR_GRIDMENU_ID, _("&Grid"), m_gridMenu);
	m_objMenu->AppendCheckItem(MENUEDITOR_OBJMENU_FIRSTBUTTON_ID, _("&First highlighted button"));
	m_objMenu->AppendSeparator();
	wxMenuItem* item = new wxMenuItem(m_objMenu, MENUEDITOR_OBJMENU_CUT_ID, _("Cu&t"));
	item->SetBitmap(wxArtProvider::GetBitmap(wxART_CUT, wxART_MENU));
	m_objMenu->Append(item);
	item = new wxMenuItem(m_objMenu, MENUEDITOR_OBJMENU_COPY_ID, _("&Copy"));
	item->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY, wxART_MENU));
	m_objMenu->Append(item);
	item = new wxMenuItem(m_objMenu, MENUEDITOR_OBJMENU_PASTE_ID, _("&Paste"));
	item->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_MENU));
	m_objMenu->Append(item);
	item = new wxMenuItem(m_objMenu, MENUEDITOR_OBJMENU_DELETE_ID, _("&Delete"));
	item->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_MENU));
	m_objMenu->Append(item);
	m_objMenu->AppendSeparator();
	m_objMenu->Append(MENUEDITOR_PROP_ID, _("&Properties..."));

	// context menu
	m_pmenu = new wxMenu;
	m_pmenu->Append(MENUEDITOR_ADDMENU_ID, _("&Add"), addMenu2);
	m_pmenu->Append(MENUEDITOR_VIEWMENU_ID, _("&View"), viewMenu);
	m_pmenu->Append(MENUEDITOR_GRIDMENU_ID, _("&Grid"), m_gridMenu);
	item = new wxMenuItem(m_pmenu, MENUEDITOR_OBJMENU_PASTE_ID, _("&Paste"));
	item->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_MENU));
	m_pmenu->Append(item);
	m_pmenu->Append(MENUEDITOR_PROP_ID, _("&Properties..."));
}

void MenuEditor::SetMenu(DVD* dvd, Menu* menu, int tsi, int pgci) {
	DoSelect();
	m_dvd = dvd;
	m_menu = menu;
	m_tsi = tsi;
	m_pgci = pgci;
	if (m_menu) {
		SetSVG(m_menu->GetSVG());
		ShowSafeTV();
		ShowGrid();
	} else
		Clear();
	Refresh();
}

bool MenuEditor::SetBackground(wxString fname) {
	if (!m_menu)
		return false;

	if (wxImage::FindHandler(fname.AfterLast(wxT('.')).Lower(), BITMAP_TYPE_ANY)) {
		wxImage img;
		if (!img.LoadFile(fname))
			return false;
	} else if (wxThumbnails::IsVideo(fname)) {
		wxFfmpegMediaDecoder decoder;
		if (!decoder.Load(fname)) {
			wxLogError(fname + _(": unknown format"));
			return false;
		}
	}

	if (m_menu->GetBackground() != fname) {
		m_menu->SetBackground(fname);
		Update();
	}

	return true;
}

void MenuEditor::SetBackgroundColour(wxColour colour) {
	if (!m_menu)
		return;

	if (m_menu->GetBackgroundColour() != colour) {
		m_menu->SetBackgroundColour(colour);
		Update();
	}
}

bool MenuEditor::AddImage(wxString fname, int x, int y) {
	if (!m_menu)
		return false;
	wxImage img;
	if (!img.LoadFile(fname))
		return false;

	DoSelect(m_menu->AddImage(fname, x, y), true, true);
	return true;
}

bool MenuEditor::AddText(wxString text, int x, int y) {
	if (!m_menu)
		return false;
	DoSelect(m_menu->AddText(text, x, y), true, true);
	return true;
}

bool MenuEditor::AddButton(wxString fname, int x, int y, int pgci, int tsi, bool isMenu, int chapter) {
	if (!m_menu || !wxFileExists(fname))
		return false;
	
	if (m_menu->GetAspectRatio() == ar16_9) {
		if (m_dvd->GetPgcArray(m_tsi, true).GetVideo().GetWidescreen() == wtAUTO && m_menu->GetButtonsCount() >= 12) {
			wxLogError(_("Wide screen DVD menu can contain maximal 12 buttons"));
			return false;
		} else if (m_menu->GetButtonsCount() >= 18) {
			wxLogError(_("Wide screen DVD menu (nopanscan/noletterbox) can contain maximal 18 buttons"));
			return false;
		}
	} else if (m_menu->GetButtonsCount() >= 34) {
		wxLogError(_("DVD menu can contain maximal 34 buttons"));
		return false;
	}

	int btNum = 1;
	while (1) {
		wxString id= wxT("button") + wxString::Format(wxT("%02d"), btNum);
		if (m_doc->GetElementById(id) == NULL)
			break;
		btNum++;
	}

	wxString param;
	tsi = tsi >= -1 ? tsi : (m_tsi > -1 || (m_tsi == -1 && isMenu) ? m_tsi : 0);

	MenuObject tmpObj(NULL, false, fname);
	MenuObjectParam* initParam = tmpObj.GetInitParam();
	if (initParam) {
		Pgc* pgc = m_dvd->GetPgc(tsi, false, pgci >= 0 ? pgci : 0);
		if (initParam->type == wxT("image")) {
			if (pgc)
				param = pgc->GetVideoFrameUri(chapter);
		} else if (pgci >= 0 && !isMenu) {
			if (pgc)
				param = wxFileName(pgc->GetVideoFrameUri(chapter)).GetName();
		} else {
			if (isMenu) {
				param = wxString::Format(tsi == -1 ? _("VMGM menu %d") : _("Menu %d"), pgci+1);
			} else
				param = wxString::Format(_("button%d"), btNum);
		}
	}
	// set action
	wxString objId = m_menu->AddObject(fname, param, x, y);
	MenuObject* obj = m_menu->GetObject(objId);
	obj->GetAction().SetTsi(tsi == m_tsi ? -2 : tsi);
	obj->GetAction().SetPgci(pgci >= 0 ? pgci : 0);
	obj->GetAction().SetMenu(isMenu);
	obj->GetAction().SetChapter(chapter);
	
	// fix position
	wxRect rect = obj->GetBBox();
	x = obj->GetX() >> m_grid << m_grid;
	y = obj->GetY() >> m_grid << m_grid;
	int maxWidth = (int) m_doc->GetRootElement()->GetWidth().GetBaseVal();
	int maxHeight = (int) m_doc->GetRootElement()->GetHeight().GetBaseVal();
	if (x < 0)
		x = 0;
	else if (x + rect.width >= maxWidth)
		x = maxWidth - rect.width - 1;
	if (y < 0)
		y = 0;
	else if (y + rect.height >= maxHeight)
		y = maxHeight - rect.height - 1;
	obj->SetX(x);
	obj->SetY(y);
	
	// select
	DoSelect(objId, true, true);
	return true;
}

bool MenuEditor::AddObject(wxString fname, int x, int y) {
	if (!m_menu || !wxFileExists(fname))
		return false;

	wxString param;

	MenuObject obj(NULL, false, fname);
	MenuObjectParam* initParam = obj.GetInitParam();
	if (initParam) {
		if (initParam->type == wxT("text") || initParam->type == wxT("string"))
			param = wxGetTextFromUser(_("Please type in text to insert"), _("Input text"), _T("Text"), this);
		else if (initParam->type == wxT("image"))
			param = wxFileSelector( _ ("Please choose an image to insert") , wxT("") ,wxGetCwd (), wxT(""),
					_("Image Files ") + wxImage::GetImageExtWildcard(), wxFD_OPEN|wxFD_FILE_MUST_EXIST);
		else
			param = _("Text");
		if (!param.length())
			return false;
	}

	DoSelect(m_menu->AddObject(fname, param, x, y), true, true);
	return true;
}

wxString MenuEditor::HitTest(int x, int y) {
	x = (int) (x / GetScaleX());
	y = (int) (y / GetScaleY());
	wxString id;
	// at first try to find a button
	bool button = true;
	wxSVGUseElement* child = (wxSVGUseElement*) m_doc->GetElementById(wxT("buttons"))->GetChildren();
	while (child || button) {
		if (!child) {
			if (id.length() > 0)
				break;
			// no button was found, try to find a object
			child = (wxSVGUseElement*) m_doc->GetElementById(wxT("objects"))->GetChildren();
			button = false;
			continue;
		}
		if (child->GetType() == wxSVGXML_ELEMENT_NODE && child->GetDtd() == wxSVG_USE_ELEMENT) {
			wxSVGPoint p = wxSVGPoint(x, y);
			if (child->GetTransform().GetBaseVal().size() > 0) {
				wxSVGMatrix matrix;
				child->UpdateMatrix(matrix);
				p = p.MatrixTransform(matrix.Inverse());
			}
			wxSVGRect bbox = wxSVGRect(child->GetX().GetAnimVal(), child->GetY().GetAnimVal(),
					child->GetWidth().GetAnimVal(), child->GetHeight().GetAnimVal());
			if (!bbox.IsEmpty()
					&& p.GetX() >= bbox.GetX() && p.GetX() <= bbox.GetX() + bbox.GetWidth()
					&& p.GetY() >= bbox.GetY() && p.GetY() <= bbox.GetY() + bbox.GetHeight()) {
				id = child->GetId();
			}
		}
		child = (wxSVGUseElement*) child->GetNext();
	}
	return id;
}

void MenuEditor::DoSelect(wxString id, bool refresh, bool sendEvent, bool ctrlDown) {
	if (!m_menu || !m_doc) {
		m_selectedIds.clear();
		return;
	}
	
	// remove old selection
	if (!ctrlDown) {
		wxRect rect;
		for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
			wxSVGRectElement* selRect = (wxSVGRectElement*) m_doc->GetElementById(wxT("selection_") + *it);
			if (selRect) {
				if (refresh)
					rect.Union(ScaleRect(selRect->GetBBox(wxSVG_COORDINATES_VIEWPORT), GetScaleX(), GetScaleY())
							.Inflate(8));
				selRect->GetParent()->RemoveChild(selRect);
			}
		}
		m_selectedIds.clear();
		if (refresh && !rect.IsEmpty())
			RefreshRect(rect.Inflate(8));
	} 
	
	if (m_selectedIds.Index(id) != wxNOT_FOUND) {
		wxSVGRectElement* selRect = (wxSVGRectElement*) m_doc->GetElementById(wxT("selection_") + id);
		if (selRect) {
			if (refresh)
				RefreshRect(ScaleRect(selRect->GetBBox(wxSVG_COORDINATES_VIEWPORT), GetScaleX(), GetScaleY())
						.Inflate(8));
			selRect->GetParent()->RemoveChild(selRect);
		}
		m_selectedIds.Remove(id);
		return;
	}
	
	// select
	if (id.length()) {
		m_selectedIds.push_back(id);
		// create selection rectangle
		wxRect rect = CreateSelection(id);
		if (refresh && !rect.IsEmpty())
			RefreshRect(rect.Inflate(8));
	}

	if (sendEvent)
		SendChangedEvent();
}

wxRect MenuEditor::UpdateSelectionRect(int x, int y) {
	x = (int) round(x / GetScaleX());
	y = (int) round(y / GetScaleY());
	wxRect refreshRect;
	wxSVGRectElement* selRect = (wxSVGRectElement*) m_doc->GetRootElement()->GetElementById(wxT("menuObjSelection"));
	if (selRect == NULL) {
		selRect = new wxSVGRectElement;
		selRect->SetId(wxT("menuObjSelection"));
		selRect->SetStroke(wxSVGPaint(255, 0, 0));
		selRect->SetFill(wxSVGPaint());
		selRect->SetX(x);
		selRect->SetY(y);
		selRect->SetWidth(1);
		selRect->SetHeight(1);
		m_doc->GetRootElement()->AppendChild(selRect);
	} else
		refreshRect = ScaleRect(selRect->GetBBox(wxSVG_COORDINATES_VIEWPORT), GetScaleX(), GetScaleY());
	selRect->SetWidth(x - selRect->GetX().GetAnimVal().GetValue());
	selRect->SetHeight(y - selRect->GetY().GetAnimVal().GetValue());
	selRect->SetCanvasItem(NULL);
	
	return refreshRect.Union(ScaleRect(selRect->GetBBox(wxSVG_COORDINATES_VIEWPORT), GetScaleX(), GetScaleY()));
}

wxRect MenuEditor::PerformSelection() {
	wxSVGRectElement* selRectElem = (wxSVGRectElement*) m_doc->GetRootElement()->GetElementById(wxT("menuObjSelection"));
	if (selRectElem == NULL) {
		return wxRect(0, 0, 0, 0);
	}
	wxSVGRect bbox = selRectElem->GetBBox(wxSVG_COORDINATES_VIEWPORT);
	wxRect selRect = wxRect((int) bbox.GetX(), (int) bbox.GetY(), (int) bbox.GetWidth(), (int) bbox.GetHeight());
	wxRect refreshRect = ScaleRect(bbox, GetScaleX(), GetScaleY());;
	selRectElem->GetParent()->RemoveChild(selRectElem);
	

	// select all objects
	for (unsigned int i = 0; i < m_menu->GetObjectsCount(); i++) {
		MenuObject* obj = m_menu->GetObject(i);
		wxString id = obj->GetId();
		if (obj->GetBBox().Intersects(selRect) && m_selectedIds.Index(id) == wxNOT_FOUND) {
			m_selectedIds.push_back(id);
			refreshRect.Union(CreateSelection(id));
		}
	}
	return refreshRect;
}

wxRect MenuEditor::CreateSelection(wxString id) {
	MenuObject* obj = m_menu->GetObject(id);
	if (!obj)
		return wxRect();
	
	wxRect refreshRect;
	wxSVGRectElement* selRect = (wxSVGRectElement*) m_doc->GetRootElement()->GetElementById(wxT("selection_") + id);
	if (selRect == NULL) {
		selRect = new wxSVGRectElement;
		selRect->SetId(wxT("selection_") + id);
		selRect->SetStroke(wxSVGPaint(255, 0, 0));
		selRect->SetFill(wxSVGPaint());
		m_doc->GetRootElement()->AppendChild(selRect);
	} else
		refreshRect = ScaleRect(selRect->GetBBox(wxSVG_COORDINATES_VIEWPORT), GetScaleX(), GetScaleY());
	wxRect bbox = obj->GetBBox();
	selRect->SetX(bbox.GetX());
	selRect->SetY(bbox.GetY());
	selRect->SetWidth(bbox.GetWidth());
	selRect->SetHeight(bbox.GetHeight());
	if (obj->GetAngle() != 0 && obj->GetObjectParam(wxT("rotation")) == NULL) {
		selRect->SetTransform(wxSVGAnimatedTransformList());
		selRect->Rotate(obj->GetAngle(), bbox.GetX() + bbox.GetWidth()/2, bbox.GetY() + bbox.GetHeight()/2);
	}
	selRect->SetCanvasItem(NULL);
	
	return refreshRect.Union(ScaleRect(selRect->GetBBox(wxSVG_COORDINATES_VIEWPORT), GetScaleX(), GetScaleY()));
}

void MenuEditor::RefreshSelected() {
	wxRect rect;
	for (wxArrayString::iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++)
		rect.Union(CreateSelection(*it));
	if (!rect.IsEmpty())
		RefreshRect(rect.Inflate(8));
}

void MenuEditor::OnMouseLeftButton(wxMouseEvent &event) {
	if (!m_menu) {
		event.Skip();
		return;
	}
	if (!event.LeftDown() && m_action == eaSELECT_RECT) {
		wxRect rect = PerformSelection();
		RefreshRect(rect.Inflate(8));
		return;
	}
	if (m_doAction) {
		m_doAction = false;
		if (m_action != eaNONE && m_action != eaSELECT && m_action != eaSELECT_RECT)
			SendChangedEvent();
	}
	int x = event.GetX();
	int y = event.GetY();
	m_action = GetEditorAction(event.GetX(), event.GetY());
	m_oldx = x;
	m_oldy = y;
	m_doCopy = event.ControlDown();
	if ((event.LeftDown() && !event.ControlDown() && m_action == eaNONE)
			|| (!event.LeftDown() && event.ControlDown() && !m_copied)) {
		// select object
		DoSelect(HitTest(x, y), true, false, event.ControlDown());
		// change mouse cursor
		event.m_leftDown = false;
		OnMouseMove(event);
		if (GetSelectedIds().size() > 0) {
			m_action = eaSELECT;
			m_doAction = true;
		}
	}
	m_copied = false;
	event.Skip();
}

void MenuEditor::OnMouseMove(wxMouseEvent &event) {
	if (!m_menu || event.RightDown())
		return;

	int x = event.GetX();
	int y = event.GetY();

	if (!event.LeftIsDown()) {
		m_action = GetEditorAction(x, y);
		// change mouse cursor
		SetMouseCursor(m_action);
		// object pointed
		wxString pointed = HitTest(x, y);
		if (GetPointed() != pointed) {
			m_pointed = pointed;
			wxCommandEvent evt(EVT_COMMAND_MENUEDITOR_OBJECT_POINTED, this->GetId());
			GetEventHandler()->ProcessEvent(evt);
		}
		return;
	}

	// left mouse button is down => move, resize or select
	if (m_action == eaMOVE || m_action == eaSELECT) { // move Object
		int x1 = x - m_oldx;
		int y1 = y - m_oldy;
		if (MoveObject(x1, y1)) {
			RefreshSelected();
			m_doAction = true;
		}
		m_oldx += x1;
		m_oldy += y1;
		m_action = eaMOVE;
	} else if (m_action == eaROTATE) { //rotate Object
		if (RotateObject(x, y)) {
			RefreshSelected();
			m_doAction = true;
			m_oldx = x;
			m_oldy = y;
		}
	} else if (m_action == eaNONE || m_action == eaSELECT_RECT) { // rectangle selection
		wxRect rect = UpdateSelectionRect(x, y);
		RefreshRect(rect.Inflate(8));
		m_action = eaSELECT_RECT;
		m_doAction = true;
	} else { // m_action=TL,TR,BL,BR => resize
		// resize object
		if (ResizeObject(x, y, m_action)) {
			DoSelect(GetSelected(), true); // refresh
			m_doAction = true;
		}
	}
}

void MenuEditor::OnMouseRightButton(wxMouseEvent &event) {
	SetFocus();
	if (event.RightDown() || !m_menu)
		return;
	// select Object
	m_actionObject = HitTest(event.GetX(), event.GetY());
	if (m_selectedIds.Index(m_actionObject) == wxNOT_FOUND)
		DoSelect(m_actionObject, true);
	m_menuPos = event.GetPosition();
	if (event.RightUp())
		PopupMenu(m_selectedIds.size() ? m_objMenu : m_pmenu, event.GetPosition());
}

EditorAction MenuEditor::GetEditorAction(int x, int y) {
	if (!m_menu || m_selectedIds.size() == 0)
		return eaNONE;
	
	m_actionObject = HitTest(x, y);
	
	if (m_selectedIds.size() > 1) {
		if (m_selectedIds.Index(m_actionObject) == wxNOT_FOUND)
			return eaNONE; // outside of the selected objects
		return eaMOVE; // inside of the selected objects
	}

	// test all covered object
	if (m_actionObject != GetSelected())
		return eaNONE; // outside of the selected object
	
	MenuObject* obj = m_menu->GetObject(m_actionObject);
	if (!obj)
		return eaNONE;
	
	if (obj->GetAngle() != 0) {
		wxSVGMatrix matrix;
		obj->UpdateMatrix(matrix);
		wxSVGPoint p(x / GetScaleX(), y / GetScaleY());
		p = p.MatrixTransform(matrix.Inverse());
		x = (int) round(p.GetX() * GetScaleX());
		y = (int) round(p.GetY() * GetScaleY());
	}
	
	wxRect rect = ScaleRect(obj->GetBBox(), GetScaleX(), GetScaleY());
	wxRect rect2 = ((const wxRect) rect).Deflate(rect.GetWidth() > 24 ? 4 : 2, rect.GetHeight() > 24 ? 4 : 2);
	if (!rect2.Contains(x, y)) {
		int x1 = rect.x + rect.width / 2;
		int y1 = rect.y + rect.height / 2;
		if ((x < x1) && (y < y1))
			return eaRESIZE_TL; // top-left
		else if ((x > x1) && (y < y1))
			return eaRESIZE_TR; // top-right
		else if ((x < x1) && (y > y1))
			return eaRESIZE_BL; // bottom-left
		else if ((x > x1) && (y > y1))
			return eaRESIZE_BR; // bottom-right
	}
	if (rect2.GetWidth() > 8 && rect2.GetHeight() > 8) {
		wxRect rectMove1 = ((const wxRect) rect2).Deflate(rect2.GetWidth() > 24 ? 8 : 4, 0);
		wxRect rectMove2 = ((const wxRect) rect2).Deflate(0, rect2.GetHeight() > 24 ? 8 : 4);
		if (!rectMove1.Contains(x, y) && !rectMove2.Contains(x, y)) {
			return eaROTATE;
		}
	}
	return eaMOVE; // inside of the selected object
}

void MenuEditor::SetMouseCursor(EditorAction tt) {
	switch (tt) {
		case eaNONE:
		case eaSELECT_RECT: SetCursor(wxCursor(wxCURSOR_ARROW)); break;
#ifdef __WXMSW__
		case eaMOVE:
		case eaSELECT: SetCursor(wxCursor(_T("wxCURSOR_MOVE"),wxBITMAP_TYPE_CUR_RESOURCE)); break;
		case eaRESIZE_TL:
		case eaRESIZE_BR: SetCursor(wxCursor(wxCURSOR_SIZENWSE)); break;
		case eaRESIZE_TR:
		case eaRESIZE_BL: SetCursor(wxCursor(wxCURSOR_SIZENESW)); break;
		case eaROTATE: SetCursor(wxCursor(_T("wxCURSOR_ROTATE"),wxBITMAP_TYPE_CUR_RESOURCE)); break;
#else
		case eaMOVE:
		case eaSELECT: SetCursor(wxImage(CURSOR_FILE(_T("move.cur"))));break;
		case eaRESIZE_TL:
		case eaRESIZE_BR: SetCursor(wxImage(CURSOR_FILE(_T("nwse.cur"))));break;
		case eaRESIZE_TR:
		case eaRESIZE_BL: SetCursor(wxImage(CURSOR_FILE(_T("nesw.cur"))));break;
		case eaROTATE: SetCursor(wxImage(CURSOR_FILE(_T("rotate.cur"))));break;
#endif
	}
}

bool MenuEditor::MoveObject(int& x, int& y) {
	if (!m_menu || m_selectedIds.size() < 1)
		return false;
	
	if (m_doCopy) {
		wxArrayString objIds;
		for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
			MenuObject* obj = m_menu->GetObject(*it);
			wxRect rect = obj->GetBBox();
			wxString id = m_menu->AddObject(obj->GetXML(DVDSTYLER_XML, m_dvd, ssmNORMAL, true), true);
			if (id.length() > 0) {
				objIds.Add(id);
				obj = m_menu->GetObject(id);
				if (obj) {
					obj->SetX(rect.x);
					obj->SetY(rect.y);
				}
				if (m_pointed == *it)
					m_pointed = id;
			}
		}
		for (wxArrayString::const_iterator it = objIds.begin(); it != objIds.end(); it++)
			DoSelect(*it, true, false, it != objIds.begin());
		wxCommandEvent evt(EVT_COMMAND_MENUEDITOR_OBJECT_POINTED, this->GetId());
		GetEventHandler()->ProcessEvent(evt);
		
		m_doCopy = false;
		m_copied = true;
	}

	x = (int) round(x / GetScaleX());
	y = (int) round(y / GetScaleY());
	bool res = MoveObjectInt(x, y);
	x = (int) round(x * GetScaleX());
	y = (int) round(y * GetScaleY());
	return res;
}

bool MenuEditor::MoveObjectInt(int& x, int& y) {
	MenuObject* obj = m_menu->GetObject(m_actionObject);
	if (!obj)
		return false;
	
	// span to grid
	if (m_grid > 0) {
		x = ((obj->GetX() + x) >> m_grid << m_grid) - obj->GetX();
		y = ((obj->GetY() + y) >> m_grid << m_grid) - obj->GetY();
	}
	
	for (wxArrayString::iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		obj = m_menu->GetObject(*it);
		if (!obj)
			continue;
		
		wxRect rect = obj->GetBBox();
		
		int maxWidth = (int) m_doc->GetRootElement()->GetWidth().GetBaseVal();
		int maxHeight = (int) m_doc->GetRootElement()->GetHeight().GetBaseVal();
		if (obj->IsButton()) {
			if (rect.x + x < 0)
				x = rect.x < 0 ? 0 : -rect.x;
			else if (rect.x + rect.width + x >= maxWidth)
				x = rect.x + rect.width >= maxWidth ? 0 : maxWidth - rect.x - rect.width - 1;
			if (rect.y + y < 0)
				y = rect.y < 0 ? 0 : -rect.y;
			else if (rect.y + rect.height + y >= maxHeight)
				y = rect.y + rect.height >= maxHeight ? 0 : maxHeight - rect.y - rect.height - 1;
		} else {
			if (rect.x + x + rect.width < 8)
				x = rect.x + rect.width < 8 ? 0 : 8 - rect.x - rect.width;
			else if (rect.x + x > maxWidth - 8)
				x = rect.x > maxWidth - 8 ? 0 : maxWidth - 8 - rect.x;
			if (rect.y + y + rect.height < 8)
				y = rect.y + rect.height < 8 ? 0 : 8 - rect.height - rect.y;
			else if (rect.y + y > maxHeight - 8)
				y = rect.y > maxHeight - 8 ? 0 : maxHeight - 8 - rect.y;
		}
	}
	
	if (x == 0 && y == 0)
		return false;
	
	// update objects
	for (wxArrayString::iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		obj = m_menu->GetObject(*it);
		if (!obj)
			continue;
		obj->SetX(obj->GetX() + x);
		obj->SetY(obj->GetY() + y);
	}
	return true;
}

bool MenuEditor::RotateObject(int x, int y) {
	if (m_selectedIds.size() != 1)
		return false;
	x = (int) round(x / GetScaleX());
	y = (int) round(y / GetScaleY());
	int oldx = (int) round(m_oldx / GetScaleX());
	int oldy = (int) round(m_oldy / GetScaleY());
	MenuObject* obj = m_menu->GetObject(m_actionObject);
	int cx = obj->GetX() + obj->GetWidth() / 2;
	int cy = obj->GetY() + obj->GetHeight() / 2;
	double angleOld = atan2(oldx - cx, oldy - cy) / M_PI * 180;
	double angleNew = atan2(x - cx, y - cy) / M_PI * 180;
	obj->SetAngle(obj->GetAngle() + angleOld - angleNew);
	return true;
}

bool MenuEditor::ResizeObject(int x, int y, EditorAction action) {
	x = (int) round(x/GetScaleX());
	y = (int) round(y/GetScaleY());
	return ResizeObjectInt(x, y, action);
}

bool MenuEditor::ResizeObjectInt(int x, int y, EditorAction action) {
	if (!m_menu || m_selectedIds.size() != 1)
		return false;

	MenuObject* obj = m_menu->GetObject(GetSelected());
	if (!obj)
		return false;
	wxRect rect = obj->GetBBox();

	if ((action == eaRESIZE_BR || action == eaRESIZE_TR) && x < 8)
		x = 8;
	if ((action == eaRESIZE_BL || action == eaRESIZE_TL)
			&& x > (int) m_doc->GetRootElement()->GetWidth().GetBaseVal() - 8)
		x = (int) m_doc->GetRootElement()->GetWidth().GetBaseVal() - 8;
	if ((action == eaRESIZE_BL || action == eaRESIZE_BR) && y < 8)
		y = 8;
	if ((action == eaRESIZE_TL || action == eaRESIZE_TR)
			&& y > (int) m_doc->GetRootElement()->GetHeight().GetBaseVal() - 8)
		y = (int) m_doc->GetRootElement()->GetHeight().GetBaseVal() - 8;
	
	if (obj->GetAngle() != 0) {
		wxSVGMatrix matrix;
		obj->UpdateMatrix(matrix);
		wxSVGPoint p(x, y);
		p = p.MatrixTransform(matrix.Inverse());
		x = p.GetX();
		y = p.GetY();
	}

	int sx, sy;

	if (action == eaRESIZE_BR || action == eaRESIZE_TR)
		sx = x - rect.x + 1;
	else
		sx = rect.x + rect.width - x + 1;

	if (action == eaRESIZE_BR || action == eaRESIZE_BL)
		sy = y - rect.y + 1;
	else
		sy = rect.y + rect.height - y + 1;

	if (sx <= 4)
		sx = 4;
	if (sy <= 4)
		sy = 4;

	obj->SetDefaultSize(false);
	obj->FixSize(sx, sy);

	// set new x, y, width, height
	if (action == eaRESIZE_TL || action == eaRESIZE_BL)
		rect.x += rect.width - sx;
	if (action == eaRESIZE_TL || action == eaRESIZE_TR)
		rect.y += rect.height - sy;
	rect.width = sx;
	rect.height = sy;

	if (obj->GetBBox() != rect) {
		obj->SetRect(rect);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////

void MenuEditor::ShowSafeTV() {
	if (!m_doc)
		return;
	wxSVGGElement* gElem =
    		(wxSVGGElement*) m_doc->GetElementById(wxT("safeTV"));
    if (gElem)
		gElem->GetParent()->RemoveChild(gElem);
    
	if (m_safeTV) {
		int safeWidth = m_menu->GetAspectRatio() == ar4_3
				|| m_dvd->GetPgcArray(m_tsi, true).GetVideo().GetWidescreen() == wtNOPANSCAN ? 48 : 90; // 6.7%/12.5%
		int safeHeight = m_menu->GetAspectRatio() == ar4_3 ? 27 : 20; // 5%
		gElem = new wxSVGGElement;
		gElem->SetId(wxT("safeTV"));
		wxSVGRectElement* rectElem = new wxSVGRectElement;
	    rectElem->SetX(0);
	    rectElem->SetY(0);
	    rectElem->SetWidth(m_menu->GetResolution().GetWidth());
	    rectElem->SetHeight(safeHeight);
	    rectElem->SetStroke(wxSVGPaint());
	    rectElem->SetFill(wxSVGPaint(255, 255, 255));
	    rectElem->SetFillOpacity(0.1);
	    gElem->AppendChild(rectElem);
	    rectElem = (wxSVGRectElement*) rectElem->CloneNode();
	    rectElem->SetY(m_menu->GetResolution().GetHeight()  - safeHeight - 1);
	    gElem->AppendChild(rectElem);
	    rectElem = (wxSVGRectElement*) rectElem->CloneNode();
	    rectElem->SetX(0);
	    rectElem->SetY(safeHeight);
	    rectElem->SetWidth(safeWidth);
	    rectElem->SetHeight(m_menu->GetResolution().GetHeight() - safeHeight*2 - 1);
	    gElem->AppendChild(rectElem);
	    rectElem = (wxSVGRectElement*) rectElem->CloneNode();
	    rectElem->SetX(m_menu->GetResolution().GetWidth() - safeWidth - 1);
	    gElem->AppendChild(rectElem);
		m_doc->GetRootElement()->AppendChild(gElem);
	}
	Refresh();
}

void MenuEditor::ShowGrid() {
	wxSVGGElement* gElem =
    		(wxSVGGElement*) m_doc->GetElementById(wxT("grid"));
    if (gElem)
		gElem->GetParent()->RemoveChild(gElem);
    
	if (m_showGrid && m_grid>0) {
		gElem = new wxSVGGElement;
		gElem->SetId(wxT("grid"));
		wxSVGLineElement* lineElem = new wxSVGLineElement;
	    lineElem->SetY1(0);
	    lineElem->SetY2(m_menu->GetResolution().GetHeight()-1);
	    lineElem->SetStroke(wxSVGPaint(m_gridColour));
	    lineElem->SetFill(wxSVGPaint());
	    lineElem->SetStrokeOpacity(0.1);
	    int step = 1<<m_grid;
	    for (int x = step; x<m_menu->GetResolution().GetWidth()-1; x+=step) {
		    lineElem = (wxSVGLineElement*) lineElem->CloneNode();
		    lineElem->SetX1(x);
		    lineElem->SetX2(x);
		    gElem->AppendChild(lineElem);
	    }
	    lineElem = (wxSVGLineElement*) lineElem->CloneNode();
	    lineElem->SetX1(0);
	    lineElem->SetX2(m_menu->GetResolution().GetWidth()-1);
	    for (int y = step; y<m_menu->GetResolution().GetHeight()-1; y+=step) {
		    lineElem = (wxSVGLineElement*) lineElem->CloneNode();
		    lineElem->SetY1(y);
		    lineElem->SetY2(y);
		    gElem->AppendChild(lineElem);
	    }
	    if (m_gridOverObejcts)
	    	m_doc->GetRootElement()->AppendChild(gElem);
	    else
	    	m_doc->GetRootElement()->InsertBefore(gElem, m_doc->GetElementById(wxT("objects")));
	}
	Refresh();
}

//////////////////////////////////////////////////////////////////////////////

void MenuEditor::OnMouseDClick(wxMouseEvent& event) {
	wxCommandEvent evt;
	OnProperties(evt);
	m_action = eaMOVE;
	SetMouseCursor(m_action);
}

void MenuEditor::OnKeyDown(wxKeyEvent& event) {
	if (!m_menu)
		return;
	switch (event.GetKeyCode()) {
		case WXK_TAB: {
			int idx = m_selectedIds.size() > 0 ? m_menu->GetIndexOfObject(GetSelected()) + 1 : 0;
			if (idx >= (int) m_menu->GetObjectsCount())
				idx = 0;
			if (m_menu->GetObject(idx))
				DoSelect(m_menu->GetObject(idx)->GetId(), true, true);
			break;
		}
		case WXK_DELETE:
			if (m_selectedIds.size() > 0) {
				wxCommandEvent evt;
				OnObjectDelete(evt);
			}
			break;
		case 'A':
			if (event.ControlDown()) {
				// select all objects
				for (unsigned int i = 0; i < m_menu->GetObjectsCount(); i++) {
					MenuObject* obj = m_menu->GetObject(i);
					wxString id = obj->GetId();
					if (m_selectedIds.Index(id) == wxNOT_FOUND) {
						m_selectedIds.push_back(id);
						CreateSelection(id);
					}
				}
				Refresh();
				SendChangedEvent();
			}
			break;
		case 'C':
			if (event.ControlDown() && m_selectedIds.size() == 1) {
				wxCommandEvent evt;
				OnObjectCopy(evt);
			}
			break;
		case 'X':
			if (event.ControlDown() && m_selectedIds.size() == 1) {
				wxCommandEvent evt;
				OnObjectCopy(evt);
				OnObjectDelete(evt);
			}
			break;
		case 'V':
			if (event.ControlDown()) {
				m_menuPos = wxPoint(-1, -1);
				wxCommandEvent evt;
				OnObjectPaste(evt);
			}
			break;
		case WXK_LEFT:
		case WXK_RIGHT:
		case WXK_UP:
		case WXK_DOWN:
			if (m_selectedIds.size() > 0) {
				if (m_actionObject.length() == 0)
					m_actionObject = m_selectedIds[0];
				int key = event.GetKeyCode();
				int x = key == WXK_LEFT ? -(1 << m_grid) : key == WXK_RIGHT ? 1 << m_grid : 0;
				int y = key == WXK_UP ? -(1 << m_grid) : key == WXK_DOWN ? 1 << m_grid : 0;
				MoveObjectInt(x, y);
				RefreshSelected();
			}
			break;
		default:
			event.Skip();
			break;
	}
}

//////////////////////////////////////////////////////////////////////////////

void MenuEditor::OnObjectCopy(wxCommandEvent& WXUNUSED(event)) {
	wxSvgXmlNode* root = new wxSvgXmlNode(wxSVGXML_ELEMENT_NODE, _T("objects"));
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++)
		root->AppendChild(m_menu->GetObject(*it)->GetXML(DVDSTYLER_XML, m_dvd, ssmNORMAL, true));
	CopyXmlToClipboard(root, DATAFORMAT_MENU_OBJECT);
}

void MenuEditor::OnObjectPaste(wxCommandEvent& WXUNUSED(event)) {
	if (wxTheClipboard->IsSupported(wxDataFormat(DATAFORMAT_MENU_OBJECT))) {
		wxSvgXmlDocument doc;
		if (!GetXmlFromClipboard(DATAFORMAT_MENU_OBJECT, doc))
			return;
		wxSvgXmlNode* child = doc.GetRoot()->GetFirstChild();
		wxArrayString objIds;
		int x = 9999;
		int y = 9999;
		while (child) {
			wxString id = m_menu->AddObject(child, true);
			if (id.length() > 0) {
				objIds.Add(id);
				MenuObject* obj = m_menu->GetObject(id);
				if (obj && x > obj->GetX())
					x = obj->GetX();
				if (obj && y > obj->GetY())
					y = obj->GetY();
			}
			child = child->GetNextSibling();
		}
		if (m_menuPos.x != -1 && m_menuPos.y != -1) {
			for (wxArrayString::const_iterator it = objIds.begin(); it != objIds.end(); it++) {
				MenuObject* obj = m_menu->GetObject(*it);
				if (obj) {
					obj->SetX(obj->GetX() - x + (int)(m_menuPos.x/GetScaleX()));
					obj->SetY(obj->GetY() - y + (int)(m_menuPos.y/GetScaleY()));
				}
			}
		}
	} else if (wxTheClipboard->IsSupported(wxDF_TEXT)) {
		wxString text = GetTextFromClipboard();
		if (text.length() == 0)
			return;
		wxString id = m_menu->AddText(text);
		MenuObject* obj = m_menu->GetObject(id);
		if (obj && m_menuPos.x != -1 && m_menuPos.y != -1) {
			obj->SetX((int)(m_menuPos.x/GetScaleX()));
			obj->SetY((int)(m_menuPos.y/GetScaleY()));
		}
	}
	Update();
}

void MenuEditor::OnUpdateUIObjectPaste(wxUpdateUIEvent& event) {
	event.Enable(m_menu && (wxTheClipboard->IsSupported(wxDF_TEXT)
			|| wxTheClipboard->IsSupported(wxDataFormat(DATAFORMAT_MENU_OBJECT))));
}

void MenuEditor::OnObjectCut(wxCommandEvent& event) {
	wxCommandEvent evt;
	OnObjectCopy(evt);
	OnObjectDelete(evt);
}

void MenuEditor::OnObjectDelete(wxCommandEvent& event) {
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++)
		m_menu->RemoveObject(*it);
	DoSelect(wxT(""), true, true);
}

void MenuEditor::OnFirstButton(wxCommandEvent& event) {
	m_menu->SetFirstButton(GetSelected());
	SendChangedEvent();
}

void MenuEditor::OnUpdateUIFirstButton(wxUpdateUIEvent& event) {
	event.Enable(m_selectedIds.size() == 1 && m_menu->GetObject(GetSelected())->IsButton());
	event.Check(m_selectedIds.size() == 1 && m_menu->IsFirstButton(GetSelected()));
}

void MenuEditor::OnObjectToFront(wxCommandEvent& event) {
	m_menu->ObjectToFront(GetSelected());
	DoSelect(GetSelected(), true, true);
}

void MenuEditor::OnObjectForward(wxCommandEvent& event) {
	m_menu->ObjectForward(GetSelected());
	DoSelect(GetSelected(), true, true);
}

void MenuEditor::OnObjectBackward(wxCommandEvent& event) {
	m_menu->ObjectBackward(GetSelected());
	DoSelect(GetSelected(), true, true);
}

void MenuEditor::OnObjectToBack(wxCommandEvent& event) {
	m_menu->ObjectToBack(GetSelected());
	DoSelect(GetSelected(), true, true);
}

void MenuEditor::OnUpdateUIObjectForward(wxUpdateUIEvent& event) {
	event.Enable(m_selectedIds.size() == 1 && !m_menu->IsLastObject(GetSelected()));
}

void MenuEditor::OnUpdateUIObjectBackward(wxUpdateUIEvent& event) {
	event.Enable(m_selectedIds.size() == 1 && !m_menu->IsFirstObject(GetSelected()));
}
void MenuEditor::OnUpdateUIAlign(wxUpdateUIEvent& event) {
	event.Enable(m_selectedIds.size() > 1);
}

int MenuEditor::SnapToGrid(int pos) {
	return m_grid > 0 ? pos >> m_grid << m_grid : pos;
}

void MenuEditor::OnAlignToCenterMH(wxCommandEvent& event) {
	int x1 = 9999;
	int x2 = -9999;
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		if (obj->GetX() < x1)
			x1 = obj->GetX();
		if (obj->GetX() + obj->GetWidth() > x2)
			x2 = obj->GetX() + obj->GetWidth();
	}
	int x = (m_menu->GetResolution().GetWidth() - (x2 - x1))/2 - x1;
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		obj->SetX(SnapToGrid(x + obj->GetX()));
	}
	RefreshSelected();
	SendChangedEvent();
}

void MenuEditor::OnAlignToCenterMV(wxCommandEvent& event) {
	int y1 = 9999;
	int y2 = -9999;
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		if (obj->GetY() < y1)
			y1 = obj->GetY();
		if (obj->GetY() + obj->GetHeight() > y2)
			y2 = obj->GetY() + obj->GetHeight();
	}
	int y = (m_menu->GetResolution().GetHeight() - (y2 - y1))/2 - y1;
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		obj->SetY(SnapToGrid(y + obj->GetY()));
	}
	RefreshSelected();
	SendChangedEvent();
}

void MenuEditor::OnAlignToCenterH(wxCommandEvent& event) {
	int x1 = 9999;
	int x2 = -9999;
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		if (obj->GetX() < x1)
			x1 = obj->GetX();
		if (obj->GetX() + obj->GetWidth() > x2)
			x2 = obj->GetX() + obj->GetWidth();
	}
	int x = (x2 + x1)/2;
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		obj->SetX(x - obj->GetWidth()/2);
	}
	RefreshSelected();
	SendChangedEvent();
}

void MenuEditor::OnAlignToCenterV(wxCommandEvent& event) {
	int y1 = 9999;
	int y2 = -9999;
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		if (obj->GetY() < y1)
			y1 = obj->GetY();
		if (obj->GetY() + obj->GetHeight() > y2)
			y2 = obj->GetY() + obj->GetHeight();
	}
	int y = (y2 + y1)/2;
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		obj->SetY(y - obj->GetHeight()/2);
	}
	RefreshSelected();
	SendChangedEvent();
}

void MenuEditor::OnAlignToLeft(wxCommandEvent& event) {
	int x1 = 9999;
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		if (obj->GetX() < x1)
			x1 = obj->GetX();
	}
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		obj->SetX(x1);
	}
	RefreshSelected();
	SendChangedEvent();
}

void MenuEditor::OnAlignToRight(wxCommandEvent& event) {
	int x1 = 0;
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		if (obj->GetX() + obj->GetWidth() > x1)
			x1 = obj->GetX() + obj->GetWidth();
	}
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		obj->SetX(x1 - obj->GetWidth());
	}
	RefreshSelected();
	SendChangedEvent();
}

void MenuEditor::OnAlignToTop(wxCommandEvent& event) {
	int y1 = 9999;
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		if (obj->GetY() < y1)
			y1 = obj->GetY();
	}
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		obj->SetY(y1);
	}
	RefreshSelected();
	SendChangedEvent();
}

void MenuEditor::OnAlignToBottom(wxCommandEvent& event) {
	int y1 = 0;
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		if (obj->GetY() + obj->GetHeight() > y1)
			y1 = obj->GetY() + obj->GetHeight();
	}
	for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
		MenuObject* obj = m_menu->GetObject(*it);
		obj->SetY(y1 - obj->GetHeight());
	}
	RefreshSelected();
	SendChangedEvent();
}

void MenuEditor::OnProperties(wxCommandEvent& event) {
	if (!m_menu)
		return;
	if (m_selectedIds.size() == 0) {
		// show menu properties
		MenuPropDlg propDlg(this, m_dvd, m_tsi, m_pgci);
		if (propDlg.ShowModal() == wxID_OK) {
			ShowSafeTV();
			ShowGrid(); 
			SendChangedEvent();
			Refresh(true, NULL);
		}
	} else {
		// show object properties
		MenuObject* actionObj = m_menu->GetObject(m_actionObject);
		if (!actionObj)
			return;
		MenuObjectPropDlg propDlg(this, m_actionObject, m_selectedIds.size() > 1, m_menu, m_dvd, m_tsi, m_pgci);
		if (propDlg.ShowModal() == wxID_OK) {
			for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
				MenuObject* obj = m_menu->GetObject(*it);
				if (obj->GetId() == m_actionObject)
					continue;
				for (int i = 0; i < actionObj->GetObjectParamsCount(); i++) {
					MenuObjectParam* param = actionObj->GetObjectParam(i);
					if (param->changeable && param->type == wxT("colour")) {
						obj->SetParamColour(param->name, param->normalColour);
						obj->SetParamColour(param->name, param->highlightedColour, mbsHIGHLIGHTED);
						obj->SetParamColour(param->name, param->selectedColour, mbsSELECTED);
					} else if (param->type == wxT("colour")) {
						obj->SetParam(param->name, actionObj->GetParam(param->name));
						obj->SetParam(param->name, actionObj->GetParam(param->name, wxT("-opacity")), wxT("-opacity"));
					} else if (param->type == wxT("text")) {
						obj->SetParamFont(param->name, actionObj->GetParamFont(param->name));
					} else if (param->type != _T("image") && param->attribute.Find(wxT('#')) == wxNOT_FOUND) {
						obj->SetParam(param->name, actionObj->GetParam(param->name));
					}
				}
				obj->UpdateSize();
			}
			RefreshSelected();
			SendChangedEvent();
		}
	}
}

void MenuEditor::OnUpdateUIProperties(wxUpdateUIEvent& event) {
	bool enable = true;
	if (m_selectedIds.size() > 0) {
		wxString type = m_menu->GetObject(GetSelected())->GetFileName();
		for (wxArrayString::const_iterator it = m_selectedIds.begin(); it != m_selectedIds.end(); it++) {
			if (type != m_menu->GetObject(*it)->GetFileName()) {
				enable = false;
				break;
			}
		}
	}
	event.Enable(enable);
}

void MenuEditor::OnSafeTV(wxCommandEvent& event) {
	m_safeTV = !m_safeTV;
	ShowSafeTV();
}

void MenuEditor::OnShowGrid(wxCommandEvent& event) {
	m_showGrid = !m_showGrid;
	ShowGrid();
}

void MenuEditor::OnShowGridOverObjects(wxCommandEvent& event) {
	m_gridOverObejcts = !m_gridOverObejcts;
	ShowGrid();
}

void MenuEditor::OnGridColour(wxCommandEvent& event) {
	wxColourData data;
	data.SetColour(m_gridColour);
	wxColourDialog dlg(this, &data);
	if (dlg.ShowModal() == wxID_OK) {
		m_gridColour = dlg.GetColourData().GetColour();
		m_showGrid = true;
		m_gridMenu->FindItem(MENUEDITOR_GRIDMENU_SHOW_ID)->Check();
		ShowGrid();
	}
}

void MenuEditor::OnGridMenu(wxCommandEvent& event) {
	long id = event.GetId() - MENUEDITOR_GRIDMENU_ITEM_ID;
	m_grid = id;
	if (m_grid > 0)
		m_grid += 1;
	if (m_showGrid)
		ShowGrid();
}

void MenuEditor::OnAddObject(wxCommandEvent& event) {
	long n = event.GetId() - MENUEDITOR_ADDMENU_OBJECT_ID;
	AddObject(m_addMenuObjects[n], (int) (m_menuPos.x / GetScaleX()), (int) (m_menuPos.y / GetScaleY()));
}

void MenuEditor::SendChangedEvent(bool updateAllItems) {
	wxCommandEvent evt(EVT_COMMAND_MENUEDITOR_MENU_CHANGED, this->GetId());
	evt.SetInt(updateAllItems);
	GetEventHandler()->ProcessEvent(evt);
}
