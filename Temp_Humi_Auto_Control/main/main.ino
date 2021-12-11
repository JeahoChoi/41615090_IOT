#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <dhtnew.h>

#define FIREBASE_HOST "home-fwgw-3.firebaseio.com/"
#define FIREBASE_AUTH "LWBeIYkb4ifC2gcWxWWcEWJFO8FZ54f6fuCsmlWe"
#define WIFI_SSID "414"
#define WIFI_PASSWORD "123456789a"

#define led  D6
#define blue  D7
#define red  D8
#define led_b  D1
#define blue_b  D0
#define red_b  D5
#define DHTPIN 0 //GPIO 0 = D3

String state1="0", state2="0", state3="0", Auto="0";
String maxT="0", minT="0", maxH="0", minH="0";
int state11=0,state22=0,state33=0;
int Led=0,Blue=0,Red=0;

//Define FirebaseESP8266 data object
FirebaseData firebaseData1;
FirebaseData firebaseData2;

unsigned long sendDataPrevMillis = 0;

String path = "/Test";

uint16_t count = 0;

void printResult(FirebaseData &data);
void printResult(StreamData &data);
void toggle();
void Getstring();

DHTNEW mySensor(DHTPIN);
int tem,hum;
void Sensor();
//String testTem = "0",testHum = "0";

void streamCallback(StreamData data)
{

  Serial.println("Stream Data1 available...");
  Serial.println("STREAM PATH: " + data.streamPath());
  Serial.println("EVENT PATH: " + data.dataPath());
  Serial.println("DATA TYPE: " + data.dataType());
  Serial.println("EVENT TYPE: " + data.eventType());
  Serial.print("VALUE: ");
  printResult(data);
  Serial.println();
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
  {
    Serial.println();
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
}

void setup()
{

  pinMode(led, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(red_b, INPUT);
  pinMode(blue_b, INPUT);
  pinMode(led_b, INPUT);
  
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData1.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData1.setResponseSize(1024);


  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData2.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData2.setResponseSize(1024);



  if (!Firebase.beginStream(firebaseData1, path))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + firebaseData1.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  Firebase.setStreamCallback(firebaseData1, streamCallback, streamTimeoutCallback);
}

void loop()
{
  if (millis() - sendDataPrevMillis > 1000)
  {
    sendDataPrevMillis = millis();
    FirebaseJson json;
    
    Serial.println("------------------------------------");
    Serial.println("Set JSON...");

    Sensor();

    Getstring();

    toggle();

    json.add("Temp", tem).add("Humi", hum).add("LED1", state1)
    .add("LED2", state2).add("LED3", state3)
    .add("Auto", Auto).add("MaxT", maxT).add("MinT", minT)
    .add("MaxH", maxH).add("MinH", minH)/*.add("Temp", testTem).add("Humi", testHum)*/;
    if (Firebase.setJSON(firebaseData2, path + "/Sensor", json))
    {
      Serial.println("PASSED");
      Serial.println("PATH: " + firebaseData2.dataPath());
      Serial.println("TYPE: " + firebaseData2.dataType());
      Serial.print("VALUE: ");
      printResult(firebaseData2);
      Serial.println("------------------------------------");
      Serial.println();
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData2.errorReason());
      Serial.println("------------------------------------");
      Serial.println();
    }
    
  }
}

void printResult(FirebaseData &data)
{

  if (data.dataType() == "int")
    Serial.println(data.intData());
  else if (data.dataType() == "float")
    Serial.println(data.floatData(), 5);
  else if (data.dataType() == "double")
    printf("%.9lf\n", data.doubleData());
  else if (data.dataType() == "boolean")
    Serial.println(data.boolData() == 1 ? "true" : "false");
  else if (data.dataType() == "string")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
  {
    Serial.println();
    FirebaseJson &json = data.jsonObject();
    //Print all object data
    Serial.println("Pretty printed JSON data:");
    String jsonStr;
    json.toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();
    Serial.println("Iterate JSON data:");
    Serial.println();
    size_t len = json.iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
      json.iteratorGet(i, type, key, value);
      Serial.print(i);
      Serial.print(", ");
      Serial.print("Type: ");
      Serial.print(type == FirebaseJson::JSON_OBJECT ? "object" : "array");
      if (type == FirebaseJson::JSON_OBJECT)
      {
        Serial.print(", Key: ");
        Serial.print(key);
      }
      Serial.print(", Value: ");
      Serial.println(value);
    }
    json.iteratorEnd();
  }
  else if (data.dataType() == "array")
  {
    Serial.println();
    //get array data from FirebaseData using FirebaseJsonArray object
    FirebaseJsonArray &arr = data.jsonArray();
    //Print all array values
    Serial.println("Pretty printed Array:");
    String arrStr;
    arr.toString(arrStr, true);
    Serial.println(arrStr);
    Serial.println();
    Serial.println("Iterate array values:");
    Serial.println();
    for (size_t i = 0; i < arr.size(); i++)
    {
      Serial.print(i);
      Serial.print(", Value: ");

      FirebaseJsonData &jsonData = data.jsonData();
      //Get the result data from FirebaseJsonArray object
      arr.get(jsonData, i);
      if (jsonData.typeNum == FirebaseJson::JSON_BOOL)
        Serial.println(jsonData.boolValue ? "true" : "false");
      else if (jsonData.typeNum == FirebaseJson::JSON_INT)
        Serial.println(jsonData.intValue);
      else if (jsonData.typeNum == FirebaseJson::JSON_DOUBLE)
        printf("%.9lf\n", jsonData.doubleValue);
      else if (jsonData.typeNum == FirebaseJson::JSON_STRING ||
               jsonData.typeNum == FirebaseJson::JSON_NULL ||
               jsonData.typeNum == FirebaseJson::JSON_OBJECT ||
               jsonData.typeNum == FirebaseJson::JSON_ARRAY)
        Serial.println(jsonData.stringValue);
    }
  }
}

