#ifndef COILS_H_
#define COILS_H_
#include "globals.h"

//#define USE_SERIAL
#define RPM_REFRESH_RATE 150  // at 1k rpm a spark fires every 120ms

const uint8_t COIL_PIN_INT_INPUT[CYLINDER_NUMBER] = { 10, 16, 14, 15 };
const uint8_t COIL_PIN_SWITCH[CYLINDER_NUMBER] = { 9, 8, 6, 5 };

uint8_t coil_state[CYLINDER_NUMBER];
bool coil_killed[CYLINDER_NUMBER];
unsigned long rpm_measurement_start_time = 0;
volatile uint16_t coil_spark_counter = 0;
uint16_t last_rpm = 0;


void coils_setup()
{
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
    {
        coil_killed[i] = false;
        
        pinMode(COIL_PIN_INT_INPUT[i], INPUT);
        digitalWrite(COIL_PIN_INT_INPUT[i], HIGH);

        pinMode(COIL_PIN_SWITCH[i], OUTPUT);
        digitalWrite(COIL_PIN_SWITCH[i], LOW);
    }

    cli();  // disable interrupts
    PCIFR |= bit(PCIE0);
    PCICR |= bit(PCIE0);  // enable PCINT0(port B) interrupts
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
        PCMSK0 |= bit (digitalPinToPCMSKbit(COIL_PIN_INT_INPUT[i]));  // enable interrupts on the appropriate pins on port B
    sei();  // enable interrupts
}

void measure_rpm()
{
    if (millis() - rpm_measurement_start_time >= RPM_REFRESH_RATE)
    {
        last_rpm = globals.current_rpm;
        float measure_time_in_minutes = float(millis() - rpm_measurement_start_time) / 60000.f;
        globals.current_rpm = coil_spark_counter / CYLINDER_NUMBER / measure_time_in_minutes;

        #ifdef USE_SERIAL
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
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
    {
        if (coil_state[i] == LOW)
        {
            digitalWrite(COIL_PIN_SWITCH[i], HIGH);
            coil_killed[i] = true;
        }
    }
    globals.spark_restore_time = millis() + duration;
}

void restore_spark()
{
    if (globals.spark_killed && millis() >= globals.spark_restore_time)
    {
        globals.spark_killed = false;
        for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
        {
            if (coil_state[i] == LOW)
            {
                digitalWrite(COIL_PIN_SWITCH[i], LOW);
                coil_killed[i] = false;
            }
        }
    }
}

ISR (PCINT0_vect)
{
    coil_spark_counter++;
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
    {
        coil_state[i] = digitalRead(COIL_PIN_INT_INPUT[i]);
        if (globals.spark_killed && !coil_killed[i])
        {
            digitalWrite(COIL_PIN_SWITCH[i], HIGH);
            coil_killed[i] = true;
        }
        else if (!globals.spark_killed && coil_killed[i])
        {
            digitalWrite(COIL_PIN_SWITCH[i], LOW);
            coil_killed[i] = false;
        }
    }
}
#endif /* COILS_H_ */
