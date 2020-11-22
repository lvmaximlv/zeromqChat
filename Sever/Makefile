CC=g++
CFLAGS=-c -Wall
LDFLAGS=-lzmq
SOURCES=main.cpp ZmqChatServer.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=chatServer

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@



clean:
	rm -rf *.o $(EXECUTABLE)