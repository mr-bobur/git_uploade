const char apn[]      = "internet.beeline.uz";
const char gprsUser[] = "";
const char gprsPass[] = "";
const char simPIN[]   = "";
const char server[] = "185.196.214.199";
const char resource[] = "/api/devices/1";
const int  port = 8089;
String apiKeyValue = "tPmAT5Ab3j7F9";
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22
#define I2C_SDA_2            18
#define I2C_SCL_2            19
#define SerialMon Serial
#define SerialAT Serial1
#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER   2048
#include <Wire.h>
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

TwoWire I2CPower = TwoWire(0);

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
HttpClient    http(client, server, port);

#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP  36

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

const int rele1 = 14;
const int rele2 = 25;
const int rele3 = 33;
const int rele4 = 32;
bool task1 = false;
String body = "";
bool setPowerBoostKeepOn(int en) {
  I2CPower.beginTransmission(IP5306_ADDR);
  I2CPower.write(IP5306_REG_SYS_CTL0);
  if (en) {
    I2CPower.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  } else {
    I2CPower.write(0x35); // 0x37 is default reg value
  }
  return I2CPower.endTransmission() == 0;
}

void setup() {
  SerialMon.begin(115200);
  I2CPower.begin(I2C_SDA, I2C_SCL, 400000);
  // initalization for PINS
  pinMode(rele1, OUTPUT);
  pinMode(rele2, OUTPUT);
  pinMode(rele3, OUTPUT);
  pinMode(rele4, OUTPUT);

  digitalWrite(rele1, LOW);
  digitalWrite(rele2, LOW);
  digitalWrite(rele3, LOW);
  digitalWrite(rele4, LOW);

  bool isOk = setPowerBoostKeepOn(1);
  SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(1000);
  SerialMon.println("Initializing modem...");
  modem.restart();

  SerialMon.print("Connecting to APN: ");
  SerialMon.println(apn);
  while (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    SerialMon.println(" fail");
  }
  SerialMon.println(" Connected to internet with APN");
}

unsigned long time4 = 0;
unsigned long time3 = 0;
unsigned long tester = 0;
int  resetCount = 0;
void loop() {
  if (time3 + 1000 < millis()) {
    time3 = millis();
    if (resetCount >= 12) {
      resetCount = 0;
      delay(1000);
      SerialMon.println("Initializing modem...");
      modem.restart();
      SerialMon.println(apn);
      while (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      }
    }
  }
  if (time4 + 5000 < millis()) {
    time4 = millis();
    char tempString2[8]; 
    Serial.println("making PUT request");
    String contentType = "application/json";  
    String putData = "{\"temp\":"+ String(tester++)+"}"; 
    http.beginRequest();
    http.put("http://185.196.214.199:8089/api/devices/1", contentType, putData);
    http.endRequest(); 
    SerialMon.println(resetCount);
    http.setHttpResponseTimeout(5000); 
    if ( http.responseStatusCode() == -3) {
      http.stop();
      resetCount++;
      return;
    }
    if (resetCount > 2) {
      resetCount--;
    }
    body = http.responseBody();
    SerialMon.println(body);
    http.stop();

  }
}
