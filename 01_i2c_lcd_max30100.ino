//Blynk Files
#define BLYNK_TEMPLATE_ID "TMPL_YZkiuXh"
#define BLYNK_DEVICE_NAME "Patients HeartBeat and SPO2"
#define BLYNK_AUTH_TOKEN "Xt9-gNZi9kyzMe-98_VY9Vv73Zy75dGX"

//LCD
#include <Wire.h> //This library help to communicate with I2C device.
#include <LiquidCrystal_I2C.h> //for better communication with display.
LiquidCrystal_I2C lcd(0x27,16,2);//The address is 0x27. . Number of columns is 16 and number of rows is 2.

//Headers
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#define BLYNK_PRINT Serial
#include <Blynk.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "Wire.h"


//Wifi Data
char auth[] = "Xt9-gNZi9kyzMe-98_VY9Vv73Zy75dGX";             // Authentication Token Sent by Blynk
char ssid[] = "IOT CUET";        //WiFi SSID
char pass[] = "C1234567";        //WiFi Password
 
// Connections : SCL PIN - D1 , SDA PIN - D2 , INT PIN - D0
//Max30100 Variables
PulseOximeter pox; 
int BPM, SpO2;
uint32_t tsLastReport = 0;


//Times
#define REPORTING_PERIOD_MS 2000
#define TIME_INTERVAL 2000                      //Time interval among sensor readings [milliseconds]

//Checking Conditions
#define check_status 5000
uint32_t tsLastCheck = 0;
int check_period=5;

//Alert Variables
int count=0;
#define bpm_low 60
#define bpm_high 100
#define spo2_low 92


//Buzzer
const int buzzer =  12;

//Beat Detection
void onBeatDetected()
{
    //Serial.println("Beat Detected!");
}


 
void setup()
{
    Serial.begin(115200);
    Blynk.begin(auth, ssid, pass);
    pinMode(buzzer, OUTPUT);
     lcd.init();
     lcd.setBacklight((uint8_t)1);

    //Max30100 Initialization
    Serial.print("Initializing Pulse Oximeter..");
    if (!pox.begin())
    {
         Serial.println("FAILED");
         for(;;);
    }
    else
    {
         Serial.println("SUCCESS");
         pox.setOnBeatDetectedCallback(onBeatDetected);
    }


}
 
void loop()
{
    pox.update();//Max30100 Updates
    Blynk.run();

    bpm_spo2();// Call Max30100 For data

    //Check Status After 5 second
     if (millis() - tsLastCheck > check_status)
    {
    alert();
    tsLastCheck = millis();
    }

}


//BPM and SPO2 Measurement Function
void bpm_spo2()
{
            BPM = pox.getHeartRate();
            SpO2 = pox.getSpO2();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS)
    {            
        Serial.print("Heart rate:");
        Serial.print(BPM);
        lcd.setCursor(0,0);
        lcd.print("BPM: ");
        lcd.print(BPM);
        lcd.print("   ");
        Serial.print(" SpO2:");
        Serial.print(SpO2);
        Serial.println(" %");

        lcd.setCursor(0,1);
        lcd.print("SPO2: ");
        lcd.print(SpO2);
        lcd.print("%");
 
        Blynk.virtualWrite(V5, BPM);
        Blynk.virtualWrite(V6, SpO2);

        tsLastReport = millis();
    }
}



void alert()
{
  String body ="None";//Email Body
  String Message1=String("Alert..Your Patients is in Critical Condition!!! Here is Details: ");
  String Message2=String("\n\n\nNormal Values are: ");
  String Message3=String("\nBPM: ")+bpm_low+" To "+ bpm_high;
  String Message4=String("\nSPO2 Above: ")+spo2_low+"%";
  String Message6=String("\n\nTake Action Please...Check your App for more information.");


if(BPM<bpm_low && BPM!=0||BPM>bpm_high||SpO2<spo2_low && SpO2!=0)
{
      count=count+1;
if(count==check_period){
  
//Value to Send email
String BPM_Level=String("\nBPM: ")+BPM;
String SPO2_Level=String("\nSPO2: ")+SpO2+"%";

body = Message1 + BPM_Level +SPO2_Level+Message2+Message3+Message4+Message6;

digitalWrite(buzzer, HIGH); //Buzz if Critical condition
    count=0;
}
}
//else { digitalWrite(buzzer, LOW);}



     if(body.equals("None")){Serial.println(body);}
      
     else
     {
       Serial.println(body);
       Blynk.logEvent("patients_condition",body);
       body ="None";
       lcd.clear();
       lcd.print("Please Relax!");
       
       Serial.println("----------------------------------");
     }
}
