CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -I./asio-1.26.0/include
SRCDIR = .
BUILD_DIR = build

SRCS_CLIENT = $(SRCDIR)/client/client.cpp
SRCS_SERVER = $(SRCDIR)/server/server.cpp

TARGET_CLIENT = $(BUILD_DIR)/client
TARGET_SERVER = $(BUILD_DIR)/server

.PHONY: all clean

all: $(TARGET_CLIENT) $(TARGET_SERVER)

$(TARGET_CLIENT): $(SRCS_CLIENT)
	$(CXX) $(CXXFLAGS) ${SRCS_CLIENT} -o ${TARGET_CLIENT}

$(TARGET_SERVER): $(SRCS_SERVER)
	$(CXX) $(CXXFLAGS) ${SRCS_SERVER} -o ${TARGET_SERVER}
clean:
	rm -f $(TARGET_CLIENT) $(TARGET_SERVER)

.PHONY: clean
