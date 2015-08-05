#ifndef __FNCOLLECTION_H_
#define __FNCOLLECTION_H_

void read_eeprom(char *);
void write_eeprom(char *);
void eeprom_init(void);
void eeprom_factory_reset(char *unused);
void ewb(uint8_t *p, uint8_t v);
uint8_t erb(uint8_t *p);
void dumpmem(uint8_t *addr, uint16_t len);

void ledfunc(char *);
void prepare_boot(char *);
void version(char *);
void do_wdt_enable(uint8_t t);

// Already used magics: c1,c2

#define EE_MAGIC_OFFSET      (uint8_t *)0                       // 2 bytes

#define EE_CC1100_CFG        (EE_MAGIC_OFFSET+2)                // Offset:  2
#define EE_CC1100_CFG_SIZE   0x29                               // 41
#define EE_CC1100_PA         (EE_CC1100_CFG+EE_CC1100_CFG_SIZE) // Offset 43/2B
#define EE_CC1100_PA_SIZE    8

#define EE_REQBL             (EE_CC1100_PA+EE_CC1100_PA_SIZE)
#define EE_LED               (EE_REQBL+1)
#define EE_FHTID             (EE_LED+1)

#define EE_FASTRF_CFG        (EE_FHTID+2)                       // Offset: 55:37
#define EE_RF_ROUTER_ID      (EE_FASTRF_CFG+EE_CC1100_CFG_SIZE)
#define EE_RF_ROUTER_ROUTER  (EE_RF_ROUTER_ID+1) 

// if device is equipped with Dudette bootloader (i.e. newer CUNO/COC/CCD/CSM) 
// there is some unique stuff programmed during manufacture:
#define EE_DUDETTE_MAC       (uint8_t *)(E2END-5)		// UNIQUE MAC ADDRESS
#define EE_DUDETTE_PRIV      (EE_DUDETTE_MAC-16)		// 128bit RSA private key 
#define EE_DUDETTE_PUBL      (EE_DUDETTE_PRIV-16)		// 128bit RSA public key


# define EE_LCD_LAST          EE_ETH_LAST

extern uint8_t led_mode;

#endif
