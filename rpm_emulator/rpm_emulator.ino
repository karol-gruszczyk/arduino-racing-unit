#define CYLINDER_NUMBER 4
const uint8_t coil_pin[CYLINDER_NUMBER] = { 10, 16, 14, 15 };

#define HIGH_LENGTH 15
#define COIL_INTERVAL 15
// rpm = 30000 / (HIGH_LENGTH + COIL_INTERVAL)


void setup()
{
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
    {
        pinMode(coil_pin[i], OUTPUT);
        digitalWrite(coil_pin[i], LOW);
    }
}

void loop()
{
    for (uint8_t i = 0; i < CYLINDER_NUMBER; i++)
    {
        digitalWrite(coil_pin[i], HIGH);
        delay(HIGH_LENGTH);
        digitalWrite(coil_pin[i], LOW);
        delay(COIL_INTERVAL);
    }
}
