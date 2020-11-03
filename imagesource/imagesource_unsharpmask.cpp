/*
 * imagesource_unsharpmask.cpp - Applies an Unsharp Mask filter to an image.
 *
 * Supports Greyscale, RGB and CMYK data
 * Doesn't (yet) support random access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "imagesource_unsharpmask.h"
#include "convkernel_unsharpmask.h"
#include "convkernel_gaussian_1D.h"

using namespace std;

// The row cache is just a simplistic ring-buffer type cache which handles
// the details of tracking several rows of "support" data.

class ISUnsharpMask_RowCache
{
	public:
	ISUnsharpMask_RowCache(ImageSource_UnsharpMask *source);
	~ISUnsharpMask_RowCache();
	ISDataType *GetRawRow(int row);
	float *GetConvRow(int row);
	private:
	ImageSource_UnsharpMask *source;
	float *convcache;
	ISDataType *rawcache;
	int cachewidth,cachehoffset;
	int bufferrows;
	int rawcurrentrow;
	int convcurrentrow;
};


ISUnsharpMask_RowCache::~ISUnsharpMask_RowCache()
{
	if(convcache)
		free(convcache);
	if(rawcache)
		free(rawcache);
}


ISUnsharpMask_RowCache::ISUnsharpMask_RowCache(ImageSource_UnsharpMask *source)
	: source(source), rawcurrentrow(-1), convcurrentrow(-1)
{
	cachewidth=source->width+source->hextra*2;
	cachehoffset=source->hextra;
	bufferrows=source->vextra*2+1;
	convcache=(float *)malloc(sizeof(float)*source->samplesperpixel*cachewidth*bufferrows);
	rawcache=(ISDataType *)malloc(sizeof(float)*source->samplesperpixel*source->width*bufferrows);
}


inline float *ISUnsharpMask_RowCache::GetConvRow(int row)
{
	if(row<0)
		row=0;
	if(row>=source->source->height)
		row=source->source->height-1;
	int crow=row%(source->vextra*2+1);
	float *rowptr=convcache+crow*source->samplesperpixel*cachewidth;
	if(row>convcurrentrow)
	{
		convcurrentrow=row;
		ISDataType *src=GetRawRow(row);

		// Convolve the temp row into the cache;
		for(int x=0;x<cachewidth;++x)
		{
			float t[5]={0.0,0.0,0.0,0.0,0.0};
			for(int kx=0;kx<source->kernel->GetWidth();++kx)
			{
				int sx=x+kx-cachehoffset*2;
				if(sx<0) sx=0;
				if(sx>source->width) sx=source->width-1;
				for(int s=0;s<source->samplesperpixel;++s)
				{
					t[s]+=source->kernel->Kernel(kx,0)*src[sx*source->samplesperpixel+s];
				}
			}
			for(int s=0;s<source->samplesperpixel;++s)
			{
				rowptr[x*source->samplesperpixel+s]=t[s];
			}
		}
	}
	return(rowptr+cachehoffset*source->samplesperpixel);		
}


inline ISDataType *ISUnsharpMask_RowCache::GetRawRow(int row)
{
	if(row<0)
		row=0;
	if(row>=source->source->height)
		row=source->source->height-1;
	int crow=row%(source->vextra*2+1);
	ISDataType *rowptr=rawcache+crow*source->samplesperpixel*source->width;
	if(row>rawcurrentrow)
	{
		rawcurrentrow=row;
		ISDataType *src=source->source->GetRow(row);

		// Store the row to be cached in a temporary buffer...
		for(int s=0;s<source->width*source->samplesperpixel;++s)
		{
			rowptr[s]=src[s];
		}
	}
	return(rowptr);		
}


ImageSource_UnsharpMask::~ImageSource_UnsharpMask()
{
	if(tmprows)
		free(tmprows);
	if(cache)
		delete cache;
	if(source)
		delete source;
	if(kernel)
		delete kernel;
}


ISDataType *ImageSource_UnsharpMask::GetRow(int row)
{
	if(row==currentrow)
		return(rowbuffer);

	int kh=kernel->GetWidth();	// Using a 1D kernel!

	for(int r=0;r<kh;++r)
	{
		tmprows[r]=cache->GetConvRow(row+(r-vextra));
	}
	ISDataType *srcrow=cache->GetRawRow(row);
	for(int x=0;x<width;++x)
	{
		float t[5]={0.0,0.0,0.0,0.0,0.0};
		for(int ky=0;ky<kh;++ky)
		{
			for(int s=0;s<samplesperpixel;++s)
			{
				t[s]+=kernel->Kernel(ky,0) * tmprows[ky][x*samplesperpixel+s];
			}
		}
		for(int s=0;s<samplesperpixel;++s)
		{
//			float out=t[s];
//			float out=tmprows[vextra][x*samplesperpixel+s];
			float out=(1+amount)*srcrow[x*samplesperpixel+s]-amount*t[s];
			float d=srcrow[x*samplesperpixel+s]-out;
			if((d*d)<threshold)
			{
				out=srcrow[x*samplesperpixel+s];
			}
			if(out<0.0) out=0.0;
			if(out>IS_SAMPLEMAX) out=IS_SAMPLEMAX;
			rowbuffer[x*samplesperpixel+s]=ISDataType(out);
		}
	}

	currentrow=row;

	return(rowbuffer);
}


ImageSource_UnsharpMask::ImageSource_UnsharpMask(struct ImageSource *source,float radius,float amount,float threshold)
	: ImageSource(source), source(source), kernel(NULL), amount(amount), threshold(threshold*threshold)
{
	kernel=new ConvKernel_Gaussian_1D(radius);
	kernel->Normalize();
	hextra=kernel->GetWidth()/2;
	vextra=hextra;
	cache=new ISUnsharpMask_RowCache(this);
	tmprows=(float **)malloc(sizeof(float *)*kernel->GetWidth());
	MakeRowBuffer();
	randomaccess=false;
}
