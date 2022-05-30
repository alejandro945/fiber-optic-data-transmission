//Libraries
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <pt.h>
/*100 palabras Example for testing no se soportan las tildes ni ñ
  Una amiga mia se compro una comadreja, que resulto ser muy caprichosa. El bicho no quiere estar en la jaula, asi que campa a sus anchas todo el dia.
  Tampoco quiere comida de lata, sino solo refinados guisos, y se enfada si la comida no esta a tiempo. No muerde, pero muestra los dientes.
  Cuando ella llega del trabajo, las botellas de licor estan vacias, y la comadreja duerme en el sofa, con la tele encendida, toda la casa sucia y revuelta.
*/
//Basic Setup
/* Download the library from http://dunkels.com/adam/pt/download.html then extract tar.gz in libraries arduino documents folder and delete 3 examples files*/
/*
  Ambos nodos deben conocer el app mode en que se operan antes de iniciar!
  Demolador Manual Config
  Connect SCL    to analog 5
  Connect SDA    to analog 4
  Connect VIN    to 5V DC INO
  Connect GROUND to common ground
  Connect LED to GROUND
  Connect v od LED to GROUND and set led pins
  /*
    R     G     B
  Inicio 255   255   0    -------> +/- 50% E
  White  255   255   255  -> 00 -> 30% E
  Blue   0     0     255  -> 01 -> 30% E
  Green  0     255   0    -> 10 -> 30% E
  Red    255   0     0    -> 11 -> 30% E
  Final  255   0     255  -------> +/- 50% E
*/
/* Initialise with default values (int time = 2.4ms, gain = 1x) "Deprecated" */
// Adafruit_TCS34725 tcs = Adafruit_TCS34725();
/* Initialise with specific int time and gain values */
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);
/* Thread Vareables Declaración */
static struct pt demThread;
static struct pt ledThread;
static struct pt transThread;
/* Port Const Configuration*/
const byte RED = 9; //RED OUTPUT
const byte BLUE = 10; //BLUE OUTPUT
const byte GREEN = 11; //GREEN OUTPUT
/* Global Variables Configuration*/
boolean isSending = false;
boolean isSensorReading = false;
String dataSent = "";
String textSent = "";
String emisorData = "";
String initialNodeType = ""; //Emisor or Receptor
String appMode = "";
/*Usefull just for Bloque Case*/
int characterCounter = 0;
int checkSumCounter = 0;
/*Usefull just for Eco Case*/
int currentIndex = -1;
String charSent = "";
boolean already = false;
//---------------------------------------------ARDUINO SET AND LOOP----------------------------------------
void setup() {
  //Initialise photothreads variables
  PT_INIT(&demThread);
  PT_INIT(&ledThread);
  PT_INIT(&transThread);
  //Enable serial output for debugging
  Serial.begin(9600);
  //Digital Outputs
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(GREEN, OUTPUT);
  //Demulador config
  if (tcs.begin()) {
    Serial.println("Found demulator sensor");
    Serial.println("Inicio de aplicativo: Ingrese % para chat, Ingrese & para bloque, Ingrese $ para eco");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
}

//Bucle repetitivo tanto para el emisor como para un receptor modelado como una misma entidad
void loop(void) {
  loopEmisor(&ledThread);
  loopDemolator(&demThread);
  //Breaks the threads :)
  loopTransmision(&transThread);
}

//---------------------------------------------EMISOR----------------------------------------
void loopEmisor(struct pt *pt) {
  PT_BEGIN(pt);
  static unsigned long timestamp = 0;
  do {
    appCases();
    PT_WAIT_UNTIL(pt, millis() - timestamp > 50 );
    timestamp = millis();
  } while (true);
  PT_END(pt);
}

void loopDemolator(struct pt *pt) {
  PT_BEGIN(pt);
  static unsigned long timestamp = 0;
  do {
    demodulatorObserver();
    PT_WAIT_UNTIL(pt, millis() - timestamp > 617 );
    timestamp = millis();
  } while (true);
  PT_END(pt);
}

void loopTransmision(struct pt *pt) {
  PT_BEGIN(pt);
  static unsigned long timestamp = 0;
  do {
    PT_WAIT_UNTIL(pt, millis() - timestamp > 50 );
    timestamp = millis();
    if (emisorData != "" && !isSending) {

      //Config
      isSending = true;
      Serial.println("App mode " + appMode);

      //Setting node type
      if (initialNodeType == "") {
        initialNodeType = "Emisor";
      }

      //Chat Estetica
      if (appMode == "%") {
        Serial.println("[Envía] Canal " + initialNodeType + ": " + emisorData);
      }

      //LED Transmision for chat and Bloque
      if (appMode != "$") {
        transmition(emisorData);
      } else if (!already) {
        //Precondición solo se puede enviar por el monitor una vez :(
        already = true;
        ecoTransmition(true);
      }

      //Clear Data
      isSending = false;
      emisorData = "";
    }
  } while (true);
  PT_END(pt)
}

//Ambos deben conocer el modo en que se esta operan
void appCases() {
  if (appMode == "") {
    String mode = dataReading();
    if (mode == "%") {
      appMode = "%";
    } else if (mode == "&") {
      appMode = "&";
    } else if (mode == "$") {
      appMode = "$";
    } else if (mode != "") {
      Serial.println("Ingrese % para chat, Ingrese & para bloque, Ingrese $ para eco");
    }
  } else {
    getData();
  }
}

//App mode already set we continue reading data in emisor module, receptor only gonna be a suscriber
void getData() {
  if (emisorData == "") {
    emisorData = dataReading();
  }
}

//Auxiliar Method
String dataReading() {
  String data = "";
  if (Serial.available()) {
    data = Serial.readStringUntil('\n');
  }
  return data;
}

//2 Bits Led Transmision
void transmition(String data) {
  transmitionCentinel(true);
  String letterToBin = "0";
  for (int i = 0; i < data.length(); i++) {
    char letter = data.charAt(i);
    if (int(letter) <= 63) {
      letterToBin += "0";
    }
    letterToBin += String(letter, BIN);
    sendColor(letterToBin);
    letterToBin = "0";
  }
  transmitionCentinel(false);
}

//Envío por caracter condicional
void ecoTransmition(boolean increase) {
  String data = (initialNodeType == "Receptor") ? textSent : emisorData;
  if (increase) {
    currentIndex++;
  }
  if (initialNodeType == "Emisor") {
    charSent = data.charAt(currentIndex);
  }
  transmitionCentinel(true);
  String letterToBin = "0";
  char letter = data.charAt(currentIndex);
  if (int(letter) <= 63) {
    letterToBin += "0";
  }
  letterToBin += String(letter, BIN);
  sendColor(letterToBin);
  letterToBin = "0";
  transmitionCentinel(false);
}

//Inicio y Fin de carrera
void transmitionCentinel(boolean isInit) {
  if (isInit) {
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, HIGH);
    delay(635);
  } else {
    digitalWrite(RED, HIGH);
    digitalWrite(BLUE, HIGH);
    delay(635);
  }
  turnOff();
}

