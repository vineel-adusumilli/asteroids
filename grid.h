/* grid.h
 * Helpful functions used to control the LED grid.
 * All operations are meant to take constant time (hence no switch statements or conditionals).
 */

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

