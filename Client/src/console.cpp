#include "console.h"

using namespace MT_Console;

std::thread*            Console::input_thread;
std::thread*            Console::write_thread;
std::string             Console::input_buffer;
std::string             Console::input_field;
std::queue<std::string> Console::writeQueue;
std::condition_variable Console::condition_variable;
std::mutex              Console::write_mutex;
std::mutex              Console::read_mutex;
std::queue<std::string> Console::readQueue;
std::atomic<bool>       Console::stop_loops (false);
std::atomic<bool>       Console::initialized (false);

void Console::input_loop() {
    while (!stop_loops) {
        uint32_t c = std::getchar();
        if (c == 8) { //Backspace
            if (!input_buffer.empty()) {
                input_buffer.pop_back();
                input_field = input_buffer;
                std::unique_lock<std::mutex> l(write_mutex);
                condition_variable.notify_one();
            }
        }
        else if (c == 13) { //Carriage return
            if (!input_buffer.empty()) {
                std::unique_lock<std::mutex> l(read_mutex);
                readQueue.push(input_buffer);
                write(input_buffer);
                input_buffer.clear();
                input_field = input_buffer;
            }
        }
        else if (c >= 32) { //Accepted input characters
            input_buffer.push_back(c);
            input_field = input_buffer;
            std::unique_lock<std::mutex> l(write_mutex);
            condition_variable.notify_one();
        }
    }
}

void Console::write_loop() {
    while (!stop_loops) {
        std::unique_lock<std::mutex> l(write_mutex);
        condition_variable.wait(l);

        if (!writeQueue.empty()) { //Update input field and add a new line
            std::string text = writeQueue.front();
            writeQueue.pop();
            std::cout << "\r" << text << "                                                                                " << std::endl;
            std::cout << "> " << input_field;
        }
        else { //Update input field
            std::cout << "\r> " << input_field << "                                                                                ";
        }
    }
}

void Console::write(std::string text) {
    std::unique_lock<std::mutex> l(write_mutex);
    writeQueue.push(text);
    condition_variable.notify_one();
}

bool Console::read(std::string& text) {
    std::unique_lock<std::mutex> l(read_mutex);
    if (!readQueue.empty()) {
        text = readQueue.front();
        readQueue.pop();
        return true;
    }
    return false;
}

void Console::initialize() {
    if (!initialized) {
        input_thread = new std::thread(&input_loop);
        write_thread = new std::thread(&write_loop);
        initialized = true;
    }
}

void Console::terminate() {
    stop_loops = true;
    condition_variable.notify_one();
    // input_thread->interrupt();
    input_thread->join();
    write_thread->join();

    delete input_thread;
    delete write_thread;
}
