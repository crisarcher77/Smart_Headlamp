#include "HMC_5883L_.h"
#include <EEPROM.h>
//#include <Wire.h>
#include <HardWire.h>

#define HMC_5883L_EEPROM_ADRESS 0x10

HardWire _wire(1, I2C_FAST_MODE);

void HMC5883L::Initialization()
{
  _wire.beginTransmission(HMC_5883L); //open communication with HMC5883
  _wire.write(0x02); //select mode register
  _wire.write(0x00); //continuous measurement mode
  _wire.endTransmission();

  _wire.beginTransmission(HMC_5883L); 
  _wire.write(byte(0x00)); // register aderes
  _wire.write(byte(0x10)); // register aderes 01110000 ( 15 usrednien i normalny tryb probkowania bez bias)
  _wire.endTransmission();
  _wire.beginTransmission(HMC_5883L); 
  _wire.write(byte(0x01)); // register aderes
  _wire.write(byte(0x00)); // register aderes 00000000 ( czuL�osc 0.88Ga) 1.3Ga
  _wire.endTransmission();
  _wire.beginTransmission(HMC_5883L); 
  _wire.write(byte(0x02)); // register aderes
  _wire.write(byte(0x00)); // register aderes 01110000 ( ciagle probkowanie )
  _wire.endTransmission(); 

  // Read calibration data from EEPROM
  EEPROM.read(HMC_5883L_EEPROM_ADRESS,    (uint16*)&CalibrCompasOffset[0]);
  EEPROM.read(HMC_5883L_EEPROM_ADRESS+1,  (uint16*)&CalibrCompasOffset[1]);
  EEPROM.read(HMC_5883L_EEPROM_ADRESS+2,  (uint16*)&CalibrCompasOffset[2]);
  EEPROM.read(HMC_5883L_EEPROM_ADRESS+3,  (uint16*)&CalibrCompasScale[0]);
  EEPROM.read(HMC_5883L_EEPROM_ADRESS+4,  (uint16*)&CalibrCompasScale[1]);
  EEPROM.read(HMC_5883L_EEPROM_ADRESS+5,  (uint16*)&CalibrCompasScale[2]);
  
  /*CalibrCompasOffset[0]=0;
  CalibrCompasOffset[1]=0;
  CalibrCompasOffset[2]=0;*/
}


void HMC5883L::DataRead()
{
	
  _wire.beginTransmission(HMC_5883L);
  _wire.write(0x03); //select register 3, X MSB register
  _wire.endTransmission();
  
 //Read data from each axis, 2 registers per axis
  _wire.requestFrom(HMC_5883L, 6);
  //if(6<=_wire.available()){
	Compas[0]=_wire.read()<<8|_wire.read();  
	Compas[1]=_wire.read()<<8|_wire.read();  
	Compas[2]=_wire.read()<<8|_wire.read();  

  // korekcja osi dla zgodnosci z kompasu z zyrozkopem
  short sTemp = Compas[0]; 
  Compas[0]= -Compas[2];
  Compas[2]= -Compas[1];
  Compas[1]=  sTemp;
} 
	
