CC=g++
CFLAGS=-c -Wall
LDFLAGS=-lzmq
SOURCES=main.cpp ZmqChatClient.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=chatClient

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@



clean:
	rm -rf *.o $(EXECUTABLE)