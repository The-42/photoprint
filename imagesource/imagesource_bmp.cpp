/*
 * imagesource_bmp.cpp
 * 24-bit RGB and 8-bit Greyscale BMP scanline-based Loader
 * Supports Random Access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * 2004-12-01: Fixed problem with interpreting RGB image data
 *
 * 2005-12-24: Fixed greyscale reader - 0 now corresponds to White! 
 *
 * TODO: Support Indexed colour images
 *
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "../support/debug.h"

#include "imagesource_bmp.h"

using namespace std;


class IS_BMPStrip
{
	public:
	IS_BMPStrip(ImageSource_BMP *header,int row);
	~IS_BMPStrip();
	private:
	int firstrow;
	int lastrow;
	unsigned char *imgdata;
	IS_BMPStrip *next,*prev;
	ImageSource_BMP *header;
	static const int stripheight=512;
	friend class ImageSource_BMP;
};


unsigned long ImageSource_BMP::GetValue(char *c,int l)
{
	unsigned long v=0;
	int i;
	for(i=l-1;i>=0;--i)
	{
		v<<=8; v|=(unsigned char)c[i];
	}
	return(v);
}


ImageSource_BMP::~ImageSource_BMP()
{
	while(strips)
		delete strips;

	if(file.is_open())
		file.close();
}


IS_BMPStrip::IS_BMPStrip(ImageSource_BMP *header,int row) : next(NULL), prev(NULL), header(header)
{
	if((prev=header->strips))
	{
		while(prev->next)
			prev=prev->next;
		prev->next=this;
	}
	else
		header->strips=this;
		
	int i=row/stripheight;
	firstrow=i*stripheight;
	lastrow=firstrow+stripheight-1;
	if(lastrow>(header->height-1))
		lastrow=header->height-1;

	int bufsize=((lastrow+1)-firstrow)*header->bytesperrow;
	imgdata=(unsigned char *)malloc(bufsize);

	int filepos=header->imagestart+firstrow*header->bytesperrow;

	header->file.seekg(filepos,ios::beg);
	header->file.read((char *)imgdata,bufsize);
	if(!(header->file.good()))
		Debug[ERROR] << "Read from position " << filepos << " failed" << endl;
}


IS_BMPStrip::~IS_BMPStrip()
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


IS_BMPStrip *ImageSource_BMP::GetStrip(int row)
{
	IS_BMPStrip *strip=strips;
	while(strip)
	{
		if((row>=strip->firstrow)&&(row<=strip->lastrow))
			return(strip);
	
		strip=strip->next;
	}
	return(NULL);
}


ISDataType *ImageSource_BMP::GetRow(int row)
{
	IS_BMPStrip *strip;
	int realrow=(height-1)-row; // BMPs are ordered bottom to top!
	unsigned char *src;
	ISDataType *dst;

	if(currentrow==row)
		return(rowbuffer);

	if(row>=height)
	{
		Debug[WARN] << "ImageSource_BMP - Warning: row " << row+1 << " of " << height << " requested." << endl;
		return(rowbuffer);
	}

	if(!(strip=GetStrip(realrow)))
	{
		while(strips)
			delete strips;
			
		if(!(strip=new IS_BMPStrip(this,realrow)))
			return(rowbuffer);
	}

	src=strip->imgdata+bytesperrow*(realrow-strip->firstrow);
	dst=rowbuffer;
	
	switch(samplesperpixel)
	{
		int x;
		case 4:
			for(x=0;x<width;++x)
			{
				unsigned int r,g,b,a;
				b=*src++;
				g=*src++;
				r=*src++;
				a=*src++;
				*dst++=EIGHTTOIS(r);
				*dst++=EIGHTTOIS(g);
				*dst++=EIGHTTOIS(b);
				*dst++=EIGHTTOIS(a);
			}
			break;
		case 3:
			switch(bitsperpixel)
			{
				case 24:
					for(x=0;x<width;++x)
					{
						unsigned int r,g,b;
						b=*src++;
						g=*src++;
						r=*src++;
						*dst++=EIGHTTOIS(r);
						*dst++=EIGHTTOIS(g);
						*dst++=EIGHTTOIS(b);
					}
					break;
				case 16:
					for(x=0;x<width;++x)
					{
						unsigned int r,g,b;
						unsigned int t1=*src++;
						unsigned int t2=*src++;
						b=(t1&0x1f)<<3;
						g=((t1&0xe0)>>3) | ((t2&0x7) <<5);
						r=(t2&0xf8);
						*dst++=EIGHTTOIS(r);
						*dst++=EIGHTTOIS(g);
						*dst++=EIGHTTOIS(b);
					}
					break;
			}
			break;
		case 1:
// FIXME: deal with paletteless greyscale case
#if 0
			for(x=0;x<width;++x)
			{
				int g=(*src++ + *src++ + *src++)/3;
				*dst++=EIGHTTOIS(255-g);
			}
			break;
#endif
			for(x=0;x<width;++x)
			{
				int c=*src++;
				int g=(palette[c][0] + palette[c][1] + palette[c][2])/3;
				*dst++=IS_SAMPLEMAX-EIGHTTOIS(g);
			}
			break;
	}
	
	currentrow=row;
	return(rowbuffer);
}


ImageSource_BMP::ImageSource_BMP(const char *filename) : ImageSource(), strips(NULL)
{
	char fileheader[14];
	char ImageHeader[64];

	strips=NULL;

	xres=yres=72;  // FIXME - read these from the file

	file.open(filename,ios::in|ios::binary);

	if(!file.is_open())
		throw "Can't open file!";

	file.read(fileheader,14);
	if((fileheader[0]!='B')||(fileheader[1]!='M'))
		throw "Not a BMP file!";

	file.read(ImageHeader,12);

	switch(int headerlen=GetValue(ImageHeader,4))
	{
		case 12:
			width=GetValue(ImageHeader+4,2);
			height=GetValue(ImageHeader+6,2);
			bitsperpixel=GetValue(ImageHeader+10,2);
			cmapbytes=3;
			if(bitsperpixel==8)
			cmapentries=256;
			break;
		case 40:
		case 56:
		case 64:
			file.read(ImageHeader+12,headerlen-12);
			width=GetValue(ImageHeader+4,4);
			height=GetValue(ImageHeader+8,4);
			bitsperpixel=GetValue(ImageHeader+14,2);
			xres=(GetValue(ImageHeader+24,2)*254+5000)/10000;
			yres=(GetValue(ImageHeader+28,2)*254+5000)/10000;
			cmapentries=GetValue(ImageHeader+32,4);
			cmapbytes=4;
			break;
		default:
			file.close();
			throw "Unknown header type";
	}

	int inbpp;

	imagestart=GetValue(fileheader+10,4);
	switch(bitsperpixel)
	{
		case 32:
			inbpp=samplesperpixel=4;
			type=IS_TYPE_RGBA;
			break;
		case 16:
			inbpp=2;
			samplesperpixel=3;
			type=IS_TYPE_RGB;
			break;
		case 24:
			inbpp=samplesperpixel=3;
			type=IS_TYPE_RGB;
			break;
		case 8:
			inbpp=samplesperpixel=1;
			type=IS_TYPE_GREY;
			break;
		default:
			file.close();
			throw "BMP loader doesn't yet support Indexed Colour";
	}

	if((bitsperpixel<=8)&&(cmapentries==0))
		cmapentries=1<<bitsperpixel;

	bytesperrow=((inbpp*width)+3)&~3;
	
	unsigned char pal[1024];
	if(cmapbytes<5)
	{
		int i,j;
		if(cmapentries>256)
			cmapentries=256;
		file.read((char *)pal,cmapentries*cmapbytes);
		for(i=0;i<cmapentries;++i)
		{
			int r,g,b;
			j=cmapbytes*i;
			r=pal[j+2]; g=pal[j+1]; b=pal[j];
			palette[i][0]=r;
			palette[i][1]=g;
			palette[i][2]=b;
		}
	}

	embeddedprofile=NULL;

	Debug[TRACE] << "BMP type: " << type << endl;

	MakeRowBuffer();
	randomaccess=true;
}

