#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
// WiFi and MQTT configuration
const char* ssid = "Nha Giang";
const char* password = "g0409794173";
const char* mqttServer = "192.168.2.16";
const int mqttPort = 1883;
const char* mqttTopic = "datas";
const char* mqttResponseTopic = "response";
//global variable
int check_1=0,check_2=0;
String dataBufferAccX = "";
String dataBufferAccY = "";
String dataBufferAccZ = "";
int fall_detection=0; 
const int ledPin = 14;
MPU6050 mpu;
WiFiClient espClient;
PubSubClient client(espClient);

// Function declarations
void connectWiFi();
void setupMPU6050();
void readAcceleration(float& accelX, float& accelY, float& accelZ);
void sendDataToServer(const String& payload);
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  setupMPU6050();
  connectWiFi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  reconnect();
  pinMode(ledPin, OUTPUT);
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  float accelX, accelY, accelZ;
  readAcceleration(accelX, accelY, accelZ);
  int absoluteValue = abs(9.808 - accelY);
  if (absoluteValue > 2){
    check_1;
    String dataX = String(accelX);
    String dataY = String(accelY);
    String dataZ = String(accelZ);
    dataBufferAccX = dataBufferAccX+","+dataX;
    dataBufferAccY = dataBufferAccY+","+dataY;
    dataBufferAccZ = dataBufferAccZ+","+dataZ;
    delay(20);
    while (check_1==10){
      check_2;
      readAcceleration(accelX, accelY, accelZ);
      String dataX = String(accelX);
      String dataY = String(accelY);
      String dataZ = String(accelZ);
      dataBufferAccX = dataBufferAccX+","+dataX;
      dataBufferAccY = dataBufferAccY+","+dataY;
      dataBufferAccZ = dataBufferAccZ+","+dataZ;
      if(check_2==140){
        DynamicJsonDocument jsonBuffer(1024);
        jsonBuffer["accelX"] = dataBufferAccX;
        jsonBuffer["accelY"] = dataBufferAccY;
        jsonBuffer["accelZ"] = dataBufferAccZ;
        String jsonString;
        serializeJson(jsonBuffer, jsonString);
        sendDataToServer(jsonString);
        check_1=0;
        check_2=0;
        dataBufferAccX = "";
        dataBufferAccY = "";
        dataBufferAccZ = "";
        return;
      }
      delay(20);
    }
  }
  else{
    delay(20);
    check_1=0;
    check_2=0;
    dataBufferAccX = "";
    dataBufferAccY = "";
    dataBufferAccZ = "";
  }
}
// connect wifi
void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }  
}
// get value to sensor
void setupMPU6050() {
  Wire.begin(19,15);
  mpu.initialize();
}
void readAcceleration(float& accelX, float& accelY, float& accelZ) {
  int16_t accelRawX, accelRawY, accelRawZ;
  mpu.getAcceleration(&accelRawX, &accelRawY, &accelRawZ);
  accelX = accelRawX / 16384.0*9.808;
  accelY = accelRawY / 16384.0*9.808;
  accelZ = accelRawZ / 16384.0*9.808;
}
// conect and send data
void sendDataToServer(const String& payload) {
  if (client.connected()) {
    client.publish(mqttTopic, payload.c_str());
    Serial.println("Dữ liệu đã gửi đến MQTT server");
  } else {
    Serial.println("Không gửi được dữ liệu. MQTT client bị ngắt kết nối");
  }
}
void reconnect() {
  while (!client.connected()) {
    Serial.println("Đang kết nối đến MQTT server...");
    if (client.connect("ESP32Client")) {
      Serial.println("Đã kết nối đến MQTT server");
      client.subscribe(mqttResponseTopic);
    } else {
      Serial.print("Thất bại, rc=");
      Serial.print(client.state());
      Serial.println(" Thử lại sau 5 giây...");
      delay(5000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, mqttResponseTopic) == 0) {
    String response = "";
    for (unsigned int i = 0; i < length; i++) {
      response += (char)payload[i];
    }
    Serial.print("Phản hồi nhận được: ");
    Serial.println(response);
  }
}