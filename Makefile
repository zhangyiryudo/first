CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDLIBS = -ltbb
TARGET = hello
SRC = main.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: clean