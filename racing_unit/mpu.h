#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>
#include "globals.h"
#include "setup_defaults/settings.h"

// MPU6050_6Axis_MotionApps20.h
// 0x02,   0x16,   0x02,   0x00, 0x09                // D_0_22 inv_set_fifo_rate
// also set the DMP FIFO rate to 20Hz

//#define USE_SERIAL
#define INTERRUPT_PIN 2

MPU6050 mpu(0x68);

bool dmpReady;
uint16_t packetSize;
uint8_t fifoBuffer[64];

Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 accel;      // [x, y, z]            accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() 
{
    mpuInterrupt = true;
}

void mpu_setup()
{
    Wire.begin();
    TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
    mpu.initialize();

    #ifdef USE_SERIAL
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
    #endif

    uint8_t devStatus = mpu.dmpInitialize();

    mpu.setXGyroOffset(-150);
    mpu.setYGyroOffset(30);
    mpu.setZGyroOffset(22);
    mpu.setXAccelOffset(-2679);
    mpu.setYAccelOffset(-597);
    mpu.setZAccelOffset(1494);

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        mpu.setDMPEnabled(true);

        attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
        
        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        #ifdef USE_SERIAL
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
        #endif
    }
}

void mpu_loop()
{
    // if programming failed, don't try to do anything
    if (!dmpReady) return;

    if (!mpuInterrupt)
        return;
    mpuInterrupt = false;

    uint8_t mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    uint16_t fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) 
    {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        #ifdef USE_SERIAL
        Serial.println(F("FIFO overflow!"));
        #endif

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    }
    else if (mpuIntStatus & 0x02) 
    {
        // wait for correct available data length, should be a VERY short wait
        fifoCount = mpu.getFIFOCount();
        if (fifoCount < packetSize)
            return;

        while (fifoCount >= packetSize){
            // read a packet from FIFO
            mpu.getFIFOBytes(fifoBuffer, packetSize);
            
            // track FIFO count here in case there is > 1 packet available
            // (this lets us immediately read more without waiting for an interrupt)
            fifoCount -= packetSize;
        }

        // display Euler angles in degrees
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(globals.ypr, &q, &gravity);
        globals.ypr[0] = globals.ypr[0] * 180.f / M_PI - settings.gyro_calibration[0];
        globals.ypr[1] = globals.ypr[1] * 180.f / M_PI - settings.gyro_calibration[1];
        globals.ypr[2] = globals.ypr[2] * 180.f / M_PI - settings.gyro_calibration[2];

        // display real acceleration, adjusted to remove gravity
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetAccel(&accel, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetLinearAccel(&globals.accel_real, &accel, &gravity);

        #ifdef USE_SERIAL
        Serial.print("ypr\t");
        Serial.print(globals.ypr[0]);
        Serial.print("\t");
        Serial.print(globals.ypr[1]);
        Serial.print("\t");
        Serial.println(globals.ypr[2]);

        Serial.print("areal\t");
        Serial.print(globals.accel_real.x);
        Serial.print("\t");
        Serial.print(globals.accel_real.y);
        Serial.print("\t");
        Serial.println(globals.accel_real.z);
        #endif
    }
}
