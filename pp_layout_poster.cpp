#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkexpander.h>

#include "support/debug.h"
#include "stpui_widgets/stpui_combo.h"
#include "stpui_widgets/dimension.h"
#include "layout.h"
#include "dialogs.h"
#include "pp_pageextent.h"
#include "pp_imagecontrol.h"
#include "pp_layout_poster_pageview.h"
#include "pp_layout_poster.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)

enum {
	CHANGED_SIGNAL,
	POPUPMENU_SIGNAL,
	SELECTIONCHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint pp_layout_poster_signals[LAST_SIGNAL] = { 0 };

static void pp_layout_poster_class_init (pp_Layout_PosterClass *klass);
static void pp_layout_poster_init (pp_Layout_Poster *stpuicombo);


static void reflow(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Poster *lo=(pp_Layout_Poster *)ob;
	pp_Layout_Poster_PageView *pv=PP_LAYOUT_POSTER_PAGEVIEW(lo->pageview);
	Layout_Poster *l=(Layout_Poster*)lo->state->layout;
	l->Reflow();
	pp_layout_poster_pageview_refresh(PP_LAYOUT_POSTER_PAGEVIEW(pv));

	int pages=l->GetPages();
	pages/=l->htiles*l->vtiles;

	gtk_widget_set_sensitive(lo->page,pages>1);
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(lo->page),1.0,pages);
	g_signal_emit(G_OBJECT (lo),pp_layout_poster_signals[CHANGED_SIGNAL], 0);
}


static void pe_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Poster *lo=(pp_Layout_Poster *)ob;
	Layout_Poster *l=(Layout_Poster*)lo->state->layout;
	l->FlushHRPreviews();
	l->TilesFromSize();
	pp_layout_poster_refresh(lo);
	pp_Layout_Poster_PageView *pv=PP_LAYOUT_POSTER_PAGEVIEW(lo->pageview);
	pp_layout_poster_pageview_refresh(pv);
	g_signal_emit(G_OBJECT (ob),pp_layout_poster_signals[CHANGED_SIGNAL], 0);
}


static void ic_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Poster *lo=(pp_Layout_Poster *)ob;
	Layout_Poster *l=(Layout_Poster*)lo->state->layout;
	l->FlushHRPreviews();
	pp_Layout_Poster_PageView *pv=PP_LAYOUT_POSTER_PAGEVIEW(lo->pageview);
	pp_layout_poster_pageview_refresh(PP_LAYOUT_POSTER_PAGEVIEW(pv));
	g_signal_emit(G_OBJECT (ob),pp_layout_poster_signals[CHANGED_SIGNAL], 0);
}


static void posterwidth_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Poster *lo=(pp_Layout_Poster *)ob;
	Layout_Poster *l=(Layout_Poster*)lo->state->layout;
	l->FlushHRPreviews();

	l->posterwidth=int(dimension_get_pt(DIMENSION(lo->posterwidth)));
	l->TilesFromSize();
	g_signal_emit(G_OBJECT (lo),pp_layout_poster_signals[CHANGED_SIGNAL], 0);
}


static void posterheight_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Poster *lo=(pp_Layout_Poster *)ob;
	Layout_Poster *l=(Layout_Poster*)lo->state->layout;
	l->FlushHRPreviews();

	l->posterheight=int(dimension_get_pt(DIMENSION(lo->posterheight)));
	l->TilesFromSize();
	g_signal_emit(G_OBJECT (lo),pp_layout_poster_signals[CHANGED_SIGNAL], 0);
}


static void htiles_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Poster *lo=(pp_Layout_Poster *)ob;
	Layout_Poster *l=(Layout_Poster*)lo->state->layout;
	l->FlushHRPreviews();
	
	l->htiles=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lo->htiles));
	l->SizeFromTiles();
	g_signal_emit(G_OBJECT (lo),pp_layout_poster_signals[CHANGED_SIGNAL], 0);
}


static void vtiles_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Poster *lo=(pp_Layout_Poster *)ob;
	Layout_Poster *l=(Layout_Poster*)lo->state->layout;
	l->FlushHRPreviews();

	Debug[TRACE] << "In VTiles_Changed" << endl;

	l->vtiles=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lo->vtiles));
	l->SizeFromTiles();
	g_signal_emit(G_OBJECT (lo),pp_layout_poster_signals[CHANGED_SIGNAL], 0);
}


static void hoverlap_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Poster *lo=(pp_Layout_Poster *)ob;
	Layout_Poster *l=(Layout_Poster*)lo->state->layout;
	l->FlushHRPreviews();

	l->hoverlap=int(dimension_get_pt(DIMENSION(lo->hoverlap)));
	l->TilesFromSize();
	g_signal_emit(G_OBJECT (lo),pp_layout_poster_signals[CHANGED_SIGNAL], 0);
}


