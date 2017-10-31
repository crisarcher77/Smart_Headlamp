#include <EEPROM.h>
#include "vect3.h"

#include "MPU_6050_.h"
#include "HMC_5883L_.h"
#include "Light_Regulator.h" 
#include "Compas_Compute.h"



#include "Adafruit_SSD1306.h"

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

MPU6050 mpu6050;
HMC5883L hmc5883;
Light_Regulator LightControler;

float f_Dt; // delta T between next iterations 

float fSetCourse=-1.0; // Course is not active

DataFrame Frame;

float Course_Computing(DataFrame);  // compas course recalculation

void Display_Course(float fCoruse,char cSymbol,int row, bool ShowCourseSide); 


float fDisplay_Course_old; // Course angle from previeus calculation
float fDisplay_Course_actual; // Actual course of display 


void setup()
{
    Serial.begin(); // Serial by uC internal USB 
    Serial2.begin(230400); // for HC-06 Bluetooth comunication
    
    pinMode(PC13, OUTPUT);

    EEPROM.PageBase0 = 0x801F000;
    EEPROM.PageBase1 = 0x801F800;
    EEPROM.PageSize  = 0x400;
	
    mpu6050.Initialization();
  	hmc5883.Initialization();  
    LightControler.Initialisaton(); 

    //Initialisation of OLED
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
    display.DirectClear();

    display.font_bit_reverse();
}


void loop()
{   
    static unsigned int uiCounter=0;
    
    static long Time_old=0.001;
	
	static unsigned char ucDataCounter=0;
	static char cText[100];
	
    if (Serial2.available())
    {
		while(Serial2.available())
		{
			char c = Serial2.read();
			cText[ucDataCounter++]=c;
			if (c==10)  // end of transision , print data
			{
				Serial.print(cText);  // temp debug
				
				cText[ucDataCounter-2]=0;
				
				if (cText[0]=='g') mpu6050.Calibration(); // gyroscope calibration
				if (cText[0]=='c') hmc5883.Calibration(); // compas calibration 10s
				
				if (cText[0]=='k')  // set actual course
				{
					fSetCourse=atof(&cText[1]);
					Serial.print("Actual Course is : "); Serial.println(fSetCourse);
				}
				
				if (cText[0]=='l')  // set actual course
				{
					LightControler.iComfortLightLevel = atoi(&cText[1]); // Set Comfort light Level
					Serial.print("Actual Comfort Light Level is : "); Serial.println(LightControler.iComfortLightLevel);
				}
				
				if (cText[0]=='.')  /// print any text begining for '.' - numbers of '.' define number of row where it should be displayed
				{	
					int i_;
					
					for (i_=0;i_<8;i_++) 
					{
						if (cText[i_]!='.') break;
					}
					
					display.DirectPrintString(0,i_,&cText[i_]);
				}
			
				ucDataCounter=0;
			}
		}
    }
    
  mpu6050.DataReadCal();
  hmc5883.DataReadCal();
  LightControler.ReCalculateLight();

  // Wyliczanie aktualnego dt 
  long actTime=micros();
  long deltTime= actTime - Time_old;
  if (deltTime>0) 
  {
    f_Dt=float(deltTime);
    Frame.time_mic_s = deltTime;
  }
  Time_old=actTime;

  Frame.uiCounter=uiCounter++;
  memcpy(Frame.Accel,mpu6050.Accel,sizeof(short)*6);
  memcpy(Frame.Compas,hmc5883.Compas,sizeof(short)*3);
  


    // tylko dla testu performance 
  float fAng;
  long llTim1=micros();
  fAng = Course_Computing(Frame);
  fDisplay_Course_actual=fAng;
  long llTim2=micros();
  
   LightControler.ReCalculateLight();
 
  char text[22];
  sprintf(text, " Ang:%0.2f",fAng);
  display.DirectPrintString(4,5,text);  
  
  sprintf(text, " t: %imicS",llTim2-llTim1);
  display.DirectPrintString(16,6,text);  
  
  sprintf(text, " Loop: %imicS",deltTime);
  display.DirectPrintString(16,7,text);
 /*
  sprintf(text, "IL:%i IR:%i ",LightControler.iInputSensorL,LightControler.iInputSensorR);
  display.DirectPrintString(10,2,text); 

  sprintf(text, "DL:%i DR:%i ",LightControler.iLedDiffuseL,LightControler.iLedDiffuseR);
  display.DirectPrintString(10,1,text);
  */

  int iOledAngle;
  float fOledScale = 4.5;

  Display_Course(   0,'N',4,false);
  Display_Course(  90,'E',4,false);
  Display_Course( 180,'S',4,false);
  Display_Course( 270,'W',4,false);
  
  Display_Course( fSetCourse,'I',4,true); 
  
  fDisplay_Course_old=fDisplay_Course_actual;
}

