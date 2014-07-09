// fscontroller.ino
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "math.h"
#include "Quadrature.h"

Quadrature quad1(3, 2);
Quadrature quad2(5, 4);
Quadrature quad3(7, 6);

#define I2C_ADDR      0x3f // I2C address of PCF8574A
#define BACKLIGHT_PIN 3
#define En_pin        2
#define Rw_pin        1
#define Rs_pin        0
#define D4_pin        4
#define D5_pin        5
#define D6_pin        6
#define D7_pin        7

#define WINDOWS       3

int CodeIn;// used on all serial reads
int KpinNo; 
int Koutpin;

// Rotary variables
int R;// first rotary
int Rold;// the old reading
int Rdif;// the difference since last loop
int R2;// second rotary
int R3;// third rotary
int Rold2;// a second loop old reading
int Rdif2; // the second test difference
int Rold3;// a third loop old reading
int Rdif3; // the third test difference

// Menu variables
uint8_t mainActiveCursor; //Control which menu to activate
boolean firstRotaryPressed;
boolean gotData;

//Autopilot variables
int autopilotCursor;
String hdg;
String alt;
String ias;
String crs;
String vsp;
String apmode;

//Radio variables
int radioCursor;
String com1mhz;
String com1stb;
String com2mhz;
String com2stb;
String nav1mhz;
String nav1stb;
String nav2mhz;
String nav2stb;

String KoldpinStateSTR, KpinStateSTR, Kstringnewstate,Kstringoldstate;

LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin, BACKLIGHT_PIN, POSITIVE);

void setup() {
 //Switch the backlight on (this doesn't work....)
 pinMode ( BACKLIGHT_PIN, OUTPUT );
 digitalWrite ( BACKLIGHT_PIN, HIGH );
 
 lcd.begin(20,4);
 lcd.clear();
 lcd.home();
 lcd.setCursor(6,0);
 lcd.print("Lcd MCP");
 lcd.setCursor(1,1);
 lcd.print("by Sebastian Vidal");
 lcd.setCursor(3,2);
 lcd.print("v0.2 - AP + RADIO");
 delay(500);
 lcd.setCursor(0,4);
 lcd.print("Loading");

 mainActiveCursor = 0;
 firstRotaryPressed = false;
 gotData == false;

 // Initialize autopilot variables
 autopilotCursor = 0;
 hdg = "000";
 alt = "00000";
 ias = "000";
 crs = "000";
 vsp = "+0000";
 apmode = "NAV";

 // Initialize radio variables
 radioCursor = 0;
 com1mhz = "000.00";
 com1stb = "000.00";
 com2mhz = "000.00";
 com2stb = "000.00";
 nav1mhz = "000.00";
 nav1stb = "000.00";
 nav2mhz = "000.00";
 nav2stb = "000.00";

 //lcd.clear();
 //printAutopilot();

 for (int KoutPin = 8; KoutPin < 20; KoutPin++)// Get all the pins ready for "Keys"  
  {
    pinMode(KoutPin, INPUT);
    digitalWrite(KoutPin, HIGH);  
  }
 Serial.begin(115200);

}

void printData(String a, String b, String c, String d){
  lcd.clear();
  printText(0,0,a);
  printText(0,1,b);
  printText(0,2,c);
  printText(0,3,d);
}
void printText(int col, int line, String text){
    lcd.setCursor(col,line);
    lcd.print(text);
}

void printAutopilot(){
  String line1 = " HDG ";
  line1 += hdg;
  line1 += "  ALT ";
  line1 += alt;
  String line2 = " CRS ";
  line2 += crs;
  line2 += "  VSP ";
  line2 += vsp;
  String line3 = " IAS ";
  line3 += ias;
  line3 += "  MODE ";
  line3 += apmode; 
    printData(line1, line2, line3, "");
    chooseItem(0);
}

String setRadioLine(String title, String active, String standby){
  String aux = title;
  aux += "  ";
  aux += active;
  aux += "/";
  aux += standby;
  return aux;
}
void printRadio(){
  String line1 = setRadioLine("COM1",com1mhz,com1stb);
  String line2 = setRadioLine("COM2",com2mhz,com2stb);
  String line3 = setRadioLine("NAV1",nav1mhz,nav1stb);
  String line4 = setRadioLine("NAV2",nav2mhz,nav2stb);
  printData(line1, line2, line3, line4);
  chooseItem(0);
}
 
