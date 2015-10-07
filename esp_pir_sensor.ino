#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define INPUT_PIN 12

const char* ssid     = "Garaza";
const char* password = "nemainterneta";


/* Configure data for UDP packets */
IPAddress timeServerIP;                         // time.nist.gov NTP server address
unsigned int localPort = 2390;                  // local port to listen for UDP packets
const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48;                 // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE];            // buffer to hold incoming and outgoing UDP packets
WiFiUDP udp;                                    // A UDP instance to let us send and receive packets over UDP

WiFiClient client;                              // Use WiFiClient class to create TCP connections

/*  GadgetKeeper Credentials                                                                    *
 *  To obtain the Thing Id, see instructions http://docs.gadgetkeeper.com/display/docs/Thing+Id */
String thing_ID = "d5b7ff9c6c3511e5accf2f3e5a24aa78";

/*  To obtain the Property Id see instructions http://docs.gadgetkeeper.com/display/docs/Property+Id */
String property_ID = "f64c25bf6c3511e5accf2f3e5a24aa78";

/*  To obtain the Event Id see instructions http://wiki.gadgetkeeper.com/display/docs/Event+Id  */
String event_ID ="b34679d36cec11e5accf2f3e5a24aa78";

/*  To obtain the API Key see instructions http://docs.gadgetkeeper.com/display/docs/API+Keys */
String api_key = "7f213b0a04434cf788e30d8ee16f0534";

const char* host = "api.gadgetkeeper.com";

//String property_url = "/v1/things/" + thing_ID + "/properties/" + property_ID + "value.json";
String event_url = "/v1/things/" + thing_ID + "/events/" + event_ID + "/datapoints.json";


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

  
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
}
 
void loop() {
  
  if(digitalRead(INPUT_PIN)==HIGH) {
    //getTimeDate();  
    Serial.println("motion detected!");
    //sendMotionData();
    motionTrigerEvent(); 
    delay(2000);
  }

      while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
}

void motionTrigerEvent(){
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
    client.print("POST " + event_url + " HTTP/1.1\r\n");
    client.print("Host: " + String(host) + "\r\n");
    client.print("X-ApiKey: " + String(api_key) + "\r\n");
    client.print("Content-Type: application/json; charset=UTF-8\r\n");
    client.print("Cache-Control: no-cache\r\n");
    client.print("Content-Length: 48\r\n");
    client.print("\n[{\"value\":true,\"at\":\"2013-06-02T21:05:04.150Z\"}]\r\n");
    delay(100);

}

void sendMotionData(){
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
    client.print("PUT /v1/things/d5b7ff9c6c3511e5accf2f3e5a24aa78/properties/f64c25bf6c3511e5accf2f3e5a24aa78/value.json HTTP/1.1\r\n");
    client.print("Host: " + String(host) + "\r\n");
    client.print("X-ApiKey: " + String(api_key) + "\r\n");
    client.print("Content-Type: application/json; charset=UTF-8\r\n");
    client.print("Cache-Control: no-cache\r\n");
    client.print("Content-Length: 4\r\n");
    client.print("\ntrue\r\n");
    delay(100);
}

void getTimeDate(){
  WiFi.hostByName(ntpServerName, timeServerIP); 

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
                                 // wait to see if a reply is available
  delay(1000);
  
  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
  }
}

void sendRequest(String method, String URL){
  
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

