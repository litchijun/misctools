@echo off
echo Creating test directory...
mkdir test >nul 2>&1

echo Generating 8K test binary file...
generate_test_file.exe

echo Running BinEncoder on test file...
BinEncoder.exe test\test.bin

echo.
echo Process completed. Generated files:
dir test
echo.

echo Showing hex dump of header packet in encrypted file:
powershell -Command "$bytes = [System.IO.File]::ReadAllBytes('test\test_enc.bin')[0..63]; for ($i=0; $i -lt 64; $i+=16) { $line = '{0:X4}: ' -f $i; for ($j=0; $j -lt 16 -and ($i+$j) -lt 64; $j++) { $line += '{0:X2} ' -f $bytes[$i+$j] }; Write-Host $line }"