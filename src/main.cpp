#include <Arduino.h>
#include <Locomotive.h>
#include <WiFiManager.h>

#define MOTOR_PWM_PIN D1
#define MOTOR_DIR_PIN D3
#define WIFI_RESET_PIN D7
#define HALL_SENSOR_A_PIN D5
#define HALL_SENSOR_B_PIN D6

Locomotive locomotive(MOTOR_PWM_PIN, MOTOR_DIR_PIN, HALL_SENSOR_A_PIN, HALL_SENSOR_B_PIN);


void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(WIFI_RESET_PIN, INPUT_PULLUP);
  delay(10);
  bool resetWiFi = !digitalRead(WIFI_RESET_PIN);
  Serial.print("WiFiConfig -> Reset WiFi button pushed? : ");
  if(resetWiFi)
  {
    Serial.println("YES");
  }
  else
  {
    Serial.println("NO");
  }

  String apName = "Locomotive_";
  apName += ESP.getChipId();

  //WiFiManager
  WiFiManager wifiManager;
  //reset saved settings
  if(resetWiFi)
  {
    wifiManager.resetSettings();
  }
  
  wifiManager.autoConnect(apName.c_str());
  //if you get here you have connected to the WiFi
  Serial.println("WiFi is connected!");

  locomotive.begin(3377, 3377);
}

void loop()
{
  locomotive.update(millis());
}