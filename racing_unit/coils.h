#ifndef COILS_H_
#define COILS_H_
#include "globals.h"

#define RPM_REFRESH_RATE 180  // at 1k rpm a spark fires every 120ms

const uint8_t COIL_PIN_INT_INPUT[CYLINDER_NUMBER] = { 4, 5, 6, 7 };  // all on port D (PD4, PD5, PD6, PD7)
const uint8_t COIL_PIN_SWITCH[CYLINDER_NUMBER] = { 8, 9, 10, 11 };

uint8_t coil_state[CYLINDER_NUMBER];
bool coil_killed[CYLINDER_NUMBER];
unsigned long rpm_measurement_start_time = 0;
volatile uint16_t coil_spark_counter = 0;
uint16_t last_rpm = 0;


void coils_setup()
{
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
    {
        pinMode(COIL_PIN_SWITCH[i], OUTPUT);
        digitalWrite(COIL_PIN_SWITCH[i], LOW);

        coil_killed[i] = false;
        
        pinMode(COIL_PIN_INT_INPUT[i], INPUT);
        digitalWrite(COIL_PIN_INT_INPUT[i], HIGH);
    }

    cli();  // disable interrupts
    PCIFR |= bit(PCIE2);
    PCICR |= bit(PCIE2);  // enable PCINT2(port D) interrupts
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
        PCMSK2 |= bit(digitalPinToPCMSKbit(COIL_PIN_INT_INPUT[i]));  // enable interrupts on the appropriate pins on port D
    sei();  // enable interrupts
}

void measure_rpm()
{
    if (millis() - rpm_measurement_start_time >= RPM_REFRESH_RATE)
    {
        last_rpm = globals.current_rpm;
        float measure_time_in_minutes = float(millis() - rpm_measurement_start_time) / 60000.f;
        globals.current_rpm = coil_spark_counter / CYLINDER_NUMBER / measure_time_in_minutes;
        
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

ISR (PCINT2_vect)
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
