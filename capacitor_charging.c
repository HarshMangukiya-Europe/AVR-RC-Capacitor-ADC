#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

// Pin definitions
#define PIN_ADC        0   // Analog pin A0 for voltage input
#define BUTTON_S1      1   // Analog pin A1 for Button S1
#define BUTTON_S2      2   // Analog pin A2 for Button S2
#define BUTTON_S3      3   // Analog pin A3 for Button S3
#define BUTTON_S4      4   // Analog pin A4 for Button S4
#define BUTTON_S5      5   // Analog pin A5 for Button S5
#define PIN_CHARGE     PB1 // Digital pin D9 (used for output toggle)
#define PIN_DISCHARGE  PB0 // Digital pin D8 (used for discharge control)

volatile uint32_t overflowCount = 0; // Count Timer1 overflows
volatile bool toggleState = false;   // Track toggle state for blinking

// Interrupt Service Routine for Timer1 Overflow Mode
ISR(TIMER1_OVF_vect) {
    if (++overflowCount >= 8) { // Toggle every ~33.6 seconds
        toggleState = !toggleState;
        if (toggleState)
            PORTB |= (1 << PIN_CHARGE);   // Set D9 HIGH
        else
            PORTB &= ~(1 << PIN_CHARGE);  // Set D9 LOW
        overflowCount = 0;
    }
}

// Function Prototypes
void stopTimer1();
void setupCTCTimer1(uint16_t ocr1a);
void setupOverflowTimer1();
void delay_ms(uint16_t ms);
void printVoltage(float v);

void setup() {
    // Serial setup for printing voltage
    UBRR0H = 0;
    UBRR0L = 16;                         // Set baud rate to 115200
    UCSR0A = (1 << U2X0);
    UCSR0B = (1 << TXEN0);               // Enable TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data

    // ADC Setup: Free running mode with AVcc as reference
    ADMUX  = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADSC) |
             (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    ADCSRB = 0; // Free running mode

    // Set buttons A1-A5 as input with internal pull-ups
    DDRC  &= ~((1 << BUTTON_S1) | (1 << BUTTON_S2) | (1 << BUTTON_S3) |
               (1 << BUTTON_S4) | (1 << BUTTON_S5));
    PORTC |=  (1 << BUTTON_S1) | (1 << BUTTON_S2) | (1 << BUTTON_S3) |
              (1 << BUTTON_S4) | (1 << BUTTON_S5);

    // Set D9 and D8 as input initially
    DDRB &= ~((1 << PIN_CHARGE) | (1 << PIN_DISCHARGE));

    sei(); // Enable global interrupts
}

void loop() {
    // Read ADC value and convert to voltage
    uint16_t adcVal = ADC;
    float voltage = adcVal * (5.0 / 1023.0);
    printVoltage(voltage); // Print voltage to serial monitor
    delay_ms(200);         // Wait 200ms

    // Read button states (active LOW)
    bool s1 = !(PINC & (1 << BUTTON_S1));
    bool s2 = !(PINC & (1 << BUTTON_S2));
    bool s3 = !(PINC & (1 << BUTTON_S3));
    bool s4 = !(PINC & (1 << BUTTON_S4));
    bool s5 = !(PINC & (1 << BUTTON_S5));

    // Button S1: Set D9 HIGH continuously
    if (s1) {
        stopTimer1();
        DDRB  |=  (1 << PIN_CHARGE);   // D9 as output
        PORTB |=  (1 << PIN_CHARGE);   // D9 HIGH
        DDRB  &= ~(1 << PIN_DISCHARGE); // D8 as input
    }
    // Button S2: Toggle D9 at slow rate using CTC mode
    else if (s2) {
        setupCTCTimer1(53878); // ~0.145 Hz (slow blink)
    }
    // Button S3: Enable overflow mode (~33s blink period)
    else if (s3) {
        static bool wasActive = false;
        if (!wasActive) {
            setupOverflowTimer1();
            wasActive = true;
        }
    }
    // Button S4: Fast toggle using CTC
    else if (s4) {
        setupCTCTimer1(5387); // ~1.45 Hz (fast blink)
    }
    // Button S5: Set D8 LOW and stop everything
    else if (s5) {
        stopTimer1();
        PORTB &= ~(1 << PIN_CHARGE);    // D9 LOW
        DDRB  &= ~(1 << PIN_CHARGE);    // D9 as input
        DDRB  |=  (1 << PIN_DISCHARGE); // D8 as output
        PORTB &= ~(1 << PIN_DISCHARGE); // D8 LOW
    }
}

// Stops Timer1 completely
void stopTimer1() {
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    TIMSK1 = 0;
}

// Setup Timer1 in CTC (Clear Timer on Compare Match) mode
void setupCTCTimer1(uint16_t ocr1a) {
    stopTimer1();
    DDRB   |=  (1 << PIN_CHARGE);              // D9 as output
    TCCR1A  = (1 << COM1A0);                   // Toggle D9 on compare match
    TCCR1B  = (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC, 1024 prescaler
    OCR1A   = ocr1a;                           // Set compare match value
}

// Setup Timer1 in Overflow mode
void setupOverflowTimer1() {
    stopTimer1();
    TCCR1B  = (1 << CS12) | (1 << CS10);      // Prescaler 1024
    TIMSK1  = (1 << TOIE1);                    // Enable overflow interrupt
    DDRB   |=  (1 << PIN_CHARGE);              // D9 as output
    PORTB  &= ~(1 << PIN_CHARGE);              // Set D9 LOW
    overflowCount = 0;
}

// Software delay in milliseconds
void delay_ms(uint16_t ms) {
    for (uint16_t i = 0; i < ms; i++) {
        _delay_ms(1); // Standard delay
    }
}

// Print voltage value via serial
void printVoltage(float v) {
    char buf[10];
    dtostrf(v, 1, 2, buf); // Convert float to string
    for (char *p = buf; *p; p++) {
        while (!(UCSR0A & (1 << UDRE0))); // Wait for TX ready
        UDR0 = *p;                         // Send character
    }
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = '\n'; // Newline
}

int main(void) {
    setup(); // Initialize everything
    while(1) {
        loop(); // Run main loop
    }
    return 0;
}
