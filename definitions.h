#ifndef definitions_h__
#define definitions_h__

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

#define INPUT_PINS {                   \
  {(uint8_t *const)&PINC, 0b00000100}, \
  {(uint8_t *const)&PIND, 0b00000100}, \
  {(uint8_t *const)&PIND, 0b00001000}, \
  {(uint8_t *const)&PINB, 0b00000010}, \
  {(uint8_t *const)&PINB, 0b00000100}, \
  {(uint8_t *const)&PINB, 0b00001000}, \
  {(uint8_t *const)&PINC, 0b00010000}  \
}

#define LAYOUT {KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G}

#endif
