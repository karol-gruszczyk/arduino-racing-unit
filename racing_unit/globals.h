#ifndef GLOBALS_H_
#define GLOBALS_H_

struct globals_t
{
    uint16_t current_rpm = 0;
    bool rpm_rising = true;

    bool launch_control_enabled = false;

    float ypr[3] = { 0.f, 0.f, 0.f };
    VectorInt16 accel_real;
} globals;

#endif /* GLOBALS_H_ */
