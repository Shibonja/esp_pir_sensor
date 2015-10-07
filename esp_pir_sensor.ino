#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Time.h> 

const int timeZone = 1;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

#define INPUT_PIN 12

const char* ssid     = "SSID";
const char* password = "WIFI password";


/* Configure data for UDP packets */
IPAddress timeServerIP;                         // time.nist.gov NTP server address
unsigned int localPort = 2390;                  // local port to listen for UDP packets
const char* ntpServerName = "time.nist.gov";

WiFiUDP udp;                                    // A UDP instance to let us send and receive packets over UDP

WiFiClient client;                              // Use WiFiClient class to create TCP connections

/*  GadgetKeeper Credentials                                                                    *
 *  To obtain the Thing Id, see instructions http://docs.gadgetkeeper.com/display/docs/Thing+Id */
String thing_ID = "Thing_Id";

/*  To obtain the Event Id see instructions http://wiki.gadgetkeeper.com/display/docs/Event+Id  */
String event_ID ="Event_Id";

/*  To obtain the API Key see instructions http://docs.gadgetkeeper.com/display/docs/API+Keys */
String api_key = "API_Key";

const char* host = "api.gadgetkeeper.com";

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
    Serial.println("motion detected!");
    setSyncProvider(getNtpTime);    //get correct time
    motionTrigerEvent();            //send Tigger Request
    delay(2000);                    //this delay (2s) is here so that the PIR doesnt send continously packets
  }

  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}

void motionTrigerEvent(){
  //first we connect to GadgetKeeper Cloud Platform
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
    /*  in this section here we are creating the POST Request for GadgetKeeper event trigger                                  *
     *  for more information about events and triggers see http://docs.gadgetkeeper.com/display/docs/Create+Event+And+Trigger */
    String Date = String(year())+ "-" + printDigits(String(month()))+"-"+printDigits(String(day()));
    String Time = printDigits(String(hour())) + ":"+ printDigits(String(minute())) +":"+printDigits(String(second()));
    client.print("POST " + event_url + " HTTP/1.1\r\n");
    client.print("Host: " + String(host) + "\r\n");
    client.print("X-ApiKey: " + String(api_key) + "\r\n");
    client.print("Content-Type: application/json; charset=UTF-8\r\n");
    client.print("Cache-Control: no-cache\r\n");
    client.print("Content-Length: 50\r\n");
    client.print("\n[{\"value\":true,\"at\":\""+ Date +"T" + Time +".000Z\"}]\r\n");
    delay(100);
}

// this fucntion add 0 to time digits 0-9
String printDigits(String digits){
  if(digits.length() < 2)
    return 0+digits;
  return digits;
}


/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48;          // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE];     // buffer to hold incoming and outgoing UDP packets

time_t getNtpTime(){
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
//    Serial.print("Seconds since Jan 1 1900 = " );
//    Serial.println(secsSince1900);
    return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
  }
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

