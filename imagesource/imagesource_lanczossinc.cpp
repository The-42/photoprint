/*
 * imagesource_lanczossinc.cpp - Interpolated scaling filter
 * Implements Sinc interpolation with Lanczos Window:
 * sin(pi*x)/(pi*x)*sample*sin(pi*x/windowsize)/(pi*x/windowsize)
 *
 * Supports Greyscale, RGB and CMYK data
 * Doesn't (yet) support random access
 *
 * Copyright (c) 2004-2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "imagesource_lanczossinc.h"

using namespace std;


// 2D scaling is implemented as a chaining of a horizontal, then a vertical scaling;


ImageSource_LanczosSinc::~ImageSource_LanczosSinc()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_LanczosSinc::GetRow(int row)
{
	return(source->GetRow(row));
}


ImageSource_LanczosSinc::ImageSource_LanczosSinc(struct ImageSource *source,int width,int height,int window)
	: ImageSource(source), source(source)
{
	this->source=new ImageSource_HLanczosSinc(this->source,width,window);
	this->source=new ImageSource_VLanczosSinc(this->source,height,window);
	xres=this->source->xres;
	yres=this->source->yres;
	this->randomaccess=this->source->randomaccess;
	this->width=width;
	this->height=height;
}


// Sinc function.

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

static double sinc(double x)
{
	x*=M_PI;
	if(x==0.0)
		return(1.0);
	else
		return(sin(x)/x);
}


// The row cache is just a simplistic ring-buffer type cache which handles
// the details of tracking several rows of "support" data.
// The temporary data does have to be float - or at least wider than ISDataType
// and must be clamped since the ringing artifiacts inherent in Lanczos Sinc
// can push values out of range.

class ISLanczosSinc_RowCache
{
	public:
	ISLanczosSinc_RowCache(ImageSource_VLanczosSinc *source);
	~ISLanczosSinc_RowCache();
	double *GetRow(int row);
	double *GetCacheRow(int row);
	private:
	ImageSource_VLanczosSinc *source;
	double *cache;
	double *rowbuffer;
	int currentrow;
};


ISLanczosSinc_RowCache::~ISLanczosSinc_RowCache()
{
	if(cache)
		free(cache);
	if(rowbuffer)
		free(rowbuffer);
}


ISLanczosSinc_RowCache::ISLanczosSinc_RowCache(ImageSource_VLanczosSinc *source)
	: source(source), cache(NULL), rowbuffer(NULL), currentrow(-1)
{
	cache=(double *)malloc(sizeof(double)*source->samplesperpixel*source->width*source->support);
	rowbuffer=(double *)malloc(sizeof(double)*source->samplesperpixel*source->width);
}


double *ISLanczosSinc_RowCache::GetRow(int row)
{
	for(int i=0;i<source->width*source->samplesperpixel;++i)
		rowbuffer[i]=0.0;

	for(int i=0;i<source->support;++i)
	{
		int p=i-source->windowsize;
		int sr=(row*source->source->height)/source->height;
		double *src=GetCacheRow(sr+p);
		double f=source->coeff[row*source->support+i];
		for(int x=0;x<source->width*source->samplesperpixel;++x)
		{
			rowbuffer[x]+=f*src[x];
		}
	}
	return(rowbuffer);
}


double *ISLanczosSinc_RowCache::GetCacheRow(int row)
{
	if(row<0)
		row=0;
	if(row>=source->source->height)
		row=source->source->height-1;
	int crow=row%source->support;
	{
		double *rowptr=cache+crow*source->samplesperpixel*source->width;
		if(row>currentrow)
		{
			currentrow=row;
			ISDataType *src=source->source->GetRow(row);
			for(int i=0;i<source->width*source->samplesperpixel;++i)
			{
				rowptr[i]=src[i];
			}
		}
		return(rowptr);		
	}
}


// Vertical scaling


ImageSource_VLanczosSinc::~ImageSource_VLanczosSinc()
{
	if(cache)
		delete cache;
	if(source)
		delete source;
	if(coeff)
		free(coeff);
}


ISDataType *ImageSource_VLanczosSinc::GetRow(int row)
{
	int i;

	if(row==currentrow)
		return(rowbuffer);

	double *srcdata=cache->GetRow(row);

	for(i=0;i<width*samplesperpixel;++i)
	{
		double s=srcdata[i];
		if(s<0.0) s=0.0;
		if(s>IS_SAMPLEMAX) s=IS_SAMPLEMAX;
		rowbuffer[i]=int(s);
	}

	currentrow=row;

	return(rowbuffer);
}


// Since Lanczos Sinc is a relatively expensive algorithm to compute in realtime
// and the row / column coefficients are constant we precompute them here.

void ImageSource_VLanczosSinc::PreCalc()
{
	for(int y=0;y<height;++y)
	{
		double c=y*source->height; c/=height;
		int ci=int(c);
		double f=c-ci;
		for(int i=0; i<support; ++i)
		{
			int p=i-windowsize;
			double v=f-p;
			coeff[support*y+i]=sinc(v)*sinc(v/windowsize);
		}
	}
}


ImageSource_VLanczosSinc::ImageSource_VLanczosSinc(struct ImageSource *source,int height,int windowsize)
	: ImageSource(source), source(source), windowsize(windowsize)
{
	this->height=height;
	yres=(source->yres*height); yres/=source->height;

	support=windowsize*2+1;
	coeff=(double *)malloc(sizeof(double)*height*support);
	PreCalc();
	cache=new ISLanczosSinc_RowCache(this);
	MakeRowBuffer();
	randomaccess=false;
}


// Horizontal scaling


ISDataType *ImageSource_HLanczosSinc::GetRow(int row)
{
	if(row==currentrow)
		return(rowbuffer);

	ISDataType *src=source->GetRow(row);
	for(int x=0;x<width;++x)
	{
		int sx=(x*source->width)/width;
		for(int s=0;s<samplesperpixel;++s)
		{
			double a=0.0;
			for(int p=0;p<support;++p)
			{
				int lsx=sx+p-windowsize;
				if(lsx<0) lsx=0;
				if(lsx>=source->width) lsx=source->width-1;

				a+=src[lsx*samplesperpixel+s]*coeff[x*support+p];
			}
			if(a<0.0) a=0.0;
			if(a>IS_SAMPLEMAX) a=IS_SAMPLEMAX;
			rowbuffer[x*samplesperpixel+s]=a;
		}
	}

	currentrow=row;

	return(rowbuffer);
}


// Since Lanczos Sinc is a relatively expensive algorithm to compute in realtime
// and the row / column coefficients are constant we precompute them here.

void ImageSource_HLanczosSinc::PreCalc()
{
	for(int x=0;x<width;++x)
	{
		double c=x*source->width; c/=width;
		int ci=int(c);
		double f=c-ci;
		for(int i=0; i<support; ++i)
		{
			int p=i-windowsize;
			double v=f-p;
			coeff[support*x+i]=sinc(v)*sinc(v/windowsize);
		}
	}
}


ImageSource_HLanczosSinc::ImageSource_HLanczosSinc(struct ImageSource *source,int width,int windowsize)
	: ImageSource(source), source(source), windowsize(windowsize)
{
	this->width=width;
	xres=(source->xres*width); xres/=source->width;

	support=windowsize*2+1;
	coeff=(double *)malloc(sizeof(double)*width*support);
	PreCalc();
	MakeRowBuffer();
}


ImageSource_HLanczosSinc::~ImageSource_HLanczosSinc()
{
	if(source)
		delete source;
	if(coeff)
		free(coeff);
}