static void voverlap_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Poster *lo=(pp_Layout_Poster *)ob;
	Layout_Poster *l=(Layout_Poster*)lo->state->layout;
	l->FlushHRPreviews();

	l->voverlap=int(dimension_get_pt(DIMENSION(lo->voverlap)));
	l->TilesFromSize();
	g_signal_emit(G_OBJECT (lo),pp_layout_poster_signals[CHANGED_SIGNAL], 0);
}


static void page_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Poster *lo=(pp_Layout_Poster *)ob;
	pp_Layout_Poster_PageView *pv=PP_LAYOUT_POSTER_PAGEVIEW(lo->pageview);
	int pagesperposter=pv->layout->htiles*pv->layout->vtiles;

	Layout_Poster *l=(Layout_Poster*)lo->state->layout;
	l->CancelRenderThreads();

	pp_layout_poster_pageview_set_page(pv,pagesperposter*(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lo->page))-1));

	pp_imagecontrol_set_image(PP_IMAGECONTROL(lo->imagecontrol));
	g_signal_emit(G_OBJECT (lo),pp_layout_poster_signals[CHANGED_SIGNAL], 0);
}


static void pageview_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Poster *lo=(pp_Layout_Poster *)ob;

	g_signal_emit(G_OBJECT (lo),pp_layout_poster_signals[CHANGED_SIGNAL], 0);
}


static void pageview_popupmenu(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Poster *lo=(pp_Layout_Poster *)ob;
	Debug[TRACE] << "Forwarding popupmenu signal..." << endl;
	g_signal_emit(G_OBJECT (lo),pp_layout_poster_signals[POPUPMENU_SIGNAL], 0);
}


void pp_layout_poster_refresh(pp_Layout_Poster *ob)
{
	Layout_Poster *l=(Layout_Poster *)ob->state->layout;

	l->LayoutToDB(ob->state->layoutdb);

	int pages=l->GetPages();
	int cpage=l->GetCurrentPage();
	pages/=l->htiles*l->vtiles;
	cpage/=l->htiles*l->vtiles;

	gtk_widget_set_sensitive(ob->page,pages!=1);
	if(pages>1)
	{
		gtk_spin_button_set_range(GTK_SPIN_BUTTON(ob->page),1.0,pages);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->page),cpage+1);
	}

	g_signal_handlers_block_matched (G_OBJECT(ob->posterwidth),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, ob);
	g_signal_handlers_block_matched (G_OBJECT(ob->posterheight),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, ob);
	g_signal_handlers_block_matched (G_OBJECT(ob->htiles),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, ob);
	g_signal_handlers_block_matched (G_OBJECT(ob->vtiles),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, ob);
	g_signal_handlers_block_matched (G_OBJECT(ob->hoverlap),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, ob);
	g_signal_handlers_block_matched (G_OBJECT(ob->voverlap),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, ob);

	dimension_set_pt(DIMENSION(ob->posterwidth),l->posterwidth);
	dimension_set_pt(DIMENSION(ob->posterheight),l->posterheight);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->htiles),l->htiles);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->vtiles),l->vtiles);
	dimension_set_pt(DIMENSION(ob->hoverlap),l->hoverlap);
	dimension_set_pt(DIMENSION(ob->voverlap),l->voverlap);

	g_signal_handlers_unblock_matched (G_OBJECT(ob->posterwidth),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, ob);
	g_signal_handlers_unblock_matched (G_OBJECT(ob->posterheight),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, ob);
	g_signal_handlers_unblock_matched (G_OBJECT(ob->htiles),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, ob);
	g_signal_handlers_unblock_matched (G_OBJECT(ob->vtiles),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, ob);
	g_signal_handlers_unblock_matched (G_OBJECT(ob->hoverlap),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, ob);
	g_signal_handlers_unblock_matched (G_OBJECT(ob->voverlap),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, ob);

	pp_imagecontrol_refresh(PP_IMAGECONTROL(ob->imagecontrol));

	pp_Layout_Poster_PageView *pv=PP_LAYOUT_POSTER_PAGEVIEW(ob->pageview);
	if(pv)
		pp_layout_poster_pageview_refresh(pv);
}


void pp_layout_poster_set_unit(GtkWidget *wid,enum Units unit)
{
	pp_Layout_Poster *ob=PP_LAYOUT_POSTER(wid);
	pp_pageextent_set_unit(PP_PAGEEXTENT(ob->pageextent),unit);
	dimension_set_unit(DIMENSION(ob->posterwidth),unit);
	dimension_set_unit(DIMENSION(ob->posterheight),unit);
	dimension_set_unit(DIMENSION(ob->hoverlap),unit);
	dimension_set_unit(DIMENSION(ob->voverlap),unit);
}


