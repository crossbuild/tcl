/*
 * tclUnixNotify.c --
 *
 *	This file contains the implementation of the select()-based
 *	Unix-specific notifier, which is the lowest-level part of the Tcl
 *	event loop. This file works together with generic/tclNotify.c.
 *
 * Copyright (c) 1995-1997 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution of
 * this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "tclInt.h"
#ifndef HAVE_COREFOUNDATION	/* Darwin/Mac OS X CoreFoundation notifier is
				 * in tclMacOSXNotify.c */
#include <signal.h>

/*
 * This code does deep stub magic to allow replacement of the notifier at
 * runtime.
 */

extern TclStubs tclStubs;
extern Tcl_NotifierProcs tclOriginalNotifier;

/*
 * This structure is used to keep track of the notifier info for a registered
 * file.
 */

typedef struct FileHandler {
    int fd;
    int mask;			/* Mask of desired events: TCL_READABLE,
				 * etc. */
    int readyMask;		/* Mask of events that have been seen since
				 * the last time file handlers were invoked
				 * for this file. */
    Tcl_FileProc *proc;		/* Function to call, in the style of
				 * Tcl_CreateFileHandler. */
    ClientData clientData;	/* Argument to pass to proc. */
    struct FileHandler *nextPtr;/* Next in list of all files we care about. */
} FileHandler;

/*
 * The following structure is what is added to the Tcl event queue when file
 * handlers are ready to fire.
 */

typedef struct FileHandlerEvent {
    Tcl_Event header;		/* Information that is standard for all
				 * events. */
    int fd;			/* File descriptor that is ready. Used to find
				 * the FileHandler structure for the file
				 * (can't point directly to the FileHandler
				 * structure because it could go away while
				 * the event is queued). */
} FileHandlerEvent;

/*
 * The following structure contains a set of select() masks to track readable,
 * writable, and exceptional conditions.
 */

typedef struct SelectMasks {
    fd_set readable;
    fd_set writable;
    fd_set exceptional;
} SelectMasks;

/*
 * The following static structure contains the state information for the
 * select based implementation of the Tcl notifier. One of these structures is
 * created for each thread that is using the notifier.
 */

typedef struct ThreadSpecificData {
    FileHandler *firstFileHandlerPtr;
				/* Pointer to head of file handler list. */
    SelectMasks checkMasks;	/* This structure is used to build up the
				 * masks to be used in the next call to
				 * select. Bits are set in response to calls
				 * to Tcl_CreateFileHandler. */
    SelectMasks readyMasks;	/* This array reflects the readable/writable
				 * conditions that were found to exist by the
				 * last call to select. */
    int numFdBits;		/* Number of valid bits in checkMasks (one
				 * more than highest fd for which
				 * Tcl_WatchFile has been called). */
#ifdef TCL_THREADS
    int onList;			/* True if it is in this list */
    unsigned int pollState;	/* pollState is used to implement a polling
				 * handshake between each thread and the
				 * notifier thread. Bits defined below. */
    struct ThreadSpecificData *nextPtr, *prevPtr;
				/* All threads that are currently waiting on
				 * an event have their ThreadSpecificData
				 * structure on a doubly-linked listed formed
				 * from these pointers. You must hold the
				 * notifierMutex lock before accessing these
				 * fields. */
#ifdef __CYGWIN__
    void *event;     /* Any other thread alerts a notifier
	 * that an event is ready to be processed
	 * by sending this event. */
    void *hwnd;			/* Messaging window. */
#else /* !__CYGWIN__ */
    Tcl_Condition waitCV;	/* Any other thread alerts a notifier that an
				 * event is ready to be processed by signaling
				 * this condition variable. */
#endif /* __CYGWIN__ */
    int eventReady;		/* True if an event is ready to be processed.
				 * Used as condition flag together with waitCV
				 * above. */
#endif /* TCL_THREADS */
} ThreadSpecificData;

static Tcl_ThreadDataKey dataKey;

#ifdef TCL_THREADS
/*
 * The following static indicates the number of threads that have initialized
 * notifiers.
 *
 * You must hold the notifierMutex lock before accessing this variable.
 */

static int notifierCount = 0;

/*
 * The following variable points to the head of a doubly-linked list of
 * ThreadSpecificData structures for all threads that are currently waiting on
 * an event.
 *
 * You must hold the notifierMutex lock before accessing this list.
 */

static ThreadSpecificData *waitingListPtr = NULL;

/*
 * The notifier thread spends all its time in select() waiting for a file
 * descriptor associated with one of the threads on the waitingListPtr list to
 * do something interesting. But if the contents of the waitingListPtr list
 * ever changes, we need to wake up and restart the select() system call. You
 * can wake up the notifier thread by writing a single byte to the file
 * descriptor defined below. This file descriptor is the input-end of a pipe
 * and the notifier thread is listening for data on the output-end of the same
 * pipe. Hence writing to this file descriptor will cause the select() system
 * call to return and wake up the notifier thread.
 *
 * You must hold the notifierMutex lock before writing to the pipe.
 */

static int triggerPipe = -1;

/*
 * The notifierMutex locks access to all of the global notifier state.
 */

TCL_DECLARE_MUTEX(notifierMutex)

/*
 * The notifier thread signals the notifierCV when it has finished
 * initializing the triggerPipe and right before the notifier thread
 * terminates.
 */

