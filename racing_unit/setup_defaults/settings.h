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
    
    bool quickshifter_enabled = DEFAULT_QUICKSHIFTER_ENABLED;
    uint16_t quickshifter_min_rpm = DEFAULT_QUICKSHIFTER_MIN_RPM;
    uint16_t quickshifter_max_rpm = DEFAULT_QUICKSHIFTER_MAX_RPM;
    uint16_t quickshifter_sensitivity = DEFAULT_QUICKSHIFTER_SENSITIVITY;
    uint16_t quickshifter_kill_time_at_rpm[QUICKSHIFTER_KILL_TIME_SIZE][2] = DEFAULT_QUICKSHIFTER_KILL_TIME_AT_RPM;
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
