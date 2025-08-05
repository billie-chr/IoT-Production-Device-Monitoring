#include "ESP8266WiFi.h"
#include "ArduinoJson.h"
#include <WiFiClientSecure.h>

WiFiClient client;
WiFiClientSecure httpsClient;

//online server name
const char* serverName = "the.server.com";
const char* HOST = "the.server.com:1234";
uint16_t port = 9198;
IPAddress ip(192, 168, 0,111);
String auth = String("\"")+"name"+ String("\"")+ String(":")+String("\"")+"pd_device_b"+ String("\"")+ String(",")+ String("\"")+ "password" + String("\"")+ String(":")+ String("\"")+"apiauthpassword"+String("\"");

String authpath = "/API/authenticate";
String postPath = "/API/Post_B_Data";
String CONTENTTYPE = "application/json";

String METHOD = "POST ";

char ssid[] = "WifiID";
char pass[] = "password";



String BODY = "{" + String("\"")+"name"+ String("\"")+ String(":")+String("\"")+"pd_device_b"+ String("\"")+ String(",")+ String("\"")+ "password" + String("\"")+ String(":")+ String("\"")+"apiauthpassword"+String("\"") + "}";
