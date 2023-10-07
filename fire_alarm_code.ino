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

#include <TridentTD_LineNotify.h> //thank you Mr.TridentTD https://github.com/TridentTD/TridentTD_LineNotify
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define ssid "xanny"                                             // you wifi router
#define pass "6410110082"                                        // password of wifi
#define LINE_TOKEN "shhXvGPgs1oteDTfteMWaqRix3aCaKO6DttOateAp15" // line token at https://notify-bot.line.me/en/
#define BUZZER_PIN D6
#define SPRINKLER_START_DELAY 3000 // 5 seconds
#define SPRINKLER_ON_TIME 3000     // 3 seconds Sprinkler on time

const char *host = "api.thingspeak.com"; // Host ของ thingspeak ไม่ต้องแก้ไข
const char *api = "DDTIP8UQMJA5G9TW";
int status = 0;

TridentTD_LineNotify myLINE(LINE_TOKEN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long previousTime = millis();

int sensorValue;
int LED_ALERT = D5;
int LED_OK = D3;
int RELAY_PIN = D4;
int SMOKE_PIN = 10;
int FLAME_PIN = D7;

int isFlame = HIGH;
int isSmoke = HIGH;

bool smoke1 = 0;
bool smoke2 = 0;
bool flame1 = 0;

void setup()
{
  Serial.begin(115200);
  pinMode(SMOKE_PIN, INPUT);
  pinMode(FLAME_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_OK, OUTPUT);
  pinMode(LED_ALERT, OUTPUT);

  lcd.init(); // Initialize the LCD
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("STATUS OK");

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(LED_OK, HIGH);
  digitalWrite(LED_ALERT, LOW);
  digitalWrite(RELAY_PIN, LOW);
}

void loop()
{                                // Wait for 1 second
  digitalWrite(BUZZER_PIN, LOW); // Turn the buzzer off
  smokeRead();
  smokeDetect();
  flameDetect();
  Serial.println(flame1);
  Serial.println(smoke1);
  Serial.println(smoke2);
  if (WiFi.status() == WL_CONNECTED)
  {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("WiFi is connected    ");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    lcd.setCursor(0, 1);
    lcd.print("WiFi is not connected    ");
  }

  if (flame1 == 1 || smoke1 == 1 || smoke2 == 1)
  {
    myLINE.notify("Fire! Fire! Fire!"); //  Messenger to line notification

    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("Warnig, Warning, Warning");

    lcd.setCursor(0, 0);
    lcd.print("FIRE DETECTED!!");
    digitalWrite(LED_OK, LOW);
    digitalWrite(LED_ALERT, HIGH);

    if (millis() - previousTime > SPRINKLER_START_DELAY)
    {
      digitalWrite(RELAY_PIN, HIGH);
      delay(SPRINKLER_ON_TIME);
    }
    delay(3000);
    status = 1;
  }
  else
  {
    analogWrite(BUZZER_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_OK, HIGH);
    digitalWrite(LED_ALERT, LOW);
    lcd.setCursor(0, 0);
    lcd.print("    STATUS OK    ");
    previousTime = millis();
    status = 0;
  }
  delay(2000);
  ThingSpeak();
}

void ThingSpeak()
{
  Serial.print("connecting to ");
  Serial.println(host);

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort))
  {
    Serial.println("connection failed");
    return;
  }

  String url = "/update?api_key=";
  url += api;
  url += "&field1=";
  url += status;
  if (status == 1)
  {
    url += "&field2=";
    url += status;
  }

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available())
  {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection");
}
/////////////////////////////////////////////////

void smokeDetect()
{
  isSmoke = digitalRead(SMOKE_PIN);
  if (isSmoke == LOW)
  {
    smoke1 = 1;
    Serial.println("SMOKE, SMOKE, SMOKE");
  }
  else
  {
    smoke1 = 0;
    Serial.println("NO SMOKE");
  }
}

void flameDetect()
{
  isFlame = digitalRead(FLAME_PIN);
  if (isFlame == LOW)
  {
    flame1 = 1;
    Serial.println("FLAME, FLAME, FLAME");
    Serial.println(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, HIGH);
  }
  else
  {
    flame1 = 0;
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("no flame");
  }
}

void smokeRead()
{
  sensorValue = analogRead(A0);
  Serial.println(sensorValue);
  if (sensorValue > 380)
  {
    smoke2 = 1;
    Serial.println("WARNING SMOKE TOO HIGH");

    lcd.setCursor(3, 0);
    lcd.print("SMOKE DETECTED!!");
  }
  else
  {
    smoke2 = 0;
    Serial.println("NO SMOKE");
  }
}
