EXECUTABLE = server 

SRC_DIR = src
OBJ_DIR = obj

SRC = $(wildcard $(SRC_DIR)/*cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CC = g++
CPPFLAGS += -Iinclude
CFLAGS += -Wall
LDFLAGS += 
LDLIBS += -lzmq -lpthread

.PHONY: all clean

all: objdir $(EXECUTABLE)

$(EXECUTABLE) : $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

objdir: 
	mkdir -p $(OBJ_DIR)

clean:
	rm -f $(OBJ) $(EXECUTABLE)
