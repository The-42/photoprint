#ifndef PHOTOPRINT_STATE_H
#define PHOTOPRINT_STATE_H

#include "layout.h"
#include "support/configdb.h"
#include "stpui_widgets/units.h"
#include "gp_cppsupport/printoutput.h"
#include "gp_cppsupport/gprinter.h"
#include "support/progress.h"
#include "profilemanager/profilemanager.h"
#include "support/searchpath.h"

class PhotoPrint_State : public ConfigFile, public ConfigDB
{
	public:
	PhotoPrint_State(bool batchmode=FALSE);
	~PhotoPrint_State();
	void SetFilename(const char *file);
	void SetDefaultFilename();
	void ParseConfigFile();

	// Parses a file but doesn't update filename or reset printer
	// used to apply supplementary options, such as layout or profiling modes.
	void ParseSupplementaryConfig(const char *filename);

	bool SaveConfigFile();
	bool NewLayout(Progress *p=NULL);
	void SetUnits(enum Units unit);
	enum Units GetUnits();
	Layout *layout;
	char *filename;
	LayoutDB layoutdb;
	PrintOutput printoutput;
	GPrinter printer;
	ProfileManager profilemanager;
	SearchPathHandler bordersearchpath;
	SearchPathHandler backgroundsearchpath;
	bool batchmode;
	protected:
	static ConfigTemplate Template[];
};

#endif
