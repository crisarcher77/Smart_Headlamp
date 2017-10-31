
#include <Arduino.h>


#include "Light_Regulator.h"


void  Light_Regulator::Initialisaton()
{

    iComfortLightLevel=1000; // read from EEPROM  
    fOutPower=1000; // read from EEPROM

    fOutPower_old=fOutPower;
    iInput_old=0;

    pinMode(LED_SPOT, PWM);  // setup the pin as PWM
    pinMode(LED_SPOT_EXTRA, PWM);  // setup the pin as PWM
    pinMode(LED_DIFFUS_L, PWM);  // setup the pin as PWM
    pinMode(LED_DIFFUS_R, PWM);  // setup the pin as PWM
    
    pinMode(PC13, OUTPUT);
    pinMode(LIGHT_SENSOR_L, INPUT_ANALOG);
    pinMode(LIGHT_SENSOR_L, INPUT_ANALOG);

	// Test StartUp
    pwmWrite(LED_SPOT, 32000 ); //_P
    delay(100);
    pwmWrite(LED_SPOT, 0 ); // _P
    pwmWrite(LED_DIFFUS_L , 32000); //_L
    delay(100);
    pwmWrite(LED_DIFFUS_L , 0); //_L
    pwmWrite(LED_SPOT_EXTRA, 32000 ); //_P
    delay(100);
    pwmWrite(LED_SPOT_EXTRA, 0 ); // _P
    pwmWrite(LED_DIFFUS_R , 32000);//iLed_Spot );
    delay(100);
    pwmWrite(LED_DIFFUS_R , 0);//iLed_Spot );
}

void Light_Regulator::ReCalculateLight()
{
        int iInput;
      
        
        iInputSensorL = analogRead(LIGHT_SENSOR_L );
        
        iInputSensorR= analogRead(LIGHT_SENSOR_R);
        iInputSensorL =int((float) (iInputSensorL ))/ 1.130; // korekcja wynikajaca z wady elementu

        iInput=( iInputSensorL + iInputSensorR ) / 2;
        //fOutPower = 0.5* fOutPower_old + 0.5* fOutPower *2.0* (float)(iLevel)/(float)((iInput+iInput_old)) ;
       // fOutPower = 0.8* fOutPower_old + 0.2* fOutPower * (float)(iComfortLightLevel)/(float)((iInput+iInput_old)/2) ;
        fOutPower =  0.7 * fOutPower + 0.3 * fOutPower * (float)(iComfortLightLevel)/(float)(iInput) ;
       // fOutPower =  0.9 * fOutPower + 0.1 * fOutPower * (float)(iComfortLightLevel)/(float)((iInput+iInput_old)/2) ;
        
        if (fOutPower<=0) fOutPower=0;
        if (fOutPower>=65535) fOutPower=65535;

        //Actual Light of Spot and Diffuse Calculation
        int iLedSpot;
        int iLedDiff;
        
        if (fOutPower<32762) 
        {
          iLedDiff=fOutPower*2.0; 
          iLedSpot=0;
        }
        
        if (fOutPower>=32762) 
        {
          iLedDiff=65535*2 - fOutPower*2.0; 
          iLedSpot= (fOutPower-32768)*2.0; 
        }
        
        if (fOutPower>65530) digitalWrite(PC13,LOW); 
        else digitalWrite(PC13,HIGH);

        
        
        if (iLedSpot>32768)
        {
            iLedSpotExtra = (iLedSpot - 32768) *2;
            iLedSpotNormal = 65535;
        }
        else 
        {
            iLedSpotExtra = 0;
            iLedSpotNormal = iLedSpot * 2 ;
        }

        
       
        float fFactor = float (iInputSensorR) / float(iInputSensorL);
        
        iLedDiffuseL = iLedDiff *  fFactor * fFactor;
        iLedDiffuseR = iLedDiff / (fFactor * fFactor);

        if (iLedDiffuseL > 65535) iLedDiffuseL = 65535;
        if (iLedDiffuseR > 65535) iLedDiffuseL = 65535;
       
        pwmWrite(LED_DIFFUS_L, iLedDiffuseL );
        pwmWrite(LED_DIFFUS_R , iLedDiffuseR );
        pwmWrite(LED_SPOT , iLedSpotNormal );
        pwmWrite(LED_SPOT_EXTRA , iLedSpotExtra );
            
         iInput_old=iInput;
         fOutPower_old=fOutPower;
}
