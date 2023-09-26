/*
 * schemetic
nodemcu         smoke detector          flame detector          buzzer
D5                    D0
D6                                            D0
D7                                                                +
A0                    A0
VCC                   VCC                     VCC
GND                   GND                     GND                 GND
 *
 */

#include <TridentTD_LineNotify.h>  //thank you Mr.TridentTD https://github.com/TridentTD/TridentTD_LineNotify
#include <ESP8266WiFi.h>

#define ssid "xanny"                                              //you wifi router
#define pass "6410110082"                                         //password of wifi
#define LINE_TOKEN "shhXvGPgs1oteDTfteMWaqRix3aCaKO6DttOateAp15"  //line token at https://notify-bot.line.me/en/
#define BUZZER_PIN D6
#define SPRINKLER_START_DELAY 3000  //5 seconds
#define SPRINKLER_ON_TIME 3000      //3 seconds Sprinkler on time

TridentTD_LineNotify myLINE(LINE_TOKEN);

unsigned long previousTime = millis();

int sensorValue;
int isSmokepin = D5;
int isFlamePin = D7;
int RELAY_PIN = D8;


int isFlame = HIGH;
int isSmoke = HIGH;

bool smoke1 = 0;
bool smoke2 = 0;
bool flame1 = 0;

void setup() {
  Serial.begin(115200);
  pinMode(isSmokepin, INPUT);
  pinMode(isFlamePin, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  WiFi.begin(ssid, pass);
  digitalWrite(RELAY_PIN, LOW);
}

void loop() {                     // Wait for 1 second
  digitalWrite(BUZZER_PIN, LOW);  // Turn the buzzer off
  smokeRead();
  smokeDetect();
  flameDetect();
  Serial.println(flame1);
  Serial.println(smoke1);
  Serial.println(smoke2);

  if (flame1 == 1 || smoke1 == 1 || smoke2 == 1) {
    myLINE.notify("Fire! Fire! Fire!");  //  Messenger to line notification
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("Warnig, Warning, Warning");
    if (millis() - previousTime > SPRINKLER_START_DELAY) {
      digitalWrite(RELAY_PIN, HIGH);
      delay(SPRINKLER_ON_TIME);
    }
    delay(3000);
  }

  delay(2000);
}

/////////////////////////////////////////////////

void smokeDetect() {
  isSmoke = digitalRead(isSmokepin);
  if (isSmoke == LOW) {
    smoke1 = 1;
    Serial.println("SMOKE, SMOKE, SMOKE");
  } else {
    smoke1 = 0;
    Serial.println("NO SMOKE");
  }
}

void flameDetect() {
  isFlame = digitalRead(isFlamePin);
  if (isFlame == LOW) {
    flame1 = 1;
    Serial.println("FLAME, FLAME, FLAME");
    Serial.println(BUZZER_PIN);

    analogWrite(BUZZER_PIN, 50);
  } else {
    flame1 = 0;
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("no flame");
  }
}

void smokeRead() {
  sensorValue = analogRead(A0);
  Serial.println(sensorValue);
  if (sensorValue > 380) {
    smoke2 = 1;
    Serial.println("SMOKE, SMOKE, SMOKE");
  } else {
    smoke2 = 0;
    Serial.println("NO SMOKE");
  }
}
