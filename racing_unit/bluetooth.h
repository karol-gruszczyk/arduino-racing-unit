#include "setup_defaults/settings.h"
#include "globals.h"

//#define USE_SERIAL

#define COMMAND_EOF_CHAR '\n'
#define COMMAND_ARGS_BEGINING_CHAR ':'
#define COMMAND_ARGS_DELIMITER ','


String command = "";

boolean isNumber(String str){
    for(byte i = 0; i < str.length(); i++)
        if(!isDigit(str[i])) 
            return false;
    return true;
} 

void bluetooth_setup()
{
    Serial1.begin(115200);
}

void bt_get(String get_what);
void bt_set(String set_what, String args);

void bluetooth()
{
    if (!Serial1 or !Serial1.available())
        return;

    bool received_full_cmd = false;
    while (Serial1.available())
    {
        char c = Serial1.read();
        if (c == COMMAND_EOF_CHAR)
        {
            received_full_cmd = true;
            break;
        }
        command += c;
    }

    if (!received_full_cmd)
        return;
    else
        received_full_cmd = true;

    if (command == "SAVE")
    {
        save_settings();
        Serial1.println(F("OK"));
    }
    else if (command == "CAL_GYRO")
    {
        settings.gyro_calibration[0] += globals.ypr[0];
        settings.gyro_calibration[1] += globals.ypr[1];
        settings.gyro_calibration[2] += globals.ypr[2];
        Serial1.println(F("OK"));
    }
    else if (command[0] == 'G')
    {
        String get_what = command.substring(1);
        bt_get(get_what);
    }
    else if(command[0] == 'S')
    {
        uint8_t pars_start = command.indexOf(COMMAND_ARGS_BEGINING_CHAR);
        String set_what = command.substring(1, pars_start);
        String args = command.substring(pars_start + 1);
        bt_set(set_what, args);
    }
    else
    {
        Serial1.println(F("ERROR: Unknown command"));
    }
    command = "";
}

void bt_get(String get_what)
{
    if (get_what == "RPM")
        Serial1.println(globals.current_rpm);
    else if (get_what == "YPR")
    {
        Serial1.print((int)globals.ypr[0]);
        Serial1.print(",");
        Serial1.print((int)globals.ypr[1]);
        Serial1.print(",");
        Serial1.println((int)globals.ypr[2]);
    }
    
    // quickshifter
    else if (get_what == "QS")
        Serial1.println(settings.quick_shifter_enabled ? "ON" : "OFF");
    else if (get_what == "QS_MIN")
        Serial1.println(settings.quick_shifter_min_rpm);
    else if (get_what == "QS_MAX")
        Serial1.println(settings.quick_shifter_max_rpm);
    else if (get_what == "QS_SENS")
        Serial1.println(settings.quick_shifter_sensitivity);
    else if (get_what == "QS_CSENS")
        Serial1.println(globals.quick_shifter_sensor);
    else if (get_what == "QS_KT")
    {
        for (uint8_t i = 0; i < QUICK_SHIFTER_KILL_TIME_ARRAY_SIZE; i++)
        {
            Serial1.print(settings.quick_shifter_kill_time_at_rpm[i][1]);
            if (i == QUICK_SHIFTER_KILL_TIME_ARRAY_SIZE - 1)
                Serial1.println();
            else
                Serial1.print(COMMAND_ARGS_DELIMITER);
        }
    }
    
    // wheelie control
    else if (get_what == "WC")
        Serial1.println(settings.wheelie_control_enabled ? "ON" : "OFF");
    else if (get_what == "WC_ANGLE")
        Serial1.println(settings.wheelie_control_max_angle);
    else if (get_what == "WC_KT")
        Serial1.println(settings.wheelie_control_kill_time);
    
    // launch control
    else if (get_what == "LC")
        Serial1.println(globals.launch_control_enabled ? "ON" : "OFF");
    else if (get_what == "LC_RPM")
        Serial1.println(settings.launch_control_rpm);
    else if (get_what == "LC_KT")
        Serial1.println(settings.launch_control_kill_time);
    else if (get_what == "LC_WT")
        Serial1.println(settings.launch_control_working_time);
}

void bt_set(String set_what, String args)
{
    bool error = true;

    if (set_what == "KS")
    {
        if (isNumber(args))
        {
            kill_spark(args.toInt());
            error = false;
        }
    }

    // quickshifter
    else if (set_what == "QS")
    {
        if (args == "ON")
        {
            settings.quick_shifter_enabled = true;
            error = false;
        }
        else if (args == "OFF")
        {
            settings.quick_shifter_enabled = false;
            error = false;
        }
    }
    else if (set_what == "QS_MIN")
    {
        if (isNumber(args))
        {
            settings.quick_shifter_min_rpm = args.toInt();
            error = false;
        }
    }
    else if (set_what == "QS_MAX")
    {
        if (isNumber(args))
        {
            settings.quick_shifter_max_rpm = args.toInt();
            error = false;
        }
    }
    else if (set_what == "QS_SENS")
    {
        if (isNumber(args))
        {
            settings.quick_shifter_sensitivity = args.toInt();
            error = false;
        }
    }
    else if (set_what == "QS_KT")
    {
        String kill_time_str = "";
        uint8_t kill_time_counter = 0;
        for (uint8_t i = 0; i < args.length(), kill_time_counter < QUICK_SHIFTER_KILL_TIME_ARRAY_SIZE; i++)
        {
            if (args[i] != COMMAND_ARGS_DELIMITER)
            {
                kill_time_str += args[i];
                if (i != args.length() - 1)
                    continue;
            }
            if (!isNumber(kill_time_str))
                break;
            settings.quick_shifter_kill_time_at_rpm[kill_time_counter++][1] = kill_time_str.toInt();
            kill_time_str = "";
            if (i == args.length() - 1)
                error = false;
        }
    }

    // wheelie control
    else if (set_what == "WC")
    {
        if (args == "ON")
        {
            settings.wheelie_control_enabled = true;
            error = false;
        }
        else if (args == "OFF")
        {
            settings.wheelie_control_enabled = false;
            error = false;
        }
    }
    else if (set_what == "WC_ANGLE")
    {
        if (isNumber(args))
        {
            settings.wheelie_control_max_angle = args.toInt();
            error = false;
        }
    }
    else if (set_what == "WC_KT")
    {
        if (isNumber(args))
        {
            settings.wheelie_control_kill_time = args.toInt();
            error = false;
        }
    }

    // launch control
    else if (set_what == "LC")
    {
        if (args == "ON")
        {
            globals.launch_control_enabled = true;
            error = false;
        }
        else if (args == "OFF")
        {
            globals.launch_control_enabled = false;
            error = false;
        }
    }
    else if (set_what == "LC_RPM")
    {
        if (isNumber(args))
        {
            settings.launch_control_rpm = args.toInt();
            error = false;
        }
    }
    else if (set_what == "LC_KT")
    {
        if (isNumber(args))
        {
            settings.launch_control_kill_time = args.toInt();
            error = false;
        }
    }
    else if (set_what == "LC_WT")
    {
        if (isNumber(args))
        {
            settings.launch_control_working_time = args.toInt();
            error = false;
        }
    }
    
    if (!error)
        Serial1.println(F("OK"));
    else
        Serial1.println(F("ERROR"));
}
