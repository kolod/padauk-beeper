# Beeper

Firmware for a tiny battery-powered melody beeper based on a Padauk
microcontroller. After power-up it plays one of 8 melodies, sleeps for about
10 seconds and repeats this until the battery is empty.

- No buttons. It starts playing as soon as power is applied.
- The melody is selected with 3 pins, read once at boot.
- Low-power sleep between melodies.
- The low-voltage reset is set to 1.8 V, so it keeps playing as far down the battery as possible.

## Supported chips

| Chip | ROM | RAM | PlatformIO env |
|---|---|---|---|
| PFS154 | 2K words | 128 B | `pfs154` |
| PFS122 | 2K words | 128 B | `pfs122` |
| PMS150C | 1K words (OTP) | 64 B | `pms150c` |
| PMS150G | 1K words (OTP) | 64 B | `pms150g` |

PFS parts can be reprogrammed. PMS parts are one-time programmable.

## Pinout

| Pin | Function |
|---|---|
| PA3 | Buzzer output (Timer2 square wave) |
| PA5 | Melody select bit 0 |
| PA6 | Melody select bit 1 |
| PA7 | Melody select bit 2 |

All other pins are left as inputs with digital input disabled, so they draw no
current even if they are floating.

The buzzer pin is driven low while silent. A piezo buzzer can be connected
directly between PA3 and GND. A magnetic buzzer needs a transistor driver.

## Melody selection

The select pins are read once at power-up with internal pull-ups. A pin tied
to GND reads as 1 and an open pin reads as 0. After reading, the pull-ups are
switched off, so a pin tied to GND does not drain the battery.

| # | PA7 | PA6 | PA5 | Melody |
|---|---|---|---|---|
| 0 | open | open | open | Ode to Joy (Beethoven) |
| 1 | open | open | GND | Jingle Bells |
| 2 | open | GND | open | Happy Birthday |
| 3 | open | GND | GND | Für Elise (Beethoven) |
| 4 | GND | open | open | Korobeiniki (Tetris theme) |
| 5 | GND | open | GND | Twinkle, Twinkle, Little Star |
| 6 | GND | GND | open | Symphony No. 5 opening (Beethoven) |
| 7 | GND | GND | GND | Gran Vals (Tárrega), the classic phone ringtone |

To change the melody, change the jumpers and power-cycle the device.

> PA5 and PA6 are also used for in-circuit programming (PA5 carries the
> programming voltage). Remove the select jumpers before reprogramming a PFS chip.
>
> PA5 is also the chip's reset input, but only when `CLKMD.0` is set; the firmware
> keeps it cleared, so PA5 works as a normal input.
>
> The SOT23-6 package of PMS150C/G has no PA7, so only melodies 0–3 can be
> selected there.

## Building and flashing

You need [PlatformIO](https://platformio.org/) (CLI or the VS Code extension)
and an [Easy PDK programmer](https://github.com/free-pdk/easy-pdk-programmer-hardware).
The toolchain (SDCC) and the platform are installed automatically on the first build.

```sh
cd software
pio run                      # build all targets
pio run -e pfs154            # build one target
pio run -e pfs154 -t upload  # build and flash
```

The firmware contains a placeholder that the Easy PDK programmer fills in with
the internal oscillator calibration during flashing. It is calibrated for
16 MHz IHRC at 4.0 V (`TARGET_VDD_MV` from the board definition).

## Configuration

These settings are in [software/include/config.h](software/include/config.h):

| Setting | Default | Meaning |
|---|---|---|
| `BUZZER_BIT` | 3 | Buzzer pin (PA3 is Timer2's output pin) |
| `SEL0_BIT`, `SEL1_BIT`, `SEL2_BIT` | 5, 6, 7 | Melody select pins on port A |
| `PAUSE_SECONDS` | 10 | Pause between melodies |
| `NOTE_GAP_MS` | 25 | Silence at the end of each note, so that repeated notes are heard separately |

The pause is timed by the low-speed internal oscillator (ILRC). That oscillator
drifts with voltage and temperature, so expect roughly ±30 %.

## Adding or changing melodies

Melodies are in [software/include/melodies.h](software/include/melodies.h). Each melody is
stored as:

```c
// length of a 16th note in ms, notes..., 0
110,
N(G5, D8D), N(G5, D16), N(A5, D4), N(G5, D4), N(C6, D4), N(B5, D2),
0,
```

- Pitches run from `B4` to `F7`. Sharps are written as `Cs6` for C♯6, and `R` is a rest.
- Durations are `D16`, `D8`, `D8D` (dotted), `D4`, `D4D`, `D2`, `D2D` and `D1`.
- The tempo value, the length of a 16th note in ms, must be greater than `NOTE_GAP_MS`.
- The melody number is its position in the list. The selection is 3 bits, so the first 8 melodies can be selected.

Each note takes 1 byte of ROM, or 1 instruction word. This leaves plenty of room
even on the 1K-word chips.

## How it works

1. **Startup.** The low-voltage reset is set to 1.8 V, the system clock is set to 1 MHz from the calibrated IHRC, and the select pins are read.
2. **Play.** Timer2 is clocked from IHRC 16 MHz and generates a square wave on PA3 at `125000 / (TM2B + 1)` Hz. Note timing uses busy-wait delays.
3. **Pause.** The system clock is switched to ILRC and IHRC is turned off. The CPU then enters `stopexe` sleep. Timer16, clocked from ILRC/64, wakes it about every 1.2 s until the pause is over.
4. **Battery empty.** Below 1.8 V the chip resets cleanly. If the voltage recovers, it starts again from step 1.

## Project layout

```
software/platformio.ini            build targets; ROM limits are given in bytes (2 per word)
software/src/main.c                firmware: selection, player, sleep, startup code
software/include/config.h          pins and timing settings
software/include/melodies.h        note table and melodies
software/include/delay.h           cycle-counted busy-wait delays
software/include/auto_sysclock.h   system clock and calibration selection from F_CPU
software/include/startup.h         SDCC startup function name for older/newer versions
```

`millis.h`, `serial.h` and `tone.h` are leftovers from the project template
and are not used.
