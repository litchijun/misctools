# BinEncoder

A binary file encryption tool that processes BIN files according to specific requirements.

## Features

1. Modifies bytes 0x1010-0x1013 of a BIN file to store the file length if the file is large enough
2. Creates a "_FF" version of the file with the modification
3. Encrypts the file into a special format with packet headers and inverted data

## Compilation

To compile the program, you need MinGW64 with g++:

```
g++ -std=c++11 -static -O2 -o BinEncoder.exe BinEncoder.cpp
```

Or simply run:
```
make
```

## Usage

```
BinEncoder.exe <bin_file_path>
```

Example:
```
BinEncoder.exe test.bin
```

This will generate two files:
1. `test_FF.bin` - The original file with modified bytes 0x1010-0x1013
2. `test_enc.bin` - The encrypted version with packet headers

## Encryption Process

1. If the input file size is greater than 0x1013 bytes, bytes 0x1010-0x1013 are updated to contain the file length
2. The modified file is saved with "_FF" appended to the filename
3. The FF file is then encrypted into the final format:
   - A header packet containing metadata with inverted data from original file 0x1000-0x100E
   - Data packets with 56-byte chunks of inverted data, each with an 8-byte header
   - Data packets are padded with 0xFF when less than 56 bytes

## Updated Requirements Implementation

1. **Max Packet Number Calculation**: 
   - Updated to `((fileLength + 55) / 56) - 1` as specified in the appendix
   - For an 8KB file, this results in 146 data packets (instead of 147)

2. **Header Packet Format**:
   - Copies and inverts data from original file positions 0x1000-0x100E (15 bytes) to header packet bytes 8-22
   - Byte 23 in the header packet is set to 0x00 as specified
   - CRC32 for header packet data (bytes 8-23) is stored in bytes 24-27 in little-endian format
   - Bytes 28-63 in the header packet are all set to 0x00

3. **CRC32 Implementation**:
   - Uses the polynomial 0x04C11DB7 with specific start and end data:
     - Start data: 0x7ce69b00 | BOARD_TYPE (where BOARD_TYPE = 0x12)
     - End data: 0x67e67e00 | BOARD_TYPE

## Testing

A test suite is included to generate a sample 8KB binary file with sequential byte values 0x00-0xFF repeating.

To run the test:
```
generate_test_file.exe
BinEncoder.exe test\test.bin
```

## Verification

The implementation has been verified to correctly:
1. Modify bytes 0x1010-0x1013 in the _FF file to store the file length
2. Invert data from original file 0x1000-0x100E when copying to the header packet
3. Set byte 23 in the header packet to 0x00
4. Calculate and store CRC32 in bytes 24-27 of the header packet in little-endian format
5. Set bytes 28-63 in the header packet to 0x00
6. Calculate max packet number using the updated formula
7. Generate correct file sizes:
   - Original test.bin: 8,192 bytes
   - Encrypted test_enc.bin: 9,472 bytes (64-byte header + 146 data packets * 64 bytes each)