float Course_Computing(DataFrame Frame)
{        
        static float vGrav_rotated[3]; 
        static float vWest_rotated[3]; 

        //----------------
  
        float vGyro[3]; 
        float vGyroNorm[3]; 
        float fGyro_len;
        float vWest[3]; 
        float fRotationFactor = 1000.0; 

        float fdt=float(Frame.time_mic_s)/1000000.0;
        
        vGyro[0]=Frame.Gyro[0]; vGyro[1]=Frame.Gyro[1]; vGyro[2]=Frame.Gyro[2];        
        fGyro_len=VecLen(vGyro);
       
        vGyroNorm[0]=vGyro[0]; vGyroNorm[1]=vGyro[1]; vGyroNorm[2]=vGyro[2];         
        VecNorm(vGyroNorm);
        
        
        fGyro_len= - fGyro_len * (fdt) *( fRotationFactor / 32767.0);                
                               
        
        vGrav_rotated[0] = vGrav_rotated[0] * 0.99 + Frame.Accel[0] * 0.01; 
        vGrav_rotated[1] = vGrav_rotated[1] * 0.99 + Frame.Accel[1] * 0.01; 
        vGrav_rotated[2] = vGrav_rotated[2] * 0.99 + Frame.Accel[2] * 0.01; 

        VecRotate(fGyro_len,vGyroNorm[0],vGyroNorm[1],vGyroNorm[2],vGrav_rotated); // obracanie wektora grawitacji o zadana rotacje 

        
        vWest[0] = Frame.Compas[0]; vWest[1] = Frame.Compas[1]; vWest[2] = Frame.Compas[2];

        VecCross(vWest,vGrav_rotated);
        VecNorm(vWest);
         
        vWest_rotated[0] = vWest_rotated[0] * 0.99 + vWest[0] * 0.01;
        vWest_rotated[1] = vWest_rotated[1] * 0.99 + vWest[1] * 0.01;
        vWest_rotated[2] = vWest_rotated[2] * 0.99 + vWest[2] * 0.01;

        VecRotate(fGyro_len,vGyroNorm[0],vGyroNorm[1],vGyroNorm[2],vWest_rotated); // obracanie wektora kierunku Zachod o zadana rotacje 
        
		float vXAxis[3];
		vXAxis[0]=1.0; vXAxis[1]=0.0; vXAxis[2]=0.0;
		
		float vGrav_rotated_Normal[3]; 
		vGrav_rotated_Normal[0] = vGrav_rotated[0]; vGrav_rotated_Normal[1] = vGrav_rotated[1]; vGrav_rotated_Normal[2] = vGrav_rotated[2];
		VecNorm(vGrav_rotated_Normal);
    
		float fPitchAngle = acos( VecDot(vGrav_rotated_Normal,vXAxis) ) * 180.0/M_PI;
		
        
		float vWest_rotated_flat[3]; 
		
		VecCross(vGrav_rotated_Normal,vXAxis);
		VecNorm(vGrav_rotated_Normal);
		
		vWest_rotated_flat[0]=vWest_rotated[0]; vWest_rotated_flat[1]=vWest_rotated[1];  vWest_rotated_flat[2]=vWest_rotated[2]; 
		VecRotate(fPitchAngle,vGrav_rotated_Normal[0],vGrav_rotated_Normal[1],vGrav_rotated_Normal[2],vWest_rotated_flat);
		
		float fAngle_Flat = Vec2Ang(vWest_rotated_flat[1] , vWest_rotated_flat[2]);
		
		return fAngle_Flat;
}

void Display_Course(float fCourse,char cSymbol,int row, bool ShowCourseSide)
{
 // display Course marked Symbol on display 
 
  int iOledAngle;
  float fOledScale = 4.5;
  
  if (fCourse>=0)
  {
  //=================== // debug fragment
 /* if (ShowCourseSide)
		{
		float fCalc=fDisplay_Course_actual;
		if (fCalc>180) fCalc=fCalc-360;
		float fCalcC =fCourse;
		if (fCalcC >180) fCalcC=fCalcC-360;
		
		float ff=fCourse - fDisplay_Course_actual;
		if (ff<0) ff=360 - ff; 
		
		if (ff>180) ff= ff-360;
		
    char text[30];  sprintf(text, " %0.0f  %0.0f :%0.0f  ", fCalcC - fCalc ,fDisplay_Course_actual,ff);  display.DirectPrintString(10,1,text);
	}*/
  //==================
  
	  iOledAngle = 128 + (-fDisplay_Course_old      + fCourse) * fOledScale ; 
	  if ((iOledAngle>0)&&(iOledAngle<122)) display.DirectPrintChar(iOledAngle,row,' ');
	  
	  iOledAngle = 128 + (-fDisplay_Course_actual   + fCourse) * fOledScale ; 
	  if ((iOledAngle>0)&&(iOledAngle<122)) display.DirectPrintChar(iOledAngle,row,cSymbol); 	  
	  else // course is out of display
	  {
		if (ShowCourseSide)
		{
			if ((-fDisplay_Course_actual      + fCourse) <0) display.DirectPrintChar(0,row,'<'); 
			else display.DirectPrintChar(121,row,'>'); 
		}	
	  }
  }
}

