@echo off
echo Cleaning test directory...
del test\* /Q >nul 2>&1

echo Generating 8K test binary file...
generate_test_file.exe

echo Running updated BinEncoder on test file...
BinEncoder.exe test\test.bin

echo.
echo Process completed. Generated files:
dir test
echo.

echo File sizes:
for %%I in (test\*) do echo %%~nxI: %%~zI bytes

echo.
echo Checking that bytes 0x1010-0x1013 in _FF file contain file length:
powershell -Command "$bytes = [System.IO.File]::ReadAllBytes('test\test_FF.bin'); Write-Host ('Byte 0x1010: 0x{0:X2}' -f $bytes[0x1010]); Write-Host ('Byte 0x1011: 0x{0:X2}' -f $bytes[0x1011]); Write-Host ('Byte 0x1012: 0x{0:X2}' -f $bytes[0x1012]); Write-Host ('Byte 0x1013: 0x{0:X2}' -f $bytes[0x1013])"

echo.
echo Showing header packet in encrypted file:
powershell -Command "$bytes = [System.IO.File]::ReadAllBytes('test\test_enc.bin')[0..63]; for ($i=0; $i -lt 64; $i+=16) { $line = '{0:X4}: ' -f $i; for ($j=0; $j -lt 16 -and ($i+$j) -lt 64; $j++) { $line += '{0:X2} ' -f $bytes[$i+$j] }; Write-Host $line }"

echo.
echo Showing first data packet:
powershell -Command "$bytes = [System.IO.File]::ReadAllBytes('test\test_enc.bin')[64..127]; for ($i=0; $i -lt 64; $i+=16) { $line = '{0:X4}: ' -f $i; for ($j=0; $j -lt 16 -and ($i+$j) -lt 64; $j++) { $line += '{0:X2} ' -f $bytes[$i+$j] }; Write-Host $line }"

echo.
echo Showing last data packet ^(with 0xFF padding^):
powershell -Command "$bytes = [System.IO.File]::ReadAllBytes('test\test_enc.bin'); $start = $bytes.Length - 64; $end = $bytes.Length - 1; $data = $bytes[$start..$end]; for ($i=0; $i -lt 64; $i+=16) { $line = '{0:X4}: ' -f $i; for ($j=0; $j -lt 16 -and ($i+$j) -lt 64; $j++) { $line += '{0:X2} ' -f $data[$i+$j] }; Write-Host $line }"