/*
 * stpui_slider.c - provides a custom widget for providing autonomous control over
 * a float or int option in the stp_vars_t structure.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


/* FIXME:  Currently, the "changed" signal will only be emitted when the user
 *         releases the mouse button having moved the slider / spin button.
 *         No signal is emitted if the user uses the keyboard.
 */

#include <string.h>

#include "stpui_slider.h"
#include "dimension.h"
#include "units.h"

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint stpui_slider_signals[LAST_SIGNAL] = { 0 };

static void stpui_slider_class_init (stpui_SliderClass *klass);
static void stpui_slider_init (stpui_Slider *stpuicombo);


static gboolean stpui_slider_update(stpui_Slider *c)
{
	stp_parameter_t desc;
	gboolean result;

	gboolean enabled=TRUE;

	if(c->checkbutton)
		enabled=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->checkbutton));

	stp_describe_parameter(c->vars,c->optionname,&desc);
	if(desc.is_active)
	{
		switch(desc.p_type)
		{
			case STP_PARAMETER_TYPE_DOUBLE:
				{
					double min,max,current;
					min=desc.bounds.dbl.lower;
					max=desc.bounds.dbl.upper;
					current=stp_get_float_parameter(c->vars,c->optionname);
					gtk_range_set_range(GTK_RANGE(c->scale),min,max);
					gtk_range_set_value(GTK_RANGE(c->scale),current);
					gtk_spin_button_set_range(GTK_SPIN_BUTTON(c->spin),min,max);
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(c->spin),current);
					break;
				}
			case STP_PARAMETER_TYPE_INT:
				{
					int min,max,current;
					min=desc.bounds.integer.lower;
					max=desc.bounds.integer.upper;
					current=stp_get_int_parameter(c->vars,c->optionname);
					gtk_range_set_range(GTK_RANGE(c->scale),min,max);
					gtk_range_set_value(GTK_RANGE(c->scale),current);
					gtk_spin_button_set_range(GTK_SPIN_BUTTON(c->spin),min,max);
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(c->spin),current);
					break;
				}
			case STP_PARAMETER_TYPE_DIMENSION:
				{
					int min,max,current;
					min=desc.bounds.integer.lower;
					max=desc.bounds.integer.upper;
					current=stp_get_dimension_parameter(c->vars,c->optionname);
					// Convert to correct unit here...
					gtk_range_set_range(GTK_RANGE(c->scale),min,max);
					gtk_range_set_value(GTK_RANGE(c->scale),current);
					dimension_set_range_pt(DIMENSION(c->spin),min,max);
					dimension_set_pt(DIMENSION(c->spin),current);
					break;
				}
			default:
				break;
		}
	}
	if(desc.is_active)
	{
		gtk_widget_show(GTK_WIDGET(c));
		if(c->checkbutton)
			gtk_widget_show(GTK_WIDGET(c->checkbutton));
		if(c->label)
			gtk_widget_show(GTK_WIDGET(c->label));
	}
	else
	{
		gtk_widget_hide(GTK_WIDGET(c));
		if(c->checkbutton)
			gtk_widget_hide(GTK_WIDGET(c->checkbutton));
		if(c->label)
			gtk_widget_hide(GTK_WIDGET(c->label));
	}
	gtk_widget_set_sensitive(GTK_WIDGET(c->scale),desc.is_active);
	gtk_widget_set_sensitive(GTK_WIDGET(c->spin),desc.is_active & enabled);
	if(c->checkbutton)
		gtk_widget_set_sensitive(GTK_WIDGET(c->checkbutton),desc.is_active);
	result=desc.is_active;
	return(result);
}

static gboolean stpui_slider_released(GtkWidget *w,GdkEventButton *e,gpointer ud)
{
	stpui_Slider *s=STPUI_SLIDER(ud);
	double value;

	switch(s->type)
	{
		case STP_PARAMETER_TYPE_DOUBLE:
			value=gtk_range_get_value(GTK_RANGE(s->scale));
			stp_set_float_parameter(s->vars,s->optionname,value);
			break;
		case STP_PARAMETER_TYPE_INT:
			value=gtk_range_get_value(GTK_RANGE(s->scale));
			stp_set_int_parameter(s->vars,s->optionname,value);
			break;
		case STP_PARAMETER_TYPE_DIMENSION:
			value=gtk_range_get_value(GTK_RANGE(s->scale));
			stp_set_dimension_parameter(s->vars,s->optionname,value);
			break;
		default:
			break;
	}	

	stpui_slider_refresh(s);

	g_signal_emit(G_OBJECT (s),
		stpui_slider_signals[CHANGED_SIGNAL], 0);

	return(FALSE);
}


