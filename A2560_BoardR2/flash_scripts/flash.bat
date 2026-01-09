@echo off
pause
REM avrdude -c usbasp -p m2560 -U lfuse:w:0xff:m -U hfuse:w:0x91:m -U efuse:w:0xfc:m -U flash:w:Digifiz.ino.hex

avrdude -c usbasp -p m2560 -U lfuse:w:0xff:m -U hfuse:w:0x91:m -U efuse:w:0xff:m -U flash:w:Digifiz.ino.hex

pause