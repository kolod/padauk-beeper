#include <pdk/device.h>
#include "auto_sysclock.h"
#include "delay.h"
#include "melodies.h"
#include "config.h"
#include "startup.h"

// Timer16 runs from ILRC / 64 and wakes the chip on every rising edge of
// counter bit 9, i.e. every 1024 * 64 = 65536 ILRC cycles (~1.2 s)
#define PAUSE_WAKEUPS ((PAUSE_SECONDS * ILRC_FREQ + 32768UL) / 65536UL)

uint8_t clkmd;  // calibrated IHRC system clock, set by the startup code

void interrupt(void) __interrupt(0) {
  INTRQ = 0;  // only used to wake up from stopexe
}

void delay_ms(uint8_t time) {
  while (time--) {
    _delay_ms(1);
  }
}

void tone(uint8_t pitch) {
  if (pitch == R) {
    // TM2B = 0 would still toggle the output at ~62 kHz, so stop the timer
    TM2C = 0;
  } else {
    TM2S = 3 << 5;  // prescaler /64, scaler /1
    TM2B = note_tm2b[pitch];
    TM2CT = 0;
    TM2C = (uint8_t)(TM2C_MODE_PERIOD | TM2C_OUT_PA3 | TM2C_CLK_IHRC);
  }
}

void playMelody(uint8_t number) {
  uint8_t i = 0;

  // Skip to the selected melody: each one ends with 0
  while (number) {
    if (melodies[i++] == 0) {
      number--;
    }
  }

  uint8_t unit = melodies[i++];
  uint8_t note;

  while ((note = melodies[i++])) {
    uint8_t length = duration_16ths[note & 7];

    tone(note >> 3);
    while (--length) {
      delay_ms(unit);
    }
    delay_ms(unit - NOTE_GAP_MS);

    tone(R);
    delay_ms(NOTE_GAP_MS);
  }
}

// Read the select pins once with pull-ups, then disconnect them again,
// so a pin tied to GND draws no current afterwards.
uint8_t readSelection() {
  PAPH = (uint8_t)SEL_MASK;
  PADIER = (uint8_t)SEL_MASK;
  _delay_ms(1);  // let the pull-ups charge the pins

  uint8_t pins = ~PA;

  PADIER = 0;
  PAPH = 0;

  uint8_t selection = 0;
  if (pins & (1 << SEL0_BIT)) selection |= 1;
  if (pins & (1 << SEL1_BIT)) selection |= 2;
  if (pins & (1 << SEL2_BIT)) selection |= 4;
  return selection;
}

// Low-power sleep between melodies: only ILRC and Timer16 keep running
void pause() {
  CLKMD = (uint8_t)(CLKMD_ILRC | CLKMD_ENABLE_ILRC | CLKMD_ENABLE_IHRC);
  CLKMD &= ~CLKMD_ENABLE_IHRC;

  T16M = (uint8_t)(T16M_CLK_ILRC | T16M_CLK_DIV64 | T16M_INTSRC_9BIT);
  T16C = 0;
  INTRQ = 0;
  INTEN = INTEN_T16;
  __engint();

  for (uint8_t i = PAUSE_WAKEUPS; i; i--) {
    __stopexe();
  }

  __disgint();
  INTEN = 0;
  T16M = T16M_CLK_DISABLE;

  // Back to the calibrated IHRC clock, delays and TM2 depend on it
  CLKMD = clkmd;
}

void main() {
  // All pins: digital input off (floating pins neither leak nor wake the chip),
  // no pull-ups; the buzzer pin is an output driven low, also while asleep
  PA = 0;
  PAC = (1 << BUZZER_BIT);
#ifdef PBDIER
  PBDIER = 0;
#endif

  clkmd = CLKMD;
  uint8_t selection = readSelection();

  // Plays until the battery is empty (low-voltage reset at 1.8 V)
  while (1) {
    playMelody(selection);
    pause();
  }
}

// Startup code - Setup/calibrate system clock
unsigned char STARTUP_FUNCTION(void) {
  // Lowest low-voltage reset level, so the device keeps working until the
  // battery is nearly empty and then resets cleanly instead of misbehaving.
  // Other fuse options are kept at their blank-chip values.
#if defined(PMS150C)
  PDK_SET_FUSE(FUSE_SECURITY_OFF | FUSE_LVR_1V8 | FUSE_IO_DRV_NORMAL | FUSE_BOOTUP_FAST);
#elif defined(PMS150G)
  PDK_SET_FUSE(FUSE_SECURITY_OFF | FUSE_LVR_1V8 | FUSE_PA5_OPEN_DRAIN | FUSE_BOOTUP_FAST);
#elif defined(MISCLVR_1V8)
  MISCLVR = (uint8_t)MISCLVR_1V8;  // PFS154 / PFS122: LVR is a register, not a fuse
#endif

  // Initialize the system clock (CLKMD register) with the IHRC, ILRC, or EOSC clock source and correct divider.
  // The AUTO_INIT_SYSCLOCK() macro uses F_CPU (defined in the Makefile) to choose the IHRC or ILRC clock source and divider.
  // Alternatively, replace this with the more specific PDK_SET_SYSCLOCK(...) macro from pdk/sysclock.h
  AUTO_INIT_SYSCLOCK();

  // Insert placeholder code to tell EasyPdkProg to calibrate the IHRC or ILRC internal oscillator.
  // The AUTO_CALIBRATE_SYSCLOCK(...) macro uses F_CPU (defined in the Makefile) to choose the IHRC or ILRC oscillator.
  // Alternatively, replace this with the more specific EASY_PDK_CALIBRATE_IHRC(...) or EASY_PDK_CALIBRATE_ILRC(...) macro from easy-pdk/calibrate.h
  AUTO_CALIBRATE_SYSCLOCK(TARGET_VDD_MV);

  return 0;   // Return 0 to inform SDCC to continue with normal initialization.
}
