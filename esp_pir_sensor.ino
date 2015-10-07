#include <ESP8266WiFi.h>

#define INPUT_PIN 12

const char* ssid     = "Garaza";
const char* password = "nemainterneta";
/*  url_GK syntax is "http://api.gadgetkeeper.com/v1/things/<THING_ID>/properties/<PROPERTY_ID>/value.json"
    to obtain THING_ID see instructions here: http://docs.gadgetkeeper.com/display/docs/Thing+Id
    to obtain PROPERTY_ID see instructions here: http://docs.gadgetkeeper.com/display/docs/Property+Id    */
String url_GK = "/v1/things/d5b7ff9c6c3511e5accf2f3e5a24aa78/properties/f64c25bf6c3511e5accf2f3e5a24aa78/value.json";
const char* host = "api.gadgetkeeper.com";
//const char* host = "http://api.gadgetkeeper.com/v1/things/d5b7ff9c6c3511e5accf2f3e5a24aa78/properties/f64c25bf6c3511e5accf2f3e5a24aa78/value.json";

void setup() {
  Serial.begin(115200);
  pinMode(INPUT_PIN, INPUT);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void loop() {

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  
  if(digitalRead(INPUT_PIN)==HIGH) {
    Serial.println("motion detected!");
//    client.print("PUT /v1/things/d5b7ff9c6c3511e5accf2f3e5a24aa78/properties/f64c25bf6c3511e5accf2f3e5a24aa78/value.json HTTP/1.1\r\n");
//    client.print("Content-Type: text/json; charset=UTF-8\r\n");
//    client.print("Accept: */*\r\n");
//    client.print("X-ApiKey: 7a55bedee24a469696467caf9c50c570\r\n");
//    client.print("Content-Length: 2\r\n");
//    client.print("35");
      
      client.print("PUT /v1/things/d5b7ff9c6c3511e5accf2f3e5a24aa78/properties/f64c25bf6c3511e5accf2f3e5a24aa78/value.json HTTP/1.1\r\n");
      client.print("Host: api.gadgetkeeper.com\r\n");
      client.print("X-ApiKey: 7a55bedee24a469696467caf9c50c570\r\n");
      client.print("Content-Type: application/json; charset=UTF-8\r\n");
      client.print("Cache-Control: no-cache\r\n");
      client.print("Content-Length: 2\r\n");
//      client.print("Postman-Token: fa498676-4f8f-6e70-2a03-fdac1376e67e\r\n");
      client.print("\n40\r\n");
    delay(100);
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
    
  }
    delay(2000);
  }

    
  delay(500);

}
