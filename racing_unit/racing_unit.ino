#include "mpu.h"
#include "bluetooth.h"

//#define USE_SERIAL
//#define DISPLAY_RPM

#define CYLINDER_NUMBER 4
#define RPM_REFRESH_RATE 200
#define LAUNCH_CONTROL_ENABLE_INPUT_PIN 4
#define LAUNCH_CONTROL_ENABLING_TIME 3000
#define WHEELIE_CONTROL_AXIS 2

const uint8_t COIL_PIN_INT_INPUT[CYLINDER_NUMBER] = { 10, 16, 14, 15 };  // PB6, PB2, PB3, PB1
const uint8_t COIL_PIN_PCINT_INPUT[CYLINDER_NUMBER] = { PCINT6, PCINT2, PCINT3, PCINT1 };
const uint8_t COIL_PIN_SWITCH[CYLINDER_NUMBER] = { 9, 8, 6, 5 };

unsigned long rpm_measurement_start_time = 0;
volatile uint16_t coil_spark_counter = 0;
uint16_t last_rpm = 0;
volatile bool spark_killed = false;
unsigned long spark_restore_time = 0;

bool launch_control_enabling_started = false;
unsigned long launch_control_enabling_start_time = 0;
bool launch_control_started = false;
unsigned long launch_control_start_time = 0;

void coils_setup();
void quickshifter();
void launch_control();
void wheelie_control();
void measure_rpm();
void bluetooth_setup();
void bluetooth();

uint8_t get_kill_time(uint16_t rpm);
void kill_spark(uint16_t duration);
void restore_spark();


void setup()
{
    #ifdef USE_SERIAL
    Serial.begin(115200);
    while(!Serial);
    #endif

    load_settings();
    bluetooth_setup();
    mpu_setup();
    coils_setup();
    pinMode(LAUNCH_CONTROL_ENABLE_INPUT_PIN, INPUT);
    digitalWrite(LAUNCH_CONTROL_ENABLE_INPUT_PIN, LOW);
}

void loop()
{
    restore_spark();
    //mpu_loop();
    measure_rpm();
    quickshifter();
    launch_control();
    wheelie_control();
    bluetooth();
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

void quickshifter()
{
    if (!settings.quickshifter_enabled)
        return;
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
            globals.launch_control_enabled = true;

    }
    else
        launch_control_enabling_started = false;

    if (globals.launch_control_enabled)
    {
        if (!launch_control_started)
        {
            if (abs(globals.accel_real.y) >= LAUNCH_CONTROL_ACTIVATION_ACCELERATION)
            {
                launch_control_started = true;
                launch_control_start_time = millis();
            }
        }
        else
        {
            if (millis() - launch_control_start_time >= settings.launch_control_working_time)
            {
                launch_control_started = false;
                globals.launch_control_enabled = false;
            }
        }

        if (globals.current_rpm >= settings.launch_control_rpm)
            kill_spark(settings.launch_control_kill_time);
    }
}

void wheelie_control()
{
    if (!settings.wheelie_control_enabled)
        return;
    if (abs(globals.ypr[WHEELIE_CONTROL_AXIS]) >= settings.wheelie_control_max_angle)
        kill_spark(settings.wheelie_control_kill_time);
}

uint8_t get_kill_time(uint16_t rpm)
{
    uint8_t array_len = sizeof(settings.quickshifter_kill_time_at_rpm) / sizeof(settings.quickshifter_kill_time_at_rpm[0]);
    for (uint8_t i = array_len - 1; i >= 0; i--)
    {
        if (settings.quickshifter_kill_time_at_rpm[i][0] < rpm)
            return settings.quickshifter_kill_time_at_rpm[i][1];
    }
}

void kill_spark(uint16_t duration)
{
    spark_killed = true;
    spark_restore_time = millis() + duration;
}

void restore_spark()
{
    if (spark_killed && millis() >= spark_restore_time)
        spark_killed = false;
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
