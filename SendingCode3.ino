  #include <ESP8266WiFi.h>
#include "PubSubClient.h"
//#include "DHT.h"

// --------------- NETPIE 2020 configuration ---------------
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "d99da51c-6a61-4ba5-80b8-6abfe32b4a7e";
const char* mqtt_username = "C9A2UdbtYb5N89UcAe2nM3fZati2ncT1";
const char* mqtt_password = "A2ZOMeLVs3u#c!RmKcA1H-BI*znY10IS";
 String data ="";
const char* SensorTopic = "@msg/Sensor" ;
const char* LEDTopic = "@msg/LEDStatus" ;
const char* ShadowTopic ="@shadow/data/update";
  // ----- Wifi configuration---------------------
  const char* ssid     = "ZIA2389";       // WiFi ssid
  const char* password = "12345678";   // WiFi password
  // --------Sensor configuration---------------/

 int lastsend = 0;
   #define LEDPIN     D0  // LED on NodeMCU board 

   
  WiFiClient espClient;
PubSubClient client(espClient); 
char msg[100];

/***************Reconnect function***************/
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(LEDTopic);
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}



void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
    Serial.println("Starting...");
      pinMode(LEDPIN, OUTPUT);    // set LEDPIN as OUTPUT
digitalWrite(LEDPIN, HIGH);  //LED OFF
   // dht.begin(); // initialize DHT Sensor
    /* Initial WIFI, this is just a basic method to configure WIFI on ESP8266.                       */
    /* You may want to use other method that is more complicated, but provide better user experience */
    if (WiFi.begin(ssid, password)) {
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
    }

    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    // ^^^^^^^^^^^^^ initialization for NETPIE 2020 ^^^^^^^^^^^^^^^^^
  client.setServer(mqtt_server, mqtt_port);
  //client.setCallback(callback);
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

void loop() {
  // put your main code here, to run repeatedly:
    if (client.connected()) {
        client.loop();
       ReadandSend();
    }
     else   {
      reconnect();
    }
}
/***************Read Sensor function***************/
void ReadandSend () {

    if (Serial.available()) {  // check if there is incoming serial data
     data = Serial.readStringUntil('\n');  // read the string until a newline character is received
    Serial.println(data);  // print the received string to the serial monitor
  }

if(millis() - lastsend > 2000){
  lastsend=millis();


      // from a string in this format --> "{humid},{temp}"
    //  String datastring = "{\"data\" : {\"Temp\" : " + String(11) + "," + "\"Humid\" : " + String(millis()) + "}}" ;
    //   String datastring = "{\"data\" : {\"frame\" : " + data + "}}" ;

//String datastring = "{\"data\" : {\"Temp\" : " + data.substring(0, data.indexOf('a')) + "," + "\"Humid\" : " + data.substring(data.indexOf('a')+1, data.indexOf('b')) + "}}" ;
     String datastring = "{\"data\" : {\"V\" : " + data.substring(0, data.indexOf('a')) + "," + \  
                                         "\"C\" : " + data.substring(data.indexOf('a')+1, data.indexOf('b')) + "," + \
                                         "\"P\" : " + data.substring(data.indexOf('b')+1, data.indexOf('c')) + "," + \      
                                           "\"S\" : " + data.substring(data.indexOf('c')+1, data.indexOf('d')) + "," + \
                                       "\"D\" : " + data.substring(data.indexOf('d')+1, data.indexOf('e')) + "," + \
                                      "\"H\" : " + data.substring(data.indexOf('e')+1, data.indexOf('f')) + "," + \
                                         "\"T\" : " + data.substring(data.indexOf('f')+1, data.indexOf('t'))  + \
                                                           
                                         "}}" ;


      // String datastring = "{\"data\" : {\"v\" : " + data.substring(0, data.indexOf('a')) + "," + \  
      //                                   "\"c\" : " + data.substring(data.indexOf('a')+1, data.indexOf('b')) + "," + \
      //                                   "\"p\" : " + data.substring(data.indexOf('b')+1, data.indexOf('c')) + "," + \
      //                                   "\"s\" : " + data.substring(data.indexOf('c')+1, data.indexOf('d')) + "," + \
      //                                 "\"d\" : " + data.substring(data.indexOf('d')+1, data.indexOf('e')) + "," + \
      //                                 "\"h\" : " + data.substring(data.indexOf('e')+1, data.indexOf('f')) + "," + \
      //                                   "\"t\" : " + data.substring(data.indexOf('f')+1, data.indexOf('t')) + "," + \
      //                                     "\"e\" : " + data.substring(data.indexOf('t')+1, data.indexOf('s')) +  \
      //                                   "}}" ;

      // String datastring = "{ \"v\" : " + data.substring(0, data.indexOf('a')) + "," + \  
      //                                   "\"c\" : " + data.substring(data.indexOf('a')+1, data.indexOf('b')) + "," + \
      //                                   "\"p\" : " + data.substring(data.indexOf('b')+1, data.indexOf('c')) + "," + \
      //                                   "\"s\" : " + data.substring(data.indexOf('c')+1, data.indexOf('d')) + "," + \
      //                                   "\"d\" : " + data.substring(data.indexOf('d')+1, data.indexOf('e')) + "," + \
      //                                   "\"h\" : " + data.substring(data.indexOf('e')+1, data.indexOf('f')) + "," + \
      //                                   "\"t\" : " + data.substring(data.indexOf('f')+1, data.indexOf('t')) + "," + \
      //                                   "\"e\" : " + data.substring(data.indexOf('t')+1, data.indexOf('s')) + \
      //                                   "}" ;


      Serial.print("Sending --> ");
      Serial.println(datastring);
       datastring.toCharArray(msg, (datastring.length() + 1));
     client.publish(ShadowTopic, msg);       
    }  }

