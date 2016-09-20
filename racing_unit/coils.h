#ifndef COILS_H_
#define COILS_H_
#include "globals.h"

//#define USE_DEBUG_SERIAL

#define PCIE_PORT PCIE2
#define PCMSK_PORT PCMSK2
const uint8_t COIL_PIN_INPUT[CYLINDER_NUMBER] = { 4, 5, 6, 7 };  // PD4, PD5, PD6, PD7, all must be at the same port, also set PCIE_PORT and PCMSK_PORT that corresponds to that port
const uint8_t COIL_PIN_SWITCH[CYLINDER_NUMBER] = { 8, 9, 10, 11 };

uint8_t coil_state[CYLINDER_NUMBER];
bool coil_killed[CYLINDER_NUMBER];
unsigned long rpm_measurement_start_time = 0;
uint16_t last_rpm = 0;


void coils_setup()
{
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
    {
        pinMode(COIL_PIN_SWITCH[i], OUTPUT);
        digitalWrite(COIL_PIN_SWITCH[i], LOW);
        
        coil_killed[i] = false;
        
        pinMode(COIL_PIN_INPUT[i], INPUT);
        digitalWrite(COIL_PIN_INPUT[i], HIGH);
    }

    cli();  // disable interrupts
    PCIFR |= bit(PCIE_PORT);
    PCICR |= bit(PCIE_PORT);  // enable PCINT2(port D) interrupts
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
        PCMSK_PORT |= bit(digitalPinToPCMSKbit(COIL_PIN_INPUT[i]));  // enable interrupts on the appropriate pins
    sei();  // enable interrupts

    rpm_measurement_start_time = micros();
}

void measure_rpm()
{
    last_rpm = globals.current_rpm;
    float crank_time = (micros() - rpm_measurement_start_time) / 2.f;  // spark every 2 cranks
    if (crank_time < 0.f)  // micros() overflow
    {
        rpm_measurement_start_time = micros();
        return;
    }
    globals.current_rpm = 60.f * 1000 * 1000 / crank_time;
        
    rpm_measurement_start_time = micros();
    globals.rpm_rising = globals.current_rpm > last_rpm;

    #ifdef USE_DEBUG_SERIAL
    Serial.print(F("crank_time: "));
    Serial.print(crank_time);
    Serial.print(F("\tRPM: "));
    Serial.println(globals.current_rpm);
    #endif
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
    if (coil_state[0] == HIGH and digitalRead(COIL_PIN_INPUT[0]) == LOW)
        measure_rpm();

    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
    {
        coil_state[i] = digitalRead(COIL_PIN_INPUT[i]);
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
