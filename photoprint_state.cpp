/*
 * photoprint_state.cpp - class containing most of PhotoPrint's state.
 * Passed around the various GUI routines.
 * Contains:
 *   Layout      - deals with rows, columns, margins and image list.
 *   LayoutDB    - deals with storing layout parameters for loading and saving.
 *   PrintOutput - keeps track of print destination
 *   GPrinter    - actual printer class, maintains printer settings as well as
 *     performing actual printing.
 *   CMSDB       - Colour management settings for both input (image) and output (printer) profiles.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


#include <iostream>
#include <string.h>
#include <stdlib.h>

#include "imagesource/imagesource_util.h"

#include "photoprint_state.h"

#include "layout_nup.h"
#include "layout_single.h"
#include "layout_poster.h"
#include "layout_carousel.h"

#include "support/debug.h"

#include "support/searchpathdbhandler.h"
#include "support/pathsupport.h"

#include "miscwidgets/generaldialogs.h"

#define DEFAULTPRESETVERSION 29
#define CURRENTPRESETVERSION 41

using namespace std;

ConfigTemplate PhotoPrint_State::Template[]=
{
	ConfigTemplate("PresetVersion",int(DEFAULTPRESETVERSION)), // Default to a pre-0.3.0 version.
	ConfigTemplate("PrintColourSpace","RGB"),
	ConfigTemplate("ScalingQuality",int(IS_SCALING_AUTOMATIC)),
	ConfigTemplate("Units","MM"),
	ConfigTemplate("RenderingResolution",int(360)),
	ConfigTemplate("Win_X",int(0)),
	ConfigTemplate("Win_Y",int(0)),
	ConfigTemplate("Win_W",int(0)),
	ConfigTemplate("Win_H",int(0)),
	ConfigTemplate("HighresPreviews",int(1)),
	ConfigTemplate("ExpanderState_SigControl",int(1)),
	ConfigTemplate("ExpanderState_Carousel",int(1)),
	ConfigTemplate("ExpanderState_Single",int(1)),
	ConfigTemplate("ExpanderState_Poster",int(1)),
	ConfigTemplate("ExpanderState_PosterOverlap",int(1)),
	ConfigTemplate("ExpanderState_PageExtent",int(1)),
	ConfigTemplate("ExpanderState_ImageInfo",int(1)),
	ConfigTemplate("ExpanderState_Histogram",int(1)),
	ConfigTemplate("ExpanderState_EffectSelector",int(1)),
	// Now obsolete, left in for compatibility with pre-0.3.0 preset files.
	ConfigTemplate("DefaultRGBProfile",""),
	ConfigTemplate("DefaultRGBProfileActive",int(0)),
	ConfigTemplate("DefaultCMYKProfile",""),
	ConfigTemplate("DefaultCMYKProfileActive",int(0)),
	ConfigTemplate("PrinterProfile",""),
	ConfigTemplate("PrinterProfileActive",int(0)),
	ConfigTemplate("MonitorProfile",""),
	ConfigTemplate("MonitorProfileActive",int(0)),
	ConfigTemplate("RenderingIntent",int(0)),
	// End of obsolete items...

#ifdef WIN32
	// FIXME - where should these be installed under Win32?  $HOME is fine for a single user...
	ConfigTemplate("BorderPath","$HOME/.photoprint/borders"),
	ConfigTemplate("BackgroundPath","$HOME/.photoprint/backgrounds"),
	ConfigTemplate("ShortcutsPath","$HOME/.photoprint/shortcuts"),
#else
	ConfigTemplate("BorderPath","/usr/share/photoprint/borders" SEARCHPATH_DELIMITER_S "/usr/local/share/photoprint/borders" SEARCHPATH_DELIMITER_S "$HOME/.photoprint/borders"),
	ConfigTemplate("BackgroundPath","/usr/share/photoprint/backgrounds" SEARCHPATH_DELIMITER_S "/usr/local/share/photoprint/backgrounds" SEARCHPATH_DELIMITER_S "$HOME/.photoprint/backgrounds"),
	ConfigTemplate("ShortcutsPath","/usr/share/photoprint/shortcuts" SEARCHPATH_DELIMITER_S "/usr/local/share/photoprint/shortcuts" SEARCHPATH_DELIMITER_S "$HOME/.photoprint/shortcuts"),
#endif

	// Null-terminated
	ConfigTemplate()
};


class PPPathDBHandler : public ConfigDBHandler
{
	public:
	PPPathDBHandler(ConfigFile *file,const char *section,ConfigDB *db,PhotoPrint_State &state)
		: ConfigDBHandler(file,section,db), db(db), state(state)
	{
		state.bordersearchpath.AddPath(db->FindString("BorderPath"));
		state.backgroundsearchpath.AddPath(db->FindString("BackgroundPath"));
	}
	virtual ~PPPathDBHandler()
	{
	}
	virtual void LeaveSection()
	{
		state.bordersearchpath.ClearPaths();
		state.bordersearchpath.AddPath(db->FindString("BorderPath"));
		state.backgroundsearchpath.ClearPaths();
		state.backgroundsearchpath.AddPath(db->FindString("BackgroundPath"));
		ConfigDBHandler::LeaveSection();
	}
	virtual void SaveSection(FILE *file)
	{
		char *p;
		p=state.bordersearchpath.GetPaths();
		db->SetString("BorderPath",p);
		free(p);
		p=state.backgroundsearchpath.GetPaths();
		db->SetString("BackgroundPath",p);
		free(p);
		ConfigDBHandler::SaveSection(file);
	}
	protected:
	ConfigDB *db;
	PhotoPrint_State &state;
};


PhotoPrint_State::PhotoPrint_State(bool batchmode)
	: ConfigFile(), ConfigDB(Template), layout(NULL), filename(NULL), layoutdb(this,"[Layout]"), printoutput(this,"[Output]"),
	printer(printoutput,this,"[Print]"), profilemanager(this,"[ColourManagement]"), bordersearchpath(), backgroundsearchpath(), batchmode(batchmode)
{
	new PPPathDBHandler(this,"[General]",this,*this);
	SetDefaultFilename();
}


PhotoPrint_State::~PhotoPrint_State()
{
	if(filename)
		free(filename);
	if(layout)
		delete layout;
}


void PhotoPrint_State::SetFilename(const char *file)
{
	if(filename)
		free(filename);
	filename=strdup(file);
}


void PhotoPrint_State::SetDefaultFilename()
{
	if(filename)
		free(filename);
	filename=NULL;

	filename=substitute_homedir("$HOME" SEARCHPATH_SEPARATOR_S ".photoprint" SEARCHPATH_SEPARATOR_S "default.preset");
}


void PhotoPrint_State::ParseSupplementaryConfig(const char *filename)
{
	printoutput.QueuesToDB();
	ConfigFile::ParseConfigFile(filename);
}


void PhotoPrint_State::ParseConfigFile()
{
	// Config files from newer than 0.2.9 will override this
	SetInt("PresetVersion",29);

	printer.Reset();
	if(!ConfigFile::ParseConfigFile(filename))
	{
		Debug[WARN] << "Parsing of config file failed" << endl;
		Debug[TRACE] << "Default queue is: " << printoutput.FindString("Queue") << endl;
//	Shoudn't need to do this any more, since the GPrinterSettings class now ensures a sane
//  default is set.
//		if(printoutput.GetPPD())
//			Debug[TRACE] << "Default PPD is: " << printoutput.GetPPD() << endl;
//		Debug[TRACE] << "Setting default driver" << endl;
//		printer.SetDriver("ps2");
	}

	// Code to update older config files goes here...
	int v=FindInt("PresetVersion");
	if(v<30)
	{
		// Transfer colour management settings to the new DB...
		// But only if there are settings wortht transferring!
		const char *pf=FindString("DefaultRGBProfile");
		if(pf && strlen(pf))
		{
			profilemanager.SetString("DefaultRGBProfile",FindString("DefaultRGBProfile"));
			profilemanager.SetInt("DefaultRGBProfileActive",FindInt("DefaultRGBProfileActive"));
			profilemanager.SetString("DefaultCMYKProfile",FindString("DefaultCMYKProfile"));
			profilemanager.SetInt("DefaultCMYKProfileActive",FindInt("DefaultCMYKProfileActive"));
			profilemanager.SetString("PrinterProfile",FindString("PrinterProfile"));
			profilemanager.SetInt("PrinterProfileActive",FindInt("PrinterProfileActive"));
			profilemanager.SetInt("RenderingIntent",FindInt("RenderingIntent"));
			// And clear the obsolete settings...
			SetString("DefaultRGBProfile","");
			SetInt("DefaultRGBProfileActive",0);
			SetString("DefaultCMYKProfile","");
			SetInt("DefaultCMYKProfileActive",0);
			SetString("PrinterProfile","");
			SetInt("PrinterProfileActive",0);
		}
	}

	printer.Validate();
}


bool PhotoPrint_State::SaveConfigFile()
{
	// Ensure that saved preset have the current PresetVersion...
	SetInt("PresetVersion",CURRENTPRESETVERSION);
	// Update the appropriate DB from the current layout...
	layout->LayoutToDB(layoutdb);

	Debug[TRACE] << "Filename : " << filename << endl;

	return(ConfigFile::SaveConfigFile(filename));
}


bool PhotoPrint_State::NewLayout(Progress *p)
{
	const char *type=layoutdb.FindString("LayoutType");

	Layout *nl=NULL;

	if(strcmp(type,"NUp")==0)
	{
		Debug[TRACE] << "Building NUp Layout" << endl;
		nl=new Layout_NUp(*this,layout);
	}
	else if(strcmp(type,"Single")==0)
	{
		Debug[TRACE] << "Building Single Layout" << endl;
		nl=new Layout_Single(*this,layout);
	}
	else if(strcmp(type,"Poster")==0)
	{
		Debug[TRACE] << "Building Poster Layout" << endl;
		nl=new Layout_Poster(*this,layout);
	}
	else if(strcmp(type,"Carousel")==0)
	{
		Debug[TRACE] << "Building Carousel Layout" << endl;
		nl=new Layout_Carousel(*this,layout);
	}
	else
		throw "Unknown layout type";

	nl->DBToLayout(layoutdb);

	if(layout)
	{
		nl->TransferImages(layout,p);
		delete layout;
	}

	layout=nl;
	return(true);
}


void PhotoPrint_State::SetUnits(enum Units unit)
{
	switch(unit)
	{
		case UNIT_POINTS:
			SetString("Units","PT");
			break;
		case UNIT_INCHES:
			SetString("Units","IN");
			break;
		case UNIT_MILLIMETERS:
			SetString("Units","MM");
			break;
		case UNIT_CENTIMETERS:
			SetString("Units","CM");
			break;	
	}
}


enum Units PhotoPrint_State::GetUnits()
{
	enum Units result=UNIT_POINTS;
	const char *u=FindString("Units");

	if(strcasecmp(u,"PT")==0)
		result=UNIT_POINTS;
	else if(strcasecmp(u,"IN")==0)
		result=UNIT_INCHES;
	else if(strcasecmp(u,"MM")==0)
		result=UNIT_MILLIMETERS;
	else if(strcasecmp(u,"CM")==0)
		result=UNIT_CENTIMETERS;
	return(result);
}

