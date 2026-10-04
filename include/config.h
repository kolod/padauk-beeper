#ifndef __CONFIG__
#define __CONFIG__

#define BUZZER_BIT 3 // PA3 (TM2PWM)

#define BUTTON_BIT 4 // PA4

#define BUTTON_DEBOUNCE_MS 20

#define isButtonActive()    (!(PA & (1 << BUTTON_BIT)))

#define sleep() __asm stopsys __endasm;


#endif