#ifndef __MELODIES_H__
#define __MELODIES_H__

#include <stdint.h>

// TM2 runs from IHRC 16 MHz, prescaler /64, scaler /1, period mode:
// f = 16 MHz / (2 * 64 * (TM2B + 1)) = 125000 / (TM2B + 1)
// TM2B is 8 bit, so the lowest note is B4 (~494 Hz).
#define TM2B_FOR(f) (uint8_t)((125000UL + (f) / 2) / (f) - 1)

// Pitch index (5 bits), 0 = rest
#define R    0
#define B4   1
#define C5   2
#define Cs5  3
#define D5   4
#define Ds5  5
#define E5   6
#define F5   7
#define Fs5  8
#define G5   9
#define Gs5 10
#define A5  11
#define As5 12
#define B5  13
#define C6  14
#define Cs6 15
#define D6  16
#define Ds6 17
#define E6  18
#define F6  19
#define Fs6 20
#define G6  21
#define Gs6 22
#define A6  23
#define As6 24
#define B6  25
#define C7  26
#define Cs7 27
#define D7  28
#define Ds7 29
#define E7  30
#define F7  31

const uint8_t note_tm2b[32] = {
  0,
  TM2B_FOR(494),
  TM2B_FOR(523), TM2B_FOR(554), TM2B_FOR(587), TM2B_FOR(622), TM2B_FOR(659), TM2B_FOR(698),
  TM2B_FOR(740), TM2B_FOR(784), TM2B_FOR(831), TM2B_FOR(880), TM2B_FOR(932), TM2B_FOR(988),
  TM2B_FOR(1047), TM2B_FOR(1109), TM2B_FOR(1175), TM2B_FOR(1245), TM2B_FOR(1319), TM2B_FOR(1397),
  TM2B_FOR(1480), TM2B_FOR(1568), TM2B_FOR(1661), TM2B_FOR(1760), TM2B_FOR(1865), TM2B_FOR(1976),
  TM2B_FOR(2093), TM2B_FOR(2217), TM2B_FOR(2349), TM2B_FOR(2489), TM2B_FOR(2637), TM2B_FOR(2794),
};

// Duration index (3 bits) -> length in 16th notes
#define D16  0  // 16th
#define D8   1  // 8th
#define D8D  2  // dotted 8th
#define D4   3  // quarter
#define D4D  4  // dotted quarter
#define D2   5  // half
#define D2D  6  // dotted half
#define D1   7  // whole

const uint8_t duration_16ths[8] = { 1, 2, 3, 4, 6, 8, 12, 16 };

// One note = 1 byte: pitch << 3 | duration (never 0)
#define N(pitch, dur) (uint8_t)(((pitch) << 3) | (dur))

// Each melody: length of a 16th note in ms (> NOTE_GAP_MS), notes..., 0
const uint8_t melodies[] = {
  // 0: Ode to Joy (Beethoven)
  100,
  N(E6, D4), N(E6, D4), N(F6, D4), N(G6, D4), N(G6, D4), N(F6, D4), N(E6, D4), N(D6, D4),
  N(C6, D4), N(C6, D4), N(D6, D4), N(E6, D4), N(E6, D4D), N(D6, D8), N(D6, D2),
  0,

  // 1: Jingle Bells (chorus)
  75,
  N(E6, D4), N(E6, D4), N(E6, D2), N(E6, D4), N(E6, D4), N(E6, D2),
  N(E6, D4), N(G6, D4), N(C6, D4D), N(D6, D8), N(E6, D1),
  0,

  // 2: Happy Birthday
  110,
  N(G5, D8D), N(G5, D16), N(A5, D4), N(G5, D4), N(C6, D4), N(B5, D2),
  N(G5, D8D), N(G5, D16), N(A5, D4), N(G5, D4), N(D6, D4), N(C6, D2),
  0,

  // 3: Fur Elise (Beethoven)
  110,
  N(E6, D16), N(Ds6, D16), N(E6, D16), N(Ds6, D16), N(E6, D16), N(B5, D16), N(D6, D16), N(C6, D16),
  N(A5, D8), N(R, D16), N(C5, D16), N(E5, D16), N(A5, D16),
  N(B5, D8), N(R, D16), N(E5, D16), N(Gs5, D16), N(B5, D16),
  N(C6, D4),
  0,

  // 4: Korobeiniki (Tetris theme)
  90,
  N(E6, D4), N(B5, D8), N(C6, D8), N(D6, D4), N(C6, D8), N(B5, D8),
  N(A5, D4), N(A5, D8), N(C6, D8), N(E6, D4), N(D6, D8), N(C6, D8),
  N(B5, D4D), N(C6, D8), N(D6, D4), N(E6, D4),
  N(C6, D4), N(A5, D4), N(A5, D2),
  0,

  // 5: Twinkle, Twinkle, Little Star
  100,
  N(C6, D4), N(C6, D4), N(G6, D4), N(G6, D4), N(A6, D4), N(A6, D4), N(G6, D2),
  N(F6, D4), N(F6, D4), N(E6, D4), N(E6, D4), N(D6, D4), N(D6, D4), N(C6, D2),
  0,

  // 6: Symphony No. 5 opening (Beethoven)
  70,
  N(G5, D8), N(G5, D8), N(G5, D8), N(Ds5, D1), N(R, D4),
  N(F5, D8), N(F5, D8), N(F5, D8), N(D5, D1),
  0,

  // 7: Gran Vals (Tarrega), the classic phone ringtone
  80,
  N(E6, D8), N(D6, D8), N(Fs5, D4), N(Gs5, D4),
  N(Cs6, D8), N(B5, D8), N(D5, D4), N(E5, D4),
  N(B5, D8), N(A5, D8), N(Cs5, D4), N(E5, D4),
  N(A5, D2),
  0,
};

#endif //__MELODIES_H__
