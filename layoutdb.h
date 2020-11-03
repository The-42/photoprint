#ifndef LAYOUT_DB_H
#define LAYOUT_DB_H

#include <iostream>

#include "support/debug.h"
#include "support/configdb.h"

using namespace std;

class Layout_NUpDB : public ConfigDB
{
	public:
	Layout_NUpDB(ConfigFile *inif,const char *section) : ConfigDB(Template)
	{
		new ConfigDBHandler(inif,section,this);
	}
	private:
	static ConfigTemplate Template[];
};


class Layout_SingleDB : public ConfigDB
{
	public:
	Layout_SingleDB(ConfigFile *inif,const char *section) : ConfigDB(Template)
	{
		new ConfigDBHandler(inif,section,this);
	}
	private:
	static ConfigTemplate Template[];
};


class Layout_PosterDB : public ConfigDB
{
	public:
	Layout_PosterDB(ConfigFile *inif,const char *section) : ConfigDB(Template)
	{
		new ConfigDBHandler(inif,section,this);
	}
	private:
	static ConfigTemplate Template[];
};


class Layout_CarouselDB : public ConfigDB
{
	public:
	Layout_CarouselDB(ConfigFile *inif,const char *section) : ConfigDB(Template)
	{
		new ConfigDBHandler(inif,section,this);
	}
	private:
	static ConfigTemplate Template[];
};


class LayoutDB : public ConfigDB
{
	public:
	LayoutDB(ConfigFile *inif,const char *section) :
		ConfigDB(Template), nupdb(inif,"[Layout_NUp]"),
		singledb(inif,"[Layout_Single]"), posterdb(inif,"[Layout_Poster]"),
		carouseldb(inif,"[Layout_Carousel]")
	{
		Debug[TRACE] << "In LayoutDB constructor" << endl;
		new ConfigDBHandler(inif,section,this);
	}
	// Add DBs for each layout type here
	Layout_NUpDB nupdb;
	Layout_SingleDB singledb;
	Layout_PosterDB posterdb;
	Layout_CarouselDB carouseldb;
	protected:
	static ConfigTemplate Template[];
};

#endif
