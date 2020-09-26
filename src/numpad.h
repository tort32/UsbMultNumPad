#ifndef _numpad_H_
#define _numpad_H_
#include "Arduino.h"

// bit operations
#define sbi_(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi_(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define rbi_(sfr, bit) (_SFR_BYTE(sfr) & _BV(bit))

// pin operations
#define spi_(port, bit) sbi_(PORT ## port, bit)
#define cpi_(port, bit) cbi_(PORT ## port, bit)
#define spi_ddr_(port, bit) sbi_(DDR ## port, bit)
#define cpi_ddr_(port, bit) cbi_(DDR ## port, bit)
#define rpi_(port, bit) rbi_(PIN ## port, bit)
#define bfi_(port, bit) _BV(bit)

// byte operations
#define setbits(port, mask) ((port) |= (mask))
#define clrbits(port, mask) ((port) &= ~(mask))
#define tglbits(port, mask) ((port) ^= (mask))
#define wrtbits(port, bits, mask) ((port) = ((port) & (~(mask))) | ((bits) & (mask)))
#define wrtbitsif(cond, port, mask) if(cond) setbits(port, mask); else clrbits(port, mask);
#define getbits(port, mask) ((port) & (mask))
#define is_bits(port, mask) (((port) & (mask)) != 0)

#define HI(wd) ((uint8_t) ((0xff00 & wd) >> 8))
#define LO(wd) ((uint8_t) (0x00ff & wd))

// Variadic macro
#ifndef cbi
  // Clear bit in byte
  #define cbi(...) cbi_(__VA_ARGS__)
#endif
#ifndef sbi
  // Set bit in byte
  #define sbi(...) sbi_(__VA_ARGS__)
#endif
#ifndef rbi
  // Read bit from byte
  #define rbi(...) rbi_(__VA_ARGS__)
#endif
// Set pin
#define spi(...) spi_(__VA_ARGS__)
// Clear pin
#define cpi(...) cpi_(__VA_ARGS__)
// Read pin
#define rpi(...) rpi_(__VA_ARGS__)
// Set pin as output
#define spi_ddr(...) spi_ddr_(__VA_ARGS__)
// Set pin as input
#define cpi_ddr(...) cpi_ddr_(__VA_ARGS__)
// Bit-field for pin
#define bfi(...) bfi_(__VA_ARGS__)

#endif /* _numpad_H_ */
