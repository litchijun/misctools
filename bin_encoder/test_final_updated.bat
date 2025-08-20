@echo off
echo Cleaning test directory...
del test\* /Q >nul 2>&1

echo Generating 8K test binary file...
generate_test_file.exe

echo Running final updated BinEncoder on test file...
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
echo Checking max packet number in header packet:
powershell -Command "$bytes = [System.IO.File]::ReadAllBytes('test\test_enc.bin'); Write-Host ('Max packet number (byte 3): {0}' -f $bytes[3]); Write-Host ('Max packet number (byte 4): {0}' -f $bytes[4]); Write-Host ('Expected max packet number: 146')"

echo.
echo Checking that header packet contains inverted data from original file 0x1000-0x100E:
echo Original data at 0x1000-0x100E:
powershell -Command "$bytes = [System.IO.File]::ReadAllBytes('test\test.bin'); for ($i=0; $i -lt 15; $i+=8) { $line = '{0:X4}: ' -f (0x1000+$i); for ($j=0; $j -lt 8 -and ($i+$j) -lt 15; $j++) { $line += '{0:X2} ' -f $bytes[0x1000+$i+$j] }; Write-Host $line }"
echo Inverted data in header packet bytes 8-22:
powershell -Command "$bytes = [System.IO.File]::ReadAllBytes('test\test_enc.bin')[8..22]; for ($i=0; $i -lt 15; $i+=8) { $line = '   {0:X2}: ' -f $i; for ($j=0; $j -lt 8 -and ($i+$j) -lt 15; $j++) { $line += '{0:X2} ' -f $bytes[$i+$j] }; Write-Host $line }"

echo.
echo Checking that byte 23 in header packet is 0x00:
powershell -Command "$bytes = [System.IO.File]::ReadAllBytes('test\test_enc.bin'); Write-Host ('Byte 23 value: 0x{0:X2}' -f $bytes[23])"

echo.
echo Checking CRC32 in header packet (bytes 24-27):
powershell -Command "$bytes = [System.IO.File]::ReadAllBytes('test\test_enc.bin'); Write-Host ('CRC32 bytes (24-27): 0x{0:X2} 0x{1:X2} 0x{2:X2} 0x{3:X2}' -f $bytes[24], $bytes[25], $bytes[26], $bytes[27])"

echo.
echo Checking that bytes 28-63 in header packet are 0x00:
powershell -Command "$bytes = [System.IO.File]::ReadAllBytes('test\test_enc.bin')[28..63]; $allZero = $true; foreach ($b in $bytes) { if ($b -ne 0) { $allZero = $false; break } }; if ($allZero) { Write-Host 'Bytes 28-63 are all 0x00: PASS' } else { Write-Host 'Bytes 28-63 are NOT all 0x00: FAIL' }"

echo.
echo Showing header packet:
powershell -Command "$bytes = [System.IO.File]::ReadAllBytes('test\test_enc.bin')[0..63]; for ($i=0; $i -lt 64; $i+=16) { $line = '{0:X4}: ' -f $i; for ($j=0; $j -lt 16 -and ($i+$j) -lt 64; $j++) { $line += '{0:X2} ' -f $bytes[$i+$j] }; Write-Host $line }"