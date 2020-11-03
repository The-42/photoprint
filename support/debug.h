#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <fstream>
#include <stack>

// FIXME Win32 namespace clash
#undef ERROR

enum DebugLevel {NONE, ERROR, WARN, COMMENT, TRACE};

class NullStream : public std::streambuf, public std::ostream
{
	public:
	NullStream() : std::streambuf(), std::ostream(this)
	{
	}
	virtual ~NullStream()
	{
	}
	protected:
	virtual int overflow(int c)
	{
		return(c);
	}
};


class DebugStream
{
	public:
	DebugStream(DebugLevel level=ERROR);
	virtual ~DebugStream();
	virtual void SetLogFile(std::string filename);
	virtual	DebugLevel SetLevel(enum DebugLevel lvl);  // returns the old level
	virtual void PushLevel(enum DebugLevel lvl);	// Use PushLevel() and PopLevel() if you want to change
	virtual void PopLevel();						// the debug level for a specific section of code, and restore afterwards.
	virtual std::ostream &operator[](int idx);
	protected:
	enum DebugLevel level;
	std::stack<enum DebugLevel> levelstack;
	NullStream nullstream;
	std::ofstream logfile;
};

extern DebugStream Debug;

#endif

