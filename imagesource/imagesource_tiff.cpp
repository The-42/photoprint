/*
 * imagesource_tiff.cpp - ImageSource loader for TIFF files.
 *
 * Supports 8 and 16 bit,
 * RGB, CMYK and Grey data
 * Indexed and monochrome files are currently converted to grey.
 * Supports random access (to the extent that libtiff does, i.e.
 * depends on the format of the file).
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * BUGFIX: 200401030 - worked around a problem with RGBA images
 *
 * TODO: Support tile-based images
 * Test with 16-bit CMYK, Grey
 * Add support for Lab
 * Add indexed->RGB conversion
 *
 */

#include <iostream>
#include <exception>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiffio.h>

#include "../support/debug.h"

#include "imagesource_tiff.h"#include "../profilemanager/lcmswrapper.h"

using namespace std;


class IS_TIFFStrip
{
	public:
	IS_TIFFStrip(ImageSource_TIFF *header,int row);
	~IS_TIFFStrip();
	private:
	void Dump(struct IS_TIFFStrip *strip);
	int firstrow;
	int lastrow;
	unsigned char *imgdata;
	IS_TIFFStrip *next,*prev;
	ImageSource_TIFF *header;
	friend class ImageSource_TIFF;
};


ImageSource_TIFF::~ImageSource_TIFF()
{
	if(file)
		TIFFClose(file);

	while(strips)
		delete strips;
}


IS_TIFFStrip::~IS_TIFFStrip()
{
	if(next)
		next->prev=prev;
	if(prev)
		prev->next=next;
	else
		header->strips=next;

	if(imgdata)
		free(imgdata);
}


IS_TIFFStrip::IS_TIFFStrip(ImageSource_TIFF *header,int row) : next(NULL), header(header)
{
	int i,j;

	if((prev=header->strips))
	{
		while(prev->next)
			prev=prev->next;
		prev->next=this;
	}
	else
		header->strips=this;
	
	i=row/header->stripheight;
	header->filerow=i*header->stripheight;

	firstrow=header->filerow;
	lastrow=header->filerow+header->stripheight-1;
	if(lastrow>=header->height)
		lastrow=header->height-1;

	imgdata=(unsigned char *)malloc(header->stripsize);

	switch(header->photometric)
	{
		case PHOTOMETRIC_MINISWHITE:
			switch(header->bps)
			{
				case 1:
					TIFFReadEncodedStrip(header->file, i, imgdata, (tsize_t)-1);
					break;
				case 8:
//					switch(header->samplesperpixel)
//					{
//						case 1:
							TIFFReadEncodedStrip(header->file, i, imgdata, (tsize_t)-1);
							break;
//						case 2:
//							TIFFReadEncodedStrip(header->file, i, imgdata, (tsize_t)-1);
//							for(j=0;j<(header->stripsize/2);++j)
//								imgdata[j]=imgdata[j*2];
//							break;
//						default:
//							break;
//					}
					break;
				case 16:
					switch(header->samplesperpixel)
					{
						case 1:
							TIFFReadEncodedStrip(header->file, i, imgdata, (tsize_t)-1);
							break;
						default:
							Debug[ERROR] << "FIXME - 16-bit greyscale data with 2 samples per pixel not yet handled" << endl;
							break;
					}
					break;
			}
			break;
		case PHOTOMETRIC_MINISBLACK:
			switch(header->bps)
			{
				case 1:
					TIFFReadEncodedStrip(header->file, i, imgdata, (tsize_t)-1);
					for(j=0;j<(header->stripsize);++j)
						imgdata[j]=imgdata[j]^255;
					break;
				case 8:
//					switch(header->samplesperpixel)
//					{
//						case 1:
							TIFFReadEncodedStrip(header->file, i, imgdata, (tsize_t)-1);
							for(j=0;j<(header->stripsize);++j)
								imgdata[j]=255-imgdata[j];
//							break;
//						case 2:
//							TIFFReadEncodedStrip(header->file, i, imgdata, (tsize_t)-1);
//							for(j=0;j<(header->stripsize/2);++j)
//								imgdata[j]=255-imgdata[j*2];
//							break;
//						default:
//							break;
//					}
					break;
				case 16:
					switch(header->samplesperpixel)
					{
						case 1:
							TIFFReadEncodedStrip(header->file, i, imgdata, (tsize_t)-1);
							{
								for(j=0;j<(header->stripsize/2);++j)
									imgdata[j]=imgdata[j]^65535;
							}
							break;
						default:
							Debug[ERROR] << "FIXME - 16-bit greyscale data with 2 samples per pixel not yet handled" << endl;
							break;
					}
					break;
			}
			break;
		case PHOTOMETRIC_PALETTE:
			TIFFReadEncodedStrip(header->file, i, imgdata, (tsize_t)-1);
			for(j=0;j<(header->stripsize);++j)
				imgdata[j]=header->greypalette[imgdata[j]];
			break;
		case PHOTOMETRIC_RGB:
		case PHOTOMETRIC_SEPARATED:
			TIFFReadEncodedStrip(header->file, i, imgdata, (tsize_t)-1);
			break;
		default:
			break;	
	}
}


