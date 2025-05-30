#include "my_MPU6050.h"
#include <Wire.h>
#include "GUI_Driver.h"

#include "sstream"
#include <iomanip>

#include "cJSON.h"

#define OUTPUT_READABLE_YAWPITCHROLL

MPU6050 mpu;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;        // [w, x, y, z]         quaternion container
VectorInt16 aa;      // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;  // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld; // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity; // [x, y, z]            gravity vector
float euler[3];      // [psi, theta, phi]    Euler angle container
float ypr[3];        // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = {'$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n'};

void MPU6050_setup(void)
{
  Wire.begin(36, 35, 400000);

  // initialize device
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();

  // verify connection
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  // mpu.setXGyroOffset(220);
  // mpu.setYGyroOffset(76);
  // mpu.setZGyroOffset(-85);
  // mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

  // make sure it worked (returns 0 if so)
  if (devStatus == 0)
  {
    // Calibration Time: generate offsets and calibrate our MPU6050
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    mpu.PrintActiveOffsets();
    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    // enable Arduino interrupt detection
    // Serial.print(F("Enabling interrupt detection (Arduino external interrupt "));
    // Serial.print(digitalPinToInterrupt(INTERRUPT_PIN));
    // Serial.println(F(")..."));
    // attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
    // mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  }
  else
  {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }
}

void MPU6050_getData()
{
  // if programming failed, don't try to do anything
  if (!dmpReady)
    return;
  // read a packet from FIFO
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer))
  { // Get the Latest packet
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
    // Serial.print("yaw\t");
    // Serial.print(ypr[0] * 180 / M_PI);
    // Serial.print("pitch\t");
    // Serial.print(ypr[1] * 180 / M_PI);
    // Serial.print("roll\t");
    // Serial.println(ypr[2] * 180 / M_PI);
  }
}

void MPU6050_GUILog(){
  std::stringstream ss;
  std::string MPU_Log;
  
  ss << "yaw:  " << std::fixed << std::setprecision(2) << ypr[0] * 180 / M_PI << " ";

  // GUI_logPrint((char*)MPU_Log.c_str());
  // ss.clear();

  ss << "pitch: " << std::fixed << std::setprecision(2) << ypr[1] * 180 / M_PI << " ";
  // MPU_Log = ss.str();
  // GUI_logPrint((char*)MPU_Log.c_str());
  // ss.clear();

  ss << "roll:  " << std::fixed << std::setprecision(2) << ypr[2] * 180 / M_PI << "\r";
  MPU_Log = ss.str();
}

void MPU6050_SendJSONPack(){
  cJSON *IMUDataJson = cJSON_CreateObject();
  cJSON_AddItemToObject(IMUDataJson, "cmd", cJSON_CreateNumber(10));
  cJSON_AddItemToObject(IMUDataJson, "yaw", cJSON_CreateNumber(ypr[0]));
  cJSON_AddItemToObject(IMUDataJson, "pitch", cJSON_CreateNumber(ypr[1]));
  cJSON_AddItemToObject(IMUDataJson, "roll", cJSON_CreateNumber(ypr[2]));

  char* JsonString = cJSON_Print(IMUDataJson);

  Serial1.print(JsonString);
  // TX_Characteristics.setValue(JsonString);
  // TX_Characteristics.notify();

  cJSON_Delete(IMUDataJson);
  free(JsonString);
}