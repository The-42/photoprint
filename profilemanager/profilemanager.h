#ifndef COLOURMANAGEMENT_H
#define COLOURMANAGEMENT_H

#include "imagesource.h"
#include "lcmswrapper.h"
#include "configdb.h"
#include "searchpath.h"

#ifndef WIN32
#include <X11/Xlib.h>
#endif

#define SYSTEMMONITORPROFILE_ESCAPESTRING "<System monitor profile>"
#define BUILTINSRGB_ESCAPESTRING "<Built-in sRGB profile>"
#define NOPROFILE_ESCAPESTRING "<None>"

enum CMColourDevice
{
	CM_COLOURDEVICE_NONE=0,
	CM_COLOURDEVICE_DISPLAY,
	CM_COLOURDEVICE_PRINTERPROOF,
	CM_COLOURDEVICE_EXPORT,
	CM_COLOURDEVICE_PRINTER,
	CM_COLOURDEVICE_DEFAULTRGB,
	CM_COLOURDEVICE_DEFAULTCMYK,
	CM_COLOURDEVICE_DEFAULTGREY
};

enum CMProofMode
{
	CM_PROOFMODE_NONE,
	CM_PROOFMODE_SIMULATEPRINT,
	CM_PROOFMODE_SIMULATEPRINTADAPTWHITE
};


class CMTransformFactory;
class ProfileInfo;

class ProfileManager : public ConfigDB, public SearchPathHandler
{
	public:
	ProfileManager(ConfigFile *Configfile,const char *section);
	virtual ~ProfileManager();
	CMSProfile *GetProfile(const char *name);
	CMSProfile *GetProfile(CMColourDevice target);
	CMSProfile *GetDefaultProfile(IS_TYPE colourspace);
	void SetProfile(CMColourDevice *target,const char *name);
	void SetDefaultProfile(IS_TYPE colourspace,const char *name);
	void SetProofMode(enum CMProofMode mode);

	CMTransformFactory *GetTransformFactory();

	// path handling - we override these functions from the SearchPathHandler
	// so we can invalidate the ProfileInfo list when the path changes.
	virtual void AddPath(const char *path);
	virtual void RemovePath(const char *path);
	virtual void ClearPaths();
	virtual char *SearchPaths(const char *path);
	virtual const char *GetNextFilename(const char *prev);
	ProfileInfo *GetFirstProfileInfo();
	ProfileInfo *GetProfileInfo(int i);
	int GetProfileInfoCount();
	ProfileInfo *FindProfileInfo(const char *fn);
	int GetIntentCount();
	const char *GetIntentName(LCMSWrapper_Intent intent);
	const char *GetIntentDescription(LCMSWrapper_Intent intent);

	protected:
	static ConfigTemplate Template[];
	void BuildProfileInfoList();
	void FlushProfileInfoList();
	ProfileInfo *first;
	void GetProfileFromDisplay();
	#ifdef WIN32
	char displayprofilename[MAX_PATH];
	#else
	Display *xdisplay;
	unsigned char *proffromdisplay;
	#endif
	long proffromdisplay_size;
	SearchPathIterator spiter;
	friend class ProfileInfo;
};


class CMTransformFactoryNode
{
	public:
	CMTransformFactoryNode(CMTransformFactory *header,CMSTransform *transform,MD5Digest &d1,MD5Digest &d2,LCMSWrapper_Intent intent,bool proof=false);
	~CMTransformFactoryNode();
	protected:
	CMTransformFactory *header;
	CMTransformFactoryNode *prev,*next;
	CMSTransform *transform;
	MD5Digest digest1;
	MD5Digest digest2;
	LCMSWrapper_Intent intent;
	bool proof;
	friend class CMTransformFactory;
};


class CMTransformFactory
{
	public:
	CMTransformFactory(ProfileManager &cm);
	~CMTransformFactory();
	CMSTransform *GetTransform(enum CMColourDevice target,CMSProfile *srcprofile,LCMSWrapper_Intent intent=LCMSWRAPPER_INTENT_DEFAULT);
	CMSTransform *GetTransform(CMSProfile *targetprofile,CMSProfile *srcprofile,LCMSWrapper_Intent intent=LCMSWRAPPER_INTENT_DEFAULT);
	CMSTransform *GetTransform(enum CMColourDevice target,ImageSource *src,LCMSWrapper_Intent intent=LCMSWRAPPER_INTENT_DEFAULT);
	CMSTransform *GetTransform(enum CMColourDevice target,IS_TYPE type,LCMSWrapper_Intent intent=LCMSWRAPPER_INTENT_DEFAULT);
	CMSTransform *GetTransform(CMSProfile *destprofile,CMSProfile *srcprofile,CMSProfile *proofprofile,LCMSWrapper_Intent intent=LCMSWRAPPER_INTENT_DEFAULT,int displayintent=LCMSWRAPPER_INTENT_DEFAULT);
	CMSTransform *Search(MD5Digest *srcdigest,MD5Digest *dstdigest,LCMSWrapper_Intent intent,bool proof=0);
	void Flush();
	protected:
	ProfileManager &manager;
	CMTransformFactoryNode *first;
	friend class CMTransformFactoryNode;
};


class ProfileInfo
{
	public:
	ProfileInfo(ProfileManager &pm,const char *filename);
	~ProfileInfo();
	ProfileInfo *Next();
	const char *GetFilename();
	const char *GetDescription();
	IS_TYPE GetColourSpace();
	bool IsDeviceLink();
	protected:
	void GetInfo();
	ProfileManager &profilemanager;
	ProfileInfo *next,*prev;
	char *filename;
	bool iscached;
	char *description;
	IS_TYPE colourspace;
	bool isdevicelink;
	friend class ProfileManager;
};
#endif
