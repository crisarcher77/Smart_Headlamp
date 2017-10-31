#ifndef VECT3_H__
#define VECT3_H__

#include <math.h>

float VecLen(float * vec) // dlugosc aktualnego wektora
{
	return sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
}

void VecNorm(float * vec) // dlugosc aktualnego wektora
{
	float l=sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]);
        if (l!=0)
        { vec[0]/=l; vec[1]/=l; vec[2]/=l; }
}


void VecRotate(float alfa,float vn0,float vn1,float vn2,float *vec)
{
  float f_pomoc,c,s;
  float vect3[3];

  alfa/=57.29577951; // konwersja na standard w radianach
  s=sin(alfa);
  c=cos(alfa);
  
  f_pomoc =vec[0]*(vn0*vn0+c*(1-vn0*vn0));
  f_pomoc+=vec[1]*(vn0*vn1*(1-c)-s*vn2);
  f_pomoc+=vec[2]*(vn0*vn2*(1-c)+s*vn1);
  vect3[0]=f_pomoc;
  
  f_pomoc= vec[0]*(vn1*vn0*(1-c)+s*vn2);
  f_pomoc+=vec[1]*(vn1*vn1+c*(1-vn1*vn1));
  f_pomoc+=vec[2]*(vn1*vn2*(1-c)-s*vn0);
  vect3[1]=f_pomoc;
  
  f_pomoc= vec[0]*(vn2*vn0*(1-c)-s*vn1);
  f_pomoc+=vec[1]*(vn2*vn1*(1-c)+s*vn0);
  f_pomoc+=vec[2]*(vn2*vn2+c*(1-vn2*vn2));
  vect3[2]=f_pomoc;
  memcpy(vec,vect3,12);
}

void VecCross(float *A,float *v) 
{  
      float vect3_pomoc[3];	
         vect3_pomoc[0]=A[1]*v[2]-A[2]*v[1];
         vect3_pomoc[1]=A[2]*v[0]-A[0]*v[2];
         vect3_pomoc[2]=A[0]*v[1]-A[1]*v[0];
         
         memcpy(A,vect3_pomoc,12); // !!! wektor A zawiera wynik operacji
}

float VecDot(float *A,float *B)
{
  return (A[0]*B[0]+ A[1]*B[1] + A[2]*B[2]);
}

float Vec2Ang(float vA,float vB) // kat wyrazony w Stopniach 
{
  float ai_Vect[3];
  ai_Vect[0]=vA;
  ai_Vect[2]=vB;
  
  float lf_kat = atan((float)abs(ai_Vect[2])/(float)abs(ai_Vect[0])) * 180/M_PI;
  if (ai_Vect[0] >= 0 && ai_Vect[2] >= 0){
    return lf_kat;
  }
  if (ai_Vect[0] < 0 && ai_Vect[2] >= 0){
    return (180.0 ) - lf_kat;
  }
  if (ai_Vect[0] < 0 && ai_Vect[2] < 0){
    return (180.0) + lf_kat; //270st + kat
  }
  if (ai_Vect[0] >= 0 && ai_Vect[2] < 0){
    return (180.0 * 2) - lf_kat; //270st + kat
  }
  return lf_kat;
}

/*
float Vec2Ang(float vA,float vB) // kat wyrazony w Stopniach 
{
	float ai_Vect[3];
	ai_Vect[0]=vA;
	ai_Vect[2]=vB;
	
	float lf_kat = atan((float)abs(ai_Vect[2])/(float)abs(ai_Vect[0]));
	if (ai_Vect[0] >= 0 && ai_Vect[2] >= 0){
		return lf_kat;
	}
	if (ai_Vect[0] < 0 && ai_Vect[2] >= 0){
		return (3.14159 ) - lf_kat;
	}
	if (ai_Vect[0] < 0 && ai_Vect[2] < 0){
		return (3.14159) + lf_kat; //270st + kat
	}
	if (ai_Vect[0] >= 0 && ai_Vect[2] < 0){
		return (3.14159 * 2) - lf_kat; //270st + kat
	}
	return lf_kat;
}*/
#endif
