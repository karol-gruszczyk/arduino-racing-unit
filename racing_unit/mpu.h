#include <I2Cdev.h>
#include <MPU6050_6Axis_MotionApps20.h>

//#define USE_SERIAL
#define USE_INTERRUPT
#ifdef USE_INTERRUPT
#define INTERRUPT_PIN 0
#endif

MPU6050 mpu(0x68);

bool dmpReady;
uint16_t fifoCount;
uint16_t packetSize;
uint8_t mpuIntStatus;
uint8_t fifoBuffer[64];

Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 accel;      // [x, y, z]            accel sensor measurements
VectorInt16 accel_real; // [x, y, z]            gravity-free accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

#define RESULT_AVERAGE_NUMBER 20.f
uint8_t avg_counter = 0;
float ypr_avg[3];
VectorInt16 accel_real_avg;

bool blinkState = false;

#ifdef USE_INTERRUPT
volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() 
{
    mpuInterrupt = true;
}
#endif


void mpu_setup()
{
    Wire.begin();
    #ifdef USE_SERIAL
    Serial.begin(115200);
    #endif
    mpu.initialize();

    #ifdef USE_SERIAL
    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
    #endif

    uint8_t devStatus = mpu.dmpInitialize();

    mpu.setXGyroOffset(-150);
    mpu.setYGyroOffset(38);
    mpu.setZGyroOffset(15);
    mpu.setXAccelOffset(-2660);
    mpu.setYAccelOffset(-665);
    mpu.setZAccelOffset(1559);

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        #ifdef USE_SERIAL
        Serial.println(F("Enabling DMP..."));
        #endif
        mpu.setDMPEnabled(true);

        #ifdef USE_INTERRUPT
        #ifdef USE_SERIAL
        Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
        #endif
        attachInterrupt(INTERRUPT_PIN, dmpDataReady, RISING);
        #endif
        
        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        #ifdef USE_SERIAL
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        #endif
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

    #ifdef USE_INTERRUPT
    if (!mpuInterrupt && fifoCount < packetSize)
        return;
    mpuInterrupt = false;
    #endif

    mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

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
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        
        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;
        // display Euler angles in degrees
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        ypr_avg[0] += ypr[0] / RESULT_AVERAGE_NUMBER;
        ypr_avg[1] += ypr[1] / RESULT_AVERAGE_NUMBER;
        ypr_avg[2] += ypr[2] / RESULT_AVERAGE_NUMBER;


        // display real acceleration, adjusted to remove gravity
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetAccel(&accel, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetLinearAccel(&accel_real, &accel, &gravity);
        accel_real_avg.x += accel_real.x / RESULT_AVERAGE_NUMBER;
        accel_real_avg.y += accel_real.y / RESULT_AVERAGE_NUMBER;
        accel_real_avg.z += accel_real.z / RESULT_AVERAGE_NUMBER;


        if (++avg_counter >= RESULT_AVERAGE_NUMBER)
        {
            avg_counter = 0;
            #ifdef USE_SERIAL
            Serial.print("ypr\t");
            Serial.print(ypr_avg[0] * 180/M_PI);
            Serial.print("\t");
            Serial.print(ypr_avg[1] * 180/M_PI);
            Serial.print("\t");
            Serial.println(ypr_avg[2] * 180/M_PI);

            Serial.print("areal\t");
            Serial.print(accel_real_avg.x);
            Serial.print("\t");
            Serial.print(accel_real_avg.y);
            Serial.print("\t");
            Serial.println(accel_real_avg.z);
            #endif

            ypr_avg[0] = 0;
            ypr_avg[1] = 0;
            ypr_avg[2] = 0;

            accel_real_avg.x = 0;
            accel_real_avg.y = 0;
            accel_real_avg.z = 0;

            // blink LED to indicate activity
            blinkState = !blinkState;
        }
    }
}

