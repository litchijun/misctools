CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall

TARGET = BinCRC.exe
SOURCE = BinCRC.cpp

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	del $(TARGET)

.PHONY: clean