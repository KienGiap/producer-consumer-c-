# Compiler and flags
CXX      := g++
CXXFLAGS := -Wall -IInc

# Folders
SRC_DIR  := Src
INC_DIR  := Inc
OBJ_DIR  := build

# Files
SRCS     := $(wildcard $(SRC_DIR)/*.cpp) main.cpp
OBJS     := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))
TARGET   := a.out

# Rules
all: $(TARGET)
# 	rm -rf $(OBJ_DIR) $(TARGET)
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@

# Compile each .cpp into .o inside build/
$(OBJ_DIR)/%.o: %.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create build folder if not exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/$(SRC_DIR)

# Cleanup
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
