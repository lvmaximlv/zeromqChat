#ifndef GLOBAL_H
#define GLOBAL_H

#include <plog/Log.h>
#include <iostream>
#include <atomic>

extern std::atomic<bool> g_quit;  //flag for exit if sigint or sigterm were catched
extern std::atomic<bool> g_isDaemon; //flag is set id programm run as a daemon


/***************************************************
 * print function gets variadic parameters,
 * stores them into the log
 * and if app is not a daemon prints them to console
 ***************************************************/

template<typename T>
void print_impl(const T &t)
{
	if(!g_isDaemon.load())
		std::cout << t << ' ';

	PLOGI << t;
}

template <typename ... T>
void print(const T& ... t)
{
	(void)std::initializer_list<int>{ (print_impl(t), 0)... };
	std::cout << '\n';
} 

#endif // GLOBAL_H