#include "mpu.h"

#define USE_SERIAL

#define CYLINDER_NUMBER 4

#define LAUNCH_CONTROL_RPM 6000
#define LAUNCH_CONTROL_KILL_TIME 100
#define LAUNCH_CONTROL_WORK_TIME 5000
#define LAUNCH_CONTROL_ACCELEROMETER_MIN_ACCELERATION 1000
#define LAUNCH_CONTROL_INPUT_PIN 0
#define LAUNCH_CONTROL_ENABLING_TIME 3000

#define WHEELIE_CONTROL_MAX_ANGLE 10
#define WHEELIE_CONTROL_KILL_TIME_MODIFIER 100
#define WHEELIE_CONTROL_AXIS 2

#define QUICKSHIFTER_MIN_RPM 2000
#define QUICKSHIFTER_MAX_RPM 14000
const int QUICKSHIFTER_KILL_TIME_AT_RPM[][2] = {
    {2000, 80},
    {5500, 75},
    {8500, 70},
    {11500, 65},
};

const int CYLINDER_FIRING_SEQUENCE[CYLINDER_NUMBER] = { 0, 2, 1, 3 };
#define COIL_1_INPUT_INT_PIN 2
#define COIL_OTHER_INPUT_INT_PIN 3
const int COIL_PIN_SWITCH[CYLINDER_NUMBER] = { 3, 4, 5, 6 };

#define RPM_REFRESH_RATE 200
unsigned long rpm_measurement_start_time = 0;
int coil_spark_counter = 0;
int last_rpm = 0;
int current_rpm = 0;
bool rpm_rising = true;
bool spark_killed = false;
bool spark_on_coil_1_killed = false;
int current_spark_counter = 0;

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

int get_kill_time(int rpm);
void kill_spark(int duration);
void spark_1_int();
void spark_other_int();

void setup()
{
    #ifdef USE_SERIAL
    Serial.begin(115200);
    #endif
    
    mpu_setup();
    coils_setup();
    pinMode(LAUNCH_CONTROL_INPUT_PIN, INPUT);
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
    for (int i = 0; i < CYLINDER_NUMBER; i++)
    {
        attachInterrupt(digitalPinToInterrupt(COIL_1_INPUT_INT_PIN), spark_1_int, FALLING);
        attachInterrupt(digitalPinToInterrupt(COIL_OTHER_INPUT_INT_PIN), spark_other_int, FALLING);
        pinMode(COIL_PIN_SWITCH[i], OUTPUT);
    }
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
    if (digitalRead(LAUNCH_CONTROL_INPUT_PIN) == HIGH)
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
        kill_spark((ypr_avg[WHEELIE_CONTROL_AXIS] - WHEELIE_CONTROL_MAX_ANGLE) * WHEELIE_CONTROL_KILL_TIME_MODIFIER);
}

int get_kill_time(int rpm)
{
    int array_len = sizeof(QUICKSHIFTER_KILL_TIME_AT_RPM) / sizeof(QUICKSHIFTER_KILL_TIME_AT_RPM[0]);
    for (int i = array_len - 1; i >= 0; i--)
    {
        if (QUICKSHIFTER_KILL_TIME_AT_RPM[i][0] < rpm)
            return QUICKSHIFTER_KILL_TIME_AT_RPM[i][1];
    }
}

void kill_spark(int duration)
{
    spark_killed = true;
    delay(duration);
    spark_killed = spark_on_coil_1_killed = false;

    for (int i = 0; i < CYLINDER_NUMBER; i++)
        digitalWrite(COIL_PIN_SWITCH[i], LOW);
}

void spark_1_int()
{
    if (spark_killed)
    {
        current_spark_counter = 0;
        digitalWrite(COIL_PIN_SWITCH[current_spark_counter++], HIGH);
        spark_on_coil_1_killed = true;
    }
    coil_spark_counter++;
}

void spark_other_int()
{
    if (spark_on_coil_1_killed)
    {
        digitalWrite(COIL_PIN_SWITCH[CYLINDER_FIRING_SEQUENCE[current_spark_counter]], HIGH);
        if (++current_spark_counter >= CYLINDER_NUMBER)
            current_spark_counter = 0;
    }
    coil_spark_counter++;
}