static Tcl_Condition notifierCV;

/*
 * The pollState bits
 *	POLL_WANT is set by each thread before it waits on its condition
 *		variable. It is checked by the notifier before it does select.
 *	POLL_DONE is set by the notifier if it goes into select after seeing
 *		POLL_WANT. The idea is to ensure it tries a select with the
 *		same bits the initial thread had set.
 */

#define POLL_WANT	0x1
#define POLL_DONE	0x2

/*
 * This is the thread ID of the notifier thread that does select.
 */

static Tcl_ThreadId notifierThread;

#endif /* TCL_THREADS */

/*
 * Static routines defined in this file.
 */

#ifdef TCL_THREADS
static void	NotifierThreadProc(ClientData clientData);
#if defined(HAVE_PTHREAD_ATFORK) && !defined(__APPLE__)
static int	atForkInit = 0;
static void	AtForkPrepareProc(void);
static void	AtForkParentProc(void);
static void	AtForkChildProc(void);
#endif /* HAVE_PTHREAD_ATFORK */
#endif /* TCL_THREADS */
static int	FileHandlerEventProc(Tcl_Event *evPtr, int flags);

/*
 *----------------------------------------------------------------------
 *
 * Tcl_InitNotifier --
 *
 *	Initializes the platform specific notifier state.
 *
 * Results:
 *	Returns a handle to the notifier state for this thread.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

#if defined(TCL_THREADS) && defined(__CYGWIN__)

typedef struct {
    void *hwnd;
    unsigned int *message;
    int wParam;
    int lParam;
    int time;
    int x;
    int y;
} MSG;

typedef struct {
  unsigned int style;
  void *lpfnWndProc;
  int cbClsExtra;
  int cbWndExtra;
  void *hInstance;
  void *hIcon;
  void *hCursor;
  void *hbrBackground;
  void *lpszMenuName;
  void *lpszClassName;
} WNDCLASS;

extern unsigned char __stdcall PeekMessageW(MSG *, void *, int, int, int);
extern unsigned char __stdcall GetMessageW(MSG *, void *, int, int);
extern unsigned char __stdcall TranslateMessage(const MSG *);
extern int __stdcall DispatchMessageW(const MSG *);
extern void __stdcall PostQuitMessage(int);
extern void * __stdcall CreateWindowExW(void *, void *, void *, DWORD, int, int, int, int, void *, void *, void *, void *);
extern unsigned char __stdcall DestroyWindow(void *);
extern unsigned char __stdcall PostMessageW(void *, unsigned int, void *, void *);
extern void *__stdcall RegisterClassW(const WNDCLASS *);
extern DWORD __stdcall DefWindowProcW(void *, int, void *, void *);
extern void *__stdcall CreateEventW(void *, unsigned char, unsigned char, void *);
extern void __stdcall CloseHandle(void *);
extern void __stdcall MsgWaitForMultipleObjects(DWORD, void *, unsigned char, DWORD, DWORD);
extern unsigned char __stdcall ResetEvent(void *);

#endif

ClientData
Tcl_InitNotifier(void)
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

#ifdef TCL_THREADS
    tsdPtr->eventReady = 0;

    /*
     * Start the Notifier thread if necessary.
     */

    Tcl_MutexLock(&notifierMutex);

#if defined(HAVE_PTHREAD_ATFORK) && !defined(__APPLE__)
    /*
     * Install pthread_atfork() handlers to deal with the notifier in the
     * forked child.
     */

    if (!atForkInit) {
	int result = pthread_atfork(AtForkPrepareProc, AtForkParentProc,
		AtForkChildProc);

	if (result) {
	    Tcl_Panic("Tcl_InitNotifier: pthread_atfork failed");
	}
	atForkInit = 1;
    }
#endif /* HAVE_PTHREAD_ATFORK */

    if (notifierCount == 0) {
	if (TclpThreadCreate(&notifierThread, NotifierThreadProc, NULL,
		TCL_THREAD_STACK_DEFAULT, TCL_THREAD_JOINABLE) != TCL_OK) {
	    Tcl_Panic("Tcl_InitNotifier: unable to start notifier thread");
	}
    }
    notifierCount++;

    /*
     * Wait for the notifier pipe to be created.
     */

    while (triggerPipe < 0) {
	Tcl_ConditionWait(&notifierCV, &notifierMutex, NULL);
    }

    Tcl_MutexUnlock(&notifierMutex);
