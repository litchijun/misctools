CXX = g++
CXXFLAGS = -std=c++11 -static -O2

TARGET = BinEncoder.exe
SOURCE = BinEncoder.cpp

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	del /Q $(TARGET)

.PHONY: clean