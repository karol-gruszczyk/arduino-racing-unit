#include "setup_defaults/settings.h"
#include "globals.h"
#include "coils.h"

#define COMMAND_EOF_CHAR '\n'
#define COMMAND_ARGS_BEGINING_CHAR ":"
#define COMMAND_ARGS_DELIMITER ','

#define COMMAND_BUFFER_MAX 64
#define COMMAND_NAME_MAX 8
char command_buffer[COMMAND_BUFFER_MAX];
uint8_t command_buffer_counter = 0;


boolean isNumber(char* str){
    for(uint8_t i = 0; i < strlen(str); i++)
        if(!isDigit(str[i])) 
            return false;
    return true;
}

void bluetooth_setup()
{
    Serial.begin(115200);
    memset(command_buffer, '\0', COMMAND_BUFFER_MAX);
}

void bt_get(char* get_what);
void bt_set(char* set_what, char* args);

void bluetooth()
{
    bool received_full_cmd = false;
    while (Serial.available())
    {
        char c = Serial.read();
        if (c == COMMAND_EOF_CHAR)
        {
            received_full_cmd = true;
            break;
        }
        command_buffer[command_buffer_counter++] = c;
    }

    if (!received_full_cmd)
        return;

    if (!strcmp(command_buffer, "SAVE"))
    {
        save_settings();
        Serial.println(F("OK"));
    }
    else if (!strcmp(command_buffer, "CAL_GYRO"))
    {
        settings.gyro_calibration[0] += globals.ypr[0];
        settings.gyro_calibration[1] += globals.ypr[1];
        settings.gyro_calibration[2] += globals.ypr[2];
        Serial.println(F("OK"));
    }
    else if (command_buffer[0] == 'G')
    {
        char get_what[COMMAND_NAME_MAX];
        memcpy(get_what, &command_buffer[1], COMMAND_NAME_MAX);
        bt_get(get_what);
    }
    else if(command_buffer[0] == 'S')
    {
        uint8_t pars_start = strcspn(command_buffer, COMMAND_ARGS_BEGINING_CHAR);
        char set_what[COMMAND_NAME_MAX];
        memcpy(set_what, &command_buffer[1], pars_start - 1);
        set_what[pars_start - 1] = '\0';
        char args[COMMAND_BUFFER_MAX];
        memcpy(args, &command_buffer[pars_start + 1], COMMAND_BUFFER_MAX - pars_start - 1);
        bt_set(set_what, args);
    }
    else
    {
        Serial.println(F("ERROR: Unknown command"));
    }
    memset(command_buffer, '\0', COMMAND_BUFFER_MAX);
    command_buffer_counter = 0;
}

void bt_get(char* get_what)
{
    if (!strcmp(get_what, "RPM"))
        Serial.println(globals.current_rpm);
    else if (!strcmp(get_what, "YPR"))
    {
        Serial.print((int)globals.ypr[0]);
        Serial.print(",");
        Serial.print((int)globals.ypr[1]);
        Serial.print(",");
        Serial.println((int)globals.ypr[2]);
    }
    else if (get_what[0] == 'L')  // launch control
    {
        if (!strcmp(get_what, "LC"))
            Serial.println(globals.launch_control_enabled ? F("ON") : F("OFF"));
        else if (!strcmp(get_what, "LC_RPM"))
            Serial.println(settings.launch_control_rpm);
        else if (!strcmp(get_what, "LC_KT"))
            Serial.println(settings.launch_control_kill_time);
        else if (!strcmp(get_what, "LC_WT"))
            Serial.println(settings.launch_control_working_time);
    }
    else if (get_what[0] == 'Q')  // quickshifter
    {
        if (!strcmp(get_what, "QS"))
            Serial.println(settings.quick_shifter_enabled ? F("ON") : F("OFF"));
        else if (!strcmp(get_what, "QS_MIN"))
            Serial.println(settings.quick_shifter_min_rpm);
        else if (!strcmp(get_what, "QS_MAX"))
            Serial.println(settings.quick_shifter_max_rpm);
        else if (!strcmp(get_what, "QS_SENS"))
            Serial.println(settings.quick_shifter_sensitivity);
        else if (!strcmp(get_what, "QS_CSENS"))
            Serial.println(globals.quick_shifter_sensor);
        else if (!strcmp(get_what, "QS_KT"))
        {
            for (uint8_t i = 0; i < QUICK_SHIFTER_KILL_TIME_ARRAY_SIZE; i++)
            {
                Serial.print(settings.quick_shifter_kill_time_at_rpm[i][1]);
                if (i == QUICK_SHIFTER_KILL_TIME_ARRAY_SIZE - 1)
                    Serial.println();
                else
                    Serial.print(COMMAND_ARGS_DELIMITER);
            }
        }
    }
    else if (get_what[0] == 'W')  // wheelie control
    {
        if (!strcmp(get_what, "WC"))
            Serial.println(settings.wheelie_control_enabled ? F("ON") : F("OFF"));
        else if (!strcmp(get_what, "WC_ANGLE"))
            Serial.println(settings.wheelie_control_max_angle);
        else if (!strcmp(get_what, "WC_KT"))
            Serial.println(settings.wheelie_control_kill_time);
    }
}

