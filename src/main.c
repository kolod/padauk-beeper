#include <pdk/device.h>
#include "auto_sysclock.h"
#include "delay.h"
#include "melodies/komarovo_optimized.h"
#include "config.h"
#include "startup.h"

uint8_t buttonState;     // debounced state, 1 = pressed
uint8_t buttonDebounce;  // ms the raw reading has differed from buttonState

// Call once per ms. Returns 1 on a debounced press (released -> pressed).
uint8_t buttonPoll() {
  if (isButtonActive() == buttonState) {
    buttonDebounce = 0;
    return 0;
  }

  if (++buttonDebounce < BUTTON_DEBOUNCE_MS) {
    return 0;
  }

  buttonDebounce = 0;
  buttonState ^= 1;
  return buttonState;
}

// Returns 1 if the button was pressed during the delay.
uint8_t delay_ms(uint16_t time) {
  for (uint16_t i = 0; i < time; i++) {
    _delay_ms(1);

    if (buttonPoll()) {
      return 1;
    }
  }

  return 0;
}

// Wait until the raw button reading matches the debounced state.
// Returns 1 if that settled into a new press.
uint8_t buttonSettle() {
  while (isButtonActive() != buttonState) {
    if (delay_ms(1)) {
      return 1;
    }
  }

  return 0;
}

void playMelody() {
  for (int thisNote = 0; thisNote < MELODY_SIZE; thisNote++) {   
    tone(MELODY_TONE(thisNote));
    if (delay_ms(MELODY_DURATION(thisNote))) {
      return;
    }
    
    tone(0);
    if (delay_ms(MELODY_NO_TONE_DURATION(thisNote))) {
      return;
    }
  }
}

// Low-power pin setup:
// - only the button keeps its digital input (it is also the wake-up source);
//   all other inputs are disconnected, so floating pins neither leak current
//   nor wake the chip. No pull-ups on unused pins: a pin tied to GND on the
//   board would draw current through them.
// - the buzzer pin is an output driven low, also while asleep
void ioSetup() {
  PADIER = (1 << BUTTON_BIT);
  PAPH = (1 << BUTTON_BIT);
  PA = 0;
  PAC = (1 << BUZZER_BIT);
#ifdef PBDIER
  PBDIER = 0;
#endif
}

void main() {
  ioSetup();

  uint8_t clkmd = CLKMD;

  while (1) {
    // Wake-ups from bounce, noise or release are filtered out here;
    // a press that stopped the melody is already in buttonState, so it won't restart it
    if (buttonSettle()) {
      playMelody();
      tone(0);
      continue;
    }

    CLKMD = 0xF4;
    CLKMD &= ~CLKMD_ENABLE_IHRC;

    sleep();

    // Back to the calibrated IHRC clock, delays depend on it
    CLKMD = clkmd;
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