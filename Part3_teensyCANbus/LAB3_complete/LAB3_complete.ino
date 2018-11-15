
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// This source code is based on the Flexcan example code 'CANtest' by Collin Kidder, which again is Based on work by Pawelsky and teachop.
// Based on 'CANtest for Teensy 3.6 dual CAN bus'
// Modified for UiA MAS234 project
// By Simen and Marcus
//
// CANbus id list:
// 0x20 is used for sending Accelerometer values
// 0x21 is used for Toggle remote and local LED (Party mode!)
// 0x22 is used for controlling LED based on least significant bit of the first byte from its message
// 0x25 is used for recieving messages that will be printed to Serial
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||


#include <FlexCAN.h>
#include <Wire.h>
#include <Metro.h>

#ifndef __MK66FX1M0__
  #error "Teensy 3.6 with dual CAN bus is required to run this example"
#endif

const int MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t acXRaw,acYRaw,acZRaw; //Raw values from accelerometer

static uint8_t hex[17] = "0123456789abcdef"; //for displaying hex values in Serial
static CAN_message_t acXYZ; //all Accelerometer values
static CAN_message_t inMsg; //incomming message
static CAN_message_t party; //party message

bool readAccelerometerValues  = true  ; //Determines whether to request Accelerometer Values or not
bool sendAccValuesToBus       = true  ; //Determines whether to send Accelerometer Values to bus or not. WARNING: Contains a timer function.
bool sendAccValuesToSerial    = true  ; //Determines whether to send Accelerometer Values to Serial or not
bool enableRemotePartyMode    = true  ; //Determines whether to send LED control signals to other teensies/project groups
bool LEDOn = true; //The LED starts ON

Metro primaryTimer = Metro(1000); // [ms], 1000ms = 1Hz, used for syncing & sending IMU data
Metro partyTimer   = Metro(404);  // [ms], used for sending party LED CANbus control messages

//non-lib functions
int accScale(); //scales Raw Acceleration data to 8 bit (1 byte)
static void hexDump(); 
void toggleLED(); 
void checkBit();

//  SETUP  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
void setup(void)
{
  delay(1000);
  Serial.println(F("Initializing..."));

  CAN_filter_t defaultMask; //defaultMask allowes all traffic through
  Can0.begin(250000, defaultMask, true, true);  //Uses a baud rate of 250000, uses defaultMask as mask and selects alternative CAN HIGH/LOW pins
//  Can1.begin(); //Not used, but can be.

  //Set CAN-pins + LED as output
  pinMode(28, OUTPUT); //CAN0 Mode pin
  pinMode(35, OUTPUT); //CAN1 Mode pin
  pinMode(13, OUTPUT); //LED Pin
  
  //Set CAN-pins to LOW, to enable high-speed-mode
  digitalWrite(28, LOW);  //CAN0 set to High Speed Mode
  digitalWrite(35, LOW);  //CAN1 set to High Speed Mode
  digitalWrite(13, HIGH); //The LED starts on

  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(9600);

  //CANbus messages constant definitions
  
  party.ext = 0;
  party.id = 0x21;
  party.len = 0; //any message will do, so the lenght is irrelevant

  acXYZ.ext = 0;
  acXYZ.id = 0x20;
  acXYZ.len = 4;
    
  Serial.println(F("Initializing complete"));
}
  
//  MAIN  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
void loop(void)
{
  while (Can0.available()) 
  {
    Can0.read(inMsg); 
    if (inMsg.id == 0x21 && enableRemotePartyMode) //Recieving any message with id 0x21 will toggle LED, if remotePartyMode is enabled
      {
        toggleLED();
      }
    else if (inMsg.id == 0x22) 
      {
        checkBit();
      }
    else if (inMsg.id == 0x25) //Messages sent with id 0x25 will be printed to serial
     {
       Serial.print("CAN bus 0: "); hexDump(inMsg.len, inMsg.buf);
       Serial.print("id: "); Serial.println(inMsg.id); //id always 0x25[h], displayed as 33[10]
       Can0.write(inMsg);
     }
  }
  
  if(enableRemotePartyMode && partyTimer.check() == 1)
  {
    //Sends party signal to bus literally 'if the time is right'
    Can0.write(party);
  }

  if (readAccelerometerValues)
    {
    //Read Accelerometer Values and store them in CANbus message 'acXYZ'
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr,6,true);  // request a total of 6 registers
    
    acXRaw=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
    acYRaw=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    acZRaw=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

    int acX = accScale(acXRaw);
    int acY = accScale(acYRaw);
    int acZ = accScale(acZRaw);

    acXYZ.buf[0] = acX;
    acXYZ.buf[1] = acY;
    acXYZ.buf[2] = acZ;
    acXYZ.buf[3] = 0; //This byte can potentially be used to send error codes/messages, for example IMU status
    }
  
  if (sendAccValuesToBus && (primaryTimer.check() == 1))
    {
      //Sends values to bus literally 'if the time is right' & it's enabled.
        Can0.write(acXYZ);
    }

  if (sendAccValuesToSerial)
    {
      // send values to Serial (no delay)
      int acX = acXYZ.buf[0];
      int acY = acXYZ.buf[1];
      int acZ = acXYZ.buf[2];
      Serial.print("acX: "); Serial.println(acX);
      Serial.print("acY: "); Serial.println(acY);
      Serial.print("acZ: "); Serial.println(acZ);
    }
}



//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// Non-lib functions
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

//Toggles LED (duh)
void toggleLED()
{
        if (LEDOn == false)  //or just if(!LEDOn)
        {
          digitalWrite(13, HIGH);
          LEDOn = true;
//          Serial.println("LED ON"); //For debugging
        }
        else if (LEDOn == true)  //or just if(LEDOn)
        {
          digitalWrite(13, LOW);
          LEDOn = false;
//          Serial.println("LED OFF"); //For debugging
        }
}
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

//Checks the least significant bit of the first byte of the message and controls the LED based on the result
void checkBit() 
{
        if ((inMsg.buf[inMsg.len-1] % 2) == 0) //Last byte even? If yes: least significant bit is 0 (low)
        {
          digitalWrite(13, LOW);  
          LEDOn = false;
        }
        else //the byte is always odd or even
        {
          digitalWrite(13, HIGH);
          LEDOn = true;
        } 
}
//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

//Re-scales values from raw -2*16384 - 2*16384 range values to 0 - 255 bit range value
int accScale(int rawValue) 
{
  int maxBit = 255;         //maximum bit value (8bit -> 255)
  int inputWidth = 4*16384; //total input with, 16384 pr g, scale from -2 to 2 g = 4g 
  int zeroGvalue = 127;     // 0 g gives this value
  int Acc = rawValue*maxBit/inputWidth+zeroGvalue; 
  return Acc; 
}

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

static void hexDump(uint8_t dumpLen, uint8_t *bytePtr)
{
  uint8_t working;
  while( dumpLen-- ) {
    working = *bytePtr++;
    Serial.write( hex[ working>>4 ] );
    Serial.write( hex[ working&15 ] );
  }
  Serial.write('\r');
  Serial.write('\n');
}

//||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
