/*
 * imagesource_scale.h - nearest-neighbour scaling filter
 *
 * Supports RGB and CMYK data
 * Supports random access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_SCALE_H
#define IMAGESOURCE_SCALE_H

#include "imagesource.h"

class ImageSource_Scale : public ImageSource
{
	public:
	ImageSource_Scale(ImageSource *source,int width,int height);
	~ImageSource_Scale();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
};

class ImageSource_HScale : public ImageSource
{
	public:
	ImageSource_HScale(ImageSource *source,int width);
	~ImageSource_HScale();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
};

class ImageSource_VScale : public ImageSource
{
	public:
	ImageSource_VScale(ImageSource *source,int height);
	~ImageSource_VScale();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
};

#endif