//Extract 2 bits of de byte ascci character
void sendColor(String bin) {
  Serial.println(bin); //Testing
  String props = "";
  props += bin.charAt(0);
  props += bin.charAt(1);
  colorCases(props);
  turnOff();
  props = "";
  props += bin.charAt(2);
  props += bin.charAt(3);
  colorCases(props);
  turnOff();
  props = "";
  props += bin.charAt(4);
  props += bin.charAt(5);
  colorCases(props);
  turnOff();
  props = "";
  props += bin.charAt(6);
  props += bin.charAt(7);
  colorCases(props);
  turnOff();
}

//Sending led Color using digital const outputs
void colorCases(String twoBits) {
  if (twoBits.compareTo("00") == 0) {
    digitalWrite(RED, HIGH);
    digitalWrite(BLUE, HIGH);
    digitalWrite(GREEN, HIGH);
    delay(635);
  } else if (twoBits.compareTo("01") == 0) {
    digitalWrite(BLUE, HIGH);
    delay(635);
  } else if (twoBits.compareTo("10") == 0) {
    digitalWrite(GREEN, HIGH);
    delay(635);
  } else {
    digitalWrite(RED, HIGH);
    delay(635);
  }
}

// Turning off de leds
void turnOff() {
  digitalWrite(RED, LOW);
  digitalWrite(BLUE, LOW);
  digitalWrite(GREEN, LOW);
}

