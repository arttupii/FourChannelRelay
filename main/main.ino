#include <EspNowFloodingMesh.h>
#include<SimpleMqtt.h>

/********NODE SETUP********/
#define ESP_NOW_CHANNEL 1
char deviceName[] = "Autotalli";
//int bsid = 0x112233;
//unsigned char secredKey[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
//unsigned char iv[16] = {0xb2, 0x4b, 0xf2, 0xf7, 0x7a, 0xc5, 0xec, 0x0c, 0x5e, 0x1f, 0x4d, 0xc1, 0xae, 0x46, 0x5e, 0x75};;
//const int ttl = 3;
#include "/home/arttu/git/myEspNowMeshConfig.h" //My secred mesh setup...
/*****************************/

#define LED 1 /*LED pin*/
bool valueRelay2 = false;
bool valueRelay3 = false;
bool valueRelay4 = false;
bool setRelay1 = false;
bool setRelay2 = false;
bool setRelay3 = false;
bool setRelay4 = false;

SimpleMQTT simpleMqtt = SimpleMQTT(ttl, deviceName);

void espNowFloodingMeshRecv(const uint8_t *data, int len, uint32_t replyPrt) {
  if (len > 0) {
    simpleMqtt.parse(data, len, replyPrt); //Parse simple Mqtt protocol messages
  }
}

void relayControl(int r, bool isOn) {
  const char r1on [] = {0xA0, 0x01, 0x01, 0xA2};
  const char r1off[] = {0xA0, 0x01, 0x00, 0xA1};
  const char r2on [] = {0xA0, 0x02, 0x01, 0xA3};
  const char r2off[] = {0xA0, 0x02, 0x00, 0xA2};
  const char r3on [] = {0xA0, 0x03, 0x01, 0xA4};
  const char r3off[] = {0xA0, 0x03, 0x00, 0xA3};
  const char r4on [] = {0xA0, 0x04, 0x01, 0xA5};
  const char r4off[] = {0xA0, 0x04, 0x00, 0xA4};
  const char *relayName;

  switch (r) {
    case 1: isOn ? Serial.write(r1on, 4) : Serial.write(r1off, 4); break;
    case 2: isOn ? Serial.write(r2on, 4) : Serial.write(r2off, 4); relayName = "relay2"; valueRelay2 = isOn; break;
    case 3: isOn ? Serial.write(r3on, 4) : Serial.write(r3off, 4); relayName = "relay3"; valueRelay3 = isOn; break;
    case 4: isOn ? Serial.write(r4on, 4) : Serial.write(r4off, 4); relayName = "relay4"; valueRelay4 = isOn; break;
    default: return;
  }

  if (r != 1) {
    if (!simpleMqtt._switch(PUBLISH, relayName, isOn ? SWITCH_ON : SWITCH_OFF)) {
      espRestart();
    }
  }
}

#define REED_SENSOR_PIN 3 /*RXPIN*/

void espRestart() {
  ESP.restart();
}
bool ledValue;
void setup() {
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);

  pinMode(REED_SENSOR_PIN, INPUT);

  //Set device in AP mode to begin with
  espNowFloodingMesh_RecvCB(espNowFloodingMeshRecv);
  espNowFloodingMesh_secredkey(secredKey);
  espNowFloodingMesh_setAesInitializationVector(iv);
  espNowFloodingMesh_setToMasterRole(false, ttl);
  espNowFloodingMesh_setToBatteryNode();
  espNowFloodingMesh_begin(ESP_NOW_CHANNEL, bsid);

  espNowFloodingMesh_ErrorDebugCB([](int level, const char *str) {
    Serial.print(level); Serial.println(str); //If you want print some debug prints
  });


  if (!espNowFloodingMesh_syncWithMasterAndWait()) {
    //Sync failed??? No connection to master????
    Serial.println("No connection to master!!! Reboot");
     espRestart();
  }

  //Handle MQTT events from master. Do not call publish inside of call back. --> Endless event loop and crash
  simpleMqtt.handleEvents([](const char *topic, const char* value) {
    simpleMqtt._ifSwitch(VALUE, "relay2", [](MQTT_switch value) { //<--> Listening topic switch/relay2/value and switch/relay2/set
      value == SWITCH_ON ? valueRelay2 = true : valueRelay2 = false;
    });
    simpleMqtt._ifSwitch(VALUE, "relay3", [](MQTT_switch value) { //Listening topic switch/relay3/value and switch/relay3/set
      value == SWITCH_ON ? valueRelay3 = true : valueRelay3 = false;
    });
    simpleMqtt._ifSwitch(VALUE, "relay4", [](MQTT_switch value) { //Listening topic switch/relay4/value and switch/relay4/set
      value == SWITCH_ON ? valueRelay4 = true : valueRelay4 = false;
    });

    simpleMqtt._ifTrigger(SET, "relay1", [](MQTT_trigger value) { //<--> Listening topic trigger/relay1/value and switch/relay1/set
      setRelay1 = true;
    });
    simpleMqtt._ifSwitch(SET, "relay2", [](MQTT_switch value) { //<--> Listening topic trigger/relay1/value  and switch/relay2/set
      value == SWITCH_ON ? setRelay2 = true : setRelay2 = false;
    });
    simpleMqtt._ifSwitch(SET, "relay3", [](MQTT_switch value) { //Listening topic switch/relay3/value and switch/relay3/set
      value == SWITCH_ON ? setRelay3 = true : setRelay3 = false;
    });
    simpleMqtt._ifSwitch(SET, "relay4", [](MQTT_switch value) { //Listening topic switch/relay4/value and switch/relay4/set
      value == SWITCH_ON ? setRelay4 = true : setRelay4 = false;
    });
  });

  if (!simpleMqtt._switch(SUBSCRIBE, {"relay2", "relay3", "relay4"})) { //Subscribe multible topics
    Serial.println("MQTT operation failed. No connection to gateway");
    espRestart();
  }
  if (!simpleMqtt._trigger(SUBSCRIBE, "relay1")) { //Subscribe multible topics
    Serial.println("MQTT operation failed. No connection to gateway");
    espRestart();
  }

}

void reed_sensor() {
  int currentState = digitalRead(REED_SENSOR_PIN);
  static int lastState = -1;
  if (lastState != currentState) {
    if (!simpleMqtt._contact(PUBLISH, "reedSensor", currentState == 1 ? CONTACT_CLOSED : CONTACT_OPEN)) { //Same as the upper but the smarter way
      espRestart();
    }
    delay(1000); //filtering time
  }
  lastState = currentState;
}

void relayLoop() {
  if (setRelay1) {
    relayControl(1, true);
    delay(3000);
    relayControl(1, false);
    setRelay1 = false;
  }
  if (setRelay2 != valueRelay2) {
    relayControl(2, setRelay2);
  }
  if (setRelay3 != valueRelay3) {
    relayControl(3, setRelay3);
  }
  if (setRelay4 != valueRelay4) {
    relayControl(4, setRelay4);
  }
}

void loop() {
  espNowFloodingMesh_loop();
  reed_sensor();
  relayLoop();
}
