#define BLYNK_TEMPLATE_ID   "user5"
#define BLYNK_TEMPLATE_NAME "user5@wyns.it"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 5   // Pin voor de DS18B20-temperatuursensor
#define ANALOG_POT_PIN  34  // Pin voor de analoge potentiometer
#define LED_PIN      18  // Pin voor de LED die de ketelstatus weergeeft

// Je Blynk-authenticatietoken
char auth[] = "yJ-uaPCTphCmQkj9v_MQEEoQqa9eW4jG";

// Je WiFi-informatie
char ssid[] = "embed";
char pass[] = "weareincontrol";

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int analogGewensteTemperatuur = 21;  // Set the default desired temperature to 21 degrees Celsius for analog potentiometer
int digitalGewensteTemperatuur = 21;  // Set the default desired temperature to 21 degrees Celsius for digital potentiometer
float gemetenTemperatuur;
bool ketelAan = false;
int lastUsedPotentiometer = 0;  // 0: No potentiometer used, 1: Analog potentiometer, 2: Digital potentiometer

BlynkTimer timer;

BLYNK_CONNECTED() {
  // Request the current state of the virtual pins
  Blynk.syncVirtual(V4, V5, V6);
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass, "server.wyns.it", 8081);
  sensors.begin();
  
  pinMode(ANALOG_POT_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  timer.setInterval(1000L, leesSensorenEnStuurBlynk);
}

void loop() {
  Blynk.run();
  timer.run();
}

void leesSensorenEnStuurBlynk() {
  // Lees gemeten temperatuur
  sensors.requestTemperatures();
  gemetenTemperatuur = sensors.getTempCByIndex(0);

  // Lees gewenste temperatuur van analoge potentiometer
  analogGewensteTemperatuur = map(analogRead(ANALOG_POT_PIN), 0, 4095, 0, 30);

  // Stuur gegevens naar Blynk
  Blynk.virtualWrite(V1, gemetenTemperatuur);
  Blynk.virtualWrite(V2, analogGewensteTemperatuur);
  Blynk.virtualWrite(V3, ketelAan);

  // Stuur gegevens naar lokale weergave (serial monitor)
  Serial.print("Gemeten temperatuur: ");
  Serial.print(gemetenTemperatuur);
  Serial.print(" *C\tAnaloge Gewenste temperatuur: ");
  Serial.print(analogGewensteTemperatuur);
  Serial.print(" *C\tDigitale Gewenste temperatuur: ");
  Serial.print(digitalGewensteTemperatuur);
  Serial.print(" *C\tVerwarmingsketel: ");
  Serial.println(ketelAan ? "AAN" : "UIT");

  // Controleer welke potentiometer is gebruikt
  if (lastUsedPotentiometer == 1 || lastUsedPotentiometer == 0) {
    // Controleer of de ketel aan moet op basis van analoge potentiometer
    if (analogGewensteTemperatuur > gemetenTemperatuur && !ketelAan) {
      ketelAan = true;
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V4, 1);
      Serial.println("Verwarmingsketel AAN (Analog)");
    } else if (gemetenTemperatuur >= analogGewensteTemperatuur && ketelAan) {
      ketelAan = false;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V4, 0);
      Serial.println("Verwarmingsketel UIT (Analog)");
    }
  } else if (lastUsedPotentiometer == 2) {
    // Controleer of de ketel aan moet op basis van digitale potentiometer
    if (digitalGewensteTemperatuur > gemetenTemperatuur && !ketelAan) {
      ketelAan = true;
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V4, 1);
      Serial.println("Verwarmingsketel AAN (Digital)");
    } else if (gemetenTemperatuur >= digitalGewensteTemperatuur && ketelAan) {
      ketelAan = false;
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V4, 0);
      Serial.println("Verwarmingsketel UIT (Digital)");
    }
  }
}

BLYNK_WRITE(V4) { // Knop in de Blynk-app voor het handmatig uitschakelen van de ketel
  int ketelStatus = param.asInt();
  if (ketelStatus == 1 && ketelAan) {
    ketelAan = false;
    digitalWrite(LED_PIN, LOW);
  }
}

BLYNK_WRITE(V5) { // Virtuele potentiometer in de Blynk-app voor het instellen van de analoge gewenste temperatuur
  analogGewensteTemperatuur = param.asInt();
  lastUsedPotentiometer = 1;  // Markeer de analoge potentiometer als laatst gebruikt
}

BLYNK_WRITE(V6) { // Virtuele potentiometer in de Blynk-app voor het instellen van de digitale gewenste temperatuur
  // Lees digitale gewenste temperatuur van virtuele potentiometer
  digitalGewensteTemperatuur = param.asInt();
  lastUsedPotentiometer = 2;  // Markeer de digitale potentiometer als laatst gebruikt
}

