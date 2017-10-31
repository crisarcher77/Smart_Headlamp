/* Reactive Light Regulation Library*/

#ifndef LIGHT_REGULATION_H__
#define LIGHT_REGULATION_H__

#define LIGHT_SENSOR_L PA1
#define LIGHT_SENSOR_R PA0

#define LED_SPOT        PA10
#define LED_SPOT_EXTRA  PB8
#define LED_DIFFUS_L    PA8
#define LED_DIFFUS_R    PA9

struct Light_Regulator
{
    int iComfortLightLevel; // actual used of Comfort light Level
    float fOutPower; // actual used power level 
    float fOutPower_old; // old used power level - for better extrapolation
    
    int iInput_old;
    

    int iInputSensorL; // data from light sensor LEFT
    int iInputSensorR; // data from light sensor RIGHT

    int iLedDiffuseL; // power on Left Diffuse LED
    int iLedDiffuseR;
    
    int iLedSpotExtra; // additional power of spot LED 
    int iLedSpotNormal;  // power on spot LED

    void Initialisaton();
    void ReCalculateLight();
   
};



#endif

