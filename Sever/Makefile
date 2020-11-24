# CC=g++
# CFLAGS=-c -Wall
# LDFLAGS=-lzmq
# SOURCES=main.cpp ZmqChatServer.cpp
# OBJECTS=$(SOURCES:.cpp=.o)
# EXECUTABLE=chatServer

# all: $(SOURCES) $(EXECUTABLE)
	
# $(EXECUTABLE): $(OBJECTS) 
# 	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# .cpp.o:
# 	$(CC) $(CFLAGS) $< -o $@



# clean:
# 	rm -rf *.o $(EXECUTABLE)

EXECUTABLE = server

SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CC = g++
CPPFLAGS += -Iinclude
CFLAGS += -Wall
LDFLAGS += 
LDLIBS += -lzmq

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE) : $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXECUTABLE)
