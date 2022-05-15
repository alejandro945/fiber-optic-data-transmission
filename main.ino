#include <Wire.h>
#include "Adafruit_TCS34725.h"

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 3.3V DC
   Connect GROUND to common ground */

/*
        R     G     B
 Inicio 255   255   0    -------> +/- 50% E
 White  255   255   255  -> 00 -> 30% E
 Blue   0     0     255  -> 01 -> 30% E
 Green  0     255   0    -> 10 -> 30% E
 Red    255   0     0    -> 11 -> 30% E 
 Final  0     255   255  -------> +/- 50% E
 */

/* Initialise with default values (int time = 2.4ms, gain = 1x) */
// Adafruit_TCS34725 tcs = Adafruit_TCS34725();
/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);
/* Const Configuration*/
const byte RED = 9; //RED OUTPUT
const byte BLUE = 10; //BLUE OUTPUT
const byte GREEN = 11; //GREEN OUTPUT
/* Global Variables Configuration*/
String dataSent = "";
String textSent = "";
//---------------------------------------------EMISOR----------------------------------------
void setup(void) {
  Serial.begin(9600); // enable serial output for debugging
  //Salidas digitales
  pinMode(RED,OUTPUT);
  pinMode(BLUE,OUTPUT);
  pinMode(GREEN,OUTPUT);
  //Demulador config
  if (tcs.begin()) {
    Serial.println("Found demulator sensor");
    Serial.println("Inicio de aplicativo: Ingrese % para chat, Ingrese & para bloque, Ingrese $ para eco");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
}

void loop(void) {
  String data = dataReading();
  /*if(!isIncase){
    transmitionCases(data);
  }*/
  transmition(data);
}

String dataReading(){
  String data = "";
  if(Serial.available()){
    data= Serial.readStringUntil('\n');
  }
  return data;
}

void transmition(String data){
  String letterToBin = "0";
  for(int i = 0; i<data.length();i++){
    char letter = data.charAt(i);
    if(int(letter)<=63){
      letterToBin += "0";
    }
    letterToBin += String(letter,BIN);
    sendColor(letterToBin);
    letterToBin = "0";
  }
}

void sendColor(String bin){
  Serial.println(bin);
  String props = "";
  props+=bin.charAt(0);
  props+=bin.charAt(1);
  colorCases(props);
  turnOff();
  props = "";
  props+=bin.charAt(2);
  props+=bin.charAt(3);
  colorCases(props);
  turnOff();
  props = "";
  props+=bin.charAt(4);
  props+=bin.charAt(5);
  colorCases(props);
  turnOff();
  props = "";
  props+=bin.charAt(6);
  props+=bin.charAt(7);
  colorCases(props);
  turnOff();
}

void colorCases(String twoBits){
     if(twoBits.compareTo("00")==0){
          digitalWrite(RED,HIGH);
          digitalWrite(BLUE,HIGH);
          digitalWrite(GREEN,HIGH);
          delay(100);
     }else if(twoBits.compareTo("01")==0){
          digitalWrite(BLUE,HIGH);
          delay(100);
     }else if(twoBits.compareTo("10")==0){
          digitalWrite(GREEN,HIGH);
          delay(100);
     }else{
          digitalWrite(RED,HIGH);
          delay(100);
     }
     demodulatorReading();
}

void turnOff(){
      digitalWrite(RED,LOW);
      digitalWrite(BLUE,LOW);
      digitalWrite(GREEN,LOW);
      delay(100);
}


//---------------------------------------------RECEPTOR----------------------------------------
void demodulatorReading(){
  delay(30);
  uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);
    /*Parsing to 8 bits*/
    r /= 256; g /= 256; b /= 256;
    Serial.println(dataSent);
    bitCases(r,g,b,dataSent);
    if(dataSent.length() == 8){
      Serial.println(valorAscii(dataSent));
      textSent += valorAscii(dataSent);
      dataSent = "";  
    }
    //For testing propositos
    Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
    Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
    Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
    Serial.println(" ");
    Serial.println(textSent);
}

void bitCases(int R,int G,int B,String current){
    if(R>150 && G>150 && B>150){ //WHITE
          dataSent += "00";
     }else if(B>150){
          dataSent += "01";
     }else if(G>150){
          dataSent += "10";
     }else if(R>150){
          dataSent += "11";
     }
}

//Regresa una cadena binaria en su correspondiente valor ascii
char valorAscii(String CadenaBinaria)
{
  int Multiplo=1;
  char Res=0;
  char Bit;  
  for(int Cont=CadenaBinaria.length()-1; Cont>=0; Cont--)
  {
    Bit=CadenaBinaria.charAt(Cont);//Obtengo el bit que estoy manipulando
    if(Bit=='1')    
      Res= Res + Multiplo;      
    Multiplo= Multiplo* 2;
  }
  return Res;
}