/*
 * pixbuf_from_imagesource.h
 * Creates a GdkPixbuf from an ImageSource
 *
 * Copyright (c) 2005 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef PIXBUF_FROM_IMAGESOURCE_H
#define PIXBUF_FROM_IMAGESOURCE_H

#include <gdk/gdkpixbuf.h>

#include "imagesource.h"
#include "progress.h"

GdkPixbuf *pixbuf_from_imagesource(ImageSource *is,
	int redbg8=255,int greenbg8=255,int bluebg8=255,Progress *prog=NULL,GdkPixbuf *pb=NULL);

GdkPixbuf *pixbuf_alpha_from_imagesource(ImageSource *is,Progress *prog=NULL,GdkPixbuf *pb=NULL);

#endif