void bt_set(char* set_what, char* args)
{
    bool error = true;

    if (!strcmp(set_what, "KS"))
    {
        if (isNumber(args))
        {
            kill_spark(atoi(args));
            error = false;
        }
    }
    else if (set_what[0] == 'Q')  // quickshifter
    {
        if (!strcmp(set_what, "QS"))
        {
            if (!strcmp(args, "ON"))
            {
                settings.quick_shifter_enabled = true;
                error = false;
            }
            else if (!strcmp(args, "OFF"))
            {
                settings.quick_shifter_enabled = false;
                error = false;
            }
        }
        else if (!strcmp(set_what, "QS_MIN"))
        {
            if (isNumber(args))
            {
                settings.quick_shifter_min_rpm = atoi(args);
                error = false;
            }
        }
        else if (!strcmp(set_what, "QS_MAX"))
        {
            if (isNumber(args))
            {
                settings.quick_shifter_max_rpm = atoi(args);
                error = false;
            }
        }
        else if (!strcmp(set_what, "QS_SENS"))
        {
            if (isNumber(args))
            {
                settings.quick_shifter_sensitivity = atoi(args);
                error = false;
            }
        }
        else if (!strcmp(set_what, "QS_KT"))
        {
            char kill_time_str[4];
            uint8_t kill_time_str_counter = 0;
            memset(kill_time_str, '\0', 4);
            uint8_t kill_time_counter = 0;
            for (uint8_t i = 0; i < strlen(args) && kill_time_counter < QUICK_SHIFTER_KILL_TIME_ARRAY_SIZE; i++)
            {
                if (args[i] != COMMAND_ARGS_DELIMITER)
                {
                    kill_time_str[kill_time_str_counter++] = args[i];
                    if (i != strlen(args) - 1)
                        continue;
                }
                if (!isNumber(kill_time_str))
                    break;
                settings.quick_shifter_kill_time_at_rpm[kill_time_counter++][1] = atoi(kill_time_str);
                memset(kill_time_str, '\0', 4);
                kill_time_str_counter = 0;
                if (i == strlen(args) - 1)
                    error = false;
            }
        }
    }
    else if (set_what[0] == 'W')  // wheelie control
    {
        if (!strcmp(set_what, "WC"))
        {
            if (!strcmp(args, "ON"))
            {
                settings.wheelie_control_enabled = true;
                error = false;
            }
            else if (!strcmp(args, "OFF"))
            {
                settings.wheelie_control_enabled = false;
                error = false;
            }
        }
        else if (!strcmp(set_what, "WC_ANGLE"))
        {
            if (isNumber(args))
            {
                settings.wheelie_control_max_angle = atoi(args);
                error = false;
            }
        }
        else if (!strcmp(set_what, "WC_KT"))
        {
            if (isNumber(args))
            {
                settings.wheelie_control_kill_time = atoi(args);
                error = false;
            }
        }
    }
    else if (set_what[0] == 'L')  // launch control
    {
        if (!strcmp(set_what, "LC"))
        {
            if (!strcmp(args, "ON"))
            {
                globals.launch_control_enabled = true;
                error = false;
            }
            else if (!strcmp(args, "OFF"))
            {
                globals.launch_control_enabled = false;
                error = false;
            }
        }
        else if (!strcmp(set_what, "LC_RPM"))
        {
            if (isNumber(args))
            {
                settings.launch_control_rpm = atoi(args);
                error = false;
            }
        }
        else if (!strcmp(set_what, "LC_KT"))
        {
            if (isNumber(args))
            {
                settings.launch_control_kill_time = atoi(args);
                error = false;
            }
        }
        else if (!strcmp(set_what, "LC_WT"))
        {
            if (isNumber(args))
            {
                settings.launch_control_working_time = atoi(args);
                error = false;
            }
        }
    }
    
    if (error)
        Serial.println(F("ERROR"));
    else
        Serial.println(F("OK"));
}
