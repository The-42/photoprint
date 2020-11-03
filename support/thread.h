#ifndef THREADCONTEXT_H
#define THREADCONTEXT_H

#include "ptmutex.h"

enum ThreadState {THREAD_IDLE,THREAD_STARTED,THREAD_RUNNING,THREAD_FINISHED,THREAD_CANCELLED};
class Thread;


class ThreadCondition : public PTMutex
{
	public:
	ThreadCondition();
	~ThreadCondition();
	virtual void Broadcast();		// Sends the signal - Mutex should be obtained first and released afterwards
	virtual void WaitCondition();	// Waits for the signal - mutex should be obtained first and released afterwards
	protected:
	pthread_cond_t cond;
};


// Demonstrates how to use the ThreadCondition - used to sync between subthread and main thread.
class ThreadSync : public ThreadCondition
{
	public:
	ThreadSync() : ThreadCondition(), received(false)
	{
	}
	~ThreadSync()
	{
	}
	virtual void Broadcast()
	{
		ObtainMutex();
		ThreadCondition::Broadcast();
		received=true;
		ReleaseMutex();
	}
	virtual void WaitCondition()
	{
		ObtainMutex();
		if(!received)
			ThreadCondition::WaitCondition();
		received=false;
		ReleaseMutex();
	}
	protected:
	bool received;
};



#ifdef WIN32
typedef void *ThreadID;
#else
typedef pthread_t ThreadID;
#endif

class ThreadFunction;
class Thread
{
	public:
	Thread(ThreadFunction *threadfunc);
	Thread();
	~Thread();
	void Stop();
	void Start();
	void WaitSync();	// Safe to use bi-directionally.
	int WaitFinished();
	int GetReturnCode();
	bool TestFinished();
	// Methods to be used from within threads
	void SendSync();	// Safe to use bi-directionally.
	bool TestBreak();
	// Static function - can be called anywhere
	static ThreadID GetThreadID();
	protected:
	static void *LaunchStub(void *ud);
	ThreadFunction *threadfunc;
	pthread_t thread;
	ThreadSync cond1;	// We use one cond for main thread -> subthread signalling...
	ThreadSync cond2;	// ...and another for subthread -> main thread signalling.
	pthread_attr_t attr;
	// This mutex is held all the time the thread's running.
	PTMutex threadmutex;
	int returncode;
	enum ThreadState state;
	ThreadID subthreadid;
};


// Base class for thread function.  Subclass this and override Entry with your own subthread code.
class ThreadFunction
{
	public:
	ThreadFunction()
	{
	}
	virtual ~ThreadFunction()
	{
	}
	virtual int Entry(Thread &t)
	{
		t.SendSync();
		return(0);
	}
	protected:
};


#endif

