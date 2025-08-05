#include "Wificonfig.h"
//Sensor Pins
#define LM35B1 32 //LM35 Sensor (Temp)
#define LM35B2 35 //LM35 Sensor (Temp)
#define LM35B3 34 //LM35 Sensor (Temp)

//sensor variables
unsigned int lm35tot1, lm35tot2, lm35tot3;
volatile float lm35in1, lm35in2, lm35in3;
volatile float temp1, temp2, temp3;

//system variables
volatile unsigned long previousMillis = 0;

//timer variables
volatile long curTime, prevTime;
const int interval = 60000;

void setup() {
  Serial.begin(115200); 
  //temp sensors
  pinMode(35,INPUT);  
  pinMode(39,INPUT);
  pinMode(34,INPUT);
  analogReadResolution(12);
  connectToWiFi();
  curTime = millis(); //registering current Time
  prevTime = curTime; //registering prev Time
}

void loop() {
  if ((curTime - prevTime) >= interval) {
    Serial.println("Gathering Data");
    prevTime = curTime;
    tempRead();
    Serial.println("Sending Data to Database");
    sendToDb();
  }
  curTime = millis();
}

void connectToWiFi(){
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid,pass);
  uint8_t i = 0;
  while(WiFi.status()!=WL_CONNECTED){
    Serial.println(".");
    delay(500);
    if ((++i % 16) == 0)
    {
      Serial.println("Connecting...");
    }
  }
  Serial.print("Connected, with IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Signal Strength: ");
  Serial.println(WiFi.RSSI());
}

//function to read temprature
void tempRead(){
  //summing digital input from LM35 for x times
  for(int x = 0; x<600;x++){
    lm35tot1 += analogRead(LM35B1);
    lm35tot2 += analogRead(LM35B2);
    lm35tot3 += analogRead(LM35B3);
  }
  //averaging the values
  lm35in1 = (lm35tot1/600);
  lm35in2 = (lm35tot2/600);
  lm35in3 = (lm35tot3/600);
  
  //changing from bit to temp (Celcius)
  temp1 = (4.85*lm35in1 *100)/4094+22.4;
  temp2 = (4.85*lm35in2 *100)/4094+6.4;
  temp3 = (4.85*lm35in3 *100)/4094+6.4;
  
  //print out the readings in serial monitor
  Serial.println("Temperature Readings:");
  Serial.print("Bilik 1 = ");
  Serial.print(temp1, 2); 
  Serial.println(" C");
  Serial.print("Bilik 2 = ");
  Serial.print(temp2, 2);
  Serial.println(" C");
  Serial.print("Bilik 3 = ");
  Serial.print(temp3, 2);
  Serial.println(" C");  
  
  //reset each summed sensor inputs value
  lm35tot1 = 0;
  lm35tot2 = 0;
  lm35tot3 = 0;
}

//Code for auth in (authorize method)
//returns JWT Token
String postAuth(){
  //httpsClient.setFingerprint(fingerprint);
  Serial.println("Authenticating");
  httpsClient.setInsecure();
  httpsClient.setTimeout(15000);
  while(!httpsClient.connect(serverName,port)){
    delay(1000);
    Serial.println(".");
  }
  String request = METHOD + authPath + " HTTP/1.1\r\n"+
                   "Host: " + HOST + "\r\n" +
                   "Content-Type: application/json" + "\r\n" +
                   "Content-Length: 54" + "\r\n" + 
                   "Connection: close" + "\r\n\r\n"+
                   auth + "\r\n\r\n";
  String payload = "{}";
  Serial.println("Posting Auth Request:");
  Serial.println(request);
  httpsClient.print(request);
  while(httpsClient.connected()){
    String line = httpsClient.readStringUntil('\n');
    Serial.println("----HEADER START----");
    Serial.println(line);
    if(line =="\r"){
      Serial.println("----HEADERS END----");
      break;
    }
  }
  int n=0;
  while(httpsClient.available()){
    String line = httpsClient.readStringUntil('\n');
    Serial.println(line);
    if(n==1){
      payload = line;
    }
    n++;
  }
  httpsClient.stop(); 
  return payload;
}

//Posting Data to Online DB through Web API
void sendToDb(){
  if (WiFi.status() == WL_CONNECTED) {
      String authKey = postAuth();
      Serial.println("Parsing token");
      Serial.println(authKey);
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, authKey);
      const char* token = doc["token"];
      if (token == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }else{
        Serial.print("Token parsed:");
        Serial.println(token);
      }
      httpsClient.setInsecure();
      httpsClient.setTimeout(15000);
      
      //Routing for Post Request
      while(!httpsClient.connect(serverName,port)){
        delay(1000);
        Serial.println(".");
      }
      
      //HTTP POST
      //Preparing HTTP POST request data
      String fAuth = "Bearer " + String(token);
      //{"comp1Temp":"121.00","comp2Temp":"122.00","comp3Temp":"121.00"}
      String dehydratorReport ="{" +
                               String("\"")+"comp1Temp"+String("\"")+String(":")+String("\"")+String(temp1, 2)+String("\"")+String(",")+
                               String("\"")+"comp2Temp"+String("\"")+String(":")+String("\"")+String(temp2, 2)+String("\"")+String(",")+
                               String("\"")+"comp3Temp"+String("\"")+String(":")+String("\"")+String(temp3, 2)+String("\"")+
                               "}";
      String request = METHOD + postPath + " HTTP/1.1\r\n" +
                       "Host: " + HOST + "\r\n" +
                       "Authorization: " + fAuth + "\r\n" +
                       "Content-Type: application/json" + "\r\n" +
                       "Content-Length: " +String(dehydratorReport.length())+ "\r\n"+
                       "Connection: close \r\n\r\n" +
                       dehydratorReport + "\r\n\r\n";
      Serial.println("Sending : ");
      Serial.println(request);
      Serial.println("To :");
      Serial.println(postPath);
      String payload = "{}";
      httpsClient.print(request);
       while(httpsClient.connected()){
        payload = httpsClient.readStringUntil('\n');
        Serial.println("-----HEADERS START-----");
        Serial.println(payload);
        if(payload == "\r") {
          Serial.println("-----HEADERS END-----");
          break;
        }
      }
      while(httpsClient.available()) {
        String line = httpsClient.readStringUntil('\n');
        Serial.println(line);
      }
      httpsClient.stop();
    } else {
      Serial.println("WiFi Disconnected, Reconnecting..");
      connectToWiFi();
    }
}
