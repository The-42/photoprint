/*
 * imagesource_bilinear.h - Interpolated scaling filter
 * Implements bilinear scaling
 *
 * Supports all colourspaces
 * Doesn't (yet) support random access
 *
 * Copyright (c) 2004-2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * 2008-06-06: split the horizontal and vertical scaling into separate stages
 *             to allow support for downsampling in one dimension while upsampling
 *             in the other.
 *
 */

#ifndef IMAGESOURCE_BILINEAR_H
#define IMAGESOURCE_BILINEAR_H

#include "imagesource.h"

class ImageSource_Bilinear : public ImageSource
{
	public:
	ImageSource_Bilinear(ImageSource *source,int width,int height);
	~ImageSource_Bilinear();
	ISDataType *GetRow(int row);
	protected:
	ImageSource *source;
};


class ImageSource_HBilinear : public ImageSource
{
	public:
	ImageSource_HBilinear(ImageSource *source,int dstwidth);
	~ImageSource_HBilinear();
	ISDataType *GetRow(int row);
	protected:
	ImageSource *source;
};


class ImageSource_VBilinear : public ImageSource
{
	public:
	ImageSource_VBilinear(ImageSource *source,int dstheight);
	~ImageSource_VBilinear();
	ISDataType *GetRow(int row);
	protected:
	ImageSource *source;
	ISDataType *lastrow;
	int cachedrow;
};

#endif
