char ssid[] = "WifiID";
char pass[] = "password";

#include <WiFiClientSecure.h>
#include "ArduinoJson.h"

WiFiClientSecure httpsClient;

const char* serverName = "the.server.com";
const char* HOST = "the.server.com:1234";
uint16_t port = 9198;
String authPath = "/API/authenticate";
String postPath = "/API/PostDehydratorData";
String CONTENTTYPE = "application/json";
String METHOD = "POST ";


String auth = String("{")+ 
              String("\"")+"name"+ String("\"")+ String(":")+String("\"")+"pd_device_a"+ String("\"")+ String(",")+ 
              String("\"")+ "password" + String("\"")+ String(":")+ String("\"")+"apiauthpassword"+String("\"")+
              String("}");
