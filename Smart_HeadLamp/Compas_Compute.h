/* Compas Computation Library */

#ifndef COMPAS_COMPUTATION_H__
#define COMPAS_COMPUTATION_H__


struct DataFrame
{
  unsigned char FrameID;
  short Accel[3];
  short Gyro[3];
  short Compas[3];
  int time_mic_s;

  unsigned int uiCounter;
};


struct Compas_Calculator
{
	
	void Calculation();
	void Display_Course(int ,unsigned char, int); // angle, symbol, display row
	
	
	
};

#endif
