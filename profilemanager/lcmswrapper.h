/*
 * lcmswrapper.cpp - encapsulates typical "user" functions of LittleCMS,
 * providing a Profile and Transform class
 *
 * Copyright (c) 2004-2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * TODO: Report pixel type, support Lab, XYZ, etc.
 *
 */

#ifndef LCMSWRAPPER_H
#define LCMSWRAPPER_H

#ifdef HAVE_CONFIG_H
#ifndef VERSION
#include "config.h"
#endif
#endif

#include <lcms.h>
#include "md5.h"
#include "imagesource_types.h"


enum LCMSWrapper_Intent
{
	LCMSWRAPPER_INTENT_DEFAULT=-1,
	LCMSWRAPPER_INTENT_PERCEPTUAL,
	LCMSWRAPPER_INTENT_RELATIVE_COLORIMETRIC,
	LCMSWRAPPER_INTENT_RELATIVE_COLORIMETRIC_BPC,
	LCMSWRAPPER_INTENT_SATURATION,
	LCMSWRAPPER_INTENT_ABSOLUTE_COLORIMETRIC,
};


class CMSWhitePoint;
class CMSRGBPrimaries;
class CMSRGBGamma;
class CMSGamma;

class CMSProfile
{
	public:
	CMSProfile(const char *filename);
	CMSProfile(char *buffer,int length); // Should remain valid for lifetime of CMSProfile
	CMSProfile(CMSRGBPrimaries &primaries,CMSRGBGamma &gamma,CMSWhitePoint &whitepoint); // Create virtual RGB profile
	CMSProfile(CMSGamma &gamma,CMSWhitePoint &whitepoint); // Create virtual Grey profile
	CMSProfile(CMSWhitePoint &whitepoint); // Create a virtual LAB profile
	CMSProfile(); // Create a virtual sRGB profile
	CMSProfile(const CMSProfile &src); // Copy constructor
	~CMSProfile();
	enum IS_TYPE GetColourSpace();
	enum IS_TYPE GetDeviceLinkOutputSpace();
	bool IsDeviceLink();
	bool IsV4();
	const char *GetName();
	const char *GetManufacturer();
	const char *GetModel();
	const char *GetDescription();
	const char *GetInfo();
	const char *GetCopyright();
	const char *GetFilename();
	MD5Digest *GetMD5();
	bool Save(const char *filename);
	bool operator==(const CMSProfile &other);
	protected:
	void CalcMD5();
	MD5Digest *md5;
	cmsHPROFILE prof;
	bool generated;	// Was this profile generated on the fly?
	char *filename;	// Only used if profile is on disk.
	char *buffer;	// Only used if profile
	int buflen;		// is loaded from memory
	friend class CMSTransform;
	friend class CMSProofingTransform;
	friend std::ostream& operator<<(std::ostream &s,CMSProfile &sp);
};


class CMSTransform
{
	public:
	CMSTransform();
	CMSTransform(CMSProfile *in,CMSProfile *out,LCMSWrapper_Intent intent=LCMSWRAPPER_INTENT_PERCEPTUAL);
	CMSTransform(CMSProfile *devicelink,LCMSWrapper_Intent intent=LCMSWRAPPER_INTENT_PERCEPTUAL);
	CMSTransform(CMSProfile *profiles[],int profilecount,LCMSWrapper_Intent intent=LCMSWRAPPER_INTENT_PERCEPTUAL);
	virtual ~CMSTransform();
	virtual void Transform(unsigned short *src,unsigned short *dst,int pixels);
	enum IS_TYPE GetInputColourSpace();
	enum IS_TYPE GetOutputColourSpace();
	protected:
	virtual void MakeTransform(CMSProfile *in,CMSProfile *out,LCMSWrapper_Intent intent);
	enum IS_TYPE inputtype;
	enum IS_TYPE outputtype;
	cmsHTRANSFORM transform;
};


class CMSProofingTransform : public CMSTransform
{
	public:
	CMSProofingTransform(CMSProfile *in,CMSProfile *out,CMSProfile *proof,int proofintent=INTENT_PERCEPTUAL,int viewintent=INTENT_ABSOLUTE_COLORIMETRIC);
	CMSProofingTransform(CMSProfile *devicelink,CMSProfile *proof,int proofintent=INTENT_PERCEPTUAL,int viewintent=INTENT_ABSOLUTE_COLORIMETRIC);
};


class CMSWhitePoint
{
	public:
	CMSWhitePoint(int degk)
	{
		cmsWhitePointFromTemp(degk,&whitepoint);
	}
	protected:
	cmsCIExyY whitepoint;
	friend class CMSProfile;
};


class CMSRGBPrimaries : public cmsCIExyYTRIPLE
{
	public:
	CMSRGBPrimaries()
	{
	}
	CMSRGBPrimaries(float rx,float ry,float gx,float gy,float bx,float by)
	{
		Red.x=rx;
		Red.y=ry;
		Red.Y=1.0;
		Green.x=gx;
		Green.y=gy;
		Green.Y=1.0;
		Blue.x=bx;
		Blue.y=by;
		Blue.Y=1.0;	
	}
	protected:
	friend class CMSProfile;
};


class CMSGamma
{
	public:
	CMSGamma(float gamma)
	{
		gammatable=cmsBuildGamma(256,gamma);
	}
	~CMSGamma()
	{
		cmsFreeGamma(gammatable);
	}
	LPGAMMATABLE GetGammaTable()
	{
		return(gammatable);
	}
	protected:
	LPGAMMATABLE gammatable;
	friend class CMSProfile;
	friend class CMSRGBGamma;
};

class CMSRGBGamma
{
	public:
	CMSRGBGamma(float rgamma,float ggamma,float bgamma)
		: redgamma(rgamma),greengamma(ggamma),bluegamma(bgamma)
	{
		gammatables[0]=redgamma.GetGammaTable();
		gammatables[1]=greengamma.GetGammaTable();
		gammatables[2]=bluegamma.GetGammaTable();
	}
	CMSRGBGamma(float gamma)
		: redgamma(gamma),greengamma(gamma),bluegamma(gamma)
	{
		gammatables[0]=redgamma.GetGammaTable();
		gammatables[1]=greengamma.GetGammaTable();
		gammatables[2]=bluegamma.GetGammaTable();
	}
	protected:
	CMSGamma redgamma,greengamma,bluegamma;
	LPGAMMATABLE gammatables[3];
	friend class CMSProfile;
};


extern CMSRGBPrimaries CMSPrimaries_Rec709;
extern CMSRGBPrimaries CMSPrimaries_Adobe;
extern CMSRGBPrimaries CMSPrimaries_NTSC;
extern CMSRGBPrimaries CMSPrimaries_EBU;
extern CMSRGBPrimaries CMSPrimaries_SMPTE;
extern CMSRGBPrimaries CMSPrimaries_HDTV;
extern CMSRGBPrimaries CMSPrimaries_CIE;


int CMS_GetIntentCount();
const char *CMS_GetIntentName(int intent);
const char *CMS_GetIntentDescription(int intent);
const int CMS_GetLCMSIntent(int intent);
const int CMS_GetLCMSFlags(int intent);

#endif