#endif /* TCL_THREADS */
    return (ClientData) tsdPtr;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_FinalizeNotifier --
 *
 *	This function is called to cleanup the notifier state before a thread
 *	is terminated.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	May terminate the background notifier thread if this is the last
 *	notifier instance.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_FinalizeNotifier(
    ClientData clientData)		/* Not used. */
{
#ifdef TCL_THREADS
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    Tcl_MutexLock(&notifierMutex);
    notifierCount--;

    /*
     * If this is the last thread to use the notifier, close the notifier pipe
     * and wait for the background thread to terminate.
     */

    if (notifierCount == 0) {
	int result;

	if (triggerPipe < 0) {
	    Tcl_Panic("Tcl_FinalizeNotifier: notifier pipe not initialized");
	}

	/*
	 * Send "q" message to the notifier thread so that it will terminate.
	 * The notifier will return from its call to select() and notice that
	 * a "q" message has arrived, it will then close its side of the pipe
	 * and terminate its thread. Note the we can not just close the pipe
	 * and check for EOF in the notifier thread because if a background
	 * child process was created with exec, select() would not register
	 * the EOF on the pipe until the child processes had terminated. [Bug:
	 * 4139] [Bug: 1222872]
	 */

	if (write(triggerPipe, "q", 1) != 1) {
	    Tcl_Panic("Tcl_FinalizeNotifier: unable to write q to triggerPipe");
	}
	close(triggerPipe);
	while(triggerPipe >= 0) {
	    Tcl_ConditionWait(&notifierCV, &notifierMutex, NULL);
	}

	result = Tcl_JoinThread(notifierThread, NULL);
	if (result) {
	    Tcl_Panic("Tcl_FinalizeNotifier: unable to join notifier thread");
	}
    }

    /*
     * Clean up any synchronization objects in the thread local storage.
     */

#ifdef __CYGWIN__
    CloseHandle(tsdPtr->event);
#else /* __CYGWIN__ */
    Tcl_ConditionFinalize(&(tsdPtr->waitCV));
#endif /* __CYGWIN__ */

    Tcl_MutexUnlock(&notifierMutex);
#endif
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_AlertNotifier --
 *
 *	Wake up the specified notifier from any thread. This routine is called
 *	by the platform independent notifier code whenever the Tcl_ThreadAlert
 *	routine is called. This routine is guaranteed not to be called on a
 *	given notifier after Tcl_FinalizeNotifier is called for that notifier.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Signals the notifier condition variable for the specified notifier.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_AlertNotifier(
    ClientData clientData)
{
#ifdef TCL_THREADS
    ThreadSpecificData *tsdPtr = (ThreadSpecificData *) clientData;
    Tcl_MutexLock(&notifierMutex);
    tsdPtr->eventReady = 1;
#ifdef __CYGWIN__
    PostMessageW(tsdPtr->hwnd, 1024, 0, 0);
#else
    Tcl_ConditionNotify(&tsdPtr->waitCV);
#endif
    Tcl_MutexUnlock(&notifierMutex);
#endif
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_SetTimer --
 *
 *	This function sets the current notifier timer value. This interface is
 *	not implemented in this notifier because we are always running inside
 *	of Tcl_DoOneEvent.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_SetTimer(
    Tcl_Time *timePtr)		/* Timeout value, may be NULL. */
{
    /*
     * The interval timer doesn't do anything in this implementation, because
     * the only event loop is via Tcl_DoOneEvent, which passes timeout values
     * to Tcl_WaitForEvent.
     */

    if (tclStubs.tcl_SetTimer != tclOriginalNotifier.setTimerProc) {
	tclStubs.tcl_SetTimer(timePtr);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ServiceModeHook --
 *
 *	This function is invoked whenever the service mode changes.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_ServiceModeHook(
    int mode)			/* Either TCL_SERVICE_ALL, or
				 * TCL_SERVICE_NONE. */
{
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_CreateFileHandler --
 *
 *	This function registers a file handler with the select notifier.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Creates a new file handler structure.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_CreateFileHandler(
    int fd,			/* Handle of stream to watch. */
    int mask,			/* OR'ed combination of TCL_READABLE,
				 * TCL_WRITABLE, and TCL_EXCEPTION: indicates
				 * conditions under which proc should be
				 * called. */
    Tcl_FileProc *proc,		/* Function to call for each selected
				 * event. */
    ClientData clientData)	/* Arbitrary data to pass to proc. */
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);
    FileHandler *filePtr;

    if (tclStubs.tcl_CreateFileHandler !=
	    tclOriginalNotifier.createFileHandlerProc) {
	tclStubs.tcl_CreateFileHandler(fd, mask, proc, clientData);
	return;
    }

    for (filePtr = tsdPtr->firstFileHandlerPtr; filePtr != NULL;
	    filePtr = filePtr->nextPtr) {
	if (filePtr->fd == fd) {
	    break;
	}
    }
    if (filePtr == NULL) {
	filePtr = (FileHandler*) ckalloc(sizeof(FileHandler));
	filePtr->fd = fd;
	filePtr->readyMask = 0;
	filePtr->nextPtr = tsdPtr->firstFileHandlerPtr;
	tsdPtr->firstFileHandlerPtr = filePtr;
    }
    filePtr->proc = proc;
    filePtr->clientData = clientData;
    filePtr->mask = mask;

    /*
     * Update the check masks for this file.
     */

    if (mask & TCL_READABLE) {
	FD_SET(fd, &(tsdPtr->checkMasks.readable));
    } else {
	FD_CLR(fd, &(tsdPtr->checkMasks.readable));
    }
    if (mask & TCL_WRITABLE) {
	FD_SET(fd, &(tsdPtr->checkMasks.writable));
    } else {
	FD_CLR(fd, &(tsdPtr->checkMasks.writable));
    }
    if (mask & TCL_EXCEPTION) {
	FD_SET(fd, &(tsdPtr->checkMasks.exceptional));
    } else {
	FD_CLR(fd, &(tsdPtr->checkMasks.exceptional));
    }
    if (tsdPtr->numFdBits <= fd) {
	tsdPtr->numFdBits = fd+1;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_DeleteFileHandler --
 *
 *	Cancel a previously-arranged callback arrangement for a file.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	If a callback was previously registered on file, remove it.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_DeleteFileHandler(
    int fd)			/* Stream id for which to remove callback
				 * function. */
{
    FileHandler *filePtr, *prevPtr;
    int i;
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    if (tclStubs.tcl_DeleteFileHandler !=
	    tclOriginalNotifier.deleteFileHandlerProc) {
	tclStubs.tcl_DeleteFileHandler(fd);
	return;
    }

    /*
     * Find the entry for the given file (and return if there isn't one).
     */

    for (prevPtr = NULL, filePtr = tsdPtr->firstFileHandlerPtr; ;
	 prevPtr = filePtr, filePtr = filePtr->nextPtr) {
	if (filePtr == NULL) {
	    return;
	}
	if (filePtr->fd == fd) {
	    break;
	}
    }

    /*
     * Update the check masks for this file.
     */

    if (filePtr->mask & TCL_READABLE) {
	FD_CLR(fd, &(tsdPtr->checkMasks.readable));
    }
    if (filePtr->mask & TCL_WRITABLE) {
	FD_CLR(fd, &(tsdPtr->checkMasks.writable));
    }
    if (filePtr->mask & TCL_EXCEPTION) {
	FD_CLR(fd, &(tsdPtr->checkMasks.exceptional));
    }

    /*
     * Find current max fd.
     */

    if (fd+1 == tsdPtr->numFdBits) {
	int numFdBits = 0;

	for (i = fd-1; i >= 0; i--) {
	    if (FD_ISSET(i, &(tsdPtr->checkMasks.readable))
		    || FD_ISSET(i, &(tsdPtr->checkMasks.writable))
		    || FD_ISSET(i, &(tsdPtr->checkMasks.exceptional))) {
		numFdBits = i+1;
		break;
	    }
	}
	tsdPtr->numFdBits = numFdBits;
    }

    /*
     * Clean up information in the callback record.
     */

    if (prevPtr == NULL) {
	tsdPtr->firstFileHandlerPtr = filePtr->nextPtr;
    } else {
	prevPtr->nextPtr = filePtr->nextPtr;
    }
    ckfree((char *) filePtr);
}

/*
 *----------------------------------------------------------------------
 *
 * FileHandlerEventProc --
 *
 *	This function is called by Tcl_ServiceEvent when a file event reaches
 *	the front of the event queue. This function is responsible for
 *	actually handling the event by invoking the callback for the file
 *	handler.
 *
 * Results:
 *	Returns 1 if the event was handled, meaning it should be removed from
 *	the queue. Returns 0 if the event was not handled, meaning it should
 *	stay on the queue. The only time the event isn't handled is if the
 *	TCL_FILE_EVENTS flag bit isn't set.
 *
 * Side effects:
 *	Whatever the file handler's callback function does.
 *
 *----------------------------------------------------------------------
 */

static int
FileHandlerEventProc(
    Tcl_Event *evPtr,		/* Event to service. */
    int flags)			/* Flags that indicate what events to handle,
				 * such as TCL_FILE_EVENTS. */
{
    int mask;
    FileHandler *filePtr;
    FileHandlerEvent *fileEvPtr = (FileHandlerEvent *) evPtr;
    ThreadSpecificData *tsdPtr;

    if (!(flags & TCL_FILE_EVENTS)) {
	return 0;
    }

    /*
     * Search through the file handlers to find the one whose handle matches
     * the event. We do this rather than keeping a pointer to the file handler
     * directly in the event, so that the handler can be deleted while the
     * event is queued without leaving a dangling pointer.
     */

    tsdPtr = TCL_TSD_INIT(&dataKey);
    for (filePtr = tsdPtr->firstFileHandlerPtr; filePtr != NULL;
	    filePtr = filePtr->nextPtr) {
	if (filePtr->fd != fileEvPtr->fd) {
	    continue;
	}

	/*
	 * The code is tricky for two reasons:
	 * 1. The file handler's desired events could have changed since the
	 *    time when the event was queued, so AND the ready mask with the
	 *    desired mask.
	 * 2. The file could have been closed and re-opened since the time
	 *    when the event was queued. This is why the ready mask is stored
	 *    in the file handler rather than the queued event: it will be
	 *    zeroed when a new file handler is created for the newly opened
	 *    file.
	 */

	mask = filePtr->readyMask & filePtr->mask;
	filePtr->readyMask = 0;
	if (mask != 0) {
	    (*filePtr->proc)(filePtr->clientData, mask);
	}
	break;
    }
    return 1;
}

#if defined(TCL_THREADS) && defined(__CYGWIN__)

static DWORD __stdcall
NotifierProc(
    void *hwnd,
    unsigned int message,
    void *wParam,
    void *lParam)
{
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    if (message != 1024) {
	return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    /*
     * Process all of the runnable events.
     */

	tsdPtr->eventReady = 1;
    Tcl_ServiceAll();
    return 0;
}
#endif /* __CYGWIN__ */

/*
 *----------------------------------------------------------------------
 *
 * Tcl_WaitForEvent --
 *
 *	This function is called by Tcl_DoOneEvent to wait for new events on
 *	the message queue. If the block time is 0, then Tcl_WaitForEvent just
 *	polls without blocking.
 *
 * Results:
 *	Returns -1 if the select would block forever, otherwise returns 0.
 *
 * Side effects:
 *	Queues file events that are detected by the select.
 *
 *----------------------------------------------------------------------
 */

int
Tcl_WaitForEvent(
    Tcl_Time *timePtr)		/* Maximum block time, or NULL. */
{
    FileHandler *filePtr;
    FileHandlerEvent *fileEvPtr;
    int mask;
    Tcl_Time vTime;
#ifdef TCL_THREADS
    int waitForFiles;
# ifdef __CYGWIN__
    MSG msg;
# endif
#else
    /*
     * Impl. notes: timeout & timeoutPtr are used if, and only if threads are
     * not enabled. They are the arguments for the regular select() used when
     * the core is not thread-enabled.
     */

    struct timeval timeout, *timeoutPtr;
    int numFound;
#endif /* TCL_THREADS */
    ThreadSpecificData *tsdPtr = TCL_TSD_INIT(&dataKey);

    if (tclStubs.tcl_WaitForEvent != tclOriginalNotifier.waitForEventProc) {
	return tclStubs.tcl_WaitForEvent(timePtr);
    }

    /*
     * Set up the timeout structure. Note that if there are no events to check
     * for, we return with a negative result rather than blocking forever.
     */

    if (timePtr != NULL) {
	/*
	 * TIP #233 (Virtualized Time). Is virtual time in effect? And do we
	 * actually have something to scale? If yes to both then we call the
	 * handler to do this scaling.
	 */

	if (timePtr->sec != 0 || timePtr->usec != 0) {
	    vTime = *timePtr;
	    (*tclScaleTimeProcPtr) (&vTime, tclTimeClientData);
	    timePtr = &vTime;
	}
#ifndef TCL_THREADS
	timeout.tv_sec = timePtr->sec;
	timeout.tv_usec = timePtr->usec;
	timeoutPtr = &timeout;
    } else if (tsdPtr->numFdBits == 0) {
	/*
	 * If there are no threads, no timeout, and no fds registered, then
	 * there are no events possible and we must avoid deadlock. Note that
	 * this is not entirely correct because there might be a signal that
	 * could interrupt the select call, but we don't handle that case if
	 * we aren't using threads.
	 */

	return -1;
    } else {
	timeoutPtr = NULL;
#endif /* TCL_THREADS */
    }

#ifdef TCL_THREADS
    /*
     * Place this thread on the list of interested threads, signal the
     * notifier thread, and wait for a response or a timeout.
     */

    Tcl_MutexLock(&notifierMutex);

    if (timePtr != NULL && timePtr->sec == 0 && (timePtr->usec == 0
#if defined(__APPLE__) && defined(__LP64__)
	    /*
	     * On 64-bit Darwin, pthread_cond_timedwait() appears to have a bug
	     * that causes it to wait forever when passed an absolute time which
	     * has already been exceeded by the system time; as a workaround,
	     * when given a very brief timeout, just do a poll. [Bug 1457797]
	     */
	    || timePtr->usec < 10
#endif
	    )) {
	/*
	 * Cannot emulate a polling select with a polling condition variable.
	 * Instead, pretend to wait for files and tell the notifier thread
	 * what we are doing. The notifier thread makes sure it goes through
	 * select with its select mask in the same state as ours currently is.
	 * We block until that happens.
	 */

	waitForFiles = 1;
	tsdPtr->pollState = POLL_WANT;
	timePtr = NULL;
    } else {
	waitForFiles = (tsdPtr->numFdBits > 0);
	tsdPtr->pollState = 0;
    }

#ifdef __CYGWIN__
	if (!tsdPtr->hwnd) {
		WNDCLASS class;

	    class.style = 0;
	    class.cbClsExtra = 0;
	    class.cbWndExtra = 0;
	    class.hInstance = TclWinGetTclInstance();
	    class.hbrBackground = NULL;
	    class.lpszMenuName = NULL;
	    class.lpszClassName = L"TclNotifier";
	    class.lpfnWndProc = NotifierProc;
	    class.hIcon = NULL;
	    class.hCursor = NULL;

	    RegisterClassW(&class);
	    tsdPtr->hwnd = CreateWindowExW(NULL, class.lpszClassName, class.lpszClassName,
		    0, 0, 0, 0, 0, NULL, NULL, TclWinGetTclInstance(), NULL);
	    tsdPtr->event = CreateEventW(NULL, 1 /* manual */,
		    0 /* !signaled */, NULL);
    }

#endif
    if (waitForFiles) {
	/*
	 * Add the ThreadSpecificData structure of this thread to the list of
	 * ThreadSpecificData structures of all threads that are waiting on
	 * file events.
	 */

	tsdPtr->nextPtr = waitingListPtr;
	if (waitingListPtr) {
	    waitingListPtr->prevPtr = tsdPtr;
	}
	tsdPtr->prevPtr = 0;
	waitingListPtr = tsdPtr;
	tsdPtr->onList = 1;

	if ((write(triggerPipe, "", 1) == -1) && (errno != EAGAIN)) {
	    Tcl_Panic("Tcl_WaitForEvent: unable to write to triggerPipe");
	}
    }

    FD_ZERO(&(tsdPtr->readyMasks.readable));
    FD_ZERO(&(tsdPtr->readyMasks.writable));
    FD_ZERO(&(tsdPtr->readyMasks.exceptional));

    if (!tsdPtr->eventReady) {
#ifdef __CYGWIN__
	if (!PeekMessageW(&msg, NULL, 0, 0, 0)) {
	    DWORD timeout;
	    if (timePtr) {
		timeout = timePtr->sec * 1000 + timePtr->usec / 1000;
	    } else {
		timeout = 0xFFFFFFFF;
	    }
	    Tcl_MutexUnlock(&notifierMutex);
	    MsgWaitForMultipleObjects(1, &tsdPtr->event, 0, timeout, 1279);
	    Tcl_MutexLock(&notifierMutex);
	}
#else
	Tcl_ConditionWait(&tsdPtr->waitCV, &notifierMutex, timePtr);
#endif
    }
    tsdPtr->eventReady = 0;

#ifdef __CYGWIN__
    while (PeekMessageW(&msg, NULL, 0, 0, 0)) {
	/*
	 * Retrieve and dispatch the message.
	 */
	DWORD result = GetMessageW(&msg, NULL, 0, 0);
	if (result == 0) {
	    PostQuitMessage(msg.wParam);
	    /* What to do here? */
	} else if (result != (DWORD)-1) {
	    TranslateMessage(&msg);
	    DispatchMessageW(&msg);
	}
    }
    ResetEvent(tsdPtr->event);
#endif

    if (waitForFiles && tsdPtr->onList) {
	/*
	 * Remove the ThreadSpecificData structure of this thread from the
	 * waiting list. Alert the notifier thread to recompute its select
	 * masks - skipping this caused a hang when trying to close a pipe
	 * which the notifier thread was still doing a select on.
	 */

	if (tsdPtr->prevPtr) {
	    tsdPtr->prevPtr->nextPtr = tsdPtr->nextPtr;
	} else {
	    waitingListPtr = tsdPtr->nextPtr;
	}
	if (tsdPtr->nextPtr) {
	    tsdPtr->nextPtr->prevPtr = tsdPtr->prevPtr;
	}
	tsdPtr->nextPtr = tsdPtr->prevPtr = NULL;
	tsdPtr->onList = 0;
	if ((write(triggerPipe, "", 1) == -1) && (errno != EAGAIN)) {
	    Tcl_Panic("Tcl_WaitForEvent: unable to write to triggerPipe");
	}
    }

#else
    tsdPtr->readyMasks = tsdPtr->checkMasks;
    numFound = select(tsdPtr->numFdBits, &(tsdPtr->readyMasks.readable),
	    &(tsdPtr->readyMasks.writable), &(tsdPtr->readyMasks.exceptional),
	    timeoutPtr);

    /*
     * Some systems don't clear the masks after an error, so we have to do it
     * here.
     */

    if (numFound == -1) {
	FD_ZERO(&(tsdPtr->readyMasks.readable));
	FD_ZERO(&(tsdPtr->readyMasks.writable));
	FD_ZERO(&(tsdPtr->readyMasks.exceptional));
    }
#endif /* TCL_THREADS */

    /*
     * Queue all detected file events before returning.
     */

    for (filePtr = tsdPtr->firstFileHandlerPtr; (filePtr != NULL);
	    filePtr = filePtr->nextPtr) {

	mask = 0;
	if (FD_ISSET(filePtr->fd, &(tsdPtr->readyMasks.readable))) {
	    mask |= TCL_READABLE;
	}
	if (FD_ISSET(filePtr->fd, &(tsdPtr->readyMasks.writable))) {
	    mask |= TCL_WRITABLE;
	}
	if (FD_ISSET(filePtr->fd, &(tsdPtr->readyMasks.exceptional))) {
	    mask |= TCL_EXCEPTION;
	}

	if (!mask) {
	    continue;
	}

	/*
	 * Don't bother to queue an event if the mask was previously non-zero
	 * since an event must still be on the queue.
	 */

	if (filePtr->readyMask == 0) {
	    fileEvPtr = (FileHandlerEvent *) ckalloc(sizeof(FileHandlerEvent));
	    fileEvPtr->header.proc = FileHandlerEventProc;
	    fileEvPtr->fd = filePtr->fd;
	    Tcl_QueueEvent((Tcl_Event *) fileEvPtr, TCL_QUEUE_TAIL);
	}
	filePtr->readyMask = mask;
    }
#ifdef TCL_THREADS
    Tcl_MutexUnlock(&notifierMutex);
#endif /* TCL_THREADS */
    return 0;
}

#ifdef TCL_THREADS
/*
 *----------------------------------------------------------------------
 *
 * NotifierThreadProc --
 *
 *	This routine is the initial (and only) function executed by the
 *	special notifier thread. Its job is to wait for file descriptors to
 *	become readable or writable or to have an exception condition and then
 *	to notify other threads who are interested in this information by
 *	signalling a condition variable. Other threads can signal this
 *	notifier thread of a change in their interests by writing a single
 *	byte to a special pipe that the notifier thread is monitoring.
 *
 * Result:
 *	None. Once started, this routine never exits. It dies with the overall
 *	process.
 *
 * Side effects:
 *	The trigger pipe used to signal the notifier thread is created when
 *	the notifier thread first starts.
 *
 *----------------------------------------------------------------------
 */

static void
NotifierThreadProc(
    ClientData clientData)	/* Not used. */
{
    ThreadSpecificData *tsdPtr;
    fd_set readableMask;
    fd_set writableMask;
    fd_set exceptionalMask;
    int fds[2];
    int i, numFdBits = 0, receivePipe;
    long found;
    struct timeval poll = {0., 0.}, *timePtr;
    char buf[2];

    if (pipe(fds) != 0) {
	Tcl_Panic("NotifierThreadProc: could not create trigger pipe");
    }

    receivePipe = fds[0];

    if (TclUnixSetBlockingMode(receivePipe, TCL_MODE_NONBLOCKING) < 0) {
	Tcl_Panic("NotifierThreadProc: could not make receive pipe non blocking");
    }
    if (TclUnixSetBlockingMode(fds[1], TCL_MODE_NONBLOCKING) < 0) {
	Tcl_Panic("NotifierThreadProc: could not make trigger pipe non blocking");
    }
    if (fcntl(receivePipe, F_SETFD, FD_CLOEXEC) < 0) {
	Tcl_Panic("NotifierThreadProc: could not make receive pipe close-on-exec");
    }
    if (fcntl(fds[1], F_SETFD, FD_CLOEXEC) < 0) {
	Tcl_Panic("NotifierThreadProc: could not make trigger pipe close-on-exec");
    }

    /*
     * Install the write end of the pipe into the global variable.
     */

    Tcl_MutexLock(&notifierMutex);
    triggerPipe = fds[1];

    /*
     * Signal any threads that are waiting.
     */

    Tcl_ConditionNotify(&notifierCV);
    Tcl_MutexUnlock(&notifierMutex);

    /*
     * Look for file events and report them to interested threads.
     */

    while (1) {
	FD_ZERO(&readableMask);
	FD_ZERO(&writableMask);
	FD_ZERO(&exceptionalMask);

	/*
	 * Compute the logical OR of the select masks from all the waiting
	 * notifiers.
	 */

	Tcl_MutexLock(&notifierMutex);
	timePtr = NULL;
	for (tsdPtr = waitingListPtr; tsdPtr; tsdPtr = tsdPtr->nextPtr) {
	    for (i = tsdPtr->numFdBits-1; i >= 0; --i) {
		if (FD_ISSET(i, &(tsdPtr->checkMasks.readable))) {
		    FD_SET(i, &readableMask);
		}
		if (FD_ISSET(i, &(tsdPtr->checkMasks.writable))) {
		    FD_SET(i, &writableMask);
		}
		if (FD_ISSET(i, &(tsdPtr->checkMasks.exceptional))) {
		    FD_SET(i, &exceptionalMask);
		}
	    }
	    if (tsdPtr->numFdBits > numFdBits) {
		numFdBits = tsdPtr->numFdBits;
	    }
	    if (tsdPtr->pollState & POLL_WANT) {
		/*
		 * Here we make sure we go through select() with the same mask
		 * bits that were present when the thread tried to poll.
		 */

		tsdPtr->pollState |= POLL_DONE;
		timePtr = &poll;
	    }
	}
	Tcl_MutexUnlock(&notifierMutex);

	/*
	 * Set up the select mask to include the receive pipe.
	 */

	if (receivePipe >= numFdBits) {
	    numFdBits = receivePipe + 1;
	}
	FD_SET(receivePipe, &readableMask);

	if (select(numFdBits, &readableMask, &writableMask, &exceptionalMask,
		timePtr) == -1) {
	    /*
	     * Try again immediately on an error.
	     */

	    continue;
	}

	/*
	 * Alert any threads that are waiting on a ready file descriptor.
	 */

	Tcl_MutexLock(&notifierMutex);
	for (tsdPtr = waitingListPtr; tsdPtr; tsdPtr = tsdPtr->nextPtr) {
	    found = 0;

	    for (i = tsdPtr->numFdBits-1; i >= 0; --i) {
		if (FD_ISSET(i, &(tsdPtr->checkMasks.readable))
			&& FD_ISSET(i, &readableMask)) {
		    FD_SET(i, &(tsdPtr->readyMasks.readable));
		    found = 1;
		}
		if (FD_ISSET(i, &(tsdPtr->checkMasks.writable))
			&& FD_ISSET(i, &writableMask)) {
		    FD_SET(i, &(tsdPtr->readyMasks.writable));
		    found = 1;
		}
		if (FD_ISSET(i, &(tsdPtr->checkMasks.exceptional))
			&& FD_ISSET(i, &exceptionalMask)) {
		    FD_SET(i, &(tsdPtr->readyMasks.exceptional));
		    found = 1;
		}
	    }

	    if (found || (tsdPtr->pollState & POLL_DONE)) {
		tsdPtr->eventReady = 1;
		if (tsdPtr->onList) {
		    /*
		     * Remove the ThreadSpecificData structure of this thread
		     * from the waiting list. This prevents us from
		     * continuously spining on select until the other threads
		     * runs and services the file event.
		     */

		    if (tsdPtr->prevPtr) {
			tsdPtr->prevPtr->nextPtr = tsdPtr->nextPtr;
		    } else {
			waitingListPtr = tsdPtr->nextPtr;
		    }
		    if (tsdPtr->nextPtr) {
			tsdPtr->nextPtr->prevPtr = tsdPtr->prevPtr;
		    }
		    tsdPtr->nextPtr = tsdPtr->prevPtr = NULL;
		    tsdPtr->onList = 0;
		    tsdPtr->pollState = 0;
		}
#ifdef __CYGWIN__
	    PostMessageW(tsdPtr->hwnd, 1024, 0, 0);
#else /* __CYGWIN__ */
	    Tcl_ConditionNotify(&tsdPtr->waitCV);
#endif /* __CYGWIN__ */
	    }
	}
	Tcl_MutexUnlock(&notifierMutex);

	/*
	 * Consume the next byte from the notifier pipe if the pipe was
	 * readable. Note that there may be multiple bytes pending, but to
	 * avoid a race condition we only read one at a time.
	 */

	if (FD_ISSET(receivePipe, &readableMask)) {
	    i = read(receivePipe, buf, 1);

	    if ((i == 0) || ((i == 1) && (buf[0] == 'q'))) {
		/*
		 * Someone closed the write end of the pipe or sent us a Quit
		 * message [Bug: 4139] and then closed the write end of the
		 * pipe so we need to shut down the notifier thread.
		 */

		break;
	    }
	}
    }

    /*
     * Clean up the read end of the pipe and signal any threads waiting on
     * termination of the notifier thread.
     */

    close(receivePipe);
    Tcl_MutexLock(&notifierMutex);
    triggerPipe = -1;
    Tcl_ConditionNotify(&notifierCV);
    Tcl_MutexUnlock(&notifierMutex);

    TclpThreadExit (0);
}

#if defined(HAVE_PTHREAD_ATFORK) && !defined(__APPLE__)
/*
 *----------------------------------------------------------------------
 *
 * AtForkPrepareProc --
 *
 *	Lock the notifier in preparation for a fork.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static void
AtForkPrepareProc(void)
{
    Tcl_MutexLock(&notifierMutex);
}

/*
 *----------------------------------------------------------------------
 *
 * AtForkParentProc --
 *
 *	Unlock the notifier in the parent after a fork.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static void
AtForkParentProc(void)
{
    Tcl_MutexUnlock(&notifierMutex);
}

/*
 *----------------------------------------------------------------------
 *
 * AtForkChildProc --
 *
 *	Assumes the notifier mutex is already held.  Reset the notifier
 *	state and then re-initializes it for the child process.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static void
AtForkChildProc(void)
{
    struct ThreadSpecificData *tsdPtr;

    /*
     * Close the trigger pipe and reset it to the default value.  This
     * should cause it to be re-opened when the notifier is subsequently
     * initialized.  Since the trigger pipe is opened via the notifier
     * thread (which may never have started?), check it before attempting
     * to close it.
     */

    if (triggerPipe >= 0) {
	close(triggerPipe);
	triggerPipe = -1;
    }

    /*
     * Since the notifier thread is not really running, zero out its ID.
     * This step is not strictly necessary since its value will not be
     * checked before it is re-created.
     */

    notifierThread = (Tcl_ThreadId) 0;

    /*
     * Reset the notifier reference count to zero (i.e. not initialized).
     * This should force the notifier initialization code to be re-run the
     * very next time Tcl_InitNotifier is called, which will be at the end
     * of this function.
     */

    notifierCount = 0;

    /*
     * Finalize and reset the condition variable.  This should force it to
     * be re-created during the next call to Tcl_ConditionWait().
     */

    Tcl_ConditionFinalize(&notifierCV);
    notifierCV = NULL;

    /*
     * Free all the thread specific data for the notifier now.  Since the
     * child has no other threads, this data should be completely useless
     * at this point.
     */

    for (tsdPtr = waitingListPtr; tsdPtr; tsdPtr = tsdPtr->nextPtr) {
	FileHandler *filePtr;
	for (filePtr = tsdPtr->firstFileHandlerPtr; filePtr != NULL;
		/* NO LOOP UPDATE */) {
	    FileHandler *nextFilePtr = filePtr->nextPtr;
	    ckfree((char *) filePtr);
	    filePtr = nextFilePtr;
	}
    }
    waitingListPtr = NULL;

    /*
     * Release, finalize, and reset the notifier mutex (which has been
     * held since the AtForkPrepareProc() was called via pthread_atfork()).
     * This should force it to be re-created during the next call to
     * Tcl_MutexLock().
     */

    Tcl_MutexUnlock(&notifierMutex);
    Tcl_MutexFinalize(&notifierMutex);
    notifierMutex = NULL;

    /*
     * Next, forcibly reset all the "one time" locks used by the threading
     * subsystem.  This is necessary on some (all?) variants of Unix where
     * the pthread internal state does not survive a call to fork().
     */

    TclpResetLocks();

    /*
     * Next, abandon the thread specific data for this file.  There is no
     * clean way to free it; however, it can no longer be used.
     */

    Tcl_UnsetThreadData(&dataKey, sizeof(ThreadSpecificData), 1 /*all */);
    dataKey = (Tcl_ThreadDataKey) 0;

    /*
     * Force the notifier subsystem to be initialized now.  This should
     * create the notifier thread in this process.  Subsequently, that new
     * thread will re-open the trigger pipe.
     */

    Tcl_InitNotifier();
}
#endif /* HAVE_PTHREAD_ATFORK */

#endif /* TCL_THREADS */

#endif /* HAVE_COREFOUNDATION */

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
