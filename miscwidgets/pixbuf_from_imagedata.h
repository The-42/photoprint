#ifndef PIXBUFFROMIMAGEDATA_H
#define PIXBUFFROMIMAGEDATA_H

// Wrapper around Gdk functions to parse inline image data as created by
// gdk-pixbuf-csource

#include <gdk/gdk.h>

GdkPixbuf *PixbufFromImageData(const guint8 *data,size_t len);

#endif

