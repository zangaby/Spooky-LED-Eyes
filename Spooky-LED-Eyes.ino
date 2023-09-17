#include <avr/sleep.h>
#include <avr/wdt.h>

//https://elektro.turanis.de/html/prj290/index.html

#define PIN_LED        PB0
#define PIN_LDR_POWER  PB4
#define PIN_LDR_SENSOR PB3

#define FADE_SPEED   4 // in ms
#define GLOW_TIME 1500 // in ms
#define WAIT_CYCLES 10 // multiplier of watchdog

volatile byte cycles = 0;

ISR(WDT_vect)
{
    cycles++;
}

void setup()
{
    pinMode(PIN_LDR_SENSOR, INPUT);
    pinMode(PIN_LDR_POWER, OUTPUT);
    pinMode(PIN_LED, OUTPUT);

    // intro sequence
    for(byte i=0; i<5; i++) {
        digitalWrite(PIN_LED, HIGH);
        delay(40);
        digitalWrite(PIN_LED, LOW);
        delay(40);
    }

    // setup of the WDT
    cli();
    wdt_reset(); // reset watchdog timer
    MCUSR &= ~(1 << WDRF); // remove reset flag
    WDTCR = (1 << WDCE); // set WDCE, access prescaler
    WDTCR = 1 << WDP0 | 1 << WDP3; // set prescaler bits to to 8s
    WDTCR |= (1 << WDTIE); // access WDT interrupt
    sei();
}

void loop()
{
    digitalWrite(PIN_LDR_POWER, HIGH);
    if (analogRead(( analog_pin_t) PIN_LDR_SENSOR) < 200) {
        if (cycles >= WAIT_CYCLES) {
            glowEyes();
            cycles = 0;
        }
    }
    digitalWrite(PIN_LDR_POWER, LOW);
    enterSleepMode();
}

void glowEyes()
{
    pinMode(PIN_LED, OUTPUT);
    for (byte i = 0; i < 255; i++) {
        analogWrite(PIN_LED, i);
        delay(FADE_SPEED);
    }
    delay(GLOW_TIME);

    for (byte i = 255; i > 0; i--) {
        analogWrite(PIN_LED, i);
        delay(FADE_SPEED);
    }

    // be sure the LEDs are turned off!
    digitalWrite(PIN_LED, LOW);
    pinMode(PIN_LED, INPUT);
}

void enterSleepMode()
{
    byte adcsra;

    adcsra = ADCSRA; // save ADC control and status register A
    ADCSRA &= ~(1 << ADEN); // disable ADC

    MCUCR |= (1 << SM1) & ~(1 << SM0); // Sleep-Modus = Power Down
    MCUCR |= (1 << SE); // set sleep enable
    sleep_cpu(); // sleep
    MCUCR &= ~(1 << SE); // reset sleep enable

    ADCSRA = adcsra; // restore ADC control and status register A
}