static gboolean stpui_spin_released(GtkWidget *w,GdkEventButton *e,gpointer ud)
{
	stpui_Slider *s=STPUI_SLIDER(ud);
	double value;
	
	switch(s->type)
	{
		case STP_PARAMETER_TYPE_DOUBLE:
			value=gtk_spin_button_get_value(GTK_SPIN_BUTTON(s->spin));
			stp_set_float_parameter(s->vars,s->optionname,value);
			break;
		case STP_PARAMETER_TYPE_INT:
			value=gtk_spin_button_get_value(GTK_SPIN_BUTTON(s->spin));
			stp_set_int_parameter(s->vars,s->optionname,value);
			break;
		case STP_PARAMETER_TYPE_DIMENSION:
			value=dimension_get_pt(DIMENSION(s->spin));
			stp_set_dimension_parameter(s->vars,s->optionname,value);
			break;
		default:
			break;
	}	

	stpui_slider_refresh(s);

	g_signal_emit(G_OBJECT (s),
		stpui_slider_signals[CHANGED_SIGNAL], 0);

	return(FALSE);
}


static void stpui_slider_changed(GtkWidget *w,gpointer *ud)
{
	stpui_Slider *s=STPUI_SLIDER(ud);
	double value;
	
	if(s->checkbutton)
		gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(s->checkbutton),TRUE);

	switch(s->type)
	{
		case STP_PARAMETER_TYPE_DOUBLE:
			value=gtk_range_get_value(GTK_RANGE(s->scale));
			stp_set_float_parameter(s->vars,s->optionname,value);
			break;
		case STP_PARAMETER_TYPE_INT:
			value=gtk_range_get_value(GTK_RANGE(s->scale));
			stp_set_int_parameter(s->vars,s->optionname,value);
			break;
		case STP_PARAMETER_TYPE_DIMENSION:
			value=gtk_range_get_value(GTK_RANGE(s->scale));
			stp_set_dimension_parameter(s->vars,s->optionname,value);
			break;
		default:
			break;
	}

	stpui_slider_refresh(s);

//	g_signal_emit(G_OBJECT (s),
//		stpui_slider_signals[CHANGED_SIGNAL], 0);
}


static void stpui_spin_changed(GtkWidget *w,gpointer *ud)
{
	stpui_Slider *s=STPUI_SLIDER(ud);
	double value;
	
	switch(s->type)
	{
		case STP_PARAMETER_TYPE_DOUBLE:
			value=gtk_spin_button_get_value(GTK_SPIN_BUTTON(s->spin));
			stp_set_float_parameter(s->vars,s->optionname,value);
			break;
		case STP_PARAMETER_TYPE_INT:
			value=gtk_spin_button_get_value(GTK_SPIN_BUTTON(s->spin));
			stp_set_int_parameter(s->vars,s->optionname,value);
			break;
		case STP_PARAMETER_TYPE_DIMENSION:
			value=dimension_get_pt(DIMENSION(s->spin));
			stp_set_dimension_parameter(s->vars,s->optionname,value);
			break;
		default:
			break;
	}	

	stpui_slider_refresh(s);

//	g_signal_emit(G_OBJECT (s),
//		stpui_slider_signals[CHANGED_SIGNAL], 0);
}


static void stpui_toggle_changed(GtkWidget *w,gpointer *ud)
{
	stpui_Slider *s=STPUI_SLIDER(ud);

	gboolean enabled=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->checkbutton));
	if(enabled)
	{
		stp_parameter_t desc;
		stp_describe_parameter(s->vars,s->optionname,&desc);
		switch(desc.p_type)
		{
			case STP_PARAMETER_TYPE_DOUBLE:
				{
					double val=gtk_range_get_value(GTK_RANGE(s->scale));
					stp_set_float_parameter(s->vars,s->optionname,val);
				}
				break;
			case STP_PARAMETER_TYPE_INT:
				{
					double val=gtk_range_get_value(GTK_RANGE(s->scale));
					stp_set_int_parameter(s->vars,s->optionname,val);
				}
				break;
			case STP_PARAMETER_TYPE_DIMENSION:
				{
					double val=gtk_range_get_value(GTK_RANGE(s->scale));
					// Convert to correct unit here
					stp_set_dimension_parameter(s->vars,s->optionname,val);
				}
				break;
			default:
				break;	
		}
	}
	else
	{
		stp_parameter_t desc;
		stp_describe_parameter(s->vars,s->optionname,&desc);
		switch(desc.p_type)
		{
			case STP_PARAMETER_TYPE_DOUBLE:
				stp_clear_float_parameter(s->vars,s->optionname);
				break;
			case STP_PARAMETER_TYPE_INT:
				stp_clear_int_parameter(s->vars,s->optionname);
				break;
			case STP_PARAMETER_TYPE_DIMENSION:
				stp_clear_dimension_parameter(s->vars,s->optionname);
				break;
			default:
				break;	
		}		
	}
	
	stpui_slider_refresh(s);
}


