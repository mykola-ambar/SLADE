
/*******************************************************************
 * SLADE - It's a Doom Editor
 * Copyright (C) 2008-2014 Simon Judd
 *
 * Email:       sirjuddington@gmail.com
 * Web:         http://slade.mancubus.net
 * Filename:    LinePropsPanel.cpp
 * Description: UI for editing line properties - has tabs for flags,
 *              special, args, sides and other properties
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *******************************************************************/


/*******************************************************************
 * INCLUDES
 *******************************************************************/
#include "Main.h"
#include "LinePropsPanel.h"
#include "MapEditor/GameConfiguration/GameConfiguration.h"
#include "MapEditor/MapEditor.h"
#include "MapEditor/UI/Dialogs/ActionSpecialDialog.h"
#include "MapObjectPropsPanel.h"
#include "SidePropsPanel.h"
#include "UI/NumberTextCtrl.h"
#include "UI/STabCtrl.h"
#include "MapEditor/MapEditContext.h"


/*******************************************************************
 * LINEPROPSPANEL CLASS FUNCTIONS
 *******************************************************************/

/* LinePropsPanel::LinePropsPanel
 * LinePropsPanel class constructor
 *******************************************************************/
LinePropsPanel::LinePropsPanel(wxWindow* parent) : PropsPanelBase(parent)
{
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(sizer);

	// Tabs
	stc_tabs = new STabCtrl(this, false);
	sizer->Add(stc_tabs, 1, wxEXPAND);

	// General tab
	stc_tabs->AddPage(setupGeneralTab(), "General");

	// Special tab
	stc_tabs->AddPage(setupSpecialTab(), "Special");

	// Args tab
	if (MapEditor::editContext().mapDesc().format != MAP_DOOM)
	{
		panel_args = new ArgsPanel(stc_tabs);
		stc_tabs->AddPage(panel_args, "Args");
		panel_special->setArgsPanel(panel_args);
	}

	// Front side tab
	panel_side1 = new SidePropsPanel(stc_tabs);
	stc_tabs->AddPage(panel_side1, "Front Side");

	// Back side tab
	panel_side2 = new SidePropsPanel(stc_tabs);
	stc_tabs->AddPage(panel_side2, "Back Side");

	// All properties tab
	mopp_all_props = new MapObjectPropsPanel(stc_tabs, true);
	mopp_all_props->hideFlags(true);
	mopp_all_props->hideTriggers(true);
	mopp_all_props->hideProperty("special");
	mopp_all_props->hideProperty("arg0");
	mopp_all_props->hideProperty("arg1");
	mopp_all_props->hideProperty("arg2");
	mopp_all_props->hideProperty("arg3");
	mopp_all_props->hideProperty("arg4");
	mopp_all_props->hideProperty("texturetop");
	mopp_all_props->hideProperty("texturemiddle");
	mopp_all_props->hideProperty("texturebottom");
	mopp_all_props->hideProperty("offsetx");
	mopp_all_props->hideProperty("offsety");
	mopp_all_props->hideProperty("id");
	stc_tabs->AddPage(mopp_all_props, "Other Properties");

	// Bind events
	cb_override_special->Bind(wxEVT_CHECKBOX, &LinePropsPanel::onOverrideSpecialChecked, this);
}

/* LinePropsPanel::~LinePropsPanel
 * LinePropsPanel class destructor
 *******************************************************************/
LinePropsPanel::~LinePropsPanel()
{
	mopp_all_props->clearGrid();
}

/* LinePropsPanel::setupGeneralTab
 * Creates and sets up the 'General' properties tab panel
 *******************************************************************/
