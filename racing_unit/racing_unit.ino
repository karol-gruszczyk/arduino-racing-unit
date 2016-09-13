#include "mpu.h"
#include "bluetooth.h"

#define LAUNCH_CONTROL_ENABLING_TIME 3000

#define WHEELIE_CONTROL_AXIS 2

#define QUICK_SHIFTER_SENSOR_ANALOG_PIN A0
#define QUICK_SHIFTER_SENSOR_CHECK_RESISNTANCE 327
#define QUICK_SHIFTER_SENSOR_INITIAL_RESISTANCE 350

bool launch_control_started = false;
unsigned long launch_control_start_time = 0;

bool led = false;
uint16_t led_counter = 0;
#define LED_COUNTER_MAX 250


void setup()
{
    #ifdef USE_SERIAL
    Serial.begin(115200);
    while(!Serial);
    #endif

    coils_setup();
    load_settings();
    mpu_setup();
    #ifndef USE_SERIAL
    bluetooth_setup();
    #endif

    pinMode(13, OUTPUT);
}

void loop()
{
    restore_spark();
    mpu_loop();
    measure_rpm();
    quick_shifter();
    launch_control();
    wheelie_control();
    #ifndef USE_SERIAL
    bluetooth();
    #endif
    if (++led_counter >= LED_COUNTER_MAX)
    {
        led_counter = 0;
        led = !led;
        digitalWrite(13, led ? HIGH : LOW);
    }
}

uint8_t get_kill_time(uint16_t rpm)
{
    for (int8_t i = QUICK_SHIFTER_KILL_TIME_ARRAY_SIZE - 1; i >= 0; i--)
    {
        if (settings.quick_shifter_kill_time_at_rpm[i][0] < rpm)
            return settings.quick_shifter_kill_time_at_rpm[i][1];
    }
    return 0;
}

void quick_shifter_sensor()
{
    float vin = 5.f;
    int raw = analogRead(QUICK_SHIFTER_SENSOR_ANALOG_PIN);
    if (raw > 0)
    {
        float vout = (raw * vin) / 1024.f;
        float sensor_resistance = (float)QUICK_SHIFTER_SENSOR_CHECK_RESISNTANCE * (vin / vout - 1.f);
        globals.quick_shifter_sensor = sensor_resistance - QUICK_SHIFTER_SENSOR_INITIAL_RESISTANCE;
    }
}

void quick_shifter()
{
    quick_shifter_sensor();

    if (!settings.quick_shifter_enabled)
        return;

    if (globals.current_rpm >= settings.quick_shifter_min_rpm && globals.current_rpm <= settings.quick_shifter_max_rpm)
    {
        if (globals.quick_shifter_sensor > settings.quick_shifter_sensitivity)
            kill_spark(get_kill_time(globals.current_rpm));
    }
}

void launch_control()
{
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
