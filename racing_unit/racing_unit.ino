#include "mpu.h"

//#define USE_SERIAL

#define CYLINDER_NUMBER 4

#define LAUNCH_CONTROL_RPM 6000
#define LAUNCH_CONTROL_KILL_TIME 100
#define LAUNCH_CONTROL_WORK_TIME 5000
#define LAUNCH_CONTROL_ACCELEROMETER_MIN_ACCELERATION 1000
#define LAUNCH_CONTROL_ENABLE_INPUT_PIN 0
#define LAUNCH_CONTROL_ENABLING_TIME 3000

#define WHEELIE_CONTROL_MAX_ANGLE 10
#define WHEELIE_CONTROL_KILL_TIME 100
#define WHEELIE_CONTROL_AXIS 2

#define QUICKSHIFTER_MIN_RPM 2000
#define QUICKSHIFTER_MAX_RPM 14000

const uint16_t QUICKSHIFTER_KILL_TIME_AT_RPM[][2] = {
    {2000, 80},
    {5500, 75},
    {8500, 70},
    {11500, 65},
};

#define ENGINE_BRAKING_RPM_FALL_PER_SECOND 500
#define ENGINE_BRAKING_KILL_TIME 100

const uint8_t COIL_PIN_INT_INPUT[CYLINDER_NUMBER] = { 10, 16, 14, 15 };  // PB6, PB2, PB3, PB1
const uint8_t COIL_PIN_PCINT_INPUT[CYLINDER_NUMBER] = { PCINT6, PCINT2, PCINT3, PCINT1 };
const uint8_t COIL_PIN_SWITCH[CYLINDER_NUMBER] = { 3, 4, 5, 6 };

#define RPM_REFRESH_RATE 200
unsigned long rpm_measurement_start_time = 0;
volatile uint16_t coil_spark_counter = 0;
uint16_t last_rpm = 0;
uint16_t current_rpm = 0;
bool rpm_rising = true;
bool spark_killed = false;

bool launch_control_enabling_started = false;
unsigned long launch_control_enabling_start_time = 0;
bool launch_control_enabled = false;
bool launch_control_started = false;
unsigned long launch_control_start_time = 0;

void coils_setup();
void quickshifter();
void launch_control();
void wheelie_control();
void measure_rpm();

uint8_t get_kill_time(uint16_t rpm);
void kill_spark(uint16_t duration);

void setup()
{
    #ifdef USE_SERIAL
    Serial.begin(115200);
    #endif
    
    mpu_setup();
    coils_setup();
    pinMode(LAUNCH_CONTROL_ENABLE_INPUT_PIN, INPUT);
}

void loop()
{
    mpu_loop();
    measure_rpm();
    quickshifter();
    launch_control();
    wheelie_control();
}

void coils_setup()
{
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
    {
        pinMode(COIL_PIN_INT_INPUT[i], INPUT);
        digitalWrite(COIL_PIN_INT_INPUT[i], HIGH);
    }

    cli();  // disable interrupts
    PCICR |= (1 << PCIE0);  // enable PCINT0(port B) interrupts
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
        PCMSK0 |= (1 << COIL_PIN_PCINT_INPUT[i]);  // enable interrupts on the appropriate pins on port B
    sei();  // enable interrupts

    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
        pinMode(COIL_PIN_SWITCH[i], OUTPUT);
}

void measure_rpm()
{
    if (millis() - rpm_measurement_start_time >= RPM_REFRESH_RATE)
    {
        last_rpm = current_rpm;
        float measure_time_in_minutes = float(millis() - rpm_measurement_start_time) / 60000.f;
        current_rpm = coil_spark_counter / 2.f / measure_time_in_minutes;

        #ifdef USE_SERIAL
        Serial.print(F("RPM: "));
        Serial.print(current_rpm);
        Serial.print(F(", sparks: "));
        Serial.println(coil_spark_counter);
        #endif
        
        rpm_measurement_start_time = millis();
        coil_spark_counter = 0;
        rpm_rising = current_rpm >= last_rpm;
    }
}

void quickshifter()
{
    
}

void launch_control()
{
    if (digitalRead(LAUNCH_CONTROL_ENABLE_INPUT_PIN) == HIGH)
    {
        if (!launch_control_enabling_started)
        {
            launch_control_enabling_started = true;
            launch_control_enabling_start_time = millis();
        }
        if (millis() - launch_control_enabling_start_time >= LAUNCH_CONTROL_ENABLING_TIME)
            launch_control_enabled = true;

    }
    else
        launch_control_enabling_started = false;

    if (launch_control_enabled)
    {
        if (abs(accel_real_avg.y) >= LAUNCH_CONTROL_ACCELEROMETER_MIN_ACCELERATION)
        {
            if (!launch_control_started)
            {
                launch_control_started = true;
                launch_control_start_time = millis();
            }
            else
            {
                if (millis() - launch_control_start_time >= LAUNCH_CONTROL_WORK_TIME)
                {
                    launch_control_started = false;
                    launch_control_enabled = false;
                }
            }
        }
        else
            launch_control_started = false;

        if (current_rpm >= LAUNCH_CONTROL_RPM)
            kill_spark(LAUNCH_CONTROL_KILL_TIME);
    }
}

void wheelie_control()
{
    if (abs(ypr_avg[WHEELIE_CONTROL_AXIS]) >= WHEELIE_CONTROL_MAX_ANGLE)
        kill_spark(WHEELIE_CONTROL_KILL_TIME);
}

uint8_t get_kill_time(uint16_t rpm)
{
    uint8_t array_len = sizeof(QUICKSHIFTER_KILL_TIME_AT_RPM) / sizeof(QUICKSHIFTER_KILL_TIME_AT_RPM[0]);
    for (uint8_t i = array_len - 1; i >= 0; i--)
    {
        if (QUICKSHIFTER_KILL_TIME_AT_RPM[i][0] < rpm)
            return QUICKSHIFTER_KILL_TIME_AT_RPM[i][1];
    }
}

void kill_spark(uint16_t duration)
{
    spark_killed = true;
    delay(duration);
    spark_killed = false;

    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
        digitalWrite(COIL_PIN_SWITCH[i], LOW);
}

void spark_int(uint8_t spark_num)
{
    digitalWrite(COIL_PIN_SWITCH[spark_num], spark_killed ? HIGH : LOW);
    coil_spark_counter++;
}

int8_t last_high_spark = -1;

ISR (PCINT0_vect)
{
    if (last_high_spark == -1)
    {
        for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
        {
            if (digitalRead(COIL_PIN_INT_INPUT[i] == HIGH))
            {
                last_high_spark = i;
                return;
            }
        }
    }
    else
    {
        if (digitalRead(COIL_PIN_INT_INPUT[last_high_spark]) == LOW)
        {
            spark_int(last_high_spark);
            last_high_spark = -1;
        }
    }
}