wxPanel* LinePropsPanel::setupGeneralTab()
{
	wxPanel* panel_flags = new wxPanel(stc_tabs, -1);
	int map_format = MapEditor::editContext().mapDesc().format;

	// Setup sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	panel_flags->SetSizer(sizer);

	// Flags
	wxStaticBox* frame_flags = new wxStaticBox(panel_flags, -1, "Flags");
	wxStaticBoxSizer* sizer_flags = new wxStaticBoxSizer(frame_flags, wxVERTICAL);
	sizer->Add(sizer_flags, 0, wxEXPAND|wxALL, 4);

	// Init flags
	wxGridBagSizer* gb_sizer_flags = new wxGridBagSizer(4, 4);
	sizer_flags->Add(gb_sizer_flags, 1, wxEXPAND|wxALL, 4);
	int row = 0;
	int col = 0;

	// Get all UDMF properties
	vector<udmfp_t> props = theGameConfiguration->allUDMFProperties(MOBJ_LINE);
	sort(props.begin(), props.end());

	// UDMF flags
	if (map_format == MAP_UDMF)
	{
		// Get all udmf flag properties
		vector<string> flags;
		for (unsigned a = 0; a < props.size(); a++)
		{
			if (props[a].property->isFlag())
			{
				flags.push_back(props[a].property->getName());
				udmf_flags.push_back(props[a].property->getProperty());
			}
		}

		// Add flag checkboxes
		int flag_mid = flags.size() / 3;
		if (flags.size() % 3 == 0) flag_mid--;
		for (unsigned a = 0; a < flags.size(); a++)
		{
			wxCheckBox* cb_flag = new wxCheckBox(panel_flags, -1, flags[a], wxDefaultPosition, wxDefaultSize, wxCHK_3STATE);
			gb_sizer_flags->Add(cb_flag, wxGBPosition(row++, col), wxDefaultSpan, wxEXPAND);
			cb_flags.push_back(cb_flag);

			if (row > flag_mid)
			{
				row = 0;
				col++;
			}
		}
	}

	// Non-UDMF flags
	else
	{
		// Add flag checkboxes
		int flag_mid = theGameConfiguration->nLineFlags() / 3;
		if (theGameConfiguration->nLineFlags() % 3 == 0) flag_mid--;
		for (int a = 0; a < theGameConfiguration->nLineFlags(); a++)
		{
			wxCheckBox* cb_flag = new wxCheckBox(panel_flags, -1, theGameConfiguration->lineFlag(a), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE);
			gb_sizer_flags->Add(cb_flag, wxGBPosition(row++, col), wxDefaultSpan, wxEXPAND);
			cb_flags.push_back(cb_flag);

			if (row > flag_mid)
			{
				row = 0;
				col++;
			}
		}
	}

	gb_sizer_flags->AddGrowableCol(0, 1);
	gb_sizer_flags->AddGrowableCol(1, 1);
	gb_sizer_flags->AddGrowableCol(2, 1);

	// Sector tag
	if (map_format == MAP_DOOM)
	{
		wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);

		hbox->Add(new wxStaticText(panel_flags, -1, "Sector Tag:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 8);
		hbox->Add(text_tag = new NumberTextCtrl(panel_flags), 1, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
		btn_new_tag = new wxButton(panel_flags, -1, "New Tag");
		hbox->Add(btn_new_tag, 0, wxEXPAND);

		// Bind event
		btn_new_tag->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &LinePropsPanel::onBtnNewTag, this);
	}

	// Id
	if (map_format == MAP_UDMF)
	{
		wxBoxSizer* hbox = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(hbox, 0, wxEXPAND|wxALL, 4);

		hbox->Add(new wxStaticText(panel_flags, -1, "Line ID:"), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 8);
		hbox->Add(text_id = new NumberTextCtrl(panel_flags), 1, wxALIGN_CENTER_VERTICAL|wxRIGHT, 4);
		hbox->Add(btn_new_id = new wxButton(panel_flags, -1, "New ID"), 0, wxEXPAND);

		// Bind event
		btn_new_id->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &LinePropsPanel::onBtnNewId, this);
	}

	return panel_flags;
}

/* LinePropsPanel::setupSpecialTab
 * Creates and sets up the 'Special' properties tab
 *******************************************************************/
wxPanel* LinePropsPanel::setupSpecialTab()
{
	wxPanel* panel = new wxPanel(stc_tabs, -1);

	// Setup sizer
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(sizer);

	// Action special panel
	panel_special = new ActionSpecialPanel(panel);
	sizer->Add(panel_special, 1, wxEXPAND|wxALL, 4);

	// 'Override Special' checkbox
	cb_override_special = new wxCheckBox(panel, -1, "Override Action Special");
	cb_override_special->SetToolTip("Differing action specials detected, tick this to set the action special for all selected lines");
	sizer->Add(cb_override_special, 0, wxEXPAND|wxLEFT|wxRIGHT|wxBOTTOM, 4);

	return panel;
}

/* LinePropsPanel::openLines
 * Loads values from all lines/sides in [lines]
 *******************************************************************/
