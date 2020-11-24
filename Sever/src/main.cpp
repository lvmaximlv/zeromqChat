#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include <iostream>
#include "plog/Log.h"
#include "plog/Initializers/RollingFileInitializer.h"

int Daemon(void); // main code

int main(int argc, char *argv[])
{
    /* init logger */
    plog::init(plog::debug, "sever.log");

    PLOG_INFO << "\n\n==============================================START SERVER==============================================\n\n";

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
        Daemon();
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
        
        /* Daemon-specific initialization goes here */
        PLOG_INFO << "Start loop";
        /* The Big Loop */
        while (1) {
        //    std::cout << "daemon...\n";
           PLOG_INFO << "daemon...";
           sleep(2); /* wait 30 seconds */
        }
    exit(EXIT_SUCCESS);
    }
    else
    {
        printf("Usage ./daemon -d for daemon or ./daemon -i for interactive\n");
        exit(1);
    }          
    return 0;
}

int Daemon(void)
{
    for(int i = 0; i < 10; ++i)
        std::cout << "Process work..." << i << '\n';
}