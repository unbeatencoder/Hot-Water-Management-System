// Include the libraries we need
#include <time.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>                                                    // esp8266 library
#include <FirebaseArduino.h>                                                // firebase library

#define FIREBASE_HOST "iot-data-aeed1.firebaseio.com"          
#define FIREBASE_AUTH "qDhmUXHyzTa6p2SZRVpF2YlSzhfZCKTPBDVuBfUS"    

#define WIFI_SSID "DEEPESH"                
#define WIFI_PASSWORD "987123456"           

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

String geyserStatus = "";
String valStatus = "";
String prio="";
String tmp = "";

int timezone = 19800+1800;
int dst = 0;
int gey = D3;
int val = D5;
int gg = -1;
int mm = 0;
int tt=0;

void setup(void)
{
  Serial.begin(9600);
  pinMode(gey, OUTPUT);
  pinMode(val,OUTPUT);
  delay(100);
  
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  
  Serial.print("Connectng to ");
  Serial.print(WIFI_SSID);
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);
  if(Firebase.failed())
  {
    Serial.println("Failed....!");
  }
  
  Firebase.setString("/GEYSER_STATUS/EMID", "OFF");
  Firebase.setString("/VAL_STATUS/EMID", "OFF"); 
  Firebase.setString("/PRIORITY_VAL_STATUS/EMID","OFF");
  
  configTime(timezone,dst,"pool.ntp.org","time.nist.gov");
  Serial.println("waiting for INternet time");

  while(!time(nullptr))
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\ntime response ok");
  
  sensors.begin();

}

void send_data()
{
  sensors.requestTemperatures(); 
  float tempC = sensors.getTempCByIndex(0);
  String temp = String(tempC);
  delay(100);
  if(tempC != DEVICE_DISCONNECTED_C) 
  {
    Serial.print("Temperature is: ");
    Serial.println(tempC);
    Firebase.pushString("/ARCHIVES/EMID/measures",temp);
    delay(100);
    Firebase.setString("/DATA/EMID/Current", temp);
    delay(100);
    if(Firebase.failed())
    {
      Serial.println("Failed....!");
      Serial.println(Firebase.error());
    }
  } 
  else
  {
    Serial.println("Error: Could not read temperature data");
  }
  delay(50);
}

void valve()
{
 valStatus = Firebase.getString("/VAL_STATUS/EMID");
 tmp = Firebase.getString("/PRIORITY_VAL_STATUS/EMID");
 if(tmp=="ON")
 {
  if (valStatus == "ON") 
  {
    digitalWrite(val,HIGH);
  }
 }
 if (valStatus == "OFF") 
 {
   digitalWrite(val,LOW);
 }
}

void geyser_start()
{
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  geyserStatus = Firebase.getString("/GEYSER_STATUS/EMID");
  
  if(geyserStatus=="OFF")
  {
   digitalWrite(gey,LOW); 
  }
  else if(geyserStatus=="ON")
  {
    digitalWrite(gey,HIGH);
  }
  delay(50);
  
  int h = p_tm->tm_hour;
  int m = p_tm->tm_min;
  h=h-1;
  m=m-30;
  if(m<0)
    m=m+60;
  int hh = Firebase.getInt("/schedule/EMID");
  int hhh = (hh+2)%24;
  Serial.print(h);
  Serial.print(" ");
  Serial.println(m);
  
  if(h>=hh && h <hhh && tt==0)
  {
   digitalWrite(gey,HIGH);
   Firebase.setString("/GEYSER_STATUS/EMID","ON");
   Firebase.setString("/PRIORITY_VAL_STATUS/EMID","OFF"); 
   tt = -1;
  }
  else if(h>=hhh && tt==-1)
  {
   tt=0;
   digitalWrite(gey,LOW);
   Firebase.setString("/GEYSER_STATUS/EMID","OFF"); 
   Firebase.setString("/PRIORITY_VAL_STATUS/EMID","OFF");
  }
  delay(50);
  if(h<hh || h>=hhh)
  {
    digitalWrite(gey,LOW);
    Firebase.setString("/GEYSER_STATUS/EMID","OFF"); 
  }
  delay(50);
}

void priority()
{
  prio = Firebase.getString("/PRIORITY_VAL_STATUS/EMID");
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  int m = p_tm->tm_min;
  Serial.println(prio);
  if(prio=="ON" && gg == -1)
  {
    gg = m;
    mm = gg+5;
    mm = mm%60;
    Serial.println(mm);
  }
  else if(prio=="ON" && gg!=-1)
  {
    if(m>=mm)
    {
      gg = -1;
      mm = 0;
      Firebase.setString("/PRIORITY_VAL_STATUS/EMID","OFF");
      Serial.println("okay!!");
      delay(10);
    }
  }
  else if(prio=="OFF")
  {
    gg = -1;  
    mm = 0;
    Firebase.setString("/PRIORITY_VAL_STATUS/EMID","OFF");
    delay(10);
  }
}
void loop(void)
{ 
  send_data();
  delay(50);
  geyser_start();
  delay(50);
  valve();
  delay(50);
  priority();
  delay(50);
}
