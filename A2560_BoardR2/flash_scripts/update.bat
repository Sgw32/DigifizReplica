@echo off
pause
avrdude -c usbasp -p m2560 -U flash:w:Digifiz.ino.hex
pause