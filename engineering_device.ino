
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
const char* ssid = "SSID";
const char* password = "PASSWORT";
const char* mqtt_server = "192.168.1.105";        //IP Adresse vom Raspberry Pi
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[200];
int value = 0;

//MainCode
int state = 0;
String esp_sensorType = "RGBLamp";                //AUSFÜLLEN
String esp_roomName   = "Kueche";                 //AUSFÜLLEN
String esp_sensorName = "Deckenlampe";            //AUSFÜLLEN

struct ColorT {
  int red;
  int green;
  int blue;
};

//PROPERTIES
bool    stt;    //State (On/Off)
int     bts;    //Brightness
String  mod;    //Mode
ColorT  clr1;   //Color
ColorT  clr2;   //Color
ColorT  clr3;   //Color
int     spd;    //Speed
int     vol;    //Volume
int     rnb;    //Room Number     //
double  xkd;    //X Koordinate
double  ykd;    //Y Koordinate
double  zkd;    //Z Koordinate
double  tmp;    //Termperature
int     lix;    //Light Index
 

bool sttValue = false;
bool btsValue = false;
bool clr1Value = false;
bool clr2Value = false;
bool clr3Value = false;
bool modValue = false;
bool spdValue = false;
bool volValue = false;
bool rnbValue = false;
bool xkdValue = false;
bool ykdValue = false;
bool zkdValue = false;
bool tmpValue = false;
bool lixValue = false;


//MUST CHANGE
bool propertiesArrived() {
  bool allArrived = true;
  if (sttValue == false) allArrived = false;
  if (btsValue == false) allArrived = false;
  if (modValue == false) allArrived = false;
  if (spdValue == false) allArrived = false;
//  if (clr1Value == false) allArrived = false;
//  if (clr2Value == false) allArrived = false;
//  if (clr3Value == false) allArrived = false;
//  if (volValue == false) allArrived = false;
//  if (rnbValue == false) allArrived = false;
//  if (xkdValue == false) allArrived = false;
//  if (ykdValue == false) allArrived = false;
//  if (zkdValue == false) allArrived = false;
//  if (tmpValue == false) allArrived = false;
//  if (lixValue == false) allArrived = false;
  return allArrived;
}
#include "MainLoop.h"
void mainLoop () {
  Serial.println("Main Loop");
  MAINLOOP(stt, bts, clr1, clr2, clr3, mod, spd, vol, rnb, xkd, ykd, zkd, tmp, lix);
}

