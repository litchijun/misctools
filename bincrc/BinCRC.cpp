#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cstring>
#include <sstream>

// CRC8 implementation based on the provided algorithm
unsigned char crc8(unsigned char *ptr, unsigned int len, unsigned short crc = 0xFF)
{
    unsigned char i;
    while (len--)
    {
        crc ^= *ptr++;
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x01)
            {
                crc = (crc >> 1) ^ 0x8C;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

// CRC16 implementation based on the provided algorithm
unsigned short crc16(unsigned char *ptr, unsigned int len, unsigned short crc = 0xFFFF)
{
    unsigned char i;

    while (len-- != 0)
    {
        for (i = 0x80; i != 0; i >>= 1)
        {
            if ((crc & 0x8000) != 0)
            {
                crc <<= 1;
                crc ^= 0x1021;
            }
            else
            {
                crc <<= 1;
            }

            if ((*ptr & i) != 0)
            {
                crc ^= 0x1021;
            }
        }
        ptr++;
    }

    return crc;
}

// CRC32 implementation based on the provided algorithm
unsigned int crc32(unsigned int *message, unsigned int msgsize, unsigned int crc = 0xFFFFFFFF)
{
    unsigned int i, j; // byte counter, bit counter
    unsigned int byte;
    unsigned int poly = 0x04C11DB7;
    i = 0;
    for (i = 0; i < msgsize; i++)
    {
        byte = message[i];
        for (j = 0; j < 32; j++)
        {
            if ((int)(crc ^ byte) < 0)
            {
                crc = (crc << 1) ^ poly;
            }
            else
            {
                crc = crc << 1;
            }
            byte = byte << 1; // Ready next msg bit.
        }
    }
    return crc;
}

// Extract file name without path and extension
std::string getFileName(const std::string& filePath) {
    size_t slashPos = filePath.find_last_of("/\\");
    size_t dotPos = filePath.find_last_of(".");
    
    if (slashPos == std::string::npos) {
        slashPos = 0;
    } else {
        slashPos++;
    }
    
    if (dotPos == std::string::npos || dotPos < slashPos) {
        return filePath.substr(slashPos);
    }
    
    return filePath.substr(slashPos, dotPos - slashPos);
}

// Extract file extension
std::string getFileExtension(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of(".");
    if (dotPos == std::string::npos) {
        return "";
    }
    return filePath.substr(dotPos);
}

// Extract file directory
std::string getFileDirectory(const std::string& filePath) {
    size_t slashPos = filePath.find_last_of("/\\");
    if (slashPos == std::string::npos) {
        return "";
    }
    return filePath.substr(0, slashPos + 1);
}

// Function to process the file according to requirements
int processFile(const std::string& method, const std::string& filePath, int length = -1) {
    // Check if file exists
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        std::cerr << "Error: Cannot open file " << filePath << std::endl;
        return 1;
    }

    // Get file size
    inFile.seekg(0, std::ios::end);
    int fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    // Determine the calculation length
    int calcLength = (length == -1) ? fileSize : length;
    
    // Read file data
    // According to new logic, we need to append 16 bytes of 0xFF at the end
    // and the last 16 bytes contain 8 bytes length + 8 bytes CRC
    std::vector<unsigned char> data(calcLength);
    inFile.read(reinterpret_cast<char*>(data.data()), fileSize);
    inFile.close();

    // Fill with 0xFF if needed
    // According to new logic, if calcLength > fileSize, fill with 0xFF
    // But we need to reserve 16 bytes at the end for length+CRC
    for (int i = fileSize; i < calcLength - 16; i++) {
        data[i] = 0xFF;
    }

    // Fill the last 16 bytes with 0xFF initially
    for (int i = calcLength - 16; i < calcLength; i++) {
        data[i] = 0xFF;
    }

    // Calculate CRC on the data excluding the last 16 bytes
    unsigned int crcValue = 0;
    if (method == "CRC8") {
        crcValue = crc8(data.data(), calcLength - 16);
    } else if (method == "CRC16") {
        crcValue = crc16(data.data(), calcLength - 16);
    } else if (method == "CRC32") {
        // For CRC32, convert byte array to word array
        int dataLength = calcLength - 16;
        std::vector<unsigned int> wordData((dataLength + 3) / 4);
        for (int i = 0; i < dataLength; i += 4) {
            unsigned int word = 0;
            for (int j = 0; j < 4 && (i + j) < dataLength; j++) {
                word |= (static_cast<unsigned int>(data[i + j]) << (j * 8));
            }
            wordData[i / 4] = word;
        }
        crcValue = crc32(wordData.data(), wordData.size());
    } else {
        std::cerr << "Error: Unknown CRC method " << method << std::endl;
        return 1;
    }

    // Store file length and CRC value in the last 16 bytes
    // First 8 bytes: file length (as 64-bit value)
    unsigned long long fileLength = static_cast<unsigned long long>(fileSize);
    for (int i = 0; i < 8; i++) {
        data[calcLength - 16 + i] = static_cast<unsigned char>((fileLength >> (i * 8)) & 0xFF);
    }
    
    // Last 8 bytes: CRC value (as 64-bit value)
    unsigned long long crc64 = static_cast<unsigned long long>(crcValue);
    for (int i = 0; i < 8; i++) {
        data[calcLength - 8 + i] = static_cast<unsigned char>((crc64 >> (i * 8)) & 0xFF);
    }

    // Generate output filename with CRC value
    std::string fileName = getFileName(filePath);
    std::string extension = getFileExtension(filePath);
    std::string fileDir = getFileDirectory(filePath);
    
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');
    
    // For the filename, use the appropriate number of digits based on CRC type
    if (method == "CRC8") {
        ss << std::setw(2) << (crcValue & 0xFF);
    } else if (method == "CRC16") {
        ss << std::setw(4) << (crcValue & 0xFFFF);
    } else if (method == "CRC32") {
        ss << std::setw(8) << crcValue;
    }
    
    std::string crcString = ss.str();
    std::string outputFileName = fileDir + fileName + "_" + crcString + extension;

    // Write the output file
    std::ofstream outFile(outputFileName, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Cannot create output file " << outputFileName << std::endl;
        return 1;
    }
    
    outFile.write(reinterpret_cast<char*>(data.data()), calcLength);
    outFile.close();
    
    std::cout << "File processed successfully. Output: " << outputFileName << std::endl;
    std::cout << "CRC Value: 0x" << std::hex << std::uppercase << crcValue << std::dec << std::endl;
    std::cout << "File Size: " << fileSize << std::endl;
    
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <crc_method> <bin_file_path> [crc_calculation_length]" << std::endl;
        std::cerr << "  crc_method: CRC8, CRC16, or CRC32" << std::endl;
        std::cerr << "  bin_file_path: Path to the binary file" << std::endl;
        std::cerr << "  crc_calculation_length: Optional, default is file size, supports hex format (0x...)" << std::endl;
        return 1;
    }
    
    std::string method = argv[1];
    std::string filePath = argv[2];
    int length = -1;
    
    if (argc > 3) {
        std::string lengthStr = argv[3];
        // Check if the length is in hexadecimal format (starts with "0x" or "0X")
        if (lengthStr.length() > 2 && 
            (lengthStr[0] == '0' && (lengthStr[1] == 'x' || lengthStr[1] == 'X'))) {
            // Parse hexadecimal string
            length = std::stoi(lengthStr.substr(2), nullptr, 16);
        } else {
            // Parse decimal string
            length = std::stoi(lengthStr);
        }
    }
    
    return processFile(method, filePath, length);
}