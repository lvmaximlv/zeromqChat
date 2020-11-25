
#include <unistd.h>
#include <string.h>

#include <iostream>
#include <signal.h>

#include "plog/Initializers/RollingFileInitializer.h"

#include "global.h"
#include "ZmqChatClient.h"

std::atomic<bool> g_quit(false);  //flag for exit if sigint or sigterm were catched

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

	PLOG_INFO << "\n\n==============================================START CLIENT==============================================\n\n";

	/* signals handle */
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = got_signal;
	sigfillset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	
	return 0;
}

void startServer()
{
	print("starting server...");
	CZmqChatServer server;
	server.start();
	print("...Ok.");

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
