#include "defaults.h"

#define CURRENT_BLUETOOTH_BAUD_RATE 9600  // after running this code once you need to change the baud rate to 115200


void setup_bluetooth();
void setup_eeprom();

void setup()
{
    Serial.begin(9600);
    Serial1.begin(CURRENT_BLUETOOTH_BAUD_RATE);
    while (!Serial or !Serial1);

    Serial.println(F("Press any character to start\n"));
    while(!Serial.available());

    setup_bluetooth();
    setup_eeprom();

    Serial.println(F("Done."));
}

void loop()
{

}

void bluetooth_response()
{
    delay(2000);  // waiting for the full message
    char c = 0;
    while(Serial1.available())
    {
        c = Serial1.read();
        Serial.print(c);
    }
    if (!c)
        Serial.println(F("No response. Did you set the correct baud rate?\n"));
    else
        Serial.println(F("\n"));
}

void setup_bluetooth()
{
    Serial.println(F("Setting Bluetooth device name to '" BLUETOOTH_NAME "'"));
    Serial1.print(F("AT+NAME" BLUETOOTH_NAME ""));
    bluetooth_response();
    
    Serial.println(F("Setting Bluetooth pin to '" BLUETOOTH_PIN "'"));
    Serial1.print(F("AT+PIN" BLUETOOTH_PIN));
    bluetooth_response();
    
    Serial.println(F("Setting Bluetooth baud rate to '" BLUETOOTH_BAUD_RATE_VERBOSE "'"));
    Serial1.print(F("AT+BAUD" BLUETOOTH_BAUD_RATE));
    bluetooth_response();
}

void setup_eeprom()
{
    
}
