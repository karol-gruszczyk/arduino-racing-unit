#ifndef COILS_H_
#define COILS_H_
#include "globals.h"

#define RPM_REFRESH_RATE 100

const uint8_t COIL_PIN_INT_INPUT[CYLINDER_NUMBER] = { 10, 16, 14, 15 };
const uint8_t COIL_PIN_SWITCH[CYLINDER_NUMBER] = { 9, 8, 6, 5 };

bool pcint_states[4] = { true, true, true, true };
unsigned long rpm_measurement_start_time = 0;
volatile uint16_t coil_spark_counter = 0;
uint16_t last_rpm = 0;


void coils_setup()
{
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
    {
        pinMode(COIL_PIN_INT_INPUT[i], INPUT);
        digitalWrite(COIL_PIN_INT_INPUT[i], HIGH);
    }

    cli();  // disable interrupts
    PCIFR |= bit(PCIE0);
    PCICR |= bit(PCIE0);  // enable PCINT0(port B) interrupts
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
        PCMSK0 |= bit (digitalPinToPCMSKbit(COIL_PIN_INT_INPUT[i]));  // enable interrupts on the appropriate pins on port B
    sei();  // enable interrupts

    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
        pinMode(COIL_PIN_SWITCH[i], OUTPUT);
}

void measure_rpm()
{
    if (millis() - rpm_measurement_start_time >= RPM_REFRESH_RATE)
    {
        last_rpm = globals.current_rpm;
        float measure_time_in_minutes = float(millis() - rpm_measurement_start_time) / 60000.f;
        globals.current_rpm = coil_spark_counter / 2.f / measure_time_in_minutes;

        #if defined(USE_SERIAL) && defined(DISPLAY_RPM) 
        Serial.print(F("RPM: "));
        Serial.print(globals.current_rpm);
        Serial.print(F(", sparks: "));
        Serial.println(coil_spark_counter);
        #endif
        
        rpm_measurement_start_time = millis();
        coil_spark_counter = 0;
        globals.rpm_rising = globals.current_rpm >= last_rpm;
    }
}

void kill_spark(uint16_t duration)
{
    globals.spark_killed = true;
    globals.spark_restore_time = millis() + duration;
}

void restore_spark()
{
    if (globals.spark_killed && millis() >= globals.spark_restore_time)
        globals.spark_killed = false;
}

void spark_int(uint8_t spark_num)
{
    digitalWrite(COIL_PIN_SWITCH[spark_num], globals.spark_killed ? HIGH : LOW);
    coil_spark_counter++;
}

ISR (PCINT0_vect)
{
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
    {
        bool state = digitalRead(COIL_PIN_INT_INPUT[i]);
        if (state && !pcint_states[i])
            spark_int(i);
        pcint_states[i] = state;
    }
}
#endif /* COILS_H_ */
