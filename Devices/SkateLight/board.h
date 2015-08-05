#ifndef _BOARD_H
#define _BOARD_H


#undef  FULL_CC1100_PA                  // PROGMEM:  108b

#undef  HAS_RAWSEND                     // PROGMEM:  198b     RAM:  4b
#undef  HAS_FASTRF                      // PROGMEM:  362+106  RAM:  1b
#undef  HAS_RF_ROUTER                   // PROGMEM:  920b  RAM: 38b
#undef  HAS_LONGMSG                     // CUR support     RAM: 20b


#define ARDUINO_MINI_PRO


#define FHTBUF_SIZE          174      //                 RAM: 174b
#define RCV_BUCKETS            4      //                 RAM: 25b * bucket
#define FULL_CC1100_PA                // PROGMEM:  108b
#define HAS_CC1101_RX_PLL_LOCK_CHECK_TASK_WAIT
#define HAS_CC1101_PLL_LOCK_CHECK_MSG
#define HAS_CC1101_PLL_LOCK_CHECK_MSG_SW


#define SPI_PORT		PORTB
#define SPI_DDR			DDRB
#define SPI_SS			2
#define SPI_MISO		4
#define SPI_MOSI		3
#define SPI_SCLK		5

#ifdef ARDUINO_MINI_PRO
#define CC1100_CS_DDR		SPI_DDR
#define CC1100_CS_PORT     SPI_PORT
#define CC1100_CS_PIN		SPI_SS

#define CC1100_OUT_DDR		DDRB        // GDO0
#define CC1100_OUT_PORT    PORTB
#define CC1100_OUT_PIN     0

#define CC1100_IN_DDR		DDRD        // GDO2
#define CC1100_IN_PORT     PIND
#define CC1100_IN_PIN      2

#define CC1100_INT			INT0
#define CC1100_INTVECT     INT0_vect
#define CC1100_ISC			ISC00
#define CC1100_EICR        EICRA

#define LED_RGB
#define LED_DDR                 DDRD
#define LED_PORT                PORTD
#define LED_PIN                 2
#define LED_PIN_GREEN           4
#define LED_PIN_RED             3
#define LED_PIN_BLUE            2
#endif

#define BOARD_ID_STR            "CSM868"
#define BOARD_ID_STR433         "CSM433"

#define HAS_UART                1
#define UART_BAUD_RATE          38400

#define TTY_BUFSIZE             128

#define BUSWARE_CSM

#endif
