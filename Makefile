CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDLIBS = -ltbb
TARGET = main
SRC = main.cpp
HEADERS = $(wildcard *.h)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDLIBS)

clean:
	rm -f $(TARGET)

.PHONY: clean