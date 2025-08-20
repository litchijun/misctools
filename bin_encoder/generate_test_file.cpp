#include <fstream>
#include <vector>

int main() {
    std::ofstream file("test/test.bin", std::ios::binary);
    if (!file) {
        return 1;
    }
    
    // Create 8K of data (8192 bytes)
    // Fill with repeating pattern 0x00 to 0xFF
    std::vector<char> data(8192);
    for (int i = 0; i < 8192; i++) {
        data[i] = i % 256;
    }
    
    file.write(data.data(), data.size());
    file.close();
    
    return 0;
}