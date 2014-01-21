#include "amautils.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>


// definitions of prefixes in output messages
#define PREFIX_DEBUG	"DEBUG: "
#define PREFIX_INFO	"INFO : "
#define PREFIX_ERROR	""

char Utils::fmtbuf[1024];
INIT_STATIC_MUTEX_VARIABLE(Utils::outputMutex);



/**
 * Prepare unused socket as TCP socket.
 * Every error in this method causes FATAL_ERROR and application terminates immediatelly.
 * @author Petr Djakow
 */
int Utils::prepareTcpSocket()
{
	struct protoent *protocol;
	int sockfd;
	
	//find TCP protocol number
	if( !(protocol = getprotobyname("tcp")) )
		FATAL_ERROR(("tcp: Cannot get protocol number"));
	
	//create TCP socket
	if( (sockfd = socket( PF_INET, SOCK_STREAM, protocol->p_proto)) == -1 )
		FATAL_ERROR(("socket()"));
	
	return sockfd;
}

/**
 * Try to close filedescriptor > 2.
 * @author Petr Djakow
 */
void Utils::closefd(int sockfd)
{
	assert(sockfd > 2);
	if( close(sockfd) == -1 )
	{
		OUT_ERROR(("close(%d)", sockfd));
		return;
	}
	OUT_DEBUG(("Filedescriptor #%d closed", sockfd));
}

/**
 * Prepare socket address based on hostname:port.
 * Call DNS lookup of the hostname.
 * @author Petr Djakow
 */
int Utils::prepareSocketAddr(	struct sockaddr_in *addr,
									const char * hostname,
									const unsigned short port)
{
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port); //network byte order

	if(hostname == NULL)
	{
		//everyone is able to connect to server
		addr->sin_addr.s_addr = INADDR_ANY;
	} else
	{	
		struct hostent *he;
		if( (he = gethostbyname(hostname)) == NULL )
		{
			OUT_ERROR(("host '%s' not found\n", hostname));
			return -1; //error
		}
	
		assert(he->h_addrtype == AF_INET);
		assert(he->h_length == sizeof(addr->sin_addr.s_addr));
	    memcpy(&addr->sin_addr.s_addr, he->h_addr_list[0], (size_t)he->h_length);
	}
	
	return 0; //success
}

/**
 * 
 * @author Petr Djakow
 */
void Utils::reuseSocketAddr(int sockfd)
{
	int opt = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		FATAL_ERROR(("Could not reuse socket addr"));
}

/**
 * Show debug messages.
 * @author Petr Djakow
 */
void Utils::debug(char * fmt, ...)
{
	snprintf(fmtbuf, sizeof(fmtbuf), PREFIX_DEBUG "%s\n", fmt);
	
	MUTEX_LOCK(outputMutex);
		va_list ap;
		va_start(ap, fmt);
		vprintf(fmtbuf, ap);
		va_end(ap);
	MUTEX_UNLOCK(outputMutex);
}

/**
 * Show info messages.
 * @author Petr Djakow
 */
void Utils::info(char * fmt, ...)
{
	snprintf(fmtbuf, sizeof(fmtbuf), PREFIX_INFO "%s\n", fmt);
	
	MUTEX_LOCK(outputMutex);
		va_list ap;
		va_start(ap, fmt);
		vprintf(fmtbuf, ap);
		va_end(ap);
	MUTEX_UNLOCK(outputMutex);
}

/**
 * Show error messages.
 * @author Petr Djakow
 */
void Utils::error(char * fmt, ...)
{
	error_t my_errno = errno;

	MUTEX_LOCK(outputMutex);
		va_list ap;
		va_start(ap, fmt);
		fputs(PREFIX_ERROR, stderr);
		vfprintf(stderr, fmt, ap);
		if(my_errno)
		{
			errno = my_errno;
			fprintf (stderr, " : [%d] %m", my_errno);
		}
		fputc('\n', stderr);
		va_end(ap);
	MUTEX_UNLOCK(outputMutex);
}

/**
 * Show error message and exit application immediately.
 * @author Petr Djakow
 */
void Utils::fatalError(char * fmt, ...)
{
	error_t my_errno = errno;

	MUTEX_LOCK(outputMutex);
		va_list ap;
		va_start(ap, fmt);
		fputs(PREFIX_ERROR, stderr);
		vfprintf(stderr, fmt, ap);
		errno = my_errno;
		fprintf (stderr, " : %m\n");
		va_end(ap);
	MUTEX_UNLOCK(outputMutex);

	exit(EXIT_FAILURE);
}

/**
 * @author Petr Djakow
 */
void Utils::mutexLock( MUTEX_VARIABLE(*mutexVar) )
{
	#ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
		error_t err = pthread_mutex_lock(mutexVar);
		if(err)
		{
			fprintf(stderr, "pthread_mutex_lock(): %s\n", strerror(err));
			exit(EXIT_FAILURE);
		}
	#else
		assert(mutexVar);
		if(mutexVar->locked_by == pthread_self())
		{
			++(mutexVar->locked_count);
			return;
		}
		else
		{
			error_t err = pthread_mutex_lock(&mutexVar->mutex);
			if(err)
			{
				fprintf(stderr, "pthread_mutex_lock(): %s\n", strerror(err));
				exit(EXIT_FAILURE);
			}
			assert(mutexVar->locked_count == 0);
			++(mutexVar->locked_count);
			mutexVar->locked_by = pthread_self();
		}
	#endif
}

/**
 * @author Petr Djakow
 */
void Utils::mutexUnlock( MUTEX_VARIABLE(*mutexVar) )
{
	#ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
		error_t err = pthread_mutex_unlock(mutexVar);
		if(err)
		{
			fprintf(stderr, "pthread_mutex_unlock(): %s\n", strerror(err));
			exit(EXIT_FAILURE);
		}
	#else
		assert(mutexVar);
		if(mutexVar->locked_by == pthread_self())
		{
			--(mutexVar->locked_count);
			if(mutexVar->locked_count == 0)
			{
				mutexVar->locked_by = 0;
				error_t err = pthread_mutex_unlock(&mutexVar->mutex);
				if(err)
				{
					fprintf(stderr, "pthread_mutex_unlock(): %s\n", strerror(err));
					exit(EXIT_FAILURE);
				}
			} else
				return;
		} else
			FATAL_ERROR(("Trying to unlock critical section locked by another thread"));
	#endif
}


/**
 * @author Petr Djakow
 */
void Utils::signalBlock(const int signum)
{
	sigset_t sigs;
	sigemptyset(&sigs);
	sigaddset(&sigs, signum);
	sigprocmask(SIG_BLOCK, &sigs, NULL);
}

/**
 * @author Petr Djakow
 */
void Utils::signalIgnore(const int signum)
{
	#ifdef _GNU_SOURCE
		if(sigignore(signum) == -1)
			FATAL_ERROR(("sigignore(%d)", signum));
		OUT_DEBUG(("Signal '%s' will be ignored", strsignal(signum)));
	#else
		set_signal_handler(signum, SIG_IGN);
		OUT_DEBUG(("Signal '%d' will be ignored", signum));
	#endif
}
