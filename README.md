# four channel relay to flooding mesh network (+reed input)


Based on EspNowFloodingMesh project:
* https://github.com/arttupii/EspNowFloodingMesh

Topics:
trigger/relay1/set   (set relay open for 3 seconds)
switch/relay2/set    (set relay open/closed)
switch/relay3/set    (set relay open/closed)
switch/relay4/set    (set relay open/closed)
switch/relay2/value    (relay current state)
switch/relay3/value    (relay current state)
switch/relay4/value    (relay current state)

contact/reedSensor/value (reed sensor state open/closed)

Hox! Reed sensor is connected to rx pin on esp01

Used:
* https://www.banggood.com/DC12V-ESP8266-Four-Channel-Wifi-Relay-IOT-Smart-Home-Phone-APP-Remote-Control-Switch-p-1317255.html?rmmds=search&cur_warehouse=CN
