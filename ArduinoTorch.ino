#include "adc.h"

// Button debounce states
#define s_rest 0
#define s_debounce 1
#define s_done 2

// Torch states
#define s_off 0
#define s_low 1
#define s_med 2
#define s_high 3

int adcVal;
uint8_t btnState = s_rest;
uint8_t torchState = s_off;
uint32_t startBounce = 0;
bool tog = false;

void setup() {
  CurrentMonitor cm;
  double r = cm.GetCurrentReading();

  SetupADC();
  SetupPWM();
  //pinMode(0, OUTPUT); // Output toggle
  pinMode(3, INPUT_PULLUP); // Button
}

void loop() {
  /*
  PORTB |= (1 << PORTB0);
  delayMicroseconds(adcVal);
  PORTB &= ~(1 << PORTB0);
  delayMicroseconds(2000);
  */
  bool buttonPressed = (digitalRead(3) == LOW);
  // millis error gain of x64 due to pwm with Timer0
  switch (btnState) {
    case s_rest:
      if (buttonPressed) {
        startBounce = millis();
        btnState = s_debounce;
      }
      break;
    case s_debounce:
      if (!buttonPressed) {
        btnState = s_rest;
      } else if ((millis() - startBounce) > 3200) {
        toggleMode();
        startBounce = millis();
        btnState = s_done;
      }  
      break;
    case s_done:
      if (!buttonPressed && (millis() - startBounce > 32000)) {
        btnState = s_rest;
      }
      break;
  }
  /*
  if ((millis() - startBounce) > 64000) { // millis error gain of 64 due to pwm with Timer0
    startBounce = millis();
    if (tog) {
      PORTB |= (1 << PORTB0);
        tog = false;
    } else {
      PORTB &= ~(1 << PORTB0);
        tog = true;
    }
  }
  */
}

ISR(ADC_vect)
{
  int val = ADCL;
  val += (int) ADCH << 8;
  adcVal = val;
  ADCSRA |= 1 << ADSC;    // Start Conversion
}

void toggleMode() {
  switch (torchState) {
    case s_off:
      OCR0A = 32;
      torchState = s_low;
      break;
    case s_low:
      OCR0A = 64;
      torchState = s_med;
      break;
    case s_med:
      OCR0A = 96;
      torchState = s_high;
      break;
    case s_high:
      OCR0A = 0;
      torchState = s_off;
      break;
  }
}

void SetupPWM() {
  TCCR0A |= (1<<WGM01) | (1<<WGM00) | (1<<COM0A1) | (1<<COM0B0);
  TCCR0B &= 0xF0;
  TCCR0B |= 0x01;

  OCR0A = 0;

  DDRB |= 1 << DDB0;
}

void SetupADC() {
  DDRB &= 0xFB; // PB2 Input.
  ADCSRA = 0x8F; // ADC enable, interrupt enable, prescaler=128.
  ADMUX = 0x01; // Vref=Vcc, R-adjusted result, ADC1 (PB2).
  sei();        // Enable Global Interrupts
  ADCSRA |= 1 << ADSC;    // Start Conversion
}

