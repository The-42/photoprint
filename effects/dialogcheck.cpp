#include <iostream>

#include <gtk/gtkmain.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "effectlist.h"
#include "effects_dialog.h"

#include "ppeffect_desaturate.h"
#include "ppeffect_temperature.h"

#include "egg-pixbuf-thumbnail.h"

using namespace std;

int main(int argc,char **argv)
{
	cerr << "Starting..." << endl;
	if(argc==2)
	{
		gtk_init(&argc,&argv);
		PPEffectHeader header;
		GError *err=NULL;
		char *filename=argv[1];
		GdkPixbuf *pb=egg_pixbuf_get_thumbnail_for_file(filename,EGG_PIXBUF_THUMBNAIL_LARGE,&err);
		EffectsDialog(header,NULL,pb);
		g_object_unref(G_OBJECT(pb));
	}
	else
	{
		cerr << "No argument supplied" << endl;
		PPEffectHeader header;
		EffectListSource fxs;
		fxs.CreateEffect(0,header);
		fxs.CreateEffect(1,header);
		fxs.CreateEffect(2,header);
		PPEffect *e=header.Find(fxs.GetID(1));
		if(e)
			delete e;
		else
			cerr << "not found" << endl;
		fxs.CreateEffect(1,header);
		e=header.Find(fxs.GetID(1));
		if(e)
			delete e;			
		else
			cerr << "not found" << endl;
		fxs.CreateEffect(1,header);
		e=header.Find(fxs.GetID(1));
		if(e)
			delete e;			
	}
	return(0);
}
