@echo off
echo Creating test directory...
mkdir test 2>nul

echo Generating 256-byte binary file with values 0x00 to 0xFF...
powershell -Command "[byte[]]$bytes = 0..255; [System.IO.File]::WriteAllBytes('test\test.bin', $bytes)"

echo Compiling BinCRC program...
mingw32-make

echo Testing CRC8...
BinCRC.exe CRC8 test\test.bin

echo Testing CRC16...
BinCRC.exe CRC16 test\test.bin

echo Testing CRC32...
BinCRC.exe CRC32 test\test.bin

echo All tests completed.
dir test