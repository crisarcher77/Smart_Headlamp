#ifndef HMC_5883_L_H__
#define HMC_5883_L_H__

#define HMC_5883L 0x1E



struct HMC5883L
{
	// field
	short		Compas[3]; // bazowe odczyty z compasu
	float 		fCompas[3];
	
	short		CalibrCompasOffset[3]; // kompensacja - centrowanie przesuniecia odczytu
	short 	CalibrCompasScale[3]; // wyrownanie skali odczytow osi
	
  short     CalibrCompasRange[3]; // ??????
   
	// methods
		
	void Initialization();
	void DataRead(); // zycztanie danych z kompasu
  void DataReadCal(); // czytanie z uwzglednieniem kalibracji 
	void DataPrint();
  void Calibration(); 
};

#endif
