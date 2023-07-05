#include "dht11.h"
dht11 DHT11;
#define DHT11PIN D6
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define WIFI_SSID "bssm_guest"
#define WIFI_PASSWORD "bssm_guest"
#define API_KEY "AIzaSyAyyRPdxywY1sPo1Q6HO69TXqFYbURrB44"
#define DATABASE_URL "https://arduino-a026d-default-rtdb.firebaseio.com/"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

float temp, humi;

const int buttonPin1 = D3;
const int buttonPin2 = D5;
const int buzzerPin = D4;
int readValue = 1;
int dangerValue = 0;

void setup() {
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(115200);

  // WiFi 연결 설정
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  // FirebaseConfig 객체 초기화
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  int chk = DHT11.read(DHT11PIN);
  humi = DHT11.humidity;
  temp = DHT11.temperature;

  int buttonState = digitalRead(buttonPin1);
  int buttonValue = digitalRead(buttonPin2);

  if (readValue == 1 && buttonState == 1) {
    readValue = 0;
    delay(1000);
    Serial.println(readValue);
  }
  else if (readValue == 0 && buttonState == 1) {
    readValue = 1;
    delay(1000);
  }
if (readValue == 0 && humi < 70) {
    tone(buzzerPin, 1000);
    if (Firebase.RTDB.setString(&fbdo, "/DHT11/miss", 4)) {
      Serial.println("humi PASSED");
    }
    else {
      Serial.println("REASON: " + fbdo.errorReason());
    }

  }
  else {
    noTone(buzzerPin);
    if (Firebase.RTDB.setString(&fbdo, "/DHT11/miss", 5)) {
      Serial.println("humi PASSED");
    }
  }

  if (dangerValue == 0 && buttonValue == 1) {
    dangerValue = 1;
    delay(1000);
    Serial.println(buttonValue);
    if (Firebase.RTDB.setString(&fbdo, "/DHT11/danger", 1)) {
      Serial.println("humi PASSED");
    }
  }
  else if (dangerValue == 1 && buttonValue == 1) {
    dangerValue = 0;
    delay(1000);
    Serial.println(readValue);

    if (Firebase.RTDB.setString(&fbdo, "/DHT11/danger", 0)) {
      Serial.println("humi PASSED");
    }
    else {
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
  if (Firebase.ready()) {
    if (Firebase.RTDB.getString(&fbdo, "/DHT11/CallArduino")) {
      String rValue = fbdo.stringData();
      if (rValue == "2")       {  tone(buzzerPin, 1000);   }
      else {noTone(buzzerPin);  }
    }
  }
}