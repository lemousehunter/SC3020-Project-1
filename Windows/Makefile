CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -I include

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
DATA_BLOCK_DIR = datablocks
INDEX_FILE = index.dat
DATA_BASE_FILE = database.dat

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
EXECUTABLE = $(BIN_DIR)/bplustree

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	rm -rf $(DATA_BLOCK_DIR)
	rm -f $(INDEX_FILE)
	rm -f $(DATA_BASE_FILE)

.PHONY: all clean