static void expander_callback (GObject *object, GParamSpec *param_spec, gpointer userdata)
{
	pp_Layout_Poster *ob=PP_LAYOUT_POSTER(userdata);
	ob->state->SetInt("ExpanderState_Poster",gtk_expander_get_expanded (GTK_EXPANDER(object)));
}


static void expander_callback2 (GObject *object, GParamSpec *param_spec, gpointer userdata)
{
	pp_Layout_Poster *ob=PP_LAYOUT_POSTER(userdata);
	ob->state->SetInt("ExpanderState_PosterOverlap",gtk_expander_get_expanded (GTK_EXPANDER(object)));
}


GtkWidget *pp_layout_poster_new (PhotoPrint_State *state)
{
	pp_Layout_Poster *ob=PP_LAYOUT_POSTER(g_object_new (pp_layout_poster_get_type (), NULL));

	gtk_container_set_border_width(GTK_CONTAINER(&ob->hbox),10);
	
	ob->state=state;
	
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;

	enum Units unit=state->GetUnits();

	GtkWidget *frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX (&ob->hbox), frame,TRUE,TRUE,0);
	gtk_widget_show (frame); 
	
	ob->pageview = pp_layout_poster_pageview_new ((Layout_Poster *)ob->state->layout);
	g_signal_connect(G_OBJECT(ob->pageview),"changed",G_CALLBACK(pageview_changed),ob);
	g_signal_connect(G_OBJECT(ob->pageview),"popupmenu",G_CALLBACK(pageview_popupmenu),ob);
	g_signal_connect(G_OBJECT(ob->pageview),"reflow",G_CALLBACK(reflow),ob);

	gtk_container_add (GTK_CONTAINER (frame), ob->pageview);
	gtk_widget_show (ob->pageview);


	// Scroll Window

	GtkWidget *scrollwin=gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwin),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	vbox = gtk_vbox_new (FALSE, 5);
	gtk_box_pack_start(GTK_BOX(&ob->hbox),vbox,FALSE,FALSE,0);
	gtk_widget_show (vbox);
	gtk_box_pack_start(GTK_BOX(vbox),scrollwin,TRUE,TRUE,0);
	gtk_widget_show (scrollwin);


	// Page number

	hbox=gtk_hbox_new(FALSE,5);
	label=gtk_label_new(_("Page:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
	gtk_widget_show(label);

	ob->page=gtk_spin_button_new_with_range(1.0,2.0,1.0);
	g_signal_connect(G_OBJECT(ob->page),"value-changed",G_CALLBACK(page_changed),ob);
	gtk_widget_show(ob->page);

	gtk_box_pack_start(GTK_BOX(hbox),ob->page,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	gtk_widget_show(hbox);


	// Contents of scrollwindow

	GtkWidget *hbox2=gtk_hbox_new(FALSE,0);
	gtk_widget_show (hbox2);
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox2),vbox,TRUE,TRUE,5);
	
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrollwin), hbox2);
//	gtk_container_foreach(GTK_CONTAINER(scrollwin),killshadow,NULL);
	gtk_widget_show (vbox);


	// PosterSize
	frame=gtk_expander_new(_("Poster Layout"));
	gtk_expander_set_expanded(GTK_EXPANDER(frame),state->FindInt("ExpanderState_Poster"));
	g_signal_connect(frame, "notify::expanded",G_CALLBACK (expander_callback), ob);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	gtk_widget_show(frame);

	GtkWidget *table=gtk_table_new(4,2,false);
	gtk_table_set_row_spacing(GTK_TABLE(table),0,5);
	gtk_table_set_row_spacing(GTK_TABLE(table),2,3);
	gtk_table_set_col_spacings(GTK_TABLE(table),3);
	gtk_container_add(GTK_CONTAINER(frame),table);
	gtk_widget_show(table);

	// Width
	
	label=gtk_label_new(_("Size"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(label),0,1,0,1);
	gtk_widget_show(label);

	ob->posterwidth=dimension_new(350,10000,unit);
	g_signal_connect(G_OBJECT(ob->posterwidth),"value-changed",G_CALLBACK(posterwidth_changed),ob);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->posterwidth),1,2,0,1);
	gtk_widget_show(ob->posterwidth);

	// Height

	label=gtk_label_new(_("by"));
	gtk_misc_set_alignment(GTK_MISC(label),0.5,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(label),2,3,0,1);
	gtk_widget_show(label);

	ob->posterheight=dimension_new(350,10000,unit);
	g_signal_connect(G_OBJECT(ob->posterheight),"value-changed",G_CALLBACK(posterheight_changed),ob);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->posterheight),3,4,0,1);
	gtk_widget_show(ob->posterheight);

	// Tiles - H
	
	label=gtk_label_new(_("Tiles"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(label),0,1,1,2);
	gtk_widget_show(label);

	ob->htiles=gtk_spin_button_new_with_range(1,10,1);
	g_signal_connect(G_OBJECT(ob->htiles),"value-changed",G_CALLBACK(htiles_changed),ob);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->htiles),1,2,1,2);
	gtk_widget_show(ob->htiles);

	// Height

	label=gtk_label_new(_("by"));
	gtk_misc_set_alignment(GTK_MISC(label),0.5,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(label),2,3,1,2);
	gtk_widget_show(label);

	ob->vtiles=gtk_spin_button_new_with_range(1,10,1);
	g_signal_connect(G_OBJECT(ob->vtiles),"value-changed",G_CALLBACK(vtiles_changed),ob);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->vtiles),3,4,1,2);
	gtk_widget_show(ob->vtiles);

	frame=gtk_expander_new(_("Overlap"));
	gtk_expander_set_expanded(GTK_EXPANDER(frame),state->FindInt("ExpanderState_PosterOverlap"));
	g_signal_connect(frame, "notify::expanded",G_CALLBACK (expander_callback2), ob);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	gtk_widget_show(frame);

	table=gtk_table_new(4,1,false);
	gtk_table_set_row_spacings(GTK_TABLE(table),3);
	gtk_table_set_col_spacings(GTK_TABLE(table),3);
	gtk_container_add(GTK_CONTAINER(frame),table);
	gtk_widget_show(table);

	// HOverlap
	
	label=gtk_label_new(_("H:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(label),0,1,0,1);
	gtk_widget_show(label);

	ob->hoverlap=dimension_new(0,72,unit);
	g_signal_connect(G_OBJECT(ob->hoverlap),"value-changed",G_CALLBACK(hoverlap_changed),ob);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->hoverlap),1,2,0,1);
	gtk_widget_show(ob->hoverlap);

	// VOverlap

	label=gtk_label_new(_("V:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(label),2,3,0,1);
	gtk_widget_show(label);

	ob->voverlap=dimension_new(0,72,unit);
	g_signal_connect(G_OBJECT(ob->voverlap),"value-changed",G_CALLBACK(voverlap_changed),ob);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->voverlap),3,4,0,1);
	gtk_widget_show(ob->voverlap);


	// PageExtent

	ob->pageextent=pp_pageextent_new((Layout_Poster *)ob->state->layout,ob->state);
	g_signal_connect(G_OBJECT(ob->pageextent),"changed",G_CALLBACK(pe_changed),ob);
	gtk_box_pack_start(GTK_BOX(vbox),ob->pageextent,FALSE,FALSE,0);
	gtk_widget_show(ob->pageextent);


