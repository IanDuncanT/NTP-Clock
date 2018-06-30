//LIBRARIES
#include <arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <LiquidCrystal.h>

//SOFTWARE VARIABLES
int timeBeforeUpdate = 30; //SECONDS BETWEEN NTP UPDATES
char ssid[] = "SSID";
char pass[] = "PASSWORD";
char hostname[] = "NTP-CLOCK";
int HH = -5; //UTC HOURS DIFFERENCE
int MM = 0; //UTC MINUTES DIFFERENCE
unsigned int localPort = 2390;
IPAddress timeServerIP;

//ALARMS
const int alarmPin = 12;

//TIME SERVERS
const char* ntpServerName = "time.nist.gov";
//const char* ntpServerName = "pool.ntp.org";

//GLOBAL VARIABLES
int ss;
int mm;
int hh;
int yy;
int da;
int mo;
int dayOfWeek;
const unsigned long seventyYears = 2208988800UL;
unsigned long epochLocal;
unsigned long secsSince1900;
const int NTP_PACKET_SIZE = 48;

//INIT PACKET AND LCD
byte packetBuffer[ NTP_PACKET_SIZE];
WiFiUDP udp;
LiquidCrystal lcd(14, 2, 0, 4, 5, 16);

//==============================================================
//                           PROGRAM
//==============================================================

void morningAlarm(){
  tone(12, 440);
  Serial.println("Alarm Started");
  Alarm.delay(200);
  noTone(12);
}

time_t sendNTPpacket(IPAddress& address) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  udp.beginPacket(address, 123);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
  Alarm.delay(1000);
  int cb = udp.parsePacket();
  if (!cb){
    return 0;
    Alarm.delay(1000);
  }
  else{
    udp.read(packetBuffer, NTP_PACKET_SIZE);
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    secsSince1900 = highWord << 16 | lowWord;
    return secsSince1900 - seventyYears;
  }
}

void displayUpdate(int hh, int mm, int ss, int yy, int dayOfWeek, int da, int mo){
  lcd.clear();
  //YEAR
  lcd.setCursor(12, 0);
  lcd.print(yy);
  //DAY OF WEEK
  lcd.setCursor(0, 0);
  switch(dayOfWeek){
      case 0:
      lcd.print("Sun");
      break;
      case 1:
      lcd.print("Mon");
      break;
      case 2:
      lcd.print("Tue");
      break;
      case 3:
      lcd.print("Wed");
      break;
      case 4:
      lcd.print("Thu");
      break;
      case 5:
      lcd.print("Fri");
      break;
      case 6:
      lcd.print("Sat");
      break;
   }
  //SECONDS
  lcd.setCursor(6, 1);
  if(ss < 10) {
    lcd.print("0");
    lcd.setCursor(7, 1);
  }
  lcd.print(ss, DEC);
  //MINUTES
  lcd.setCursor(3, 1);
  if(mm < 10) {
    lcd.print("0");
    lcd.setCursor(4, 1);
  }
  lcd.print(mm, DEC);
  lcd.setCursor(5, 1);
  lcd.print(":");
  //HOURS
  lcd.setCursor(0, 1);
  if(hh < 10) {
    lcd.print("0");
    lcd.setCursor(1, 1);
  }
  lcd.print(hh, DEC);
  lcd.setCursor(2, 1);
  lcd.print(":");
  //DAY
  lcd.setCursor(8, 0);
  if(da < 10) {
    lcd.print("0");
    lcd.setCursor(9, 0);
  }
  lcd.print(da);
  lcd.setCursor(10, 0);
  lcd.print(",");
  //MONTH
  lcd.setCursor(4, 0);
  switch(mo){
    case 1:
    lcd.print("Jan");
    break;
    case 2:
    lcd.print("Feb");
    break;
    case 3:
    lcd.print("Mar");
    break;
    case 4:
    lcd.print("Apr");
    break;
    case 5:
    lcd.print("May");
    break;
    case 6:
    lcd.print("Jun");
    break;
    case 7:
    lcd.print("Jul");
    break;
    case 8:
    lcd.print("Aug");
    break;
    case 9:
    lcd.print("Sep");
    break;
    case 10:
    lcd.print("Oct");
    break;
    case 11:
    lcd.print("Nov");
    break;
    case 12:
    lcd.print("Dec");
    break;
  }
}

void timeCarry(){
  if(ss > 59){
    mm += 1;
    ss -= 60;
  }
  if(mm > 59){
    hh += 1;
    mm -= 60;
  }
  if(hh > 23){
    dayOfWeek += 1;
    hh-=24;
  }
}

void timeUpdate() {
  for(int i= -1; i<timeBeforeUpdate; i++){
    ss ++;
    timeCarry();
    displayUpdate(hh, mm, ss, yy, dayOfWeek, da, mo);
    Alarm.delay(1000 - (1000 / timeBeforeUpdate));
  }
}

void secondsToVariables(time_t secs){
  time_t epochLocal = secs + (HH * 3600)+(MM * 60);
  ss = second(epochLocal);
  mm = minute(epochLocal);
  hh = hour(epochLocal);
  yy = year(epochLocal);
  da = day(epochLocal);
  mo = month(epochLocal);
  dayOfWeek = (((epochLocal / 86400) + 4) % 7);
}

void setup(){
  //INIT
  Alarm.alarmRepeat(9,45,0, morningAlarm);
  Serial.begin(115200);
  Serial.println("Serial Started");
  lcd.begin(16, 2);
  lcd.setCursor(4, 0);
  lcd.print("Welcome!");
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED){
    Alarm.delay(500);
  }
  Alarm.delay(2500);
  ArduinoOTA.onStart([](){
    Serial.println("Start");

  });
  ArduinoOTA.onEnd([](){
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    lcd.setCursor(0,0);
    if (error == OTA_AUTH_ERROR) lcd.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) lcd.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) lcd.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) lcd.println("Receive Failed");
    else if (error == OTA_END_ERROR) lcd.println("End Failed");
    Alarm.delay(2500);
  });
  ArduinoOTA.begin();
  udp.begin(localPort);
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("WiFi");
  lcd.setCursor(2, 1);
  lcd.print("Connected");
  Alarm.delay(2500);
}

void loop(){
  ArduinoOTA.handle();
  WiFi.hostByName(ntpServerName, timeServerIP);
  time_t secs = sendNTPpacket(timeServerIP);
  secondsToVariables(secs);
  timeUpdate();
}
