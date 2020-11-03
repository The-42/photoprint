#ifndef IMAGESOURCE_DOWNSAMPLE_H
#define IMAGESOURCE_DOWNSAMPLE_H

#include "imagesource.h"

class ImageSource_Downsample : public ImageSource
{
	public:
	ImageSource_Downsample(ImageSource *source,int dstwidth,int dstheight);
	~ImageSource_Downsample();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
};


class ImageSource_HDownsample : public ImageSource
{
	public:
	ImageSource_HDownsample(ImageSource *source,int dstwidth);
	~ImageSource_HDownsample();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
};


class ImageSource_VDownsample : public ImageSource
{
	public:
	ImageSource_VDownsample(ImageSource *source,int dstheight);
	~ImageSource_VDownsample();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
	double *tmp;
	int srcrow;
	int acc;
};

#endif
