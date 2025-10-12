[![Build Digifiz Next](https://github.com/Sgw32/DigifizReplica/actions/workflows/build_main.yml/badge.svg)](https://github.com/Sgw32/DigifizReplica/actions/workflows/build_main.yml)

[![Build Digifiz Replica Arduino](https://github.com/Sgw32/DigifizReplica/actions/workflows/build_legacy_arduino.yml/badge.svg)](https://github.com/Sgw32/DigifizReplica/actions/workflows/build_legacy_arduino.yml)

# Official Digifiz Replica repository

![Digifiz Replica](/images/digifiz_photo.jpg)

Digifiz Replica project code

Digifiz Replica is a Digifiz replacement for Golf/ Jetta/ Scirocco cars of mk2 generation. The project is not an official product(actually, it is a completely new product) and just resembles the look of the device.
Currently Audi Digifiz with Red and Green version is also supported. 


Requirements for Atmega2580 version: Arduino IDE >1.8.13 (however it may work on previous versions)
Board set to: "Arduino Mega/Atmega2560"
Libraries: MD_MAX72xx (not needed anymore, please do not use it)
RTClib(not by Neuron)
SparkFun EEPROM library    
Adafruit_BusIO
MedianFilterLib2

Ctl+Shift+U for firmware upload using USBasp

![Digifiz Replica](/images/digifiz_next_photo.jpg)

Requirements for ESP32 version:

ESP32-IDF v5.2.1



## Digifiz Replica information

As tens of units were already dispatched some info should be here. 
The project itself started in 2021 and many units were sold to Russia and abroad(mainly to USA, Germany, UK, Italy and other countries from EU)

Original Russian community for Digifiz Replica: https://vk.com/digifiz
Mirror (also on RU language) in Telegram with news and so on: https://t.me/digifiz
English versions of communities and chat are coming soon and also will be in Telegram. 

Currently (2023) firmware supports such cars as:

- **Volkswagen Jetta Mk2 (1984-1992)** both CE1/CE2
- **Volkswagen Golf Mk2 (1983-1992)** both CE1/CE2
- **Volkswagen Passat B2 (1984-1992)** - not tested - should work
- **Volkswagen Scirocco 2 (1984-1992)** - needs adapter for contacts

- **Audi 80 B2 (1978–1986)** - both Red and Green Digifiz
- **Audi 80 Coupe B2 (1980-1988)** - both Red and Green Digifiz

More info in manual (see Docs)

## Audi Digifiz

Now Audi Digifiz Replicas are also supported. 
Both Green and Red versions are working (see setup.h file):
![Digifiz Replica Audi Red](/images/audi_red.jpg)
![Digifiz Replica Audi Green](/images/audi_green.jpg)


## Current roadmap

- More precise fuel level measurement
- More stable configuration
- Compile through PlatformIO
- Docs using Doxygen
- Very far plans: mk2 hardware redesign, Golf 3, Audi 80 B3, AvtoVAZ dashboards 
- LCD Digifiz

## License

Digifiz Replica firmware is open source under MIT license. 
Potential hardware files disclosure may be done on one of Open Hardware Licenses.
Currently, hardware is closed source and is available for sale. 

## Support us

Paypal: sgw32nc@gmail.com
Crypto: 
0xeDc17cb23241eACe19DF3617291aa7d2d92E62DC (ETH, USDT ERC20)
Buy something here (3d print and just 3d models):
https://www.cgtrader.com/sgw32

## Trademarks

VDO is a Trademark of the Continental Corporation
The Volkswagen logo, the word "Volkswagen," all Volkswagen vehicle names, and the Volkswagen advertising slogan are registered trademarks of Volkswagen AG
AUDI is a trademark of AUDI AG
AvtoVAZ is a trademark of AvtoVAZ Concern

Any images or text of trademarks used on this site are for information purposes only.

Digifiz Replica dashboards, as well as Digifiz Audi are replacement parts only for the respective cars, with OEM VAG numbers assigned:

191 919 065 B / 191 919 019 B / 87001181 / 191 919 005 B / 191919065B
191919033LR 616.051.2001 6160512001

And Audi OEM part number:
88481435
