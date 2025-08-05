#include "Wificonfig.h"
#define IRPin 5//Infrared Sensor (RPM Count)
#define LM35Pin A0//LM35 Sensor (Temperature)

//timer variables
volatile long curTime, prevTime;
const int interval = 60000;

//rpm sensor variables
volatile long rpmPrevTime; 
volatile bool irIn, prevIrIn;//input from IR Sensor (0 or 1)
volatile float rpm;//to be sent
int n =0;
int count; //count the number of studs
volatile long rpmLastIrIn;

//temp sensor variables
volatile long lm35inTotal;//summed sensor reading
volatile float lm35inAvg;//averaged sensor reading
volatile float temp;//calculated temp

void setup() {
  Serial.begin(115200);
  pinMode(IRPin, INPUT);//IR Sensor
  pinMode(LM35Pin, INPUT);//Temp Sensor
  connectToWiFi();
  curTime = millis(); //registering current Time
  prevTime = curTime; //registering prev Time
}

void loop() {
  if ((curTime - prevTime) == interval || (curTime - prevTime) >= interval) {
    Serial.println("Gathering Data");
    prevTime = curTime;
    tempRead();
    Serial.println("Sending Data to Database");
    sendToDb();
    resetVariables();
  }
  //RPMCount();
  RPMIntervalCount();
  curTime = millis();
}

//code to connect device using wifi library to wifi
void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(500);
    if ((++i % 16) == 0)
    {
      Serial.println("Connecting...");
    }
  }
  Serial.print("Connected. IP address is: ");
  Serial.println(WiFi.localIP());
  Serial.print("Signal Strength: ");
  Serial.println(WiFi.RSSI());
}

//function to count IR Sensor low Impulses 
void RPMCount() {
  prevIrIn = irIn;
  irIn = digitalRead(5);
  if (prevIrIn != irIn) {
    if (irIn == 0) {
      //curTime = millis();
      n++;
    }
  }
  rpm = n / 4;
}

void RPMIntervalCount(){
  //int n = 0;//number of rpm scanning to average the readings
  irIn = digitalRead(5);
  if(prevIrIn != irIn){
    if(irIn ==1 && prevIrIn == 0){
      rpmLastIrIn = curTime;
      if( (curTime-rpmLastIrIn)>50){
        prevIrIn = 1;
        if(count ==0){
          rpmPrevTime = curTime; //note the beginning of period (T)
        }
      }else{
        prevIrIn = 0;
      }
    }else if(irIn ==0 && prevIrIn==1){
      if( (curTime-rpmLastIrIn)>50){
        prevIrIn = 0;
        count++;
        Serial.print("count = ");
        Serial.println(count);
        if(count >= 4){
          float period = curTime - rpmPrevTime;
          rpm = 1/(period/60000);
          Serial.print("rpm = ");
          Serial.println(rpm);
          count = 0;
          rpmPrevTime = curTime;
        }
      }else
      {
        prevIrIn = 1;
      }
    }
    
  }
  prevIrIn = irIn;
}

//function to read temprature
void tempRead() {
  //storing data from A0 pin to lm35inTotal variable
  int x = 0;
  for (x = 0; x < 300; x++) {
    lm35inTotal += analogRead(A0) ;
  }
  //convert to celcius, store to temp
  lm35inAvg = (lm35inTotal / 300);
  temp = (((4.89 * lm35inAvg) / 1023) / 0.01)-19.6;
  if(temp <=0){
    temp = 0;
  }
  Serial.print("Temperature of Dryer :");
  Serial.println(temp);
  lm35inTotal = 0;
}


//Code for auth in (authorize method)
//returns JWT Token
String postAuth(){
//void postAuth(){
  //httpsClient.setFingerprint(fingerprint);
  httpsClient.setInsecure();
  httpsClient.setTimeout(15000);
  while(!httpsClient.connect(serverName,port)){
    delay(1000);
    Serial.println(".");
  }
  Serial.println("Declaring Auth");
  String request = METHOD + authpath + " HTTP/1.1\r\n" +
               "Host: " + HOST + "\r\n" +
               "Content-Type: application/json"+ "\r\n" +
               "Content-Length: 49" + "\r\n" +
               "Connection: close"+"\r\n\r\n"+
               BODY + "\r\n\r\n";
  //String authServerName = serverName + authHttpRequest;
  String payload = "{}";
  Serial.println("Posting Auth Request:");
  Serial.println(request);
  httpsClient.print(request);

  while(httpsClient.connected()){
    String line = httpsClient.readStringUntil('\n');
    Serial.println("-----HEADERS START-----");
    Serial.println(line);
    if(line == "\r") {
        Serial.println("-----HEADERS END-----");
        break;
    }
  }
  int n=0;
  while(httpsClient.available()) {
      String line = httpsClient.readStringUntil('\n');
      Serial.println(line);
      if(n==1){
        payload=line;    
      }
      n++;
  }
  return payload;
}

void resetVariables(){
  if(rpm>0){
    rpm = 0;
  }
  temp = 0;
  n = 0;
}

//Posting Data to Online DB through Web API
void sendToDb(){
  if (WiFi.status() == WL_CONNECTED) {
      String authKey = postAuth();
      Serial.print("Parsing token:");
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
      //Routing for Post Request
      while(!httpsClient.connect(serverName,port)){
        delay(1000);
        Serial.println(".");
      }
      //httpsClient.begin(client, serverName);
      
      //add Auth
      String fAuth = "Bearer " + String(token);
      
      //HTTP POST
      //Preparing HTTP POST request data
      //{"drumTemp":"20.22","drumRPM":"0.00"}
      String dryerReport ="{" + 
                          String("\"")+"drumTemp"+ String("\"")+ String(":")+String("\"")+String(temp,2)+ String("\"")+ String(",")+ 
                          String("\"")+ "drumRPM" + String("\"")+ String(":")+ String("\"")+ String(rpm,2)+String("\"") 
                          + "}";
      String request = METHOD + postPath + " HTTP/1.1\r\n" +
               "Host: " + HOST + "\r\n" +
               "Authorization: " + fAuth + "\r\n" +
               "Content-Type: application/json"+ "\r\n" +
               "Content-Length:"+ String(dryerReport.length()) + "\r\n" +
               "Connection: close"+"\r\n\r\n"+
               dryerReport + "\r\n\r\n"; 
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
  }else{
    Serial.println("WiFi Disconnected");
    connectToWiFi();
  }
}
