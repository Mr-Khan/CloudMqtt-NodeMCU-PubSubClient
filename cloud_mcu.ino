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
char msg[50];       //publish buffer
char msg_buff[50];  //subscribe buffer

//https://github.com/esp8266/Arduino/blob/master/variants/nodemcu/pins_arduino.h#L37-L59
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
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    msg_buff[i] = payload[i];
  }
  //Serial.println();
  String msgString = String(msg_buff);
  Serial.println(msgString);
  
  if (strcmp(topic, inTopic)==0){ // write your unique ID here
    if(msgString == "1"){
      switch1 = 1;
      digitalWrite(switch1Pin, HIGH);   // Turn the Switch on
    }
    if(msgString == "0"){
      switch1 = 0;
      digitalWrite(switch1Pin, LOW);    // Turn the Switch off
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
    snprintf (msg, 75, "{\"temperature\":\"%ld\",\"switch\":%ld}", random(10,30), switch1);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(outTopic, msg);
  }
}
