/*
 * imagesource_gdkpixbuf.cpp
 * GdkPixbuf-based image loader
 * Supports Random Access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 *
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../support/debug.h"

#include "imagesource_gdkpixbuf.h"

using namespace std;


ImageSource_GdkPixbuf::~ImageSource_GdkPixbuf()
{
	if(pixbuf)
		g_object_unref(G_OBJECT(pixbuf));
}


#define OFFSET(pb, x, y) ((x) * (gdk_pixbuf_get_n_channels(pb)) + (y) * gdk_pixbuf_get_rowstride(pb))

ISDataType *ImageSource_GdkPixbuf::GetRow(int row)
{
	unsigned char *src;
	ISDataType *dst;

	if(currentrow==row)
		return(rowbuffer);

	if(row>=height)
	{
		Debug[WARN] << "ImageSource_GdkPixbuf - Warning: row " << row+1 << " of " << height << " requested." << endl;
		return(rowbuffer);
	}

	src=gdk_pixbuf_get_pixels(pixbuf)+OFFSET(pixbuf,0,row);
	dst=rowbuffer;

	switch(type)
	{
		case IS_TYPE_RGB:
			if(hasalpha)
			{
				for(int x=0;x<width;++x)
				{
					unsigned int r,g,b,a;
					r=*src++;
					g=*src++;
					b=*src++;
					a=*src++;
					r=IS_SAMPLEMAX-(a*EIGHTTOIS(255-r))/255;
					g=IS_SAMPLEMAX-(a*EIGHTTOIS(255-g))/255;
					b=IS_SAMPLEMAX-(a*EIGHTTOIS(255-b))/255;
					*dst++=r;
					*dst++=g;
					*dst++=b;
				}
			}
			else
			{
				for(int x=0;x<width;++x)
				{
					unsigned int g;
					g=*src++;
					*dst++=EIGHTTOIS(g);
					g=*src++;
					*dst++=EIGHTTOIS(g);
					g=*src++;
					*dst++=EIGHTTOIS(g);
				}
			}
			break;
		case IS_TYPE_RGBA:
			for(int x=0;x<width;++x)
			{
				unsigned int g;
				g=*src++;
				*dst++=EIGHTTOIS(g);
				g=*src++;
				*dst++=EIGHTTOIS(g);
				g=*src++;
				*dst++=EIGHTTOIS(g);
				g=*src++;
				*dst++=EIGHTTOIS(g);
			}
			break;
		default:
			throw "Only RGB pixbufs are currently supported";
			break;
	}
	
	currentrow=row;
	return(rowbuffer);
}


ImageSource_GdkPixbuf::ImageSource_GdkPixbuf(const char *filename) : pixbuf(NULL)
{
	GError *err=NULL;
	pixbuf=gdk_pixbuf_new_from_file(filename,&err);
	if(!pixbuf)
		throw err->message;
	Init();
}


ImageSource_GdkPixbuf::ImageSource_GdkPixbuf(GdkPixbuf *pixbuf) : pixbuf(pixbuf)
{
	g_object_ref(G_OBJECT(pixbuf));
	Init();
}

		
void ImageSource_GdkPixbuf::Init()
{
	xres=yres=72;  // FIXME - read these from the file
	embeddedprofile=NULL;

	if(gdk_pixbuf_get_colorspace(pixbuf)!=GDK_COLORSPACE_RGB)
		throw "GdkPixbuf loader only supports RGB images!";

	if(gdk_pixbuf_get_bits_per_sample(pixbuf)!=8)
		throw "GdxPixbuf loader doesn't yet support 16 bit images\n"\
		"Please contact me at blackfive@fakenhamweb.co.uk\n"\
		"And I'll try to add native support for this file format.";

	type=IS_TYPE_RGB;
	samplesperpixel=3;

	width=gdk_pixbuf_get_width(pixbuf);
	height=gdk_pixbuf_get_height(pixbuf);
	rowstride=gdk_pixbuf_get_rowstride(pixbuf);
	pixels=gdk_pixbuf_get_pixels(pixbuf);
	hasalpha=gdk_pixbuf_get_has_alpha(pixbuf);

	if(hasalpha)
	{
		type=IS_TYPE_RGBA;
		++samplesperpixel;
	}

	MakeRowBuffer();
	randomaccess=true;
}

