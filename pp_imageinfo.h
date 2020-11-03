#ifndef __PP_IMAGEINFO_H__
#define __PP_IMAGEINFO_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkexpander.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>

#include "layout.h"

G_BEGIN_DECLS

#define PP_IMAGEINFO_TYPE			(pp_imageinfo_get_type())
#define PP_IMAGEINFO(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_IMAGEINFO_TYPE, pp_ImageInfo))
#define PP_IMAGEINFO_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_IMAGEINFO_TYPE, pp_ImageInfoClass))
#define IS_PP_IMAGEINFO(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_IMAGEINFO_TYPE))
#define IS_PP_IMAGEINFO_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_IMAGEINFO_TYPE))

typedef struct _pp_ImageInfo pp_ImageInfo;
typedef struct _pp_ImageInfoClass pp_ImageInfoClass;

struct Signature;
class Thread;
struct _pp_ImageInfo
{
	GtkExpander	expander;
	GtkWidget *table;
	GtkWidget *filename;
	GtkWidget *dimensions;
	GtkWidget *physicalsize;
	GtkWidget *resolution;
	GtkWidget *profile;
	GtkWidget *scrollwin;
	ThreadFunction *thread;
	Layout *layout;
};


struct _pp_ImageInfoClass
{
	GtkExpanderClass parent_class;

	void (*changed)(pp_ImageInfo *ii);
};

GType pp_imageinfo_get_type (void);
GtkWidget* pp_imageinfo_new (Layout *layout);
void pp_imageinfo_refresh(pp_ImageInfo *ob);
void pp_imageinfo_change_image(pp_ImageInfo *ob);
G_END_DECLS

#endif /* __PP_IMAGEINFO_H__ */
