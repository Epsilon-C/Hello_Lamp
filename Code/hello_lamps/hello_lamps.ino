#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>  
#include <time.h>

//////////////////////////////Parameters////////////////////////////////

//define the pins where the touch sensor & leds are plugged in 
// and the number of pixels in strip

#define TouchSensor D7
#define LED D6
#define NUMPIXELS 16

//enter the details of your own MQTT Broker

const char* MQTT_ID = "YOUR_MQTT_ID";
const char* MQTT_User = "YOUR_USER";
const char* MQTT_Pass = "YOUR_PASSW0RD";
const char* host = "BROKER_IP_ADDRESS_OR_URL";
const int port = BROKER_PORT;
const char* MQTT_TOPIC = "YOUR_TOPIC";

//////////////////////////////////WiFi//////////////////////////////////

//Utilises the WifiManager to connect to your own wifi

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());;
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

//////////////////////////////////LED///////////////////////////////////

//Utilises the Adafruit NeoPixel library.  

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, LED, NEO_GRB + NEO_KHZ800);
int delayval = 5;
bool up = true;
static int colours = 40;
static bool cycle = false;

//////////////////////////////////MQTT//////////////////////////////////

void setLED(bool turnOn){
  cycle = turnOn;  
}

//Includes some debugging for validation
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    if ((char)payload[i] == '0'){
       setLED(false);
    }  
    if ((char)payload[i] == '1'){
      setLED(true);
    }
  }
  Serial.println();
}

WiFiClientSecure espClient;
PubSubClient client(host, port, callback, espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_ID,MQTT_User,MQTT_Pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement of this id joining 
      client.publish(MQTT_TOPIC, "Lamp Online!");
      client.subscribe(MQTT_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//////////////////////////////////SETUP////////////////////////////////

void setup() {
  Serial.begin(115200);
  strip.begin();
  WiFiManager wifiManager;
  for(int i=0;i<NUMPIXELS;i++){
      strip.setPixelColor(i, strip.Color(0,120,0)); // Green on startup
      strip.show();
    }
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setConnectTimeout(20);

  //reset saved settings
  //wifiManager.resetSettings();
    
  wifiManager.autoConnect("AutoConnectAP");
  
  //if you get here you have connected to the WiFi
  Serial.println("Connected to WiFi:)");
  client.setCallback(callback);
 
  Serial.println("LEDs online");
  pinMode(TouchSensor, INPUT);
  for(int i=0;i<NUMPIXELS;i++){
      strip.setPixelColor(i, strip.Color(0,0,0)); 
      strip.show();
  }
}

bool LEDon = false;
bool push = false;

void sendIT(){
  push = true;  
  if (LEDon ==true){
    client.publish(MQTT_TOPIC, "0");
    LEDon = false;
  }else{
    client.publish(MQTT_TOPIC, "1");
    LEDon = true;
  }
  delay(1000);
  push = false;
}

//////////////////////////////////LOOP/////////////////////////////////

void loop() {
  if (digitalRead(TouchSensor) == HIGH && push == false){
      sendIT();
  }
  if(!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if(cycle){
    if (colours >130){
      up = false;
    }else if (colours<80){
      up = true;
    }
    
    if(up){
      colours += 1;
      Serial.println(colours);
    }else{
      colours -= 1;
      Serial.println(colours);

    }
    for(int i=0;i<NUMPIXELS;i++){
      strip.setPixelColor(i, strip.Color(255-colours,0,255)); 
      strip.show();
      delay(delayval);
    }
  }else{
    for(int i=0;i<NUMPIXELS;i++){
      strip.setPixelColor(i, strip.Color(0,0,0)); 
      strip.show();
    }
  }
}