void printResult(StreamData &data)
{

  if (data.dataType() == "int")
    Serial.println(data.intData());
  else if (data.dataType() == "float")
    Serial.println(data.floatData(),2);
  else if (data.dataType() == "double")
    printf("%.9lf\n", data.doubleData());
  else if (data.dataType() == "boolean")
    Serial.println(data.boolData() == 1 ? "true" : "false");
  else if (data.dataType() == "string")
    Serial.println(data.stringData());
  else if (data.dataType() == "json")
  {
    Serial.println();
    FirebaseJson *json = data.jsonObjectPtr();
    //Print all object data
    Serial.println("Pretty printed JSON data:");
    String jsonStr;
    json->toString(jsonStr, true);
    Serial.println(jsonStr);
    Serial.println();
    Serial.println("Iterate JSON data:");
    Serial.println();
    size_t len = json->iteratorBegin();
    String key, value = "";
    int type = 0;
    for (size_t i = 0; i < len; i++)
    {
      json->iteratorGet(i, type, key, value);
      Serial.print(i);
      Serial.print(", ");
      Serial.print("Type: ");
      Serial.print(type == FirebaseJson::JSON_OBJECT ? "object" : "array");
      if (type == FirebaseJson::JSON_OBJECT)
      {
        Serial.print(", Key: ");
        Serial.print(key);
      }
      Serial.print(", Value: ");
      Serial.println(value);
    }
    json->iteratorEnd();
  }
  else if (data.dataType() == "array")
  {
    Serial.println();
    //get array data from FirebaseData using FirebaseJsonArray object
    FirebaseJsonArray *arr = data.jsonArrayPtr();
    //Print all array values
    Serial.println("Pretty printed Array:");
    String arrStr;
    arr->toString(arrStr, true);
    Serial.println(arrStr);
    Serial.println();
    Serial.println("Iterate array values:");
    Serial.println();

    for (size_t i = 0; i < arr->size(); i++)
    {
      Serial.print(i);
      Serial.print(", Value: ");

      FirebaseJsonData *jsonData = data.jsonDataPtr();
      //Get the result data from FirebaseJsonArray object
      arr->get(*jsonData, i);
      if (jsonData->typeNum == FirebaseJson::JSON_BOOL)
        Serial.println(jsonData->boolValue ? "true" : "false");
      else if (jsonData->typeNum == FirebaseJson::JSON_INT)
        Serial.println(jsonData->intValue);
      else if (jsonData->typeNum == FirebaseJson::JSON_DOUBLE)
        printf("%.9lf\n", jsonData->doubleValue);
      else if (jsonData->typeNum == FirebaseJson::JSON_STRING ||
               jsonData->typeNum == FirebaseJson::JSON_NULL ||
               jsonData->typeNum == FirebaseJson::JSON_OBJECT ||
               jsonData->typeNum == FirebaseJson::JSON_ARRAY)
        Serial.println(jsonData->stringValue);
    }
  }
}

void Sensor(){
  mySensor.read();
  tem = mySensor.getTemperature();
  hum = mySensor.getHumidity();
  Serial.print("Tem : ");
  Serial.println(tem);
  Serial.print("Hum : ");
  Serial.println(hum);
}

void Getstring(){
  Firebase.getString(firebaseData1, path + "/Sensor/LED1", state1);
  Firebase.getString(firebaseData1, path + "/Sensor/LED2", state2);
  Firebase.getString(firebaseData1, path + "/Sensor/LED3", state3);
  Firebase.getString(firebaseData1, path + "/Sensor/Auto", Auto);
  Firebase.getString(firebaseData1, path + "/Sensor/MaxT", maxT);
  Firebase.getString(firebaseData1, path + "/Sensor/MinT", minT);
  Firebase.getString(firebaseData1, path + "/Sensor/MaxH", maxH);
  Firebase.getString(firebaseData1, path + "/Sensor/MinH", minH);
  //Firebase.getString(firebaseData1, path + "/Sensor/Temp", testTem);
  //Firebase.getString(firebaseData1, path + "/Sensor/Humi", testHum);
  }

void toggle(){
  int a=0,b=0,c=0;
  int MaT=0, MiT=0, MaH=0, MiH=0;
  MaT = maxT.substring(2,4).toInt();
  MiT = minT.substring(2,4).toInt();
  MaH = maxH.substring(2,4).toInt();
  MiH = minH.substring(2,4).toInt();
  //tem = testTem.toInt();
  //hum = testHum.toInt();
  if(Auto=="1"){
    if(tem<MiT) state3="1";
    else if(tem>MaT) state3="0";
    if(hum<MiH) state2="1";
    else if (hum>MaT) state2="0";
  }
  Led=digitalRead(led_b);
  if(state11==0&&Led==1) state1 = (state1=="1")? "0":"1";
  state11=Led;
  if(state1=="1") a=HIGH;
  else a=LOW;
  digitalWrite(led,a);
  Blue=digitalRead(blue_b);
  if(state22==0&&Blue==1) state2 = (state2=="1")? "0":"1";
  state22=Blue;
  if(state2=="1") b=HIGH;
  else b=LOW;
  digitalWrite(blue,b);
  Red=digitalRead(red_b);
  if(state33==0&&Red==1) state3 = (state3=="1")? "0":"1";
  state33=Red;
  if(state3=="1") c=HIGH;
  else c=LOW;
  digitalWrite(red,c);
  }