void LinePropsPanel::openObjects(vector<MapObject*>& lines)
{
	if (lines.empty())
		return;

	int map_format = MapEditor::editContext().mapDesc().format;

	// Load flags
	if (map_format == MAP_UDMF)
	{
		bool val = false;
		for (unsigned a = 0; a < udmf_flags.size(); a++)
		{
			if (MapObject::multiBoolProperty(lines, udmf_flags[a], val))
				cb_flags[a]->SetValue(val);
			else
				cb_flags[a]->Set3StateValue(wxCHK_UNDETERMINED);
		}
	}
	else
	{
		for (int a = 0; a < theGameConfiguration->nLineFlags(); a++)
		{
			// Set initial flag checked value
			cb_flags[a]->SetValue(theGameConfiguration->lineFlagSet(a, (MapLine*)lines[0]));

			// Go through subsequent lines
			for (unsigned b = 1; b < lines.size(); b++)
			{
				// Check for mismatch			
				if (cb_flags[a]->GetValue() != theGameConfiguration->lineFlagSet(a, (MapLine*)lines[b]))
				{
					// Set undefined
					cb_flags[a]->Set3StateValue(wxCHK_UNDETERMINED);
					break;
				}
			}
		}
	}

	// Load special/trigger(s)/args
	panel_special->openLines(lines);

	// Check action special
	int special = lines[0]->intProperty("special");
	for (unsigned a = 1; a < lines.size(); a++)
	{
		if (lines[a]->intProperty("special") != special)
		{
			special = -1;
			break;
		}
	}
	if (special >= 0)
	{
		// Enable special tab
		cb_override_special->Enable(false);
		cb_override_special->Show(false);
		//panel_special->Enable(true);
	}
	else
	{
		cb_override_special->Enable(true);
		cb_override_special->SetValue(false);
		cb_override_special->Enable(true);
		//panel_special->Enable(false);
	}

	// Sector tag
	if (map_format == MAP_DOOM)
	{
		int tag = -1;
		if (MapObject::multiIntProperty(lines, "arg0", tag))
			text_tag->setNumber(tag);
	}

	// Line ID
	if (map_format == MAP_UDMF)
	{
		int id = -1;
		if (MapObject::multiIntProperty(lines, "id", id))
			text_id->setNumber(id);
	}

	// First side
	vector<MapSide*> sides;
	for (unsigned a = 0; a < lines.size(); a++)
	{
		if (MapSide* s = ((MapLine*)lines[a])->s1())
			sides.push_back(s);
	}
	if (sides.empty())
		panel_side1->Enable(false);
	else
		panel_side1->openSides(sides);

	// Second side
	sides.clear();
	for (unsigned a = 0; a < lines.size(); a++)
	{
		if (MapSide* s = ((MapLine*)lines[a])->s2())
			sides.push_back(s);
	}
	if (sides.empty())
		panel_side2->Enable(false);
	else
		panel_side2->openSides(sides);

	// Load all properties
	mopp_all_props->openObjects(lines);

	// Update internal objects list
	this->objects.clear();
	for (unsigned a = 0; a < lines.size(); a++)
		this->objects.push_back(lines[a]);

	// Update layout
	Layout();
	Refresh();
}

/* LinePropsPanel::applyChanges
 * Applies values to [lines]
 *******************************************************************/
void LinePropsPanel::applyChanges()
{
	int map_format = MapEditor::editContext().mapDesc().format;

	// Apply general properties
	for (unsigned l = 0; l < objects.size(); l++)
	{
		// Flags
		if (map_format == MAP_UDMF)
		{
			// UDMF
			for (unsigned a = 0; a < udmf_flags.size(); a++)
			{
				if (cb_flags[a]->Get3StateValue() == wxCHK_UNDETERMINED)
					continue;

				//for (unsigned l = 0; l < objects.size(); l++)
					objects[l]->setBoolProperty(udmf_flags[a], cb_flags[a]->GetValue());
			}
		}
		else
		{
			// Other
			for (unsigned a = 0; a < cb_flags.size(); a++)
			{
				if (cb_flags[a]->Get3StateValue() == wxCHK_UNDETERMINED)
					continue;

				//for (unsigned l = 0; l < objects.size(); l++)
					theGameConfiguration->setLineFlag(a, (MapLine*)objects[l], cb_flags[a]->GetValue());
			}
		}

		// Sector tag
		if (map_format == MAP_DOOM && !text_tag->IsEmpty())
			objects[l]->setIntProperty("arg0", text_tag->getNumber(objects[l]->intProperty("arg0")));

		// Line ID
		if (map_format == MAP_UDMF && !text_id->IsEmpty())
			objects[l]->setIntProperty("id", text_id->getNumber(objects[l]->intProperty("id")));
	}

	// Apply special
	panel_special->applyTo(objects, !cb_override_special->IsShown() || cb_override_special->GetValue());

	// Apply first side
	vector<MapSide*> sides;
	for (unsigned a = 0; a < objects.size(); a++)
	{
		if (MapSide* s = ((MapLine*)objects[a])->s1())
			sides.push_back(s);
	}
	if (!sides.empty())
		panel_side1->applyTo(sides);

	// Apply second side
	sides.clear();
	for (unsigned a = 0; a < objects.size(); a++)
	{
		if (MapSide* s = ((MapLine*)objects[a])->s2())
			sides.push_back(s);
	}
	if (!sides.empty())
		panel_side2->applyTo(sides);

	// Apply other properties
	mopp_all_props->applyChanges();
}


/*******************************************************************
 * LINEPROPSPANEL CLASS EVENTS
 *******************************************************************/

/* LinePropsPanel::onOverrideSpecialChecked
 * Called when the 'Override Action Special' checkbox is changed
 *******************************************************************/
void LinePropsPanel::onOverrideSpecialChecked(wxCommandEvent& e)
{
	if (cb_override_special->IsChecked())
		panel_special->Enable(true);
	else
		panel_special->Enable(false);
}

/* LinePropsPanel::onBtnNewTag
 * Called when the 'New Tag' button is clicked
 *******************************************************************/
void LinePropsPanel::onBtnNewTag(wxCommandEvent& e)
{
	text_tag->setNumber(MapEditor::editContext().getMap().findUnusedSectorTag());
}

/* LinePropsPanel::onBtnNewId
 * Called when the 'New ID' button is clicked
 *******************************************************************/
void LinePropsPanel::onBtnNewId(wxCommandEvent& e)
{
	text_id->setNumber(MapEditor::editContext().getMap().findUnusedLineId());
}
