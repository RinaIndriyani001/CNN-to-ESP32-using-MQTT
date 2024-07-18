#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 25
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// WiFi
const char *ssid = "Testing";
const char *password = "rinasukses";
//const int oneWireBus = 4;
int suhuMin, suhuMax;
int payloadTemp = -1; // Inisialisasi dengan nilai -1 untuk menunjukkan bahwa belum ada payload yang diterima
bool payloadReceived = false; // Flag untuk mengecek apakah payload sudah diterima
int fan = 27;
int heater = 14;

// MQTT Broker
const char *mqtt_broker = "xxxx";
const char *topic = "AIumur";
const char *mqtt_username = "yyyy";
const char *mqtt_password = "zzzz";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  pinMode(fan, OUTPUT);
  pinMode(heater, OUTPUT);
  Serial.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();
  
  // Connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
      lcd.setCursor(0,0);
      lcd.print("Connecting to WiFi");
  }
  Serial.println("Connected to the Wi-Fi network");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");


  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
      String client_id = "AI-Esp32";
      Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public EMQX MQTT broker connected");
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("MQTT Connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
    }
  client.subscribe(topic);
}
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  
  payloadTemp = 0; // Reset payloadTemp
  
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
    payloadTemp += (char)payload[i] - '0'; // Convert char to int
  }
  payloadReceived = true; // Set flag to true indicating payload has been received
  Serial.println();
  Serial.println("-----------------------");
  lcd.setCursor(0, 1);
  lcd.print("Klasifikasi: ");
  lcd.print(payloadTemp);
}

void loop() {
  int temp = dht.readTemperature();
  Serial.print(temp);
  Serial.println("C");
  client.loop();
  lcd.setCursor(0, 2);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" C");
  

  if (payloadReceived) { // Lakukan logika suhu hanya jika payload telah diterima
   umur();

   if(temp >= suhuMin && temp <= suhuMax)
   {
     digitalWrite(fan, LOW);
     digitalWrite(heater, LOW);
     Serial.println("Suhu sesuai target");
     lcd.setCursor(0, 3);
     lcd.print("                    ");
     lcd.setCursor(0, 3);
     lcd.print("Suhu sesuai");
   }
   else if(temp > suhuMax) 
   {
     digitalWrite(fan, HIGH);
     digitalWrite(heater, LOW);
     Serial.println("Suhu terlalu panas");
     lcd.setCursor(0, 3);
     lcd.print("                    ");
     lcd.setCursor(0, 3);
     lcd.print("Suhu Terlalu Panas ");
   }
   else if(temp < suhuMin)
   {
     digitalWrite(fan, LOW);
     digitalWrite(heater, HIGH);
     Serial.println("Suhu terlalu dingin");
     lcd.setCursor(0, 3);
     lcd.print("                    ");
     lcd.setCursor(0, 3);    
     lcd.print("Suhu Terlalu Dingin ");
   }
   delay(2000);
  }
 }

void umur() {
  switch(payloadTemp){
    case 1:
      suhuMin = 30;
      suhuMax = 32;
    break;
    case 2:
      suhuMin = 28;
      suhuMax = 30;
    break;
    case 3:
      suhuMin = 25;
      suhuMax = 27;
    break;
    case 4:
      suhuMin = 22;
      suhuMax = 24;
    break;
  }
}