void printInfo(){

}
void printDebug(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(com1mhz);
}

void moveMenu(int direction) {
  uint8_t position;
  mainActiveCursor = (mainActiveCursor + direction + WINDOWS) % WINDOWS;
  switch (mainActiveCursor) {
      case 0: // Autopilot menu
        printAutopilot();
        break;
      case 1: // Radio menu
        printRadio();
        break;
      case 2:
        printDebug();
        break;
      default:
        // do something
        break;
  }
  /*printMenu();
  lcd.setCursor(position,0);
  lcd.print(char(126));
  lcd.setCursor(0,2);           //Testing
  lcd.print(String(position));        //Testing
  lcd.setCursor(0,3);           //Testing
  lcd.print(String(mainActiveCursor));*/  //Testing
}

void chooseItem(int direction){
  switch (mainActiveCursor) {
      case 0:
        lcd.setCursor(19,autopilotCursor); lcd.print(" ");
        autopilotCursor = (autopilotCursor + direction + 4) % 4;
        lcd.setCursor(19,autopilotCursor); lcd.print(char(127));
        break;
      case 1:
        lcd.setCursor(19,radioCursor); lcd.print(" ");
        radioCursor = (radioCursor + direction + 4) % 4;
        lcd.setCursor(19,abs(radioCursor)); lcd.print(char(127));
        break;
      case 2:
       
        break;
      default:
        // do something
        break;
  }

}

void firstRotary(int direction){
  if (firstRotaryPressed == false){
    moveMenu(direction);
  } else {
    chooseItem(direction);
  }
}

void sendRotaryControl(int direction, String pos, String neg, String push){
  if (direction == 1 and pos != "") { Serial.println(pos); } else {
    if (direction == -1 and neg != "") { Serial.println(neg); } else {
      if (direction == 0 and push != "") { Serial.println(push); }
    }
  }
}

void secondRotary(int direction){
  switch (mainActiveCursor) {
      case 0:
        if (autopilotCursor == 0) { sendRotaryControl(direction,"A57","A58","B04"); }
        if (autopilotCursor == 1) { sendRotaryControl(direction,"A56","A55","B10"); }
        if (autopilotCursor == 2) { sendRotaryControl(direction,"B15","B16","B26"); }
        break;
      case 1:
        if (autopilotCursor == 0) { sendRotaryControl(direction,"A02","A01","A06"); }
        if (autopilotCursor == 1) { sendRotaryControl(direction,"A08","A07","A12"); }
        if (autopilotCursor == 2) { sendRotaryControl(direction,"A14","A13","A18"); }
        if (autopilotCursor == 3) { sendRotaryControl(direction,"A20","A19","A24"); }
        break;
      case 2:
        break;
      default:
        // do something
        break;
  }
}
void thirdRotary(int direction){
  switch (mainActiveCursor) {
      case 0:
        if (autopilotCursor == 0) { sendRotaryControl(direction,"B11","B12","B05"); }
        if (autopilotCursor == 1) { sendRotaryControl(direction,"B13","B14","B04"); }
        if (autopilotCursor == 2) { sendRotaryControl(direction,"","","A54"); }
        break;
      case 1:
        if (autopilotCursor == 0) { sendRotaryControl(direction,"A04","A03","A45"); }
        if (autopilotCursor == 1) { sendRotaryControl(direction,"A10","A09","A46"); }
        if (autopilotCursor == 2) { sendRotaryControl(direction,"A16","A15","A18"); }
        if (autopilotCursor == 3) { sendRotaryControl(direction,"A22","A21","A24"); }
        break;
      case 2:
        break;
      default:
        // do something
        break;
  }
}

void readSerial(){
  if (Serial.available()) {// Check if serial data has arrived from PC
    CodeIn = getChar();
    if (CodeIn == '=') {EQUALS();} // The first identifier is "="
   // if (CodeIn == '<') {LESSTHAN();}// The first identifier is "<"
   // if (CodeIn == '?') {QUESTION();}// The first identifier is "?"
   // if (CodeIn == '/') {SLASH();}// The first identifier is "/" (Annunciators)
    gotData == true;
  } else {
    if (gotData == true) {
      moveMenu(0);
      gotData = false;
    }
  }
}


