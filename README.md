# A Wemos D1 Mini temperature and humidity sensor
## Relevant parts of this code
Each device gets its unique name from its chip Id

```const String tipoNodo = "sensor";

chipId = ESP.getChipId();
nombreNodo = tipoNodo + chipId;
```
The devide creates an MQTT topic tu publish its information.
First we define some relevant strings that will be used as bricks to create the final topics.
```
const String mqttBase     = "casa/";
const String mqttType = "/sensor/";
const String mqttMon  = "/monitor/";
const String mqttTemp = "temperatura/";
const String mqttHum  = "humedad/";
```
Then we create some strings to store the final topics.
```
String mqttTempThisChip;
String mqttHumThisChip;
String mqttTempThisChipStatus;
String mqttHumThisChipStatus;
```
Then we build the final topics that will be specific for this device.
``` 
String mqttBaseThisChip = mqttBase + chipId;
String mqttBaseThisChipType = mqttBaseThisChip + mqttType;
mqttTempThisChip = mqttBaseThisChipType + mqttTemp;
mqttHumThisChip = mqttBaseThisChipType + mqttHum;
mqttTempThisChipStatus = mqttTempThisChip + "status/";
mqttHumThisChipStatus = mqttHumThisChip + "status/";
```
Every minute the device publishes its temperature reads on its mqtt topic.
```
if (currentMillis - previousMillis >= DELAY_MS)
  {
    // save the last time we entered here
    previousMillis = currentMillis;

    hum = dht.getHumidity();
    temp = dht.getTemperature();
    status = dht.getStatusString();
    
    if (client.isConnected())
    {
      client.publish(mqttTempThisChipStatus, status);
      if ( status == "OK")
      {
        client.publish(mqttTempThisChip, String(temp));
        client.publish(mqttHumThisChip, String(hum));
      }
    }
  }
```





