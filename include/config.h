#ifndef __CONFIG__
#define __CONFIG__

#define BUZZER_BIT 3 // PA3 (TM2PWM)

#define BUTTON_BIT 4 // PA4

#define BUTTON_DEBOUNCE_MS 20

#define isButtonActive()    (!(PA & (1 << BUTTON_BIT)))

#define sleep() __asm stopsys __endasm;

// Pin is driven low while the timer is stopped (rests); tone() starts/stops TM2
#define buzzerOn()\
    PA &= ~(1 << BUZZER_BIT);\
    PAC |= (1 << BUZZER_BIT);

#define buzzerOff()\
    PAC &= ~(1 << BUZZER_BIT);\
    TM2C = 0;

#define buttonSetup()\
  PADIER |= (1 << BUTTON_BIT);\
  PAPH |= (1 << BUTTON_BIT);

#endif