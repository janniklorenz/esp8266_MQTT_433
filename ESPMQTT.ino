#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

const int RF_OSC = 200;

#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);


// called when we receive a message in a room we have subscribed
// topic      the room the message came from
// payload    raw payload data
// length     length of the payload
void mqttReceivedCallback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived ");
	Serial.print(topic);

	// put the payload in a null terminated string
	char *message = (char*)malloc(sizeof(char)*length+1);
	strncpy(message, (char*)payload,length);
	message[length] = '\0';

	// check if the topic is mqtt_set
	if (strcmp(topic, mqtt_set) == 0) {
		int value = atoi(message);
		mySwitch.send(value, 24);
		delay(100);
		client.publish(mqtt_didSet, message);
	}

}



// (Re)Connect to MQTT Server
void connectMQTT() {
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect("ESP8266 Client")) {
			Serial.println("connected");

			// ... and subscribe to topic
			client.subscribe(mqtt_set);
		}
		else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			Serial.print("IP address: ");
			Serial.println(WiFi.localIP());

			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

// (Re)Connect the WIFI
void connectWIFI() {
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
}



void setup() {
	// Set up pin rfPin
	pinMode(rfPin, OUTPUT);
	mySwitch.enableTransmit(rfPin);

	// Debug serial
	Serial.begin(9600);

	// Init MQTT client
	client.setServer(mqtt_server, 1883);
	client.setCallback(mqttReceivedCallback);
}

void loop() {
	// If we have no WIFI connecton, trigger connectWIFI
	if (WiFi.status() != WL_CONNECTED) {
		connectWIFI();
	}

	// If we have no connecton to the MQTT Server, trigger connectMQTT
	if (client.connected() == false) {
		connectMQTT();
	}

	// Otherwise run MQTT client loop
	client.loop();
}
