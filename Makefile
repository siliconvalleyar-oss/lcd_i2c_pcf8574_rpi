CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -pedantic -std=c++17
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
TARGET = $(BIN_DIR)/App

SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/lcd.cpp
OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/lcd.o
DEPS = $(INC_DIR)/lcd.hpp

.PHONY: all clean distclean debug release docs

all: release

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS) | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(OBJS) -o $@

debug: CXXFLAGS += -g -O0 -DDEBUG
debug: clean $(TARGET)

release: CXXFLAGS += -O2
release: clean $(TARGET)

clean:
	rm -f $(OBJ_DIR)/*.o

distclean: clean
	rm -f $(TARGET)
	rm -rf $(BIN_DIR) $(OBJ_DIR)

docs:
	@echo "Documentation available in $(DOCS_DIR)/"

run: $(TARGET)
	sudo ./$(TARGET)
