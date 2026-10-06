#ifndef __CONFIG__
#define __CONFIG__

#define BUZZER_BIT 3 // PA3 (TM2PWM)

// Melody select, read once at boot with internal pull-ups.
// Pin tied to GND = 1, open = 0; all open selects melody 0.
#define SEL0_BIT 5 // PA5 (PRSTB reset function stays disabled: CLKMD.0 = 0)
#define SEL1_BIT 6 // PA6
#define SEL2_BIT 7 // PA7
#define SEL_MASK ((1 << SEL0_BIT) | (1 << SEL1_BIT) | (1 << SEL2_BIT))

// Pause between melodies. Approximate: it is timed by the ILRC oscillator,
// which drifts with voltage and temperature.
#define PAUSE_SECONDS 10

// Silence at the end of every note, so repeated notes are heard separately
#define NOTE_GAP_MS 25

#endif
