#ifndef GLOBALS_H_
#define GLOBALS_H_

struct globals_t
{
    uint16_t current_rpm = 0;
    bool rpm_rising = true;

    bool launch_control_enabled = false;
    int quickshifter_sensor = 0;

    float ypr[3] = { 0.f, 0.f, 0.f };
    VectorInt16 accel_real;

    volatile bool spark_killed = false;
    unsigned long spark_restore_time = 0;
} globals;


void kill_spark(uint16_t duration)
{
    globals.spark_killed = true;
    globals.spark_restore_time = millis() + duration;
}

#endif /* GLOBALS_H_ */
