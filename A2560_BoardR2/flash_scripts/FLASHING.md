# Flashing or updating your Digifiz Replica GEN1 cluster

First copy avrdude.exe to the folder on Windows (on Linux refer bat files and use commands from them directly). Binary files should be here as well, Exported from Arduino IDE or Arduino CLI. 

- flash.bat - Initial flashing. Recommended if you don't care about parameters and in case of initially installed old firmware
- update.bat - EEPROM-aware update. Preserves EEPROM contents. No clusters were dispatched with EEPROM-saving feature. Please first update. 
- dump_eeprom.bat - dumps EEPROM content. Probably I'll write a conversion utility next time. 