Original License see: ./license/
Taken from: http://culfw.de/culfw.html


My CULFW fork for the Arduinio Mini Pro 8MHZ board

This is a project based on CULFW for my WS2812 projects.

My intention is to use the FS20 remote part for my own led projects.


Changed for working with custom Arduino Mini Pro 8MHz and CC1101 board.

Port mapping:

```
#ifdef ARDUINO_MINI_PRO
#define CC1100_CS_DDR	SPI_DDR
#define CC1100_CS_PORT	SPI_PORT
#define CC1100_CS_PIN	SPI_SS

#define CC1100_OUT_DDR	DDRB        // GDO0
#define CC1100_OUT_PORT	PORTB
#define CC1100_OUT_PIN	0

#define CC1100_IN_DDR	DDRB        // GDO2
#define CC1100_IN_PORT	PINB
#define CC1100_IN_PIN	1

```

Flash (Arduino Mini Pro 8MHZ bootloader):

```
avrdude -pm328p -b57600 -cSTK500v1 -P<serial port> -Uflash:w:<firmware hex-file>:i
```
