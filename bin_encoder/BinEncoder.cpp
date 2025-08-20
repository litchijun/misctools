#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

#define CRC32_POLY 0x04C11DB7
#define BOARD_TYPE 0x12
#define CRC32_INIT (0x7ce69b00 | BOARD_TYPE)

unsigned int _crc32(unsigned int *message, unsigned int msgsize, unsigned int crc = 0xFFFFFFFF)
{
    unsigned int i, j; // byte counter, bit counter
    unsigned int byte;
    unsigned int poly = CRC32_POLY;
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
            byte = byte << 1;    // Ready next msg bit.
        }
    }
    return crc;
}

uint32_t crc32(const uint8_t* message, size_t msgsize) {
    uint32_t crc = 0xFFFFFFFF;
    uint32_t poly = CRC32_POLY;

    // Add start data
    uint32_t start_data = CRC32_INIT;
    crc = _crc32(&start_data, 1);

    // Process message
    for (size_t i = 0; i < msgsize; i+=4) {
        uint32_t data_in = *((uint32_t*)(message + i));
        crc = _crc32(&data_in, 1, crc);
    }

    // Add end data
    uint32_t end_data = 0x67e67e00 | BOARD_TYPE;
    crc = _crc32(&end_data, 1, crc);

    return crc;
}

// Calculate simple checksum
uint8_t checksum(const uint8_t* data, size_t size) {
    uint8_t sum = 0;
    for (size_t i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <bin_file_path>" << std::endl;
        return 1;
    }

    std::string inputFilePath = argv[1];
    
    // Read the input file
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    if (!inputFile) {
        std::cerr << "Error: Cannot open file " << inputFilePath << std::endl;
        return 1;
    }

    // Read file into buffer
    std::vector<uint8_t> bufferData(std::istreambuf_iterator<char>(inputFile), {});
    inputFile.close();
    
    size_t fileLength = bufferData.size();
    
    // Step 1: Modify bytes 0x1010-0x1013 if file length > 0x1013
    if (fileLength > 0x1013) {
        bufferData[0x1010] = fileLength & 0xFF;
        bufferData[0x1011] = (fileLength >> 8) & 0xFF;
        bufferData[0x1012] = (fileLength >> 16) & 0xFF;
        bufferData[0x1013] = (fileLength >> 24) & 0xFF;
    }
    
    // Step 2: Save with "_FF" appended to filename
    size_t lastDot = inputFilePath.find_last_of('.');
    std::string ffFilePath;
    if (lastDot != std::string::npos) {
        ffFilePath = inputFilePath.substr(0, lastDot) + "_FF" + inputFilePath.substr(lastDot);
    } else {
        ffFilePath = inputFilePath + "_FF";
    }
    
    std::ofstream ffFile(ffFilePath, std::ios::binary);
    if (!ffFile) {
        std::cerr << "Error: Cannot create file " << ffFilePath << std::endl;
        return 1;
    }
    ffFile.write(reinterpret_cast<const char*>(bufferData.data()), bufferData.size());
    ffFile.close();
    
    // Step 3: Encrypt to create test_enc.bin
    std::string encFilePath;
    if (lastDot != std::string::npos) {
        encFilePath = inputFilePath.substr(0, lastDot) + "_EN" + inputFilePath.substr(lastDot);
    } else {
        encFilePath = inputFilePath + "_EN";
    }
    
    std::ofstream encFile(encFilePath, std::ios::binary);
    if (!encFile) {
        std::cerr << "Error: Cannot create file " << encFilePath << std::endl;
        return 1;
    }
    
    // Calculate max packet number (only for data packets, not including header)
    // Updated calculation: max_packet_num = ((fileLength + (56 -1)) / 56) - 1
    size_t maxPacketNum = ((fileLength + 55) / 56) - 1;
    
    // Create header packet (64 bytes) -引导包
    std::vector<uint8_t> headerPacket(64, 0);
    headerPacket[0] = 0xFE;
    headerPacket[1] = 0x06;
    headerPacket[2] = (fileLength + 256 + 1023) / 1024;
    headerPacket[3] = maxPacketNum & 0xFF;
    headerPacket[4] = (maxPacketNum >> 8) & 0xFF;
    
    // Copy and invert original data from 0x1000-0x100E to header packet bytes 8-22
    // Note: 0x1000-0x100E is 15 bytes, so we copy to bytes 8-22 (15 bytes)
    if (fileLength >= 0x100F) {
        for (int i = 0; i < 15 && (0x1000 + i) < fileLength; i++) {
            headerPacket[8 + i] = ~bufferData[0x1000 + i]; // Invert data
        }
    }
    
    // Byte 23 is 0x00 as specified
    headerPacket[23] = 0x00;
    
    // Calculate checksum for bytes 0-4
    headerPacket[5] = checksum(headerPacket.data(), 5);
    
    // Calculate CRC32 for bytes 8-23 and store in bytes 24-27 (little-endian)
    uint32_t headerCrc = crc32(headerPacket.data() + 8, 16);
    headerPacket[24] = headerCrc & 0xFF;
    headerPacket[25] = (headerCrc >> 8) & 0xFF;
    headerPacket[26] = (headerCrc >> 16) & 0xFF;
    headerPacket[27] = (headerCrc >> 24) & 0xFF;
    
    // Bytes 28-63 are 0x00 as specified
    // They are already 0x00 due to initialization

    // Write header packet
    encFile.write(reinterpret_cast<const char*>(headerPacket.data()), headerPacket.size());

    // Process data packets
    for (size_t packetIndex = 0; packetIndex <= maxPacketNum; packetIndex++) {
        std::vector<uint8_t> dataPacket(64, 0xFF); // Initialize with 0xFF
        dataPacket[0] = 0xFE;
        dataPacket[1] = 0x07;
        dataPacket[2] = packetIndex & 0xFF;
        dataPacket[3] = (packetIndex >> 8) & 0xFF;
        
        // Copy and invert data
        size_t dataOffset = packetIndex * 56;
        size_t dataBytesToCopy = 56;
        if (dataOffset + dataBytesToCopy > fileLength) {
            dataBytesToCopy = fileLength - dataOffset;
        }
        
        for (size_t i = 0; i < dataBytesToCopy; i++) {
            dataPacket[8 + i] = ~bufferData[dataOffset + i]; // Invert data
        }
        // Remaining bytes are already 0xFF due to initialization
        
        // Calculate checksum for bytes 8-63
        dataPacket[4] = checksum(dataPacket.data() + 8, 56);
        
        // Calculate CRC32 for bytes 8-63
        uint32_t dataCrc = crc32(dataPacket.data() + 8, 56);
        dataPacket[5] = dataCrc & 0xFF;
        dataPacket[6] = (dataCrc >> 8) & 0xFF;
        dataPacket[7] = 0x00;
        
        // Write data packet
        encFile.write(reinterpret_cast<const char*>(dataPacket.data()), dataPacket.size());
    }
    
    encFile.close();
    
    std::cout << "Processing completed successfully!" << std::endl;
    std::cout << "Generated files:" << std::endl;
    std::cout << "  " << ffFilePath << std::endl;
    std::cout << "  " << encFilePath << std::endl;
    
    return 0;
}