#ifndef CONSOLE_H
#define CONSOLE_H

#include <iostream>
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>
// #include <conio
#include <thread>

namespace MT_Console
{
class Console
{
public:
    static void initialize();
    static void terminate();
    static void write(std::string text);
    static bool read(std::string &text);

private:
    static void input_loop();
    static void write_loop();
    static std::string              input_buffer; //Current input buffer
    static std::string              input_field; //Printed at the bottom of the console
    static std::queue<std::string>  writeQueue;
    static std::queue<std::string>  readQueue;
    static std::condition_variable  condition_variable;
    static std::mutex               write_mutex;
    static std::mutex               read_mutex;
    static std::thread*           write_thread;
    static std::thread*           input_thread;
    static std::atomic<bool>        stop_loops;
    static std::atomic<bool>        initialized;

};
}


#endif // CONSOLE_H
