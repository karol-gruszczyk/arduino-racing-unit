#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <avr/eeprom.h>
#include "defaults.h"


struct settings_t
{
    float gyro_calibration[3] = { 0, 0, 0 };
    
    uint16_t launch_control_rpm = DEFAULT_LAUNCH_CONTROL_RPM;
    uint8_t launch_control_kill_time = DEFAULT_LAUNCH_CONTROL_KILL_TIME;
    uint16_t launch_control_working_time = DEFAULT_LAUNCH_CONTROL_WORKING_TIME;
    
    bool wheelie_control_enabled = DEFAULT_WHEELIE_CONTROL_ENABLED;
    uint8_t wheelie_control_max_angle = DEFAULT_WHEELIE_CONTROL_MAX_ANGLE;
    uint8_t wheelie_control_kill_time = DEFAULT_WHEELIE_CONTROL_KILL_TIME;
    
    bool quick_shifter_enabled = DEFAULT_QUICK_SHIFTER_ENABLED;
    uint16_t quick_shifter_min_rpm = DEFAULT_QUICK_SHIFTER_MIN_RPM;
    uint16_t quick_shifter_max_rpm = DEFAULT_QUICK_SHIFTER_MAX_RPM;
    int16_t quick_shifter_sensitivity = DEFAULT_QUICK_SHIFTER_SENSITIVITY;
    uint16_t quick_shifter_kill_time_at_rpm[QUICK_SHIFTER_KILL_TIME_ARRAY_SIZE][2] = DEFAULT_QUICK_SHIFTER_KILL_TIME_AT_RPM;
} settings;


void load_settings()
{
    eeprom_read_block((void*)&settings, (void*)0, sizeof(settings));
}

void save_settings()
{
    eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));
}

#endif /* SETTINGS_H_ */