GtkWidget*
stpui_slider_new (stp_vars_t *vars,const char *optname,GtkWidget *checkbutton)
{
	stp_parameter_t desc;
	double step=1.0;
	stpui_Slider *c=STPUI_SLIDER(g_object_new (stpui_slider_get_type (), NULL));
	gboolean active=FALSE;

	c->vars=vars;
	c->optionname=optname;
	c->checkbutton=NULL;
	c->label=NULL;
	if(GTK_IS_CHECK_BUTTON(checkbutton))
		c->checkbutton=GTK_CHECK_BUTTON(checkbutton);
	else
		c->label=checkbutton;

	stp_describe_parameter(c->vars,c->optionname,&desc);
	c->type=desc.p_type;

	switch(c->type)
	{
		case STP_PARAMETER_TYPE_DOUBLE:
			active=stp_check_float_parameter(c->vars,c->optionname,STP_PARAMETER_DEFAULTED);
			step=0.01;
			break;
		case STP_PARAMETER_TYPE_INT:
			active=stp_check_int_parameter(c->vars,c->optionname,STP_PARAMETER_DEFAULTED);
			step=1.0;
			break;
		case STP_PARAMETER_TYPE_DIMENSION:
			active=stp_check_dimension_parameter(c->vars,c->optionname,STP_PARAMETER_DEFAULTED);
			step=1.0;
			break;
		default:
			break;
	}
	stp_parameter_description_destroy(&desc);

	if(c->checkbutton)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->checkbutton),active);
		g_signal_connect(G_OBJECT(c->checkbutton),"toggled",G_CALLBACK(stpui_toggle_changed),c);
	}

	c->scale=gtk_hscale_new_with_range(0,1.0,step);
	gtk_scale_set_draw_value(GTK_SCALE(c->scale),FALSE);
	
	switch(desc.p_type)
	{
		case STP_PARAMETER_TYPE_DOUBLE:
		case STP_PARAMETER_TYPE_INT:
			c->spin=gtk_spin_button_new_with_range(0,1.0,step);
			break;
		case STP_PARAMETER_TYPE_DIMENSION:
			c->spin=dimension_new(0,1.0,UNIT_POINTS);
			dimension_show_unit(DIMENSION(c->spin));
			break;
		default:
			break;
	}
	
	g_signal_connect(GTK_WIDGET(c->scale),"button-release-event",G_CALLBACK(stpui_slider_released),c);
	g_signal_connect(GTK_WIDGET(c->spin),"button-release-event",G_CALLBACK(stpui_spin_released),c);
	g_signal_connect(GTK_WIDGET(c->scale),"value-changed",G_CALLBACK(stpui_slider_changed),c);
	g_signal_connect(GTK_WIDGET(c->spin),"value-changed",G_CALLBACK(stpui_spin_changed),c);

	stpui_slider_refresh(c);

	gtk_box_pack_start(GTK_BOX(c),GTK_WIDGET(c->scale),TRUE,TRUE,0);
	gtk_widget_show(c->scale);

	gtk_box_pack_start(GTK_BOX(c),GTK_WIDGET(c->spin),FALSE,TRUE,0);
	gtk_widget_show(c->spin);
	
	return(GTK_WIDGET(c));
}


GType
stpui_slider_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo stpui_slider_info =
		{
			sizeof (stpui_SliderClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) stpui_slider_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (stpui_Slider),
			0,
			(GInstanceInitFunc) stpui_slider_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "stpui_Slider", &stpui_slider_info, 0);
	}
	return stpuic_type;
}


static void
stpui_slider_class_init (stpui_SliderClass *klass)
{
	stpui_slider_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (stpui_SliderClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
stpui_slider_init (stpui_Slider *c)
{
}


gboolean stpui_slider_refresh(stpui_Slider *c)
{
	gboolean result;

	g_signal_handlers_block_matched (G_OBJECT (c->scale), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, c);

	g_signal_handlers_block_matched (G_OBJECT (c->spin), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, c);

	result=stpui_slider_update(c);

	g_signal_handlers_unblock_matched (G_OBJECT (c->scale), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, c);

	g_signal_handlers_unblock_matched (G_OBJECT (c->spin), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, c);

	return(result);
}
