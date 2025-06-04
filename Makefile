CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -Wpedantic -O2
TARGET = p2p_chat
RELAY_TARGET = relay_server
SRC = src/p2p_chat.cpp
RELAY_SRC = src/relay_server.cpp

ifeq ($(OS),Windows_NT)
    TARGET := $(TARGET).exe
    RELAY_TARGET := $(RELAY_TARGET).exe
    LDFLAGS = -lws2_32
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LDFLAGS = -pthread
    endif
    ifeq ($(UNAME_S),Darwin)
        LDFLAGS = -pthread
    endif
endif

all: $(TARGET) $(RELAY_TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

$(RELAY_TARGET): $(RELAY_SRC)
	$(CXX) $(CXXFLAGS) -o $(RELAY_TARGET) $(RELAY_SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(RELAY_TARGET)

run: $(TARGET)
	./$(TARGET)

run-relay: $(RELAY_TARGET)
	./$(RELAY_TARGET)

.PHONY: all clean run run-relay