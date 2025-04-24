CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3
# For MacOS, use the following: and you should brew install onnxruntime nlohmann-json
INCLUDES = -I/opt/homebrew/include/onnxruntime -I/opt/homebrew/Cellar/nlohmann-json/3.12.0/include
LDFLAGS = -L/opt/homebrew/lib -lonnxruntime

# Source files
SRC = rank.cpp
TARGET = rank

# Default target
all: $(TARGET)

# Compile and link
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) $< -o $@

# Clean build files
clean:
	rm -f $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run 