/* MPU 6050 Library*/

#ifndef MPU_6050_H__
#define MPU_6050_H__



#define MPU_6050 0x68  // I2C address of the MPU-6050

struct MPU6050
{
	// fields
	short Accel[3];
	short Gyro[3];

  short GravityLen;
    
	short Temperature;

  short Gyro_Calibration[3]; // poprawka kalibracji zyroskopu
	
		
	void Initialization(); 
	void Calibration(); 
	void DataPrint();
	void DataRead();  // zycztanie danych z kompasu
  void DataReadCal();   // czytanie z uwzglednieniem kalibracji
  
  
	
};
#endif
