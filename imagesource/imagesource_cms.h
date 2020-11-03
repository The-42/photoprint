/*
 * imagesource_cms.h
 * ImageSource Colour Management filter.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * TODO: Clean up handling of image types other than RGB
 */

#ifndef IMAGESOURCE_CMS_H
#define IMAGESOURCE_CMS_H

#include "imagesource.h"
#include "lcmswrapper.h"


class ImageSource_CMS : public ImageSource
{
	public:
//	ImageSource_CMS(ImageSource *source,CMSDB &inp,CMSDB &outp);
//	ImageSource_CMS(ImageSource *source,CMSProfile *inp,CMSDB &outp);
	ImageSource_CMS(ImageSource *source,CMSProfile *inp,CMSProfile *outp);
	ImageSource_CMS(ImageSource *source,CMSTransform *transform);
	virtual ~ImageSource_CMS();
	ISDataType *GetRow(int row);
	private:
	void Init();
	ImageSource *source;
	CMSTransform *transform;
	bool disposetransform;
	int tmpsourcespp;
	int tmpdestspp;
	unsigned short *tmp1;
	unsigned short *tmp2;
};

#endif
