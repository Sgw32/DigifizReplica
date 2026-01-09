@echo off
setlocal enabledelayedexpansion

set "HEX_FILE="
for %%F in (*.hex) do (
  set "HEX_FILE=%%F"
  goto :found
)

:found
if "%HEX_FILE%"=="" (
  echo No .hex file found in %CD%.
  exit /b 1
)

avrdude -C avrdude.conf -c usbasp -p m2560 -e -U lfuse:w:0xff:m -U hfuse:w:0x99:m -U efuse:w:0xff:m -U flash:w:%HEX_FILE% -B 3
