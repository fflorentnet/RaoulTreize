#include <lib74HC595.h>

boolean registers74hc595[numOfRegisterPins];	

void setupShift(){
  pinMode(pinSER, OUTPUT);
  pinMode(pinRCLK, OUTPUT);
  pinMode(pinSRCLK, OUTPUT);
 
  clearRegisters();
  writeRegisters();
}
void clearRegisters(){
  for(int i = numOfRegisterPins - 1; i >=  0; i--){
     registers74hc595[i] = LOW;
  }
} 
 
void writeRegisters(){
  digitalWrite(pinRCLK, LOW);
  for(int i = numOfRegisterPins - 1; i >=  0; i--){
    digitalWrite(pinSRCLK, LOW);
 
    int val = registers74hc595[i];
 
    digitalWrite(pinSER, val);
    digitalWrite(pinSRCLK, HIGH);
 
  }
  digitalWrite(pinRCLK, HIGH);
}
 
void setRegisterPin(int index, int value){
  registers74hc595[index] = value;
}