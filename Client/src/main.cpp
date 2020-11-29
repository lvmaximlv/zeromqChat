#include <iostream>
#include <signal.h>

#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

#include "ZmqChatClient.h"

/// flag for exit if sigint or sigterm were catched
std::atomic<bool> g_quit(false);

/**
 * @brief s_signal_handler
 *
 * s_signal_handler(int) is connected to SIGINT and SIGTERM
 *
 * @param signal_value number of calling signal
 */
static void s_signal_handler(int signal_value)
{
	PLOGI << "quitting via signal [" << signal_value << "]"; 
	g_quit.store(true);
}


/**
 * @brief s_catch_signals
 *
 * Connect SIGINT and SIGTERM to s_signal_handler
 */
static void s_catch_signals()
{
	struct sigaction action;
	action.sa_handler = s_signal_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
}


/**
 * @brief Entry point
 *
 * Execution of the program starts here
 *
 * @return Program exit status
 */
int main()
{
	// init logger
	plog::init(plog::debug, "client.log");
	PLOG_INFO << "\n\n==============================================START CLIENT==============================================";

	// connect signals
	s_catch_signals();

	// create and start client
	try {
		ZmqChatClient::CZmqChatClient client(g_quit);
		client.start();
	}
	catch ( ... )
	{
		PLOG_INFO << "^^^^ end with exception ^^^^^";
	}
	PLOG_INFO << "==============================================END CLIENT==============================================\n\n";

	return 0;
}