//////////////////////////////////////////////////////////////////////
//                              MQTT                                //
//////////////////////////////////////////////////////////////////////
//-----------------------WIFI Setup-------------------//
void setup_wifi() {
  Serial.println("setup_wifi");
  delay(10);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
//-----------------------callback Funktion-----------//
void callback(char* topic, byte* payload, unsigned int length) {

  String tpc = String(topic);
  String pld = "";
  for (int i = 0; i < length; i++) {
    pld += char(payload[i]);
  }
  Serial.print("Message arrived [");
  Serial.print(tpc);
  Serial.print("]    ");
  Serial.println(pld);
  
  interpretMQTTSignal(tpc, pld);
}
//-----------------------reconnect Funktion-----------//
void reconnect() {
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      //Serial.println("connected");
      subscribeMQTT();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
//-----------------------subscribe Funktion-------------//
void subscribeMQTT () {
  String strSub1 = "home/" + esp_roomName + "/" + esp_sensorType + "/" + esp_sensorName;
  const char* sub1 = strSub1.c_str();

  String strSub2 = "home/" + esp_sensorType;
  const char* sub2 = strSub2.c_str();

  Serial.println("Subscribe to:");
  Serial.println(strSub1);
  Serial.println(strSub2);
  client.subscribe(sub1);
  client.subscribe(sub2);
}
//-----------------------interpret MQTT Signal---------//
void interpretMQTTSignal (String topic, String payload) {
  //OWN CODE FOR INTERPRETATION
  String MQTTSignal[100];
  int arrayPos = 0;
  char symbol;
  
  Serial.println(topic);
  
  for (int i = 0; i < payload.length(); i++) {
    symbol = char(payload[i]);
    if (symbol == '/') {
      arrayPos++;
      continue;
    }
    else {
      MQTTSignal[arrayPos] += symbol;
    }
  }
  arrayPos = 0;
  while (MQTTSignal[arrayPos] != NULL) {
    Serial.println(MQTTSignal[arrayPos]);
    arrayPos++;
  }
  
  //GO TO NAME EDIT MODE
  if (MQTTSignal[0] == "EDIT_NAME") {
    state = 4;
    return;
  } 
  setProperties(MQTTSignal);
}
//-----------------------set Sensor Properties-------//
void setProperties(String message[]) {
  int arrayPos = 0;
  while (message[arrayPos] != NULL) {
    if (message[arrayPos] == "stt") {
      stt = returnBoolFromString(message[arrayPos + 1]);
      sttValue = true;
    }
    if (message[arrayPos] == "bts") {
      bts = returnIntFromString(message[arrayPos + 1]);
      btsValue = true;
    }
    if (message[arrayPos] == "clr1") {
      clr1.red = returnColorFromString(message[arrayPos + 1], "red");
      clr1.green = returnColorFromString(message[arrayPos + 1], "green");
      clr1.blue = returnColorFromString(message[arrayPos + 1], "blue");
      clr1Value = true;
    }
    if (message[arrayPos] == "clr2") {
      clr2.red = returnColorFromString(message[arrayPos + 1], "red");
      clr2.green = returnColorFromString(message[arrayPos + 1], "green");
      clr2.blue = returnColorFromString(message[arrayPos + 1], "blue");
      clr2Value = true;
    }
    if (message[arrayPos] == "clr3") {
      clr3.red = returnColorFromString(message[arrayPos + 1], "red");
      clr3.green = returnColorFromString(message[arrayPos + 1], "green");
      clr3.blue = returnColorFromString(message[arrayPos + 1], "blue");
      clr3Value = true;
    }
    if (message[arrayPos] == "mod") {
      mod = message[arrayPos + 1];
      modValue = true;
    }
    if (message[arrayPos] == "spd") {
      spd = returnIntFromString(message[arrayPos + 1]);
      spdValue = true;
    }
    if (message[arrayPos] == "vol") {
      vol = returnIntFromString(message[arrayPos + 1]);
      volValue = true;
    }
    if (message[arrayPos] == "rnb") {
      rnb = returnIntFromString(message[arrayPos + 1]);
      rnbValue = true;
    }
    if (message[arrayPos] == "xkd") {
      xkd = returnDoubleFromString(message[arrayPos + 1]);
      xkdValue = true;
    }
    if (message[arrayPos] == "ykd") {
      ykd = returnDoubleFromString(message[arrayPos + 1]);
      ykdValue = true;
    }
    if (message[arrayPos] == "zkd") {
      zkd = returnDoubleFromString(message[arrayPos + 1]);
      zkdValue = true;
    }
    if (message[arrayPos] == "tmp") {
      tmp = returnDoubleFromString(message[arrayPos + 1]);
      tmpValue = true;
    }
    if (message[arrayPos] == "lix") {
      lix = returnIntFromString(message[arrayPos + 1]);
      lixValue = true;
    }
    //FOLLOWING IN THE FUTURE-------------------------------------------------------!!!!
    arrayPos += 2;
  }
}

//////////////////////////////////////////////////////////////////////
//                              OTHER                               //
//////////////////////////////////////////////////////////////////////
int returnColorFromString(String _color, String _whichColor) {
  //--> ColorString == "FF2A28" --- ColorString != "#FF2A28"
  String hex;
  if (_whichColor == "red") {
    hex = _color[0] + _color[1];
    return returnDECfromHEX(hex);
  }
  if (_whichColor == "green") {
    hex = _color[2] + _color[3];
    return returnDECfromHEX(hex);
  }
  if (_whichColor == "blue") {
    hex = _color[4] + _color[5];
    return returnDECfromHEX(hex);
  }
}

bool returnBoolFromString(String _bool) {
  if (_bool == "true") {
    return true;
  }
  if (_bool == "false") {
    return false;
  }
}

int returnIntFromString(String _int) {
  int value = 0;
  String character[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
  int number[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  for (int i = 0; i < _int.length(); i++) {
    int pos = 0;
    while (String(_int[i]) != String(character[pos])) {
      pos++;
    }
    value = value + number[pos]*pow(10, (int(_int.length()) - 1)-i);
  }
  return value;
}

double returnDoubleFromString(String _double) {
  return _double.toDouble();
}

int returnDECfromHEX (String _hex) {
  int value = 0;
  String character[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f"};
  int number[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  for (int i = 0; i < _hex.length(); i++) {
    int pos = 0;
    while (String(_hex[i]) != String(character[pos])) {
      pos++;
    }
    value = value + number[pos]*pow(16, (int(_hex.length()) - 1)-i);
  }
  return value;
}
//////////////////////////////////////////////////////////////////////

void setup() {
  EEPROM.begin(EEPROM_Byte_lenght);
  Serial.begin(115200);
  delay(2000);
  Serial.println("Setup");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {  
  delay(500);

  switch (state) {
    case 0: {
          //SETUP OLD
          bool arrived = false;
          long lastMsg = 0;
          
          while (arrived == false) {
            if (!client.connected()) {
              reconnect();
            }
            client.loop();
            long now = millis();
            if (now - lastMsg > 4000) {
              lastMsg = now;
              snprintf (msg, 50, "SETUP_DEVICE");
              Serial.println("Ask for Properties");
              String strSub1 = "home/" + esp_roomName + "/" + esp_sensorType + "/" + esp_sensorName;
              const char* sub1 = strSub1.c_str();
              client.publish(sub1, msg);
            }
            if (propertiesArrived() == true) {
              arrived = true;
            }
          }
          state = 1;// --> 1
          } break;
    case 1: {
          //MQTT HEARING
          if (!client.connected()) {
            reconnect();
          }
          client.loop();
          mainLoop();      
        } break;
    default: {
      
        } break;
  }
}
