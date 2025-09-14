//Single board has 3 outputs for heating and cooling, 2 thermometer buses, 2 analog temperature pots and 1 output for the pump control
//From left to right at the top of the board, 1..3 are heat1, heat2, cool1 and pump
//The pump needs external circuit to regulate the input voltage. But, I could also change the code to use the second pot as a voltage setter and use pump output as a PWM output.But, I don't know how the gate drive will hold

#include <OneWire.h>
#include <DallasTemperature.h>
#include <LCD_I2C.h>
#include <WiFi.h>

LCD_I2C lcd(0x27, 20, 4);  


#define BUS_TERMO1 39
#define BUS_TERMO2 34


int runtime;			 //Timer for LCD
//Wroom32 pinout https://lastminuteengineers.com/esp32-wroom-32-pinout-reference/
//GPIO pins 2 and 5 are strapping pins, can't be used
const int heat1 = 25;  
const int heat2 = 33;  
const int heat3 = 32; 
const int heat4 = 4;

const int tempThreshold1Pin = 13; 
const int tempThreshold2Pin = 12; //D12 trebuie conectat la pinul 2 care trebuie taiat

const int analogPowerPin = 23;

int temp1Threshold = 0;
int temp2Threshold = 0;

OneWire oneWire(BUS_TERMO1);
OneWire oneWire2(BUS_TERMO2);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
DallasTemperature sensors2(&oneWire2);


void setup() {

  WiFi.disconnect(true);  // Disconnect WiFi
  WiFi.mode(WIFI_OFF);    // Turn off WiFi

  lcd.begin(21,22); //sda,scl
  lcd.backlight();

  pinMode(heat1, OUTPUT);
  pinMode(heat2, OUTPUT);
 
  pinMode(heat3, OUTPUT);
  pinMode(heat4, OUTPUT);

  pinMode(analogPowerPin,OUTPUT);
  pinMode(tempThreshold1Pin,INPUT);
  pinMode(tempThreshold2Pin,INPUT);

  outputsOff();
  digitalWrite(analogPowerPin,LOW);
}

unsigned long previousMillis1=0;
unsigned long timeStampPumpOn=0;
unsigned long timeStampPumpOff=0;
int i=0;

char displayBuffer[80]="";
char temp1Buffer[20];
char temp2Buffer[20];


float temp1=0;
float temp2=0;
float temp3=0;
float temp4=0;

void outputsOff() {
    digitalWrite(heat1,LOW);digitalWrite(heat2,LOW);

    digitalWrite(heat3,LOW);
    digitalWrite(heat4,LOW);
}


void Heaters1On()
{
   digitalWrite(heat1,HIGH);
   digitalWrite(heat2,HIGH);
}

void Heaters1Off()
{
   digitalWrite(heat1,LOW);
   digitalWrite(heat2,LOW);
}

void Heaters2On()
{
   digitalWrite(heat3,HIGH);
   digitalWrite(heat4,HIGH);
}
void Heaters2Off()
{
   digitalWrite(heat3,LOW);
   digitalWrite(heat4,LOW);
}

//this is with a 100k resistor, so it needs to change
int getAnalogTemp(int reading)
{
  float tempThresh= (reading-1309)/(4096.0-1309.0);
  if(tempThresh<0) tempThresh=0;
  int tempAdd = 60*tempThresh;
  return 40+tempAdd;
}

bool startedTermometers = false;

void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis1 >= 3000) {
    digitalWrite(analogPowerPin,HIGH); //we power up the analog sensors

   if(startedTermometers==false) {
      sensors.begin(); 
      sensors2.begin();
      startedTermometers = true;
   }
    
    //Read the temperature thresholds
    temp1Threshold = analogRead(tempThreshold1Pin);
    temp1Threshold = getAnalogTemp(temp1Threshold);

    temp2Threshold = analogRead(tempThreshold2Pin);
    temp2Threshold = getAnalogTemp(temp2Threshold);

    previousMillis1 = currentMillis;

    sensors.requestTemperatures();
    sensors2.requestTemperatures(); 

    temp1=sensors.getTempCByIndex(0);
    temp2=sensors2.getTempCByIndex(0);

    sprintf(temp1Buffer,"%3.1f",temp1);
    sprintf(temp2Buffer,"%3.1f",temp2);
    
    memset(displayBuffer,0,80);
    sprintf(displayBuffer,"T1: %s T2: %s",temp1Buffer,temp2Buffer);
    displayBuffer[20]=0; //don't wrap on next line
    lcd.setCursor(0, 0); 
    lcd.print(displayBuffer);
    memset(displayBuffer,0,80);

    //print the temperature thresholds
     sprintf(temp1Buffer,"%d",temp1Threshold);
     sprintf(temp2Buffer,"%d",temp2Threshold);
     memset(displayBuffer,0,80);
     sprintf(displayBuffer,"Set1-2: %s %s",temp1Buffer,temp2Buffer);
     lcd.setCursor(0,1);
     lcd.print(displayBuffer);

//First thermostat control
      if(temp1>0)
      {
        if(temp1<temp1Threshold)
          {
            Heaters1On();
          }
          else {  //turn off the heating elements and start the vapor pump
              Heaters1Off();
          }
      }

//Second thermostat control
      if(temp2>0)
      {
        if(temp2<temp2Threshold)
          {
            Heaters2On();
          }
          else { 
              Heaters2Off();
          }
      }
  }
}