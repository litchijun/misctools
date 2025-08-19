CXX = g++
CXXFLAGS = -std=c++11 -O2 -Wall -static -static-libgcc -static-libstdc++

TARGET = BinCRC.exe
SOURCE = BinCRC.cpp

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	del $(TARGET) 2>nul || exit 0
	if exist test rmdir /s /q test

.PHONY: clean