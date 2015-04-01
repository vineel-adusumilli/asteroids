#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL
#include <util/delay.h>

#include "grid.h"
#include "mma8452.h"

// Buffers used to keep track of the state of the grid.
// One buffer will be what's currently displayed, while the
// other represents an updated state. The buffers constantly
// switch, with the variable g pointing to the currently
// displayed state.
uint8_t grid[][8] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};

uint8_t g = 0; // Keep track of which grid is currently displayed.
uint8_t c = 0; // Counter used to wait multiple timer interrupts in between updates;

// Update the state of the grid.
void update() {
  uint8_t nextg = g ^ 1; // nextg is the "flip" of g
  int8_t i;
  //int8_t i, j, n;

  // clear grid
  for (i = 0; i < 8; i++) {
    grid[nextg][i] = 0;
  }

  if (accel_read_x() < 0) {
    grid[nextg][2] = (1 << 2);
  } else {
    grid[nextg][2] = (1 << 6);
  }

  /*for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      n = 0;
      if (i - 1 >= 0 && grid[g][j] & (1 << (i - 1)))
        n++;
      if (i - 1 >= 0 && j - 1 >= 0 && grid[g][j - 1] & (1 << (i - 1)))
        n++;
      if (i - 1 >= 0 && j + 1 < 8 && grid[g][j + 1] & (1 << (i - 1)))
        n++;
      if (j - 1 >= 0 && grid[g][j - 1] & (1 << i))
        n++;
      if (j + 1 < 8 && grid[g][j + 1] & (1 << i))
        n++;
      if (i + 1 < 8 && grid[g][j] & (1 << (i + 1)))
        n++;
      if (i + 1 < 8 && j - 1 >= 0 && grid[g][j - 1] & (1 << (i + 1)))
        n++;
      if (i + 1 < 8 && j + 1 < 8 && grid[g][j + 1] & (1 << (i + 1)))
        n++;

      if (n == 3 || (n == 2 && (grid[g][j] & (1 << i)))) {
        grid[nextg][j] |= (1 << i);
      } else {
        grid[nextg][j] &= ~(1 << i);
      }
    }
  }*/

  g = nextg;
}

ISR(TIMER0_OVF_vect) {
  c++;
  if (c > 2) {
    c = 0;
    update();
  }
}

// Read a dummy value from the analog input to seed the random number generator.
void seed_rng() {
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  ADMUX |= (1 << REFS0) | (1 << MUX1); // enable ADC2 pin
  ADMUX |= (1 << ADLAR); // left-adjust the result

  ADCSRA |= (1 << ADEN); // enable ADC
  ADCSRA |= (1 << ADSC); // start conversion

  while (!(ADCSRA & (1 << ADIF))); // wait for conversion to finish
  srand(ADC); // seed random number generator based on randomly read ADC value
  ADCSRA &= ~(1 << ADEN); // disable ADC
}

int main() {
  seed_rng();
  accel_init();

  uint8_t i, j, row_hit;

  // Start grid with random values
  for (i = 0; i < 8; i++) {
    grid[0][i] = rand() % 256;
  }

  // set up timers
  TIMSK0 |= (1 << TOIE0); // enable timer 0 overflow interrupt
  TCNT0 = 0x00; // set timer 0 counter to 0
  TCCR0B |= (1 << CS02) | (1 << CS00); // set timer 0 presdcaler to /1024
  sei(); // enable interrupts

  while (1) {
    for (i = 0; i < 8; i++) {
      (*enable[i])();
      (*high[i])();
      for (j = 0; j < 8; j++) {
        if (grid[g][j] & (1 << (7 - i))) {
          row_hit = i + j < 8 ? 8 - j : 7 - j;
          (*enable[row_hit])();
          _delay_us(1000);
          (*disable[row_hit])();
        }
      }
      (*disable[i])();
      (*low[i])();
    }
  }
  
  return 1;
}
