@echo off
pause
avrdude -c usbasp -p m2560 -e -U lfuse:w:0xff:m -U hfuse:w:0x99:m -U efuse:w:0xff:m -U flash:w:Digifiz_8K_UK.hex -B 3
pause