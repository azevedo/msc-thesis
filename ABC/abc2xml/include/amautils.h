#ifndef _UTILS_H_
#define _UTILS_H_


#include <netdb.h>
#include <assert.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

/** example: assert(NOT_REACHED) */
#define NOT_REACHED 0

// workaround for compillers which don't support variable parameters in preprocessor
//----------------------------------------------------------------------------------
/** show ordinary message to stdout */
#define OUT_INFO(args) Utils::info args

/** show error message to stderr */
#define OUT_ERROR(args) Utils::error args

/** show error message to stderr and exit program with EXIT_FAILED return code */
#define FATAL_ERROR(args) Utils::fatalError args

// enable/disable debug messages in application according to ENABLE_DEBUG_MESSAGES
// you can specify -D ENABLE_DEBUG_MESSAGES option for preprocessor in Makefile
#ifdef ENABLE_DEBUG_MESSAGES
	/** enable debug messages in application */
	#define OUT_DEBUG(args) Utils::debug args
#else
	/** disable all debug messages */
	#define OUT_DEBUG(args)
#endif

// =====================================================================================
#define INIT_MUTEX_VARIABLE(mutexVar) Utils::mutexInit(&mutexVar)
#define MUTEX_LOCK(mutexVar) Utils::mutexLock(&mutexVar)
#define MUTEX_UNLOCK(mutexVar) Utils::mutexUnlock(&mutexVar)

#ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
	#define GET_MUTEX(mutexVar) mutexVar
	#define MUTEX_VARIABLE(mutexVar) pthread_mutex_t mutexVar
	#define INIT_STATIC_MUTEX_VARIABLE(mutexVar) pthread_mutex_t mutexVar = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
#else
	#define GET_MUTEX(mutexVar) mutexVar.mutex
	#define INIT_STATIC_MUTEX_VARIABLE(mutexVar) struct portable_recursive_mutex mutexVar = {PTHREAD_MUTEX_INITIALIZER, 0, 0}
	#define MUTEX_VARIABLE(mutexVar) struct portable_recursive_mutex mutexVar
	struct portable_recursive_mutex
	{
		pthread_mutex_t mutex;
		pthread_t locked_by;
		unsigned int locked_count;
	};
#endif
// =====================================================================================

/**
 * Useful functions encapsulated into single class.
 * All methods of this class are static.
 * @author Petr Djakow
 */
class Utils
{
public:

	// sockets
	//-------------------------------------
	static int prepareTcpSocket();
	static void closefd(int sockfd);
	static int prepareSocketAddr(struct sockaddr_in *addr, const char * hostname, const unsigned short port);
	static void reuseSocketAddr(int sockfd);

	// messages
	//-------------------------------------
	static void debug(char * fmt, ...);
	static void info(char * fmt, ...);
	static void error(char * fmt, ...);
	static void fatalError(char * fmt, ...);

	// threads
	//-------------------------------------
	static void mutexInit( MUTEX_VARIABLE(*) );
	static void mutexLock( MUTEX_VARIABLE(*) );
	static void mutexUnlock( MUTEX_VARIABLE(*) );

	// signals
	//-------------------------------------
	static void signalBlock(const int signum);
	static void signalIgnore(const int signum);

private:

	/** string formatting buffer */
	static char fmtbuf[];
	
	/** mutex for output methods */
	static MUTEX_VARIABLE(outputMutex);
};

#endif //_UTILS_H_