#if 0
	// Spacer box

	tmp=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),tmp,TRUE,TRUE,0);
	gtk_widget_show(tmp);
#endif

	// ImageControl

	ob->imagecontrol=pp_imagecontrol_new(ob->state->layout);
	gtk_box_pack_start(GTK_BOX(vbox),ob->imagecontrol,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(ob->imagecontrol),"changed",G_CALLBACK(ic_changed),ob);
	gtk_widget_show(ob->imagecontrol);

#if 0
	// Page number

	hbox=gtk_hbox_new(FALSE,5);
	label=gtk_label_new(_("Page:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
	gtk_widget_show(label);

	ob->page=gtk_spin_button_new_with_range(1.0,2.0,1.0);
	g_signal_connect(G_OBJECT(ob->page),"value-changed",G_CALLBACK(page_changed),ob);
	gtk_widget_show(ob->page);

	gtk_box_pack_start(GTK_BOX(hbox),ob->page,FALSE,FALSE,0);


	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	gtk_widget_show(hbox);
#endif

	pp_layout_poster_refresh(ob);
	pp_pageextent_refresh(PP_PAGEEXTENT(ob->pageextent));

	return(GTK_WIDGET(ob));
}


GType
pp_layout_poster_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_layout_poster_info =
		{
			sizeof (pp_Layout_PosterClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_layout_poster_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_Layout_Poster),
			0,
			(GInstanceInitFunc) pp_layout_poster_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "pp_Layout_Poster", &pp_layout_poster_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_layout_poster_class_init (pp_Layout_PosterClass *klass)
{
	pp_layout_poster_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_PosterClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	pp_layout_poster_signals[POPUPMENU_SIGNAL] =
	g_signal_new ("popupmenu",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_PosterClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	pp_layout_poster_signals[SELECTIONCHANGED_SIGNAL] =
	g_signal_new ("selection_changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_PosterClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_layout_poster_init (pp_Layout_Poster *ob)
{
	ob->state=NULL;
	ob->pageview=NULL;
	ob->imagecontrol=NULL;
}
