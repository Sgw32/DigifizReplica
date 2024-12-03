[![Build Digifiz Next](https://github.com/Sgw32/DigifizReplica/actions/workflows/build_main.yml/badge.svg)](https://github.com/Sgw32/DigifizReplica/actions/workflows/build_main.yml)

[![Build Digifiz Replica Arduino](https://github.com/Sgw32/DigifizReplica/actions/workflows/build_legacy_arduino.yml/badge.svg)](https://github.com/Sgw32/DigifizReplica/actions/workflows/build_legacy_arduino.yml)

# Official Digifiz Replica repository

![Digifiz Replica](/images/digifiz_photo.jpg)

Digifiz Replica project code

Digifiz Replica is a Digifiz replacement for Golf/ Jetta/ Scirocco cars of mk2 generation.
Currently Audi Digifiz with Red and Green version is also supported. 


Requirements: Arduino IDE >1.8.13 (however it may work on previous versions)
Board set to: "Arduino Mega/Atmega2560"
Libraries: MD_MAX72xx (not needed anymore, please do not use it)
RTClib(not by Neuron)
SparkFun EEPROM library    
Adafruit_BusIO
MedianFilterLib2

Ctl+Shift+U for firmware upload

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
- Very far plans: mk2 hardware redesign, Golf 3, Audi 80 B3, AutoVAZ dashboards 
- LCD Digifiz

## License

Digifiz Replica firmware is open source under MIT license. 
Potential hardware files disclosure may be done on one of Open Hardware Licenses.
Currently, hardware is closed source and is available for sale. 

## Support us

Paypal is now blocked in Russia, unfortuntaly I have no residence permit in other countries, where paypal is working.
Several ways of PayPal donation still exist. 
Crypto: 
0xeDc17cb23241eACe19DF3617291aa7d2d92E62DC (ETH, USDT ERC20)
Buy something here (3d print and just 3d models):
https://www.cgtrader.com/sgw32
