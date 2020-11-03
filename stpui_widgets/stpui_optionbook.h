#ifndef __STPUI_OPTIONBOOK_H__
#define __STPUI_OPTIONBOOK_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtknotebook.h>

#include <gutenprint/gutenprint.h>

#include "stpui_combo.h"
#include "stpui_optionpage.h"

G_BEGIN_DECLS

#define STPUI_OPTIONBOOK_TYPE			(stpui_optionbook_get_type())
#define STPUI_OPTIONBOOK(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), STPUI_OPTIONBOOK_TYPE, stpui_OptionBook))
#define STPUI_OPTIONBOOK_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), STPUI_OPTIONBOOK_TYPE, stpui_OptionBookClass))
#define IS_STPUI_OPTIONBOOK(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), STPUI_OPTIONBOOK_TYPE))
#define IS_STPUI_OPTIONBOOK_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), STPUI_OPTIONBOOK_TYPE))

typedef struct _stpui_OptionBook stpui_OptionBook;
typedef struct _stpui_OptionBookClass stpui_OptionBookClass;

#define STPUI_OPTIONBOOK_PAGES 5

struct stpui_optionbook_custompage
{
	/* Inialised by the user */
	const char *name;
	GtkWidget *widget;

	/* Private - for use by the OptionBook */
	GtkWidget *scrollwin;
	GtkWidget *label;
};


struct _stpui_OptionBook
{
	GtkNotebook notebook;
	stp_vars_t *vars;
	GtkWidget *page[STPUI_OPTIONBOOK_PAGES];
	GtkWidget *scrollwin[STPUI_OPTIONBOOK_PAGES];
	GtkWidget *label[STPUI_OPTIONBOOK_PAGES];
	struct stpui_optionbook_custompage *custompages;
	int custompagecount;
};


struct _stpui_OptionBookClass
{
	GtkNotebookClass parent_class;

	void (*changed)(stpui_OptionBook *book);
};


GType stpui_optionbook_get_type (void);
GtkWidget* stpui_optionbook_new (stp_vars_t *vars,struct stpui_optionbook_custompage *custompages,int custompagecount);
void stpui_optionbook_refresh(stpui_OptionBook *ob);
void stpui_optionbook_rebuild(stpui_OptionBook *ob);

G_END_DECLS

#endif /* __STPUI_OPTIONBOOK_H__ */
