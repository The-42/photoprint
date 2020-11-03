/*
 * imageselector.cpp - provides a custom widget for selecting a border image.
 *
 * Copyright (c) 2006-2009 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


#include <iostream>
#include <string>
#include <vector>

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <gtk/gtkentry.h>
#include <gtk/gtklist.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtktreemodel.h>
#include <gtk/gtkscrolledwindow.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixdata.h>

#include "../support/debug.h"

#include "egg-pixbuf-thumbnail.h"
#include "generaldialogs.h"

#include "imageselector.h"

#include "imageselector_noborder_image.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
 
#define _(x) gettext(x)

using namespace std;

enum {
	CHANGED_SIGNAL,
	DOUBLECLICKED_SIGNAL,
	LAST_SIGNAL
};

static guint imageselector_signals[LAST_SIGNAL] = { 0 };

static void imageselector_class_init (ImageSelectorClass *klass);
static void imageselector_init (ImageSelector *sel);


struct ImageEntry
{
	GdkPixbuf *pixbuf;
	char *filename;
};


static ImageEntry *find_filename(ImageSelector *il,const char *filename)
{
	GList *iter=il->imagelist;
	while(iter)
	{
		ImageEntry *ii=(ImageEntry *)iter->data;
		if(ii && (ii->filename==NULL) && (filename==NULL))
			return(ii);
		if(ii && ii->filename && strcmp(ii->filename,filename)==0)
			return(ii);
		iter=g_list_next(iter);
	}
	return(NULL);
}


static ImageEntry *find_pixbuf(ImageSelector *il,GdkPixbuf *pb)
{
	GList *iter=il->imagelist;
	while(iter)
	{
		ImageEntry *ii=(ImageEntry *)iter->data;
		if(ii && ii->pixbuf==pb)
			return(ii);
		iter=g_list_next(iter);
	}
	return(NULL);
}


static gint is_cmpfunc(const void *p1,const void *p2)
{
	ImageEntry *i1=(ImageEntry *)p1;
	ImageEntry *i2=(ImageEntry *)p2;
	return(strcmp(i1->filename,i2->filename));
}


static void clear_list(ImageSelector *il)
{
	gtk_list_store_clear(il->liststore);

	GList *iter=il->imagelist;
	while(iter)
	{
		ImageEntry *ii=(ImageEntry *)iter->data;
		if(ii->pixbuf)
			g_object_unref(G_OBJECT(ii->pixbuf));
		if(ii->filename);
			free(ii->filename);
		GList *niter=g_list_next(iter);
		il->imagelist=g_list_delete_link(il->imagelist,iter);
		delete ii;
		iter=niter;
	}
	il->imagelist=NULL;
}


static ImageEntry *add_node(ImageSelector *il,const char *filename,GdkPixbuf *pb=NULL)
{
	ImageEntry *ii=NULL;
	GError *err=NULL;

	if(il->searchpath)
	{
		char *rel=il->searchpath->MakeRelative(filename);
		if((ii=find_filename(il,rel)))
		{
			free(rel);
			return(ii);
		}
		free(rel);
	}

	if(filename && !pb)
		pb=egg_pixbuf_get_thumbnail_for_file(filename,EGG_PIXBUF_THUMBNAIL_NORMAL,&err);
	if(pb)
	{
		ii=new ImageEntry;
		if(il->searchpath)
			ii->filename=il->searchpath->MakeRelative(filename);
		else
			ii->filename=strdup(filename);
		ii->pixbuf=pb;
		
		il->imagelist=g_list_append(il->imagelist,ii);

		GtkTreeIter iter1;
		gtk_list_store_append(il->liststore,&iter1);
		gtk_list_store_set(il->liststore,&iter1,0,ii->pixbuf,-1);
	}
	else
		cerr << "Thumbnail loading failed" << endl;
	return(ii);
}


static void populate_list(ImageSelector *il)
{
	if(!il->searchpath)
		return;

	GtkTreeIter iter1;

	cerr << "Fetching filenames..." << endl;

	SearchPathIterator spi(*il->searchpath);

	const char *path=spi.GetNextFilename(NULL);
	while(path)
	{
		if(!find_filename(il,path))
		{
			ImageEntry *ii=new ImageEntry;
			ii->filename=strdup(path);
			ii->pixbuf=NULL;
			il->imagelist=g_list_append(il->imagelist,ii);
		}
		path=spi.GetNextFilename(path);
	}

	cerr << "Sorting list..." << endl;
	
	il->imagelist=g_list_sort(il->imagelist,is_cmpfunc);

    GdkPixdata pd;
    GError *err=NULL;
	ImageEntry *ii=new ImageEntry;

	gdk_pixdata_deserialize(&pd,sizeof(noborder),noborder,&err);
	ii->pixbuf=gdk_pixbuf_from_pixdata(&pd,false,&err);
	ii->filename=NULL;

	il->imagelist=g_list_prepend(il->imagelist,ii);


	cerr << "Fetching thumbnails and showing list nodes..." << endl;

	GList *liter=il->imagelist;
	while(liter)
	{
		GError *err=NULL;
		ImageEntry *ii=(ImageEntry *)liter->data;
		if(!ii)
			cerr << "PANIC - Null node!" << endl;

		GdkPixbuf *pb=NULL;
		if(ii->filename)
			pb=egg_pixbuf_get_thumbnail_for_file(il->searchpath->SearchPaths(ii->filename),EGG_PIXBUF_THUMBNAIL_NORMAL,&err);
		else
			pb=ii->pixbuf;
		if(pb)
		{
			ii->pixbuf=pb;

			gtk_list_store_append(il->liststore,&iter1);
			gtk_list_store_set(il->liststore,&iter1,0,ii->pixbuf,-1);

			while(gtk_events_pending())
				gtk_main_iteration_do(false);

			liter=g_list_next(liter);
		}
		else
		{
			free(ii->filename);
			GList *niter=g_list_next(liter);
			il->imagelist=g_list_delete_link(il->imagelist,liter);
			delete ii;
			liter=niter;
		}
	}

	// If we've already got a filename selected, make sure it's added
	// if it's not already part of the list.
	ii=find_filename(il,il->filename);
	if(!ii)
		ii=add_node(il,il->filename);
}


static void rebuild_liststore(ImageSelector *c)
{
	if(!c->imagelist)
		populate_list(c);
//	else
//	{
		// Rebuild list view from ImageSelector
		GList *iter=c->imagelist;
	
		GtkTreeIter iter1;
	
		gtk_list_store_clear(c->liststore);
	
		while(iter)
		{
			ImageEntry *ii=(ImageEntry *)iter->data;
			gtk_list_store_append(c->liststore,&iter1);
			gtk_list_store_set(c->liststore,&iter1,0,ii->pixbuf,-1);
			iter=g_list_next(iter);
		}	
//	}
}


class multiselectdata
{
	public:
	multiselectdata(ImageSelector *sel,GtkTreeModel *model) : sel(sel), model(model)
	{
		if(sel->selectionlist)
			delete sel->selectionlist;
		sel->selectionlist=new vector<string>;
	}
	~multiselectdata()
	{
	}
	static void add_name(gpointer data,gpointer userdata)
	{
		DebugLevel oldlevel=Debug.SetLevel(TRACE);

		multiselectdata *msd=(multiselectdata *)userdata;
		GtkTreePath *treepath=(GtkTreePath *)data;
		GtkTreeIter iter;

		GdkPixbuf *pb;
		if(gtk_tree_model_get_iter(msd->model,&iter,treepath))
		{
			gtk_tree_model_get(msd->model,&iter,0,&pb,-1);
			ImageEntry *ii=find_pixbuf(msd->sel,pb);
			if(ii && ii->filename && msd->sel->selectionlist)
			{
				msd->sel->selectionlist->push_back(string(ii->filename));
				Debug[TRACE] << "MultiSelectData - Found: " << ii->filename << endl;
			}
		}
		else
			Debug[WARN] << "MultiSelectData: Unable to select path" << endl;

		Debug.SetLevel(oldlevel);
	}
	protected:
	ImageSelector *sel;
	GtkTreeModel *model;
};

static void selection_changed(GtkTreeSelection *select,gpointer user_data)
{
	ImageSelector *pe=IMAGESELECTOR(user_data);
	if(pe->selmode==GTK_SELECTION_MULTIPLE)
	{
		GtkTreeModel *model;
		GList *list=gtk_tree_selection_get_selected_rows(select,&model);

		multiselectdata msd(pe,model);

		g_list_foreach (list, multiselectdata::add_name,&msd);

		g_list_foreach (list, (GFunc)gtk_tree_path_free, NULL);
		g_list_free (list);

		cerr << "Selection_changed - list size: " << pe->selectionlist->size() << endl;
	}
	else
	{
		GtkTreeIter iter;
		GtkTreeModel *model;
		
		if (gtk_tree_selection_get_selected (select,&model, &iter))
		{
			GdkPixbuf *pb;

			gtk_tree_model_get (model, &iter, 0, &pb, -1);
			ImageEntry *ii=find_pixbuf(pe,pb);
			if(ii)
			{
				if(pe->filename)
					free(pe->filename);
				pe->filename=NULL;

				if(ii->filename)
				{
					if(pe->searchpath)
						pe->filename=pe->searchpath->SearchPaths(ii->filename);
					else
						pe->filename=strdup(ii->filename);
				}
			}
			g_signal_emit(G_OBJECT (pe),imageselector_signals[CHANGED_SIGNAL], 0);
		}
	}
}


static void row_activated(GtkTreeView *tree_view,GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
	ImageSelector *pe=IMAGESELECTOR(user_data);
	cerr << "Got row-activated signal" << endl;
	g_signal_emit(G_OBJECT (pe),imageselector_signals[DOUBLECLICKED_SIGNAL], 0);
}


static void imageselector_other(GtkTreeSelection *select,gpointer user_data)
{
	ImageSelector *is=IMAGESELECTOR(user_data);
	char *fn;
	if((fn=File_Dialog("Select image...",NULL,NULL,true)))
	{
		imageselector_set_filename(is,fn);
		g_free(fn);
		g_signal_emit(G_OBJECT (is),imageselector_signals[CHANGED_SIGNAL], 0);
	}
}


#define TARGET_URI_LIST 1


static GtkTargetEntry dnd_file_drop_types[] = {
	{ "text/uri-list", 0, TARGET_URI_LIST }
};
static gint dnd_file_drop_types_count = 1;

static void get_dnd_data(GtkWidget *widget, GdkDragContext *context,
				     gint x, gint y,
				     GtkSelectionData *selection_data, guint info,
				     guint time, gpointer data)
{
	gchar *urilist=g_strdup((const gchar *)selection_data->data);
	ImageSelector *is=IMAGESELECTOR(widget);

	while(*urilist)
	{
		if(strncmp(urilist,"file:",5))
		{
			g_print("Warning: only local files (file://) are currently supported\n");
			while(*urilist && *urilist!='\n' && *urilist!='\r')
				++urilist;
			while(*urilist=='\n' || *urilist=='\r')
				*urilist++;
		}
		else
		{
			gchar *uri=urilist;
			while(*urilist && *urilist!='\n' && *urilist!='\r')
				++urilist;
			if(*urilist)
			{
				while(*urilist=='\n' || *urilist=='\r')
					*urilist++=0;
				gchar *filename=g_filename_from_uri(uri,NULL,NULL);
				imageselector_set_filename(is,filename);
			}
		}
	}
	g_signal_emit_by_name (GTK_OBJECT (widget), "changed");
}


GtkWidget*
imageselector_new (SearchPathHandler *sp,GtkSelectionMode selmode,bool allowother)
{
	ImageSelector *c=IMAGESELECTOR(g_object_new (imageselector_get_type (), NULL));

	c->liststore=gtk_list_store_new(1,GDK_TYPE_PIXBUF);
	c->searchpath=sp;
	c->selmode=selmode;

	if(allowother)
	{
		gtk_drag_dest_set(GTK_WIDGET(c),
				  GtkDestDefaults(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP),
				  dnd_file_drop_types, dnd_file_drop_types_count,
	                          GdkDragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_LINK));
		g_signal_connect(G_OBJECT(c), "drag_data_received",
				 G_CALLBACK(get_dnd_data), NULL);
	}

	GtkWidget *sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (c), sw, TRUE, TRUE, 0);
	gtk_widget_show(sw);

	c->treeview=gtk_tree_view_new_with_model(GTK_TREE_MODEL(c->liststore));
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	renderer=gtk_cell_renderer_pixbuf_new();
	g_object_set(G_OBJECT(renderer),"xpad",4,NULL);
	g_object_set(G_OBJECT(renderer),"ypad",4,NULL);
	column=gtk_tree_view_column_new_with_attributes(_("Image"),renderer,"pixbuf",0,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(c->treeview),column);
	g_signal_connect (G_OBJECT (c->treeview), "row-activated",
		G_CALLBACK (row_activated),c);

	GtkTreeSelection *select;
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (c->treeview));
	gtk_tree_selection_set_mode (select, c->selmode);
	g_signal_connect (G_OBJECT (select), "changed",
		G_CALLBACK (selection_changed),c);

	gtk_container_add(GTK_CONTAINER(sw),c->treeview);
	gtk_widget_show(c->treeview);
	
	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(c),hbox,FALSE,FALSE,0);
	gtk_widget_show(hbox);

	if((c->selmode==GTK_SELECTION_SINGLE) && allowother)
	{
		GtkWidget *addbutton=gtk_button_new_with_label(_("Other..."));	
		gtk_box_pack_start(GTK_BOX(c),addbutton,FALSE,FALSE,0);
		g_signal_connect(addbutton,"clicked",G_CALLBACK(imageselector_other),c);
		gtk_widget_show(addbutton);
	}
	
	return(GTK_WIDGET(c));
}


GType
imageselector_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo imageselector_info =
		{
			sizeof (ImageSelectorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) imageselector_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (ImageSelector),
			0,
			(GInstanceInitFunc) imageselector_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "ImageSelector", &imageselector_info, GTypeFlags(0));
	}
	return stpuic_type;
}


static void *parent_class=NULL;

static void imageselector_destroy(GtkObject *object)
{
	ImageSelector *il=(ImageSelector *)object;
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);

	clear_list(il);

	if(il->filename)
		free(il->filename);
	il->filename=NULL;
}


static void
imageselector_class_init (ImageSelectorClass *cls)
{
	GtkObjectClass *object_class=(GtkObjectClass *)cls;
//	GtkWidgetClass *widget_class=(GtkWidgetClass *)cls;

	parent_class = gtk_type_class (gtk_widget_get_type ());

	object_class->destroy = imageselector_destroy;

	imageselector_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (cls),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (ImageSelectorClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	imageselector_signals[DOUBLECLICKED_SIGNAL] =
	g_signal_new ("double-clicked",
		G_TYPE_FROM_CLASS (cls),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (ImageSelectorClass, doubleclicked),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
imageselector_init (ImageSelector *c)
{
	c->searchpath=NULL;
	c->imagelist=NULL;
	c->filename=NULL;
	c->selectionlist=NULL;
}


gboolean imageselector_refresh(ImageSelector *c)
{
	cerr << "Clearing existing list" << endl;
	clear_list(c);
	cerr << "Rebuilding list" << endl;
	rebuild_liststore(c);
	return(true);
}


const char *imageselector_get_filename(ImageSelector *c,unsigned int idx)
{
	if(c->selectionlist)
	{
		Debug[TRACE] << "Checking selection list" << endl;
		if(idx<c->selectionlist->size())
			return((*c->selectionlist)[idx].c_str());
		else
			Debug[TRACE] << "No filename in selectionlist at index " << idx << endl;
		return(NULL);
	}
	else
		return(c->filename);
}


bool imageselector_add_filename(ImageSelector *c,const char *filename,GdkPixbuf *pb)
{
	ImageEntry *ii=find_filename(c,filename);
	if(!ii)
		ii=add_node(c,filename,pb);
	return(ii!=NULL);
}


void imageselector_set_filename(ImageSelector *c,const char *filename)
{
	ImageEntry *ii=find_filename(c,filename);

	if(!ii)
		ii=add_node(c,filename);

	if(ii)
	{
		if(c->filename)
			free(c->filename);
		c->filename=NULL;
	
		if(filename)
			c->filename=strdup(filename);
		
		GtkTreeIter iter;
		GtkTreePath *path;
	
		if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(c->liststore),&iter))
		{
			do
			{
				GdkPixbuf *pb;
				gtk_tree_model_get(GTK_TREE_MODEL(c->liststore),&iter,0,&pb,-1);
				if(pb==ii->pixbuf)
				{
					path=gtk_tree_model_get_path(GTK_TREE_MODEL(c->liststore),&iter);
					gtk_tree_view_set_cursor(GTK_TREE_VIEW(c->treeview),path,NULL,false);
					gtk_tree_path_free(path);
					break;
				}
			} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(c->liststore),&iter));
		}
	}
}


static void removebyname(ImageSelector *c,const char *filename)
{
	if(filename)
	{
		GList *iter=c->imagelist;
		while(iter)
		{
			ImageEntry *ii=(ImageEntry *)iter->data;
			if(ii && ii->filename && strcmp(ii->filename,filename)==0)
			{
				c->imagelist=g_list_delete_link(c->imagelist,iter);
				delete ii;
				break;
			}
			iter=g_list_next(iter);
		}
	}
}

void imageselector_remove(ImageSelector *c,const char *fn)
{
	if(fn)
	{
		removebyname(c,fn);
	}
	else
	{
		if(c->selectionlist)
		{
			Debug[TRACE] << "Checking selection list" << endl;
			for(int i=0;i<c->selectionlist->size();++i)
				removebyname(c,(*c->selectionlist)[i].c_str());
		}
		else
			removebyname(c,c->filename);
	}
	rebuild_liststore(c);
}

