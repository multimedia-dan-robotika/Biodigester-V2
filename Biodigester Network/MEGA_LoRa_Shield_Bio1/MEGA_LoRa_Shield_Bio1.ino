#include <Wire.h>              // Library Wire
#include <LiquidCrystal_I2C.h> // Library LCD 16x2 I2C
#include <DS3231.h>            // Library DS3231
#include <SPI.h>               // Library SPI
#include <LoRa.h>              // Library Lora
#include <MQ135.h>             // Library MQ135
#include <max6675.h>           // Library max6675

#define methaneMQ2          A8
#define pressureSensor1     A9
#define pressureSensor2     A10
#define phSensor            A11
#define carbonMQ135         A12

const byte tempSCK          = 24;
const byte tempCS           = 26;
const byte tempSO           = 28;

const byte strobeRelay      = 31;
const byte valveRelay       = 33;
const byte motorRelay       = 35;
const byte pumpRelay        = 37;

String flag                 = "B1";

int methaneGas              = 0;

int pressureAnalog1;
int pressureAnalog2;
float pressure1;                        //TEKANAN TANGKI 1
float pressure2;                        //TEKANAN TANGKI 2

int temperatureValue;                   //TEMPERATUR TANGKI 1

float phAnalog;
float finalphValue          = 0.0;      //PH TANGKI 1

String loraMessage = "";

MAX6675 thermocouple(tempSCK, tempCS, tempSO);

MQ135 CO2Sensor (carbonMQ135);

LiquidCrystal_I2C lcd1 = LiquidCrystal_I2C (0x27, 20, 4);
LiquidCrystal_I2C lcd2 = LiquidCrystal_I2C (0x25, 20, 4);
DS3231 rtc(SDA,SCL);
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

   if (!LoRa.begin (915E6))
    {
        Serial.println ("LoRa Gagal Dimulai");
        while (1);
    }
    
  rtc.begin();
  lcd1.init();
  lcd2.init();
  lcd1.backlight();
  lcd2.backlight();

  // LCD 1

  lcd1.setCursor(0, 0);
  lcd1.print("Sistem Biodigester");

  lcd1.setCursor(0, 1);
  lcd1.print("Tekanan 1  : ");
  lcd1.setCursor(17, 1);
  lcd1.print("Bar");

  lcd1.setCursor(0, 2);
  lcd1.print("Tekanan 2  : ");
  lcd1.setCursor(17, 2);
  lcd1.print("Bar");

  lcd1.setCursor(0, 3);
  lcd1.print("Temperature: ");
  lcd1.setCursor(18, 3);
  lcd1.print((char)223);
  lcd1.print("C");

  // LCD 2

  lcd2.setCursor(0, 0);
  lcd2.print("Tanggal : ");
  lcd2.setCursor(0, 1);
  lcd2.print("PH      : ");
  lcd2.setCursor(0, 2);
  lcd2.print("CO2     : ");
  lcd2.setCursor(17, 2);
  lcd2.print("PPM");
  lcd2.setCursor(0, 3);
  lcd2.print("Metana  : ");
  lcd2.setCursor(17, 3);
  lcd2.print("PPM");

   pinMode (methaneMQ2, INPUT);        //MQ2 Sensor
   pinMode (pressureSensor1, INPUT);   //Pressure Sensor 1
   pinMode (pressureSensor2, INPUT);   //Pressure Sensor 2
   pinMode (phSensor, INPUT);          //pH Sensor
   pinMode (carbonMQ135, INPUT);       //MQ135 Sensor

   pinMode (strobeRelay, OUTPUT);      //Strobe Lamp Relay
   pinMode (valveRelay, OUTPUT);       //Valve Relay
   pinMode (motorRelay, OUTPUT);       //AC Motor Relay 
}

void loop() {
  
    methaneGas = analogRead (methaneMQ2);
    float CH4Voltage = methaneGas * (5.0 / 1023.0);
    int methanePPM = (15000 / 1024) * ((CH4Voltage / 5) * 1024);
    
    int carbonPPM = CO2Sensor.getPPM();
    
    pressureAnalog1     = analogRead (pressureSensor1);
    pressureAnalog2     = analogRead (pressureSensor2);
    pressure1           = pressure (pressureAnalog1);
    pressure2           = pressure (pressureAnalog2);
    
    temperatureValue = thermocouple.readCelsius();

    phAnalog = analogRead (phSensor);
    finalphValue = (0.002888143 * phAnalog)-0.50;

    if ((finalphValue >= 6.5 && finalphValue <= 7.5)  || (temperatureValue >= 20 && temperatureValue <= 30))
    {
        digitalWrite (valveRelay, LOW);
    }
    else
    {
        digitalWrite (valveRelay, HIGH);
    }

    if (pressure1 >= 0.4)
    {
        digitalWrite (strobeRelay, LOW);
    }
    else
    {
        digitalWrite (strobeRelay, HIGH);
    }

    Serial.println ("C6");
    
    lcd1.setCursor (12, 1);
    lcd1.print (pressure1);
    //Serial.println (press1);

    lcd1.setCursor (12, 2);
    lcd1.print (pressure2);
    //Serial.print (press2);

    lcd1.setCursor (14, 3);
    lcd1.print (int (temperatureValue));
    //Serial.println (temperatureValue);
    lcd2.setCursor(10, 0);
    lcd2.print(rtc.getDateStr());
    
    lcd2.setCursor (10, 1);
    lcd2.print (finalphValue);
    //Serial.println (finalphValue);

    lcd2.setCursor (10, 2);
    lcd2.print ("       ");
    lcd2.setCursor (10, 2);
    lcd2.print (carbonPPM);
    //Serial.println (carbonPPM);
    
    lcd2.setCursor (10, 3);
    lcd2.print ("       ");
    lcd2.setCursor (10, 3);
    lcd2.print (methanePPM);    
    //Serial.println (methanePPM);
    
    LoRa.beginPacket();
    LoRa.print(flag);
    LoRa.print("#");
    LoRa.print(String (methanePPM));
    LoRa.print("#");
    LoRa.print(String (pressure1));
    LoRa.print("#");
    LoRa.print(String (pressure2));
    LoRa.print("#");
    LoRa.print(String (finalphValue));
    LoRa.print("#");
    LoRa.print(String (carbonPPM));
    LoRa.print("#");
//    LoRa.print(String (vol));
//    LoRa.print("#");
    LoRa.print(String (temperatureValue));
    LoRa.print("#");
//    LoRa.print(String (liquidLevel));
//    LoRa.print("#");
    LoRa.endPacket();
    
    delay (1000);
    
    Serial.println ("C7");
}

float pressure (int pressureValue)
{
    float voltage = (pressureValue*5.0)/1024.0;
    float pressurePascal = (3.0*((float)voltage-0.47))*1000000.0;
    float pressureBar = pressurePascal/10e5;
    return pressureBar; 
}
