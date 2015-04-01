#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL
#include <util/delay.h>

#define ENABLE(DDRk, PINk) DDR ## DDRk |= (1 << P ## DDRk ## PINk)
#define DISABLE(DDRk, PINk) DDR ## DDRk &= ~(1 << P ## DDRk ## PINk)

#define HIGH(PORTk, PINk) PORT ## PORTk |= (1 << P ## PORTk ## PINk)
#define LOW(PORTk, PINk) PORT ## PORTk &= ~(1 << P ## PORTk ## PINk)

void enable0() { ENABLE(C, 1); }
void enable1() { ENABLE(C, 0); }
void enable2() { ENABLE(B, 2); }
void enable3() { ENABLE(B, 1); }
void enable4() { ENABLE(B, 0); }
void enable5() { ENABLE(D, 7); }
void enable6() { ENABLE(D, 6); }
void enable7() { ENABLE(D, 5); }
void enable8() { ENABLE(B, 7); }

void (*enable[])() = {
  enable0, enable1, enable2, enable3, enable4,
  enable5, enable6, enable7, enable8
};

void disable0() { DISABLE(C, 1); }
void disable1() { DISABLE(C, 0); }
void disable2() { DISABLE(B, 2); }
void disable3() { DISABLE(B, 1); }
void disable4() { DISABLE(B, 0); }
void disable5() { DISABLE(D, 7); }
void disable6() { DISABLE(D, 6); }
void disable7() { DISABLE(D, 5); }
void disable8() { DISABLE(B, 7); }

void (*disable[])() = {
  disable0, disable1, disable2, disable3, disable4,
  disable5, disable6, disable7, disable8
};

void high0() { HIGH(C, 1); }
void high1() { HIGH(C, 0); }
void high2() { HIGH(B, 2); }
void high3() { HIGH(B, 1); }
void high4() { HIGH(B, 0); }
void high5() { HIGH(D, 7); }
void high6() { HIGH(D, 6); }
void high7() { HIGH(D, 5); }
void high8() { HIGH(B, 7); }

void (*high[])() = {
  high0, high1, high2, high3, high4,
  high5, high6, high7, high8
};

void low0() { LOW(C, 1); }
void low1() { LOW(C, 0); }
void low2() { LOW(B, 2); }
void low3() { LOW(B, 1); }
void low4() { LOW(B, 0); }
void low5() { LOW(D, 7); }
void low6() { LOW(D, 6); }
void low7() { LOW(D, 5); }
void low8() { LOW(B, 7); }

void (*low[])() = {
  low0, low1, low2, low3, low4,
  low5, low6, low7, low8
};

uint8_t grid[][8] = {
  { 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0x40, 0x20, 0xE0, 0, 0, 0, 0, 0 }
};

uint8_t g = 0;
uint8_t c = 0;

ISR(TIMER0_OVF_vect) {
  c++;
  if (c > 2) {
    c = 0;
    uint8_t nextg = (g + 1) % 2;
    int8_t i, j, n;

    for (i = 0; i < 8; i++) {
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
    }

    g = nextg;
  }
}

int main() {
  uint8_t i, j, row_hit;

  // randomize grid
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  ADMUX |= (1 << REFS0) | (1 << MUX1); // enable ADC2
  ADMUX |= (1 << ADLAR);

  ADCSRA |= (1 << ADEN);
  ADCSRA |= (1 << ADSC);

  while (!(ADCSRA & (1 << ADIF)));
  srand(ADC);
  ADCSRA &= ~(1 << ADEN);

  for (i = 0; i < 8; i++) {
    grid[0][i] = rand() % 256;
  }

  // set up timers
  TIMSK0 |= (1 << TOIE0); // enable timer 0 overflow interrupt
  TCNT0 = 0x00; // set timer 0 counter to 0
  TCCR0B |= (1 << CS02) | (1 << CS00); // set timer 0 presdcaler to /1024
  sei(); // enable interrupts

  /*while (1) {
    for (i = 0; i < 8; i++) {
      row_hit = 0;
      (*enable[i])();
      (*high[i])();
      for (j = 0; j < 8; j++) {
        if (grid[g][j] & (1 << (7 - i))) {
          row_hit = 1;
          (*enable[i + j < 8 ? 8 - j : 7 - j])();
        }
      }
      if (row_hit)
        _delay_us(1000);
      for (j = 0; j <= 8; j++)
        (*disable[j])();
      (*low[i])();
    }
  }*/
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
