#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL
#include <util/delay.h>

#include "grid.h"
#include "mma8452.h"
#include "font8x8_basic.h"
#include "explosion.h"

#define TRUE 1
#define FALSE 0

#define NUM_ASTEROIDS 8

// Buffer used to keep track of the state of the grid.
uint8_t grid[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

uint8_t c = 0;
uint8_t ac = 0;

// Keep track of game state
enum game_state { INTRO, GAME, END };
enum game_state state;

// Keep track of the x-position of the ship
uint8_t ship;

// Keeps track of asteroids
struct asteroid {
  uint8_t x; // column of asteroid, stays the same
  int8_t y; // row of asteroid, increments over time
  uint8_t live; // boolean of whether or not this asteroid is currently being used
};

struct asteroid asteroids[NUM_ASTEROIDS]; // keep track of all of the asteroids

// increment all live asteroids 1 in the y direction
void update_asteroids() {
  uint8_t i;

  for (i = 0; i < NUM_ASTEROIDS; i++) {
    if (asteroids[i].live) {
      asteroids[i].y++;

      // check for collision with spaceship
      if ((asteroids[i].y == 6 && asteroids[i].x == ship)
          || (asteroids[i].y == 7 && asteroids[i].x >= ship - 1 && asteroids[i].x <= ship + 1)) {
        // we've been hit, game over
        c = 0;
        state = END;
      }

      // stop updating if it's fallen off the board
      if (asteroids[i].y > 10) {
        asteroids[i].live = FALSE;
      }
    }
  }
}

// Scroll given text through screen.
// Calculate display based on index i.
void scroll_text(char* text, uint8_t strlen, uint8_t i) {
  uint8_t char_index = i / 8;
  uint8_t offset = i % 8;
  
  uint8_t ord = (uint8_t) text[char_index];
  grid[0] = font8x8_basic[ord][0] >> offset;
  grid[1] = font8x8_basic[ord][1] >> offset;
  grid[2] = font8x8_basic[ord][2] >> offset;
  grid[3] = font8x8_basic[ord][3] >> offset;
  grid[4] = font8x8_basic[ord][4] >> offset;
  grid[5] = font8x8_basic[ord][5] >> offset;
  grid[6] = font8x8_basic[ord][6] >> offset;
  grid[7] = font8x8_basic[ord][7] >> offset;

  char_index++;
  offset = 8 - offset;
  if (offset != 0 && char_index < strlen) {
    ord = (uint8_t) text[char_index];
    grid[0] |= font8x8_basic[ord][0] << offset;
    grid[1] |= font8x8_basic[ord][1] << offset;
    grid[2] |= font8x8_basic[ord][2] << offset;
    grid[3] |= font8x8_basic[ord][3] << offset;
    grid[4] |= font8x8_basic[ord][4] << offset;
    grid[5] |= font8x8_basic[ord][5] << offset;
    grid[6] |= font8x8_basic[ord][6] << offset;
    grid[7] |= font8x8_basic[ord][7] << offset;
  }
}

// Display text from specified font
void display_font(char font[][8], uint8_t i) {
  grid[0] = font[i][0];
  grid[1] = font[i][1];
  grid[2] = font[i][2];
  grid[3] = font[i][3];
  grid[4] = font[i][4];
  grid[5] = font[i][5];
  grid[6] = font[i][6];
  grid[7] = font[i][7];
}

// Load character from font and into grid
void display_center(char c) {
  grid[0] = font8x8_basic[(uint8_t) c][0] << 1;
  grid[1] = font8x8_basic[(uint8_t) c][1] << 1;
  grid[2] = font8x8_basic[(uint8_t) c][2] << 1;
  grid[3] = font8x8_basic[(uint8_t) c][3] << 1;
  grid[4] = font8x8_basic[(uint8_t) c][4] << 1;
  grid[5] = font8x8_basic[(uint8_t) c][5] << 1;
  grid[6] = font8x8_basic[(uint8_t) c][6] << 1;
  grid[7] = font8x8_basic[(uint8_t) c][7] << 1;
}

// Count down during intro
void intro_update() {
  c++;

  if (c < 166) {
    scroll_text(" ASTEROIDS ", 11, c / 2);
  } else if (c > 170 && c < 190) {
    display_center('3');
  } else if (c > 195 && c < 210) {
    display_center('2');
  } else if (c > 215 && c < 230) {
    display_center('1');
  } else if (c >= 230) {
    c = 0;
    state = GAME;
  } else {
    display_center(' ');
  }
}

// Update the state of the game.
void game_update() {
  int8_t i, y;

  y = accel_read_y();

  // clear grid
  for (i = 0; i < 8; i++) {
    grid[i] = 0;
  }

  y = (y >> 3) + 3;
  y = (y < 1) ? 1 : y;
  y = (y > 6) ? 6 : y;
  ship = 7 - y;

  // draw spaceship
  grid[6] |= (1 << ship);
  grid[7] |= (1 << (ship - 1));
  grid[7] |= (1 << ship);
  grid[7] |= (1 << (ship + 1));

  // draw asteroids
  if (++c >= 3) {
    update_asteroids();
    c = 0;
    if (++ac >= 10) {
      // enable 1 asteroid
      asteroids[0].x = ((rand() ^ rand()) >> 3) % 8;
      asteroids[0].y = -1;
      asteroids[0].live = TRUE;
      ac = 0;
    }
  }
  for (i = 0; i < NUM_ASTEROIDS; i++) {
    if (asteroids[i].live) {
      grid[asteroids[i].y] |= (1 << asteroids[i].x);
    }
  }
}

// exploding sequence for game over
void end_update() {
  c++;

  uint8_t explosion_index = c / 3;
  if (explosion_index > EXPLOSION_LENGTH) {
    c = 0;
    return;
  }

  display_font(explosion_sequence, explosion_index);
}

ISR(TIMER0_OVF_vect) {
  switch (state) {
    case INTRO:
      intro_update();
      break;
    case GAME:
      game_update();
      break;
    default:
      end_update();
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
  uint32_t z = accel_read_z();
  z |= (z << 8);
  z |= (z << 16);
  srand(ADC ^ z); // seed random number generator based on randomly read ADC value
  ADCSRA &= ~(1 << ADEN); // disable ADC
}

int main() {
  accel_init();
  seed_rng();

  uint8_t i, j, row_hit;

  // initialize game state
  state = INTRO;

  // initialize asteroids
  for (i = 0; i < NUM_ASTEROIDS; i++) {
    asteroids[0].x = 0;
    asteroids[0].y = 0;
    asteroids[0].live = FALSE;
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
        if (grid[j] & (1 << i)) {
          row_hit = i + j < 8 ? 8 - j : 7 - j;
          (*enable[row_hit])();
          _delay_us(500);
          (*disable[row_hit])();
        }
      }
      (*disable[i])();
      (*low[i])();
    }
  }
  
  return 1;
}
