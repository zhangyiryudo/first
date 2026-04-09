CXX = g++
CXXFLAGS = -std=c++17 -Wall -fsanitize=address
TARGET = hello
SRC = main.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: clean