IS_TIFFStrip *ImageSource_TIFF::GetStrip(int row)
{
	IS_TIFFStrip *strip;

	strip=strips;
	while((strip) && ((strip->firstrow>row) || (strip->lastrow<row)))
	{
		IS_TIFFStrip *next=strip->next;
		delete strip;
		strip=next;
	}

	if(!(strip))
		strip=new IS_TIFFStrip(this,row);

	return(strip);
}


ISDataType *ImageSource_TIFF::GetRow(int row)
{
	unsigned char *srcrow=NULL;
	struct IS_TIFFStrip *strip;

	if(currentrow==row)
		return(rowbuffer);

	strip=GetStrip(row);

	if(strip)
	{
		int i,j,t,t2;
		if(bps==16)
			srcrow=strip->imgdata+(row-strip->firstrow)*spr*2;
		else
			srcrow=strip->imgdata+(row-strip->firstrow)*spr;
		
		switch(bps)
		{
			case 1:
				for(i=0;i<spr;++i)
				{
					t=srcrow[i];
					for(j=0;j<((width-i*8)>7 ? 8 : width-i*8 );++j)
					{
						t2=0;
						if(t&128) t2=255;
						t<<=1;
						rowbuffer[i*8+j]=EIGHTTOIS(t2);
					}
				}
				break;
			case 8:
				for(i=0;i<spr;++i)
					rowbuffer[i]=EIGHTTOIS(srcrow[i]);
				break;
			case 16:
				unsigned short *srcrow16=(unsigned short *)srcrow;
				for(i=0;i<spr;++i)
					rowbuffer[i]=srcrow16[i];
				break;
		}
	}
	currentrow=row;

	return(rowbuffer);
}


int ImageSource_TIFF::CountTIFFDirs(const char *filename,int &largestdir)
{
	largestdir=0;
	int count=0;
	long largestarea=0;
	if(!(file=TIFFOpen(filename,"r")))
	{
		throw "Can't open file...";
	}

	do
	{
		++count;	
		uint32 width,height;
		TIFFGetField(file, TIFFTAG_IMAGEWIDTH, &width);
		TIFFGetField(file, TIFFTAG_IMAGELENGTH, &height);		Debug[TRACE] << "Got image with dimensions " << width << " x " << height << endl;
		if((width*height)>largestarea)
		{
			largestarea=width*height;
			largestdir=count-1;
		}
	} while(TIFFReadDirectory(file));
	if(file)
		TIFFClose(file);
	file=NULL;

	Debug[TRACE] << "A total of " << count << "sub-images, the largest being " << largestdir << endl;

	return(count);
}

