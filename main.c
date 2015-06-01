#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "usb_keyboard_debug.h"
#include "print.h"
#include "keycode.h"
#include "definitions.h"

struct {uint8_t *const pin; const uint8_t bit;} input[7] = INPUT_PINS;
struct {bool is_modifier; uint8_t value;} layout[7] = LAYOUT;
struct {bool is_pressed; uint8_t bounce;} key[7] = {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}};
uint8_t queue[7] = {255, 255, 255, 255, 255, 255, 255};
uint8_t mod_keys = 0;

void init(void);
void send(void);
void key_press(uint8_t k);
void key_release(uint8_t k);
void interrupt_enable(void);
void interrupt_disable(void);

int main(void) {
  init();
  interrupt_enable();
  sei();
  for(;;);
}

uint16_t interrupt_counter = 0;
uint8_t red = 1;
uint8_t green = 1;
uint8_t blue = -1;
void interrupt_enable(void)  { TIMSK0 |=  (1<<OCIE0A); }
void interrupt_disable(void) { TIMSK0 &= ~(1<<OCIE0A); }
ISR(TIMER0_COMPA_vect) {
  interrupt_disable();
  for(uint8_t k=0; k<7; k++) {
    key[k].bounce |= (*input[k].pin & input[k].bit) == 0;
    if(key[k].bounce == 0b01111111 && !key[k].is_pressed)
      key_press(k);
    if(key[k].bounce == 0b10000000 &&  key[k].is_pressed)
      key_release(k);
    key[k].bounce <<= 1;
  }
//  pbin(PINB); print(" ");
//  pbin(PINC); print(" ");
//  pbin(PIND); print("\n");

  OCR1A+=red;   if(OCR1A==0 || OCR1A==255) red=-red;
  OCR1B+=green; if(OCR1B==0 || OCR1B==255) green=-green;
  OCR1C+=blue;  if(OCR1C==0 || OCR1C==255) blue=-blue;
  /* if(--OCR1A < 196) OCR1A = 255; */
  /* if(--OCR1B < 196) OCR1B = 255; */
  /* if(--OCR1C < 196) OCR1C = 255; */
  
  interrupt_enable();
}


void key_press(uint8_t k) {
  uint8_t i;
  key[k].is_pressed = true;
  for(i = 6-1; i > 0; i--)
    queue[i] = queue[i-1];
  queue[0] = k;
  send();
}
void key_release(uint8_t k) {
  uint8_t i;
  key[k].is_pressed = false;
  for(i = 0; i < 6; i++)
    if(queue[i]==k)
      break;
  for(i = i; i < 6; i++)
    queue[i] = queue[i+1];
  send();
}
void send(void) {
  uint8_t i;
  for(i = 0; i < 6; i++)
    keyboard_keys[i] = queue[i] != 255 ? layout[queue[i]].value : 0;
  keyboard_modifier_keys = 0b00000000;
  usb_keyboard_send();
}

void init(void) {
  usb_init();
  CLKPR = 0x80;  CLKPR = 0;
  MCUCR |= 0x80; MCUCR |= 0x80;

  DDRB  &= 0b00000000;
  DDRB  |= 0b10000000;
  PORTB &= 0b00000000;
  PORTB |= 0b11111111;

  DDRC  &= 0b00001011;
  DDRC  |= 0b01100000;
  PORTC &= 0b00001011;
  PORTC |= 0b01110100;

  DDRD  &= 0b10000000;
  DDRD  |= 0b00000000;
  PORTD &= 0b10000000;
  PORTD |= 0b00001100;

  TCCR0A |=      // Timer0 control register A: timer mode
    (1<<WGM01);  // Set CTC, clear timer on compare
  TCCR0B |=      // Timer0 control register B: step frequency
    (1<<CS00) |  // Prescaler 1024, frequency 15.6kHz (Combined with next line)
    (1<<CS02);   // Prescaler 256, frequency 62.5kHz (This line alone)
  OCR0A = 64;    // Set Timer0 comparison to 16 (the number of steps)

  /* LEDs
     Red:   OC1A PORTC5
     Green: OC1B PORTC6
     Blue:  OC1C PORTB7
   */
  TCCR1A |=      // Timer control register 1A
    (1<<WGM10) | // Fast PWM 8-bit
    (1<<COM1A1)| // Clear OC1A on match, set at TOP
    (1<<COM1B1)| // Clear OC1B on match, set at TOP
    (1<<COM1C1); // Clear OC1C on match, set at TOP
  TCCR1B |=      // Timer control register 1B
    (1<<WGM12) | // Fast PWM 8-bit
    (1<<CS12);   // Prescaler 256
  // Output on PWM pins are turned off when the timer 
  // reaches the value in the output compare register,
  // and turned on when it reaches TOP (=256).
  OCR1A = 255-1;    // Output compare register 1A
  OCR1B = 255-85;    // Output compare register 1B
  OCR1C = 255-170;    // Output compare register 1C

  //  while(!usb_configured());
}
