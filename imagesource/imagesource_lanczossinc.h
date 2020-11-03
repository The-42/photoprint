/*
 * imagesource_lanczossinc.h - Interpolated scaling filter
 * Implements Sinc interpolation with Lanczos Window:
 * sin(pi*x)/(pi*x)*sample*sin(pi*x/windowsize)/(pi*x/windowsize)
 *
 * Supports all colourspaces
 * Doesn't (yet) support random access
 *
 * Copyright (c) 2004,2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_LANCZOSSINC_H
#define IMAGESOURCE_LANCZOSSINC_H

#include "imagesource.h"

class ISLanczosSinc_RowCache;


class ImageSource_LanczosSinc : public ImageSource
{
	public:
	ImageSource_LanczosSinc(ImageSource *source,int width,int height,int windowsize=6);
	~ImageSource_LanczosSinc();
	ISDataType *GetRow(int row);
	protected:
	ImageSource *source;
};


class ImageSource_VLanczosSinc : public ImageSource
{
	public:
	ImageSource_VLanczosSinc(ImageSource *source,int height,int windowsize=6);
	~ImageSource_VLanczosSinc();
	ISDataType *GetRow(int row);
	protected:
	void PreCalc();
	int support;
	ImageSource *source;
	int windowsize;
	double *coeff;
	ISLanczosSinc_RowCache *cache;
	friend class ISLanczosSinc_RowCache;
};


class ImageSource_HLanczosSinc : public ImageSource
{
	public:
	ImageSource_HLanczosSinc(ImageSource *source,int width,int windowsize=6);
	~ImageSource_HLanczosSinc();
	ISDataType *GetRow(int row);
	protected:
	void PreCalc();
	int support;
	ImageSource *source;
	int windowsize;
	double *coeff;
};

#endif
