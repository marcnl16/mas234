    //
    // Code based on 'MPU-6050 Short Example Sketch' By Arduino User JohnChi, 17.08.2014
    // Uses IMU Accelerometer (Z-direction) to detect a change of 1g, and toggles LED.
    // Tied to UiA MAS234 project
    // By Marcus and Henrik
    // 
    #include<Wire.h>
    const int MPU_addr=0x68;  // I2C address of the MPU-6050
    int16_t AcZraw; 
    float AcZ;
    const int led = 13;
    const int deltaTimeMicro = 50000; // us
    const int numberOfElements = 1000000/deltaTimeMicro; //, 1 000 000 us = 1 s;
    float gForceList[numberOfElements]; //an array of the last n g recordings, which is dependent on deltaTime and total time
    int elementNumber = numberOfElements - 1; //initial condition
    bool LEDon = false; //initial condition
    long tLastMicro = 0, tNowMicro; // variabels used to calculate time 


    void setup(){
      Wire.begin();
      Wire.beginTransmission(MPU_addr);
      Wire.write(0x6B);  // PWR_MGMT_1 register
      Wire.write(0);     // set to zero (wakes up the MPU-6050)
      Wire.endTransmission(true);
      Serial.begin(9600);
      pinMode(led, OUTPUT); 
    }


//The main program loop:
    void loop(){
      tNowMicro = micros();
 
      if (tNowMicro - tLastMicro > deltaTimeMicro)//checking to make shure the correct amount of time has passed between reedings 
      { 
          tLastMicro = tNowMicro;
          updateInput();
        
        if (elementNumber == 0) 
          {
            shiftArray();
            if (gForceComparing())
              {
                  toggleLED();   
              } 
          }
        else 
          {
            fillArray();// for the first run: Fills up the array with values
          }
      }
  }
    
// Calculates the difference between g1 and g2, and returns the absolute value of this:
    float deltaG(float g1, float g2) 
    {
      return abs(g1-g2);
    }

//Toggeling the LED on or off
    void toggleLED()
    {
      if (LEDon)
      {
        digitalWrite(led, LOW);
        Serial.println("toggle OFF"); //for debugging
        LEDon = false;
      }
      else
      {
        digitalWrite(led, HIGH);
        Serial.println("toggle ON"); //for debugging
        LEDon = true;
      }
    }


//the prosess of shifts each value one step to the right:  
    void shiftArray()
    {
      for (int n = numberOfElements; n > 0; n --) 
        {
            gForceList[n]=gForceList[n-1]; //shifts each value (except the last) one step to the right
        }
        gForceList[0] = AcZ; //sets the first element to our current value
    }
    
// the prosess of filling values in the emty Array:
    void fillArray()
    {     
        gForceList[elementNumber] = AcZ;
        elementNumber --;
    }

//uppdating the input from the sensor:
    void updateInput()
      {  
        Wire.beginTransmission(MPU_addr);
        Wire.write(0x3F);  // starting with register 0x3B (ACCEL_XOUT_H)
        Wire.endTransmission(false);
        Wire.requestFrom(MPU_addr,2,true);  // request a total of 2 registers
        AcZraw=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
        AcZ = AcZraw/16384.0f; // each G is 16384 raw units
      }

//compares the current g with the g captured one second ago:
    bool gForceComparing()
      {
        float g1 = gForceList[0], g2 = AcZ;  
        return (deltaG(g1,g2) > 1); 
      }

      