ImageSource_TIFF::ImageSource_TIFF(const char *filename) : ImageSource()
{
	ttile_t stripcount=0;
	uint16 photometric=0,spp=0,bps=0;
	uint32 width=0,height=0,sl=0;
	uint32 stripsize=0;
	float xres=72.0;
	float yres=72.0;
	uint16 resunit=RESUNIT_INCH;
	uint16 *palettered,*palettegreen,*paletteblue;
	uint16 inkset;
	int i;

	type=IS_TYPE_NULL;

	strips=NULL;

	int largestimage=0;
	CountTIFFDirs(filename,largestimage);

	if(!(file=TIFFOpen(filename,"r")))
	{
		throw "Can't open file...";
	}

//	TIFFReadDirectory(file);

	TIFFGetField(file, TIFFTAG_PHOTOMETRIC, &photometric);
	TIFFGetField(file, TIFFTAG_SAMPLESPERPIXEL, &spp);
	TIFFGetField(file, TIFFTAG_BITSPERSAMPLE, &bps);
	TIFFGetField(file, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(file, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(file, TIFFTAG_ROWSPERSTRIP, &sl);

	// If striplength is undefined, assume the image data is in a single strip.
	if(sl==0) sl=height;

	switch(bps)
	{
		case 1:
		case 8:
		case 16:
			break;
		default:
			throw "Source file should have either 1, 8 or 16 bits per sample...";
			break;
	}
	
	stripsize=TIFFStripSize(file);
	stripcount=TIFFNumberOfStrips(file);

	switch(photometric)
	{
		case PHOTOMETRIC_MINISWHITE:
		case PHOTOMETRIC_MINISBLACK:
			if(bps==1)
				type=IS_TYPE_BW;
			else
			{
				switch(spp)
				{
					case 1:
						type=IS_TYPE_GREY;
						break;
					case 2:
						type=IS_TYPE_GREYA;
						break;
					default:
						break;
				}
			}
			break;
		case PHOTOMETRIC_PALETTE:
			TIFFGetField(file, TIFFTAG_COLORMAP,&palettered,&palettegreen,&paletteblue);
			for(i=0;i<256;++i)
			{
				int r=palettered[i]>>8;
				int g=palettegreen[i]>>8;
				int b=paletteblue[i]>>8;
				int grey=(r+g+b)/3;
				greypalette[i]=grey;
			}
			if(bps==1)
			{
				if((greypalette[0]==0) && (greypalette[1]==255))
				{
					photometric=PHOTOMETRIC_MINISBLACK;
				}
				else if((greypalette[0]==255) && (greypalette[1]==0))
				{
					photometric=PHOTOMETRIC_MINISWHITE;
				}
				type=IS_TYPE_BW;
			}
			else
				type=IS_TYPE_GREY;
			break;
		case PHOTOMETRIC_RGB:
			switch(spp)
			{
				case 3:
					type=IS_TYPE_RGB;
					break;
				case 4:
					type=IS_TYPE_RGBA;
					break;
				default:
					throw "ISTIFF Panic: RGB images must have 3 or 4 samples per pixel!";
					break;
			}
			break;
		case PHOTOMETRIC_SEPARATED:
			TIFFGetField(file, TIFFTAG_INKSET, &inkset);
			switch(spp)
			{
				case 4:
					type=IS_TYPE_CMYK;
					break;
				case 5:
					type=IS_TYPE_CMYKA;
					break;
				default:
					type=IS_TYPE_DEVICEN;
					break;
			}
			break;
		default:
			throw "Unsupported file format - must be either 1, 8 or 16-bit greyscale, RGB or CMYK...";
	}

	TIFFGetField(file, TIFFTAG_RESOLUTIONUNIT, &resunit);
	TIFFGetField(file, TIFFTAG_XRESOLUTION, &xres);
	TIFFGetField(file, TIFFTAG_YRESOLUTION, &yres);

	char *profbuffer;
	int proflen;

	if(TIFFGetField(file, TIFFTAG_ICCPROFILE, &proflen, &profbuffer))
	{
		SetEmbeddedProfile(new CMSProfile(profbuffer,proflen),true);
	}

	if(resunit==RESUNIT_CENTIMETER)
	{
		xres*=2.54;
		yres*=2.54;
	}

	Debug[TRACE] << "Resolution: " << xres << " by " << yres << endl;

	this->width=width;
	this->height=height;
	this->stripheight=sl;
	this->stripsize=stripsize;
	this->stripcount=stripcount;
	this->xres=int(xres);
	this->yres=int(yres);
	this->resunit=resunit;
	this->samplesperpixel=spp;
	this->bps=bps;
	this->photometric=photometric;
	filerow=0;
	
	randomaccess=true;

	this->spr=(width*samplesperpixel);
	if(bps==1)
		this->spr=(this->spr+7)/8;

	source_spp=samplesperpixel;

//  This is no longer valid.  If memory serves it was to hack around Greyscale TIFFs with Alpha.
//  FIXME: Verify that greyscale TIFFs with Alpha still work OK!
//	if(samplesperpixel==2)
//	{
//		this->spr/=2;
//	}
	
	Debug[TRACE] << "TIFF Samples per pixel: " << samplesperpixel << endl;
	Debug[TRACE] << "Samples per row: " << this->spr << endl;
	
	MakeRowBuffer();
}
