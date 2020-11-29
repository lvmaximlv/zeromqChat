#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <signal.h>

#include "plog/Initializers/RollingFileInitializer.h"

#include "global.h"
#include "ZmqChatServer.h"

std::atomic<bool> g_quit(false);  //flag for exit if sigint or sigterm were catched
std::atomic<bool> g_isDaemon(false); //flag is set id programm run as a daemon

/*********************************
 * got_signal(int) called when
 * sigint or sigterm were catched
 *********************************/ 
inline void got_signal(int)
{
	g_quit.store(true);
	print ("Signal caught");
}

void startServer(); // main code

int main(int argc, char *argv[])
{
	/* init logger */
	plog::init(plog::debug, "sever.log");

	PLOG_INFO << "\n\n==============================================START SERVER==============================================\n\n";

	/* signals handle */
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = got_signal;
		sigfillset(&sa.sa_mask);
		sigaction(SIGINT, &sa, NULL);
		sigaction(SIGTERM, &sa, NULL);

	/* Our process ID and Session ID */
	pid_t pid, sid;

	if (argc < 2)
	{
		printf("Usage ./daemon -d for daemon or ./daemon -i for interactive\n");
		exit(1);
	}
	if (strcmp(argv[1],"-i")==0)
	{
		PLOG_INFO << "Start interactive mode";
		startServer();
	}
	else if (strcmp(argv[1],"-d")==0)
	{
		PLOG_VERBOSE<< "START Daemon mode";
		/* Fork off the parent process */
		pid = fork();
		if (pid < 0) {
			PLOG_FATAL << "EXIT:44";
			exit(EXIT_FAILURE);
		}
		/* If we got a good PID, then
		   we can exit the parent process. */
		if (pid > 0) {
			exit(EXIT_SUCCESS);
		}

		/* Change the file mode mask */
		umask(0);

		/* Create a new SID for the child process */
		sid = setsid();
		if (sid < 0) {
			PLOG_FATAL << "EXIT:60";
			exit(EXIT_FAILURE);
		}

		/* Change the current working directory */
		if ((chdir("/")) < 0) {
			PLOG_FATAL << "EXIT:66";
			exit(EXIT_FAILURE);
		}

		/* Close out the standard file descriptors */
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		startServer();
		exit(EXIT_SUCCESS);
	}
	else
	{
		printf("Usage ./daemon -d for daemon or ./daemon -i for interactive\n");
		exit(1);
	}
	return 0;
}

void startServer()
{
	print("starting server...");
	CZmqChatServer server;
	server.start();
	print("...Ok.\n");

	while(1)
	{
		//wait for exit code...
		sleep(1);

		if(g_quit.load())
		{
			PLOG_INFO << "EXIT VIA SIGNAL...";
			break;
		}
	}
}
