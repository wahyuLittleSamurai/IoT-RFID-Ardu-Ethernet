#include <SoftwareSerial.h> 
#include <TinyGPS++.h>
#include <SPI.h>
#include <Ethernet.h>
#include <RFID.h>

#define relayPinKey  A1
#define relayPinCamp  A0

#define MACADDRESS 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
#define MYIPADDR 192,168,3,26
#define MYIPMASK 255,255,255,0
#define MYDNS 8,8,8,8
#define MYGW 192,168,3,254
#define LISTENPORT 80

char server[] = "gpsmotor.mdtsolution.com";
EthernetClient client;                                             

    // CONFIG RFID
#define SS_PIN 9
#define RST_PIN 8
RFID rfid(SS_PIN, RST_PIN);  
           
TinyGPSPlus gpsModule;
SoftwareSerial sserial(2, 3);

char latitude[12];
char longitude[12];

String id = "";
String myKey = "2101023721772";

unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 1L * 60L * 1000L;

boolean rfidRead = false;

void setup()
{    Serial.begin(9600);
     sserial.begin(9600);
     SPI.begin(); 
     rfid.init();

     pinMode(relayPinKey, OUTPUT);
     pinMode(relayPinCamp, OUTPUT);
     pinMode(A5, OUTPUT);
     digitalWrite(A5, HIGH);
     delay(100);
     digitalWrite(A5, LOW);
     delay(50);
     digitalWrite(relayPinCamp, HIGH);
     digitalWrite(relayPinKey, HIGH);
     
         // disable rfid
     pinMode(9, OUTPUT);
     digitalWrite(9, HIGH);
     
         // disable sd card
     pinMode(4, OUTPUT);
     digitalWrite(4, HIGH);
      
         // enable ethernet
     pinMode(10, OUTPUT);
     digitalWrite(10, LOW); 
     
        // acquiring ip without dhcp
      uint8_t mac[6] = {MACADDRESS};
      uint8_t myIP[4] = {MYIPADDR};
      uint8_t myMASK[4] = {MYIPMASK};
      uint8_t myDNS[4] = {MYDNS};
      uint8_t myGW[4] = {MYGW};
      
      Ethernet.begin(mac,myIP,myDNS,myGW,myMASK);

      Serial.print("localIP: ");
      Serial.println(Ethernet.localIP());
      Serial.print("subnetMask: ");
      Serial.println(Ethernet.subnetMask());
      Serial.print("gatewayIP: ");
      Serial.println(Ethernet.gatewayIP());
      Serial.print("dnsServerIP: ");
      Serial.println(Ethernet.dnsServerIP());
  
     Serial.println("Ready");
     digitalWrite(relayPinKey, HIGH);
     digitalWrite(relayPinCamp, HIGH);
}

void loop()
{   // if a card is read, then get the id from the card and call make_request() for validation 
  
  if (rfid.isCard() && !rfidRead)
     {    if (rfid.readCardSerial())
          {    
               id = "";
               id+=rfid.serNum[0];
               id+=rfid.serNum[1];
               id+=rfid.serNum[2];
               id+=rfid.serNum[3];
               id+=rfid.serNum[4];
               Serial.println(id);   
               rfidRead = true;   
          }
     }
    if(rfidRead)
    {
      if(id == myKey)
      {
        digitalWrite(relayPinKey, LOW);
        Serial.println("yeayyyy...");
      }
      else
      {
        while(1)
        {
          Serial.println(millis());
          if(millis() > 120000)
          {
            digitalWrite(relayPinKey, HIGH);
            digitalWrite(relayPinCamp, HIGH);
          }
          else
          {
            digitalWrite(relayPinKey, LOW);
            digitalWrite(relayPinCamp, LOW);      
          }
          
          Serial.println("Undefine Key");
          while (sserial.available() > 0)
            gpsModule.encode(sserial.read());
          
          if (gpsModule.location.isValid()){
            Serial.println("GPS Module Online"); 
            Serial.println("GPS Module GET Coordinate"); 
            
            dtostrf(gpsModule.location.lat(), 1, 6, latitude);
            dtostrf(gpsModule.location.lng(), 1, 6, longitude);
            
            Serial.print("Latitude : ");
            Serial.print(latitude);
            Serial.print(", Longitude : ");
            Serial.println(longitude);
        
          }else{
            Serial.println("GPS Module Offline");  
          }
           
          if (millis() - lastConnectionTime > postingInterval) {
            httpPostData(latitude, longitude);
          } 
        }
      }
      rfidRead=false;
      delay(500);
    }
    
}

void httpPostData(char* latitude, char* longitude) {

  digitalWrite(A5, HIGH);
  delay(100);
  digitalWrite(A5, LOW);
  delay(50);
  digitalWrite(A5, HIGH);
  delay(100);
  digitalWrite(A5, LOW);
  delay(50);
  
  client.stop();
  
  String data;
  data.concat("latitude=");
  data.concat(latitude);
  data.concat("&longitude=");
  data.concat(longitude);
  
  Serial.println("Kirim koordinat...");
  if (client.connect(server, 80)) {
    Serial.println("Connected");
    client.println("POST /postdata.php HTTP/1.1"); 
    client.println("Host: gpsmotor.mdtsolution.com");
    client.println("Content-Type: application/x-www-form-urlencoded"); 
    client.print("Content-Length: "); 
    client.println(data.length()); 
    client.println(); 
    client.print(data);
    lastConnectionTime = millis();
    Serial.println("Sending Is Done");
  } else {
    Serial.println("connection failed");
  }
}


 


