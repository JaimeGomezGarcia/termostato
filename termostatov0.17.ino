// Import required libraries
#include "EspMQTTClient.h"
#include "DHTesp.h"
#include <user_interface.h>

const char TERMOMETRO_VERSION[] = "0.17";

#define DHTPIN D3     // Digital pin connected to the DHT sensor 

// Número de milisegundos de pausa entre chequeo y chequeo del estado de los termostatos. No hay necesidad de estar procesando permanentemente.
const int DELAY_MS = 60000; //Mandamos lecturas de temperatura cada 1 minutos
const int MONITORING_DELAY_MS = 60000; //Mandamos lecturas de monitorizacion cada minuto
unsigned long previousMillis = 0; //Control del tiempo de bucle sin delay()
unsigned long previousMillisMon = 0; //Control del tiempo de bucle sin delay()
  
DHTesp dht;


String chipId="";

const String mqttBase     = "casa/";
const String mqttType = "/sensor/";
const String mqttMon  = "/monitor/";
const String mqttTemp = "temperatura/";
const String mqttHum  = "humedad/";


String mqttTempThisChip;
String mqttHumThisChip;
String mqttTempThisChipStatus;
String mqttHumThisChipStatus;
String mqttMonThisChip;

const String tipoNodo = "sensor";
char myname[32];

const char *ssid = "<MYSSID>"; // cannot be longer than 32 characters!
const char *pass = "<MYPASS>";
const char *mqtt_server = "<MYMQTTBROKER>";
const int mqtt_port = 1883;
const char *mqttuser = "<MYUSER>";
const char *mqttpass = "<MYMQTTPASS>";


EspMQTTClient client(
  ssid,
  pass,
  mqtt_server,  // MQTT Broker server ip
  "",   // Can be omitted if not needed
  "",   // Can be omitted if not needed
  myname,     // Client name that uniquely identify your device
  mqtt_port              // The MQTT port, default to 1883. this line can be omitted
);

void setup(){
  wifi_country_t country = { .cc = "ES", .schan = 1, .nchan = 13, .policy = WIFI_COUNTRY_POLICY_AUTO };
  String nombreNodo;
  
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  Serial.print(F("Termometro v"));
  Serial.println(TERMOMETRO_VERSION);
  chipId = ESP.getChipId();
  nombreNodo = tipoNodo + chipId;
  Serial.println();
  Serial.print(F("ESP8266 Chip id = "));
  Serial.println(chipId);
  Serial.print(F("Mi nombre: "));
  Serial.println(nombreNodo);
  nombreNodo.toCharArray(myname, nombreNodo.length() + 1);

  String mqttBaseThisChip = mqttBase + chipId;
  String mqttBaseThisChipType = mqttBaseThisChip + mqttType;
  mqttMonThisChip = mqttBaseThisChip + mqttMon;
  
  mqttTempThisChip = mqttBaseThisChipType + mqttTemp;
  mqttHumThisChip = mqttBaseThisChipType + mqttHum;
  mqttTempThisChipStatus = mqttTempThisChip + "status/";
  mqttHumThisChipStatus = mqttHumThisChip + "status/";
    
  Serial.print(F("mqttTempThisChip: "));
  Serial.println(mqttTempThisChip);
  Serial.print(F("mqttHumThisChip: "));
  Serial.println(mqttHumThisChip);
  Serial.print(F("mqttMonThisChip: "));
  Serial.println(mqttMonThisChip);
  wifi_set_country(&country);

  
  dht.setup(DHTPIN, DHTesp::DHT22); 
  
// Optional functionnalities of EspMQTTClient : 
//  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
//  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  wifi_country_t country;
  const String mqttMonThisChipConnectedAtMillis = mqttMonThisChip + "connectedatmillis/";
  const String mqttMonThisChipIP = mqttMonThisChip + "ip/";
  const String mqttMonThisChipHostname = mqttMonThisChip + "hostname/";
  const String mqttMonThisChipVersion = mqttMonThisChip + "version/";
  
  
  // Print WiFi data to serial
  Serial.println();
  Serial.print(F("Connected, IP address: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("Hostname: "));
  Serial.println(WiFi.hostname());
  Serial.print(F("DNS Server: "));
  Serial.print(WiFi.dnsIP());
  Serial.print(F(", "));
  Serial.println(WiFi.dnsIP(1));
  Serial.print(F("RSSI: "));
  Serial.print(WiFi.RSSI());
  Serial.println(F(" dBm"));
  wifi_get_country(&country);
  Serial.print(F("Wifi Country: "));
  Serial.print(country.cc);
  Serial.print(F(" schan: "));
  Serial.print(country.schan);
  Serial.print(F(" nchan: "));
  Serial.print(country.nchan);
  Serial.print(F(" policy: "));
  Serial.println(country.policy);
    
  client.publish(mqttMonThisChipConnectedAtMillis, String(millis()), true);
  client.publish(mqttMonThisChipIP, WiFi.localIP().toString(), true);
  client.publish(mqttMonThisChipHostname, String(WiFi.hostname()), true);
  client.publish(mqttMonThisChipVersion, TERMOMETRO_VERSION, true);

  float hum = dht.getHumidity();
  float temp = dht.getTemperature();
  String status = dht.getStatusString();
  client.publish(mqttTempThisChipStatus, status);
  client.publish(mqttTempThisChip, String(temp));
  client.publish(mqttHumThisChip, String(hum));
}

void reportMonitoringData()
{
  const String mqttMonThisChipFreeHeap = mqttMonThisChip + "freeheap/";
  const String mqttMonThisChipUptime = mqttMonThisChip + "uptime/";
  const String mqttMonThisChipRSSI = mqttMonThisChip + "rssi/";
  
  if (client.isConnected())
  {
    client.publish(mqttMonThisChipFreeHeap, String(ESP.getFreeHeap()));
    client.publish(mqttMonThisChipUptime, String(millis()));
    client.publish(mqttMonThisChipRSSI, String(WiFi.RSSI()));
  }
}

void loop(){
  unsigned long currentMillis = millis();
  float hum;
  float temp;
  String status;
  
  client.loop();

  //delay(dht.getMinimumSamplingPeriod());
  
  if (currentMillis - previousMillisMon >= MONITORING_DELAY_MS)
  {
    // save the last time we entered here
    previousMillisMon = currentMillis;
    reportMonitoringData();
  }
  
  if (currentMillis - previousMillis >= DELAY_MS)
  {
    // save the last time we entered here
    previousMillis = currentMillis;

    hum = dht.getHumidity();
    temp = dht.getTemperature();
    status = dht.getStatusString();
    Serial.println(status);
    Serial.print(F("Temperatura: "));
    Serial.print(temp);
    Serial.print(F("ºC - "));
    Serial.print(F("Humedad: "));
    Serial.print(hum);
    Serial.println(F("%"));
    
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
}