//---------------------------------------------RECEPTOR----------------------------------------
//Lectura constante para los receptores que estan esperendo a leer el inicio de carrera
void demodulatorObserver() {
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  /*Parsing to 8 bits*/
  r /= 256; g /= 256; b /= 256;
  /*Testing*/
  Serial.println(r, DEC);
  //Maxim
  float Maximum;
  Maximum = max(r, g);
  Maximum = max(Maximum, b);

  //Validar Inicio
  if (r > 150 && g > 150 && !isSensorReading) {
    isSensorReading = true;
    //Setting initial node type
    if (initialNodeType == "") {
      initialNodeType = "Receptor";
    }
  }
  //Validar Fin
  if (r > 150 && b > 150 && isSensorReading) {
    isSensorReading = false;
  }
  //Leer señales electricas transmitidas por la fibra optica
  if (isSensorReading) {
    demodulatorReading(r, g, b);
  }
  //Receptor cases
  if (!isSensorReading && textSent != "") {
    receptorCases();
  }
}

void demodulatorReading(int r, int g, int b) {
  Serial.println(dataSent); //For testing porpouses
  bitCases(r, g, b, dataSent);
  if (dataSent.length() == 8) {
    //---------------------Bloque Funcionamiento----------------------
    characterCounter++;
    checkSumCounter += 8;
    if (appMode == "&") {
      receptorBloque(false);
    }
    //---------------------Bloque Funcionamiento----------------------
    Serial.println(valorAscii(dataSent)); //For testing porpouses letter
    textSent += valorAscii(dataSent);
    dataSent = "";
  }
  //For testing propositos
  Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.println(" ");
}

void bitCases(int R, int G, int B, String current) {
  if (R > 150 && G > 150 && B > 150) { //WHITE
    dataSent += "00";
  } else if (B > 150) { //BLUE
    dataSent += "01";
  } else if (G > 150) { //GREEN
    dataSent += "10";
  } else if (R > 150) { //RED
    dataSent += "11";
  }
}

//Regresa una cadena binaria en su correspondiente valor ascii
char valorAscii(String CadenaBinaria)
{
  int Multiplo = 1;
  char Res = 0;
  char Bit;
  for (int Cont = CadenaBinaria.length() - 1; Cont >= 0; Cont--)
  {
    //Obtengo el bit que se esta manipulando
    Bit = CadenaBinaria.charAt(Cont);
    if (Bit == '1')
      Res = Res + Multiplo;
    Multiplo = Multiplo * 2;
  }
  return Res;
}


void receptorCases() {
  if (appMode == "&") {
    receptorBloque(true);
  } else if (appMode == "$") {
    if (initialNodeType == "Receptor") {
      ecoTransmition(true);
    } else if (charSent == textSent) {
      ecoTransmition(true);
    } else {
      ecoTransmition(false);
    }
  } else {
    Serial.println("[Recibe] Canal " + appMode + ": " + textSent);


  }
}

void receptorBloque(boolean context) {
  if (context) {
    Serial.println("CheckSum 16 = " + checkSumCounter);
  } else {
    Serial.println("Character Amount = " + characterCounter);
  }
}