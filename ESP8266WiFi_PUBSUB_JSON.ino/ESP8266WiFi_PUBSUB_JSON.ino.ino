/*
 Basic CloudMCU example
 
 It connects to an MQTT server then:
  - publishes "json data" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the message of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Update these with values suitable for your network.
// mqtt topics
const char* syncTopic = "building/floor-1/hvac/sync";
const char* inTopic = "building/floor-1/hvac/in";
const char* outTopic = "building/floor-1/hvac/out";

const char* ssid = "Hasib";           //wifi name
const char* password = "Hasib@Khan";  //wifi password
const char* mqtt_server = "m20.cloudmqtt.com";  //cloudmqtt server
const char* device_id = "myUniqueDeviceID"; // assign a unique device ID

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
long utcTimestamp = millis();  //Math.floor((new Date()).getTime() / 1000)
char msg[128];       //publish buffer
char msg_buff[128];  //subscribe buffer

//https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h#L37-L59
int temperature = 10;
int switch1 = 0;
int switch1Pin = 16;   

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char msg_buff[128];
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    msg_buff[i] = payload[i];
  }
  //Serial.println();
  String msgString = String(msg_buff);
  Serial.println(msgString);
  
  if (strcmp(topic, inTopic)==0){ // write your unique ID here
    
    StaticJsonDocument<128> InDoc;
    Serial.println("JSON covert..");
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(InDoc, msgString);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }else{
      
      // Is there a value named "switch" in the object?
      JsonVariant InTimestamp = InDoc["timestamp"];
      if (!InTimestamp.isNull()) {
        Serial.print("In_timestamp >> ");
        utcTimestamp = long(InDoc["timestamp"]);
        //SET NEW
        
        // Print values.
        Serial.println(utcTimestamp);
      }

      // Is there a value named "switch" in the object?
      JsonVariant InSwitch = InDoc["switch"];
      if (!InSwitch.isNull()) {
        Serial.print("In_switch >> ");
        switch1 = int(InDoc["switch"]);
        //SET NEW
        if(InDoc["switch"] == "1"){
          digitalWrite(switch1Pin, HIGH);   // Turn the Switch on
        }
        if(InDoc["switch"] == "0"){
          digitalWrite(switch1Pin, LOW);    // Turn the Switch off
        }
        // Print values.
        Serial.println(switch1);
      }

      // Is there a value named "temperature" in the object?
      JsonVariant InTemperature = InDoc["temperature"];
      if (!InTemperature.isNull()) {
        Serial.print("In_temperature >> ");
        temperature = int(InDoc["temperature"]);
        //SET NEW
        //if(InDoc["temperature"] < "5"){
        //  digitalWrite(switch1Pin, HIGH);   // Turn the Switch on as temrature is very Low
        //}
        //if(InDoc["switch"] > "45"){
        //  digitalWrite(switch1Pin, HIGH);    // Turn the Switch on as temrature is very High
        //}
        // Print values.
        Serial.println(temperature);
      }
    }
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(device_id, "hasib_khan", "hasib@khan")) { // enter cloudmqtt user  username and password provided in cloudmqtt console
      Serial.println("connected");
      // Once connected, publish an announcement...
      
      client.publish(syncTopic, "Sync");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(switch1Pin, OUTPUT);     // Initialize the switch1Pin pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 14679);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    
    //prepare json data
    snprintf (msg, 128, "{\"millisS\":\"%ld\",\"timestamp\":\"%ld\",\"temperature\":\"%ld\",\"switch\":%ld}", now, utcTimestamp, temperature, switch1);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(outTopic, msg);
  }
}