void loop() {
  {KEYS();}
  {ENCODER();}
  {readSerial();}
}

char getChar()// Get a character from the serial buffer
{
  while(Serial.available() == 0);// wait for data
  return((char)Serial.read());// Thanks Doug
}

String getString(int chars){
  String aux = "";
  for(int i=0; i<chars; i++){
    aux += getChar();
  }
  return aux;
}

void EQUALS(){
  String aux;
  CodeIn = getChar();
  switch (CodeIn) {
      case 'A':
        com1mhz = getString(6); if (mainActiveCursor == 1) { printText(6,0,com1mhz);}
        break;
      case 'B':
        com1stb = getString(6); if (mainActiveCursor == 1) { printText(13,0,com1stb);}
        break;
      case 'C':
        com2mhz = getString(6); if (mainActiveCursor == 1) { printText(6,1,com2mhz);}
        break;
      case 'D':
        com2stb = getString(6); if (mainActiveCursor == 1) { printText(13,1,com2stb);}
        break;
      case 'E':
        nav1mhz = getString(6); if (mainActiveCursor == 1) { printText(6,2,nav1mhz);}
        break;
      case 'F':
        nav1stb = getString(6); if (mainActiveCursor == 1) { printText(13,2,nav1stb);}
        break;
      case 'G':
        nav2mhz = getString(6); if (mainActiveCursor == 1) { printText(6,3,nav2mhz);}
        break;
      case 'H':
        nav2stb = getString(6); if (mainActiveCursor == 1) { printText(13,3,nav2stb);}
        break;
      case 'b':
        alt = getString(5); if (mainActiveCursor == 0) { printText(14,0,alt); }
        break;
      case 'c':
        vsp = getString(5); if (mainActiveCursor == 0) { printText(14,1,vsp); }
        break;
      case 'd':
        hdg = getString(3); if (mainActiveCursor == 0) { printText(5,0,hdg); }
        break;
      case 'e':
        crs = getString(3); if (mainActiveCursor == 0) { printText(5,1,crs); }
        break;
      case 'f':
        ias = getString(3); if (mainActiveCursor == 0) { printText(5,2,ias); }  
        break;
      case 'l':
        aux = getString(1); if (aux == "0") { apmode = "NAV"; } else { apmode = "GPS"; }
        if (mainActiveCursor == 0) { printText(15,2,apmode); }
        break;
      default:
        // do something
        break;
  }
}

void KEYS() 
{
  Kstringnewstate = "";
  for (int KpinNo = 8; KpinNo < 20; KpinNo++){
    KpinStateSTR = String(digitalRead(KpinNo)); 
    KoldpinStateSTR = String(Kstringoldstate.charAt(KpinNo - 8));
    if (KpinStateSTR != KoldpinStateSTR)
    {
      if (KpinNo != 13){
      Serial.print ("D"); 
      if (KpinNo < 10) Serial.print ("0");
      Serial.print (KpinNo);
      Serial.println (KpinStateSTR);
      }
      switch (KpinNo) {
          case 8:
            firstRotaryPressed = !firstRotaryPressed;
            break;
          case 9:
            if (KpinStateSTR == "0") { secondRotary(0); }
            break;
          case 10:
            if (KpinStateSTR == "0") { thirdRotary(0); }
            break;  
          default:
            // do something
            break;
      }
    }
    Kstringnewstate += KpinStateSTR;
  }
  Kstringoldstate = Kstringnewstate;
}

void ENCODER(){
R =(quad1.position()/2); //The /2 is to suit the encoder
  if (R != Rold) { // checks to see if it different
    (Rdif = (R-Rold));// finds out the difference
   if (Rdif == 1) firstRotary(1);
   if (Rdif == -1) firstRotary(-1);; 
   Rold = R; // overwrites the old reading with the new one.
  } 
  R2 =(quad2.position()/2);
  if (R2 != Rold2) {
    (Rdif2 = (R2-Rold2));
    if (Rdif2 == 1) secondRotary(1);; 
    if (Rdif2 == -1) secondRotary(-1);;
    Rold2 = R2;
  }
  R3 =(quad3.position()/2);
  if (R3 != Rold3) {
    (Rdif3 = (R3-Rold3));
    if (Rdif3 == 1) thirdRotary(1); 
    if (Rdif3 == -1) thirdRotary(-1); 
    Rold3 = R3;
  }
  
}
