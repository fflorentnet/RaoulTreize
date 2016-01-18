#ifndef LIB74HC595_h
#define LIB74HC595_h
#include <Arduino.h>
	/*
	* Configuration du 74hc595
	*/
	#define pinSER 9
	#define pinRCLK 10
	#define pinSRCLK 11
	#define number_of_74hc595s 2

	#define numOfRegisterPins number_of_74hc595s * 8
	#ifdef __cplusplus
extern "C"{
#endif
		extern boolean registers74hc595[numOfRegisterPins];	
		void setupShift();
		void clearRegisters();
		void writeRegisters();
		void setRegisterPin(int index, int value);

#ifdef __cplusplus
} // extern "C"
#endif
#else
#endif