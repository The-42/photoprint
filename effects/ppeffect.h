#ifndef PPEFFECT_H
#define PPEFFECT_H

#include <gtk/gtkwindow.h>
#include <gdk/gdkpixbuf.h>

#include "../imagesource/imagesource.h"
#include "../support/thread.h"
#include "../support/rwmutex.h"

// These can be compared with bitwise and.
enum PPEFFECT_STAGE {PPEFFECT_PRESCALE=1,PPEFFECT_POSTSCALE,PPEFFECT_DONTCARE};

class PPEffect;

class PPEffectHeader : public RWMutex
{
	public:
	PPEffectHeader();
	PPEffectHeader(PPEffectHeader &pp);
	virtual ~PPEffectHeader();
	ImageSource *ApplyEffects(ImageSource *source,enum PPEFFECT_STAGE stage=PPEFFECT_DONTCARE);
	int EffectCount(enum PPEFFECT_STAGE stage);
	PPEffect *GetFirstEffect(enum PPEFFECT_STAGE stage=PPEFFECT_DONTCARE);
	PPEffect *Find(const char *ID);
	virtual void ObtainMutex();
	private:
	PPEffect *firsteffect;
	friend class PPEffect;
};


class PPEffect
{
	public:
	PPEffect(PPEffectHeader &header,int priority,enum PPEFFECT_STAGE stage);
	virtual ~PPEffect();
	virtual PPEffect *Clone(PPEffectHeader &header)=0;
	virtual ImageSource *Apply(ImageSource *source)=0;
//	virtual	bool Dialog(GtkWindow *parent,GdkPixbuf *preview)=0;
	virtual	PPEffect *Next(enum PPEFFECT_STAGE stage=PPEFFECT_DONTCARE);
	virtual const char *GetID()=0;
	virtual const char *GetName()=0;
	protected:
	int priority;
	enum PPEFFECT_STAGE stage;
	PPEffectHeader &header;
	PPEffect *prev,*next;
	friend class PPEffectHeader;
};

#endif