void HMC5883L::DataReadCal()
{
  
  DataRead();
  
  Compas[0] -= CalibrCompasOffset[0]; // offsetX correction 
  Compas[1] -= CalibrCompasOffset[1]; // offsetY correction
  Compas[2] -= CalibrCompasOffset[2]; // offsetZ correction
 
  Compas[0] =  (1000 * long(Compas[0])) / CalibrCompasScale[0]; // scaleX correction
  Compas[1] =  (1000 * long(Compas[1])) / CalibrCompasScale[1]; // scaleY correction
  Compas[2] =  (1000 * long(Compas[2])) / CalibrCompasScale[2]; // scaleZ correction
}
	
	
void HMC5883L::DataPrint()
{
	float fCompasLen;
 
	Serial.print(Compas[0]); Serial.print(" ");
	Serial.print(Compas[1]); Serial.print(" ");
	Serial.print(Compas[2]); Serial.print(" ");
	
/*	Serial.print(CalibrCompasMax[0]-CalibrCompasMin[0]); Serial.print(" ");
	Serial.print(CalibrCompasMax[1]-CalibrCompasMin[1]); Serial.print(" ");
	Serial.print(CalibrCompasMax[2]-CalibrCompasMin[2]); Serial.print(" ");
	
	Serial.print((CalibrCompasMax[0]+CalibrCompasMin[0])/2); Serial.print(" ");
	Serial.print((CalibrCompasMax[1]+CalibrCompasMin[1])/2); Serial.print(" ");
	Serial.print((CalibrCompasMax[2]+CalibrCompasMin[2])/2); Serial.print(" ");
	*/
	
	fCompasLen = sqrt(Compas[0]*Compas[0]+Compas[1]*Compas[1]+Compas[2]*Compas[2]);
	
	Serial.print(fCompasLen); Serial.print(" ");
	
	/*Serial.print(fCompas[0]); Serial.print(" ");
	Serial.print(fCompas[1]); Serial.print(" ");
	Serial.print(fCompas[2]); Serial.print(" ");*/
	
	//fCompasLen = sqrt(fCompas[0]*fCompas[0]+fCompas[1]*fCompas[1]+fCompas[2]*fCompas[2]);
	
	//Serial.print(fCompasLen); Serial.print(" ");
	
}	

void HMC5883L::Calibration()
{

short CalibrCompasMax[3];
short CalibrCompasMin[3];
long lTime = millis();

CalibrCompasMax[0]=CalibrCompasMax[1]=CalibrCompasMax[2]=0;
CalibrCompasMin[0]=CalibrCompasMin[1]=CalibrCompasMin[2]=0;

while ((millis()-lTime)<10000)
{
  DataRead();


  if ((Compas[0]<1000)&&(Compas[0]>-1000)&&(Compas[1]<1000)&&(Compas[1]>-1000)&&(Compas[2]<1000)&&(Compas[2]>-1000)) // zabezpieczenie przed szumami 
  {
      if (Compas[0]>CalibrCompasMax[0]) CalibrCompasMax[0]=Compas[0];
      if (Compas[1]>CalibrCompasMax[1]) CalibrCompasMax[1]=Compas[1];
      if (Compas[2]>CalibrCompasMax[2]) CalibrCompasMax[2]=Compas[2];
      if (Compas[0]<CalibrCompasMin[0]) CalibrCompasMin[0]=Compas[0];
      if (Compas[1]<CalibrCompasMin[1]) CalibrCompasMin[1]=Compas[1];
      if (Compas[2]<CalibrCompasMin[2]) CalibrCompasMin[2]=Compas[2];
  }

  if (((millis()-lTime)>>8)&0x1) digitalWrite(PC13, LOW);
  else digitalWrite(PC13, HIGH);
  
  delay(1000/14);
} // end while 

  CalibrCompasOffset[0] = (CalibrCompasMax[0]+CalibrCompasMin[0])/2;
  CalibrCompasOffset[1] = (CalibrCompasMax[1]+CalibrCompasMin[1])/2;
  CalibrCompasOffset[2] = (CalibrCompasMax[2]+CalibrCompasMin[2])/2;

  CalibrCompasScale[0] = (CalibrCompasMax[0] - CalibrCompasMin[0])/2;
  CalibrCompasScale[1] = (CalibrCompasMax[1] - CalibrCompasMin[1])/2;
  CalibrCompasScale[2] = (CalibrCompasMax[2] - CalibrCompasMin[2])/2;

  EEPROM.write(HMC_5883L_EEPROM_ADRESS,    (uint16)CalibrCompasOffset[0]);
  EEPROM.write(HMC_5883L_EEPROM_ADRESS+1,  (uint16)CalibrCompasOffset[1]);
  EEPROM.write(HMC_5883L_EEPROM_ADRESS+2,  (uint16)CalibrCompasOffset[2]);
  EEPROM.write(HMC_5883L_EEPROM_ADRESS+3,  (uint16)CalibrCompasScale[0]);
  EEPROM.write(HMC_5883L_EEPROM_ADRESS+4,  (uint16)CalibrCompasScale[1]);
  EEPROM.write(HMC_5883L_EEPROM_ADRESS+5,  (uint16)CalibrCompasScale[2]);
  
} //end  HMC5883L::Calibration()

