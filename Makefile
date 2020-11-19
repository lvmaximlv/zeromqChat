CC=g++
CFLAGS=-c -Wall
LDFLAGS=
#SOURCES=main.cpp
#OBJECTS=$(SOURCES:.cpp=.o)
#EXECUTABLE=test

#all: $(SOURCES) $(EXECUTABLE)

#$(EXECUTABLE): $(OBJECTS)
#	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

#.cpp.o:
#	$(CC) $(CFLAGS) $< -o $@

# clean:
# 	rm -rf *.o test server client
#-------------------------------------------------------

all: server client

server: 
	g++ -o server server.cpp -lzmq

client: client.cpp
	g++ -o client client.cpp -lzmq

clean:
	rm -rf *.o test server client