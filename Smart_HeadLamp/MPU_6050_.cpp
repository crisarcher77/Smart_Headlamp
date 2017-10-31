#include "MPU_6050_.h"
#include <EEPROM.h>
//#include <Wire.h>

#include <HardWire.h>

#define MPU_6050_EEPROM_ADRES 0x0

extern HardWire _wire;

void MPU6050::Initialization()
{
	_wire.begin();
	_wire.beginTransmission(MPU_6050);
	_wire.write(0x6B);  // PWR_MGMT_1 register
	_wire.write(0x00);     // set to zero (wakes up the MPU-6050)
  _wire.endTransmission();
    
  _wire.beginTransmission(MPU_6050);
  _wire.write(0x1B);  // PWR_MGMT_1 register
  _wire.write(0x2<<3);     // set gyro sensitivity 0 - 250, 1 - 500 , 2 - 1000 , 3 - 2000 deg/s
	_wire.endTransmission();

    EEPROM.read(MPU_6050_EEPROM_ADRES,  (uint16*)&Gyro_Calibration[0]);
    EEPROM.read(MPU_6050_EEPROM_ADRES+1,(uint16*)&Gyro_Calibration[1]);
    EEPROM.read(MPU_6050_EEPROM_ADRES+2,(uint16*)&Gyro_Calibration[2]);
    EEPROM.read(MPU_6050_EEPROM_ADRES+3,(uint16*)&GravityLen);
  
/*	Gyro_Calibration[0]=0;
	Gyro_Calibration[1]=0;
	Gyro_Calibration[2]=0;*/
	
	// read Calibration Data from EEPROM;
}


void MPU6050::DataRead()
{
	_wire.beginTransmission(MPU_6050);
	_wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
	_wire.endTransmission(); //?? 
	_wire.requestFrom(MPU_6050,14);  // request a total of 14 registers
	Accel[0]=_wire.read()<<8|_wire.read();  
	Accel[1]=_wire.read()<<8|_wire.read();  
	Accel[2]=_wire.read()<<8|_wire.read();  
	Temperature=_wire.read()<<8|_wire.read();  
	Gyro[0]=_wire.read()<<8|_wire.read();  
	Gyro[1]=_wire.read()<<8|_wire.read();  
	Gyro[2]=_wire.read()<<8|_wire.read();  
	  
	
}

void MPU6050::DataReadCal()
{
  DataRead();

  Gyro[0]-=Gyro_Calibration[0];
  Gyro[1]-=Gyro_Calibration[1];
  Gyro[2]-=Gyro_Calibration[2];
}


void MPU6050::DataPrint()
{
	Serial.print(Accel[0]); Serial.print(" ");
	Serial.print(Accel[1]); Serial.print(" ");
	Serial.print(Accel[2]); Serial.print(" ");

	Serial.print(Gyro[0]); Serial.print(" ");
	Serial.print(Gyro[1]); Serial.print(" ");
	Serial.print(Gyro[2]); Serial.print(" ");

	//Serial.print(Temperature); Serial.print(" ");
	//Serial.println();
}

void MPU6050::Calibration()
{

long GyroPreCalibr[3];
long AccelPreCalibr[3];
long Counter=0;

long lTime = millis();

GyroPreCalibr[0]=0;
GyroPreCalibr[1]=0;
GyroPreCalibr[2]=0;
AccelPreCalibr[0]=0;
AccelPreCalibr[1]=0;
AccelPreCalibr[2]=0;

while ((millis()-lTime)<1000)
{
    static bool bSwitch=true;
    if (bSwitch) { bSwitch=false; digitalWrite(PC13, HIGH); }
    else { bSwitch=true; digitalWrite(PC13, LOW); }
    delay(10);
  
    DataRead();
    
    
    GyroPreCalibr[0]+=Gyro[0];
    GyroPreCalibr[1]+=Gyro[1];
    GyroPreCalibr[2]+=Gyro[2];

    AccelPreCalibr[0]+=Accel[0];
    AccelPreCalibr[1]+=Accel[1];
    AccelPreCalibr[2]+=Accel[2];

    Counter++;
}
    digitalWrite(PC13, HIGH);
    delay(100);
    digitalWrite(PC13, LOW);  
     
    delay(100);
    GyroPreCalibr[0]/=Counter;
    GyroPreCalibr[1]/=Counter;
    GyroPreCalibr[2]/=Counter;
    
    Gyro_Calibration[0]=GyroPreCalibr[0];
    Gyro_Calibration[1]=GyroPreCalibr[1];
    Gyro_Calibration[2]=GyroPreCalibr[2];

    AccelPreCalibr[0]/=Counter;
    AccelPreCalibr[1]/=Counter;
    AccelPreCalibr[2]/=Counter;

    float fLen=sqrt(AccelPreCalibr[0]*AccelPreCalibr[0] + AccelPreCalibr[1]*AccelPreCalibr[1] + AccelPreCalibr[2]*AccelPreCalibr[2]);

    GravityLen = fLen;

    EEPROM.write(MPU_6050_EEPROM_ADRES,  (uint16)Gyro_Calibration[0]);
    EEPROM.write(MPU_6050_EEPROM_ADRES+1,(uint16)Gyro_Calibration[1]);
    EEPROM.write(MPU_6050_EEPROM_ADRES+2,(uint16)Gyro_Calibration[2]);
    EEPROM.write(MPU_6050_EEPROM_ADRES+3,(uint16)GravityLen);
    
    digitalWrite(PC13, HIGH);
}

