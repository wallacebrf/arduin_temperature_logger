#include <avr/io.h>
#include <avr/pgmspace.h>
#include <Ethernet.h>//needed for older 5200 Ethernet shield chip sets 
#include <OneWire.h>
#include <avr/wdt.h>
#include <Dns.h>

//USER DEFINED VARIABLES
IPAddress subnet(255,255,255,0);
IPAddress healthcheckio_IP;
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x05 };       //MAC address assigned to the Arduino Ethernet Shield
byte serverip[] = {192, 168, 1, 13};                      //target server IP address running the PHP script to ingest temperature data
#define CSG1wirepin 22                                    //arduino PIN used by the 1-wire temp sensor #1
#define MSG1wirepin 23                                    //arduino PIN used by the 1-wire temp sensor #2
long interval_temp = 60000;                               // READING INTERVAL --> how often is a data sample taken, this default value = every 60 seconds
long interval_heartbeat = 900000;                         // READING INTERVAL --> how often is healthchecks.io contacted for a heartbeat update? value = 15 minutes 
byte debug=0;
const char* ip_to_str(const uint8_t*);

EthernetClient client;
DNSClient dnClient;

long previousMillis_temp = 0;
long previousMillis_heartbeat = 0;
unsigned long currentMillis = 0;
byte middle_temp_whole;                            
byte cold_side_temp_whole; 
byte middle_temp_fract;
byte cold_side_temp_fract;
byte middle_temp_status;                              //the status of the the middle side thermal sesnor. 1 = good, 0 = bad. 
byte cold_side_temp_status;                              //the status of the the cold side thermal sesnor. 1 = good, 0 = bad
int cold_badsensorcount =0;
int middle_badsensorcount =0;
byte temp_scale = 1;

// Just a utility function to nicely format an IP address.
const char* ip_to_str(const uint8_t* ipAddr)
{
  static char buf[16];
  sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}

void setup() { 
  Serial.begin(115200);
  //Ethernet.begin(mac, ip, myDNS, gateway,subnet); 
  
  Serial.println(F("Version 3.0 3/8/2023. booting....."));
  Serial.println(F("Initialize Ethernet with DHCP:"));

  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println(F("Ethernet cable is not connected."));
    }

    // no point in carrying on, so do nothing forevermore:

    while (true) {
      delay(1);
    }

  }

  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  
  dnClient.begin(Ethernet.dnsServerIP());
  if(dnClient.getHostByName("hc-ping.com",healthcheckio_IP) == 1) {
    Serial.print(F("hc-ping.com = "));
    Serial.println(healthcheckio_IP);
    Serial.println("");
    Serial.println("");
  }else{ 
    Serial.print(F("dns lookup failed"));
  }
 
  wdt_enable(WDTO_4S);
}

void loop(){
  //maintain DHCP IP lease
  switch (Ethernet.maintain()) {
    case 1:
      //renewed fail
      Serial.println(F("Error: renewed fail"));
      break;
    case 2:
      //renewed success
      Serial.println(F("Renewed success"));
      //print your local IP address:
      Serial.print(F("My IP address: "));
      Serial.println(Ethernet.localIP());
      break;
    case 3:
      //rebind fail
      Serial.println(F("Error: rebind fail"));
      break;
    case 4:
      //rebind success
      Serial.println(F("Rebind success"));
      //print your local IP address:
      Serial.print(F("My IP address: "));
      Serial.println(Ethernet.localIP());
      break;
    default:
      //nothing happened
      break;
  }
     wdt_reset();
     byte temp_whole, temp_fract, temp_status;
     currentMillis = millis();
     CONVERT_TEMP(MSG1wirepin, temp_whole, temp_fract, temp_status);
     middle_temp_whole = temp_whole;
     middle_temp_fract = temp_fract;
     middle_temp_status = temp_status;
     //Serial.print(temp_whole);
     //Serial.print(".");
    // Serial.println(temp_fract);
     CONVERT_TEMP(CSG1wirepin, temp_whole, temp_fract, temp_status);
     cold_side_temp_whole = temp_whole;
     cold_side_temp_fract = temp_fract;
     cold_side_temp_status = temp_status;
     //Serial.print(temp_whole);
     //Serial.print(".");
     //Serial.println(temp_fract);
     
  if(currentMillis - previousMillis_temp > interval_temp) { // READ ONLY ONCE PER INTERVAL
      previousMillis_temp = currentMillis; 
      if(debug==1){
        Serial.print(F("Temperature Times Up"));
      }

      if (client.connect(serverip,80)) { 
        Serial.println(F("Temperature Client Connected"));
        client.print(F("GET /admin/equipment_cabinet_add.php?"));
        client.print(F("middle_temp_whole="));
        client.print(middle_temp_whole); 
        client.print(F("&middle_temp_fract="));
        client.print(middle_temp_fract); 
        client.print(F("&middle_temp_status="));
        client.print(middle_temp_status); 
        client.print(F("&cold_side_temp_whole="));
        client.print(cold_side_temp_whole); 
        client.print(F("&cold_side_temp_fract="));
        client.print(cold_side_temp_fract); 
        client.print(F("&cold_side_temp_status="));
        client.print(cold_side_temp_status);
        client.print(F("&cold_badsensorcount="));
        client.print(cold_badsensorcount);
        client.print(F("&middle_badsensorcount="));
        client.print(middle_badsensorcount);
        client.println(F(" HTTP/1.1"));
        client.print(F("Host: "));
        client.println(ip_to_str(serverip));
        client.println(F("Content-Type: application/x-www-form-urlencoded"));
        client.println(F("Connection: close"));
        client.println();
        client.println();
        client.println(F("Connection: close"));
        client.println();
        client.println();
        client.println(F("Connection: close"));
        client.println();
        client.println();
        client.stop();
        client.stop();
      } else{
          Serial.print(F("Temperature Logging could not connect to server"));
      }

      if(debug==1){
        Serial.print(F("middle_temp="));
        Serial.print(middle_temp_whole); 
        Serial.print(F("."));
        Serial.println(middle_temp_fract); 
        Serial.print(F("&middle_temp_status="));
        Serial.println(middle_temp_status); 
        Serial.print(F("&middle_badsensorcount="));
        Serial.println(middle_badsensorcount); 
        Serial.println(); 
        Serial.print(F("&cold_side_temp"));
        Serial.print(cold_side_temp_whole); 
        Serial.print(F("."));
        Serial.println(cold_side_temp_fract); 
        Serial.print(F("&cold_side_temp_status="));
        Serial.println(cold_side_temp_status); 
        Serial.print(F("&cold_badsensorcount="));
        Serial.println(cold_badsensorcount); 
        Serial.println(); 
      }
    }

if(currentMillis - previousMillis_heartbeat > interval_heartbeat) { // PERFORM ONLY ONCE PER INTERVAL
      previousMillis_heartbeat = currentMillis; 
      if(dnClient.getHostByName("hc-ping.com",healthcheckio_IP) == 1) {
		Serial.print(F("hc-ping.com = "));
		Serial.println(healthcheckio_IP);
	  }else{
		Serial.print(F("dns lookup failed"));
	  }
      if(debug==1){
        Serial.println(F("Heartbeat Times Up"));
      }

      if (client.connect(healthcheckio_IP,80)) {
        Serial.println(F("Heartbeat Client Connected"));
        Serial.println("");
        client.println(F("GET /xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx HTTP/1.1"));
        client.println(F("Host: hc-ping.com"));
        client.println(F("Connection: close"));
        client.println();
        client.stop();
      } else{
          Serial.print(F("Heartbeat could not connect to server"));
      }
  }
}


void CONVERT_TEMP(byte Sensor_PIN, byte & temp_whole, byte & temp_fract, byte & temp_status) {
  OneWire  dSensor1(Sensor_PIN); 
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  int HighByte, LowByte, TReading, SignBit, Tc_100;
  
  
  if ( !dSensor1.search(addr)) {
    dSensor1.reset_search();
    delay(250);
    temp_whole = 0;
    temp_fract = 0;
    temp_status = 0;
    if (Sensor_PIN==CSG1wirepin){
      cold_badsensorcount++;
    }else if (Sensor_PIN==MSG1wirepin){
      middle_badsensorcount++;
    }
    return;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      temp_whole = 0;
    temp_fract = 0;
    temp_status = 0;
    if (Sensor_PIN==CSG1wirepin){
      cold_badsensorcount++;
    }else if (Sensor_PIN==MSG1wirepin){
      middle_badsensorcount++;
    }
    return;
  }

  // The DallasTemperature library can do all this work for you!

  dSensor1.reset();
  dSensor1.select(addr);
  dSensor1.write(0x44,0);         // start conversion, without parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a dSensor1.depower() here, but the reset will take care of it.
  
  present = dSensor1.reset();
  dSensor1.select(addr);    
  dSensor1.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = dSensor1.read();
  }

  LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
  if (SignBit) // negative
  {
    TReading = (TReading ^ 0xffff) + 1; // 2's comp
  }
  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25
  
  if (temp_scale == 1){//if using degrees F, the celcius reading from the sensor must be converted
     temp_whole = (int)(((Tc_100)*(1.8))+3200)/100;
     temp_fract = (int)(((Tc_100)*(1.8))+3200)%100;
     if (temp_whole > 32){
       temp_status = 1;//if the temperature is above 32 degrees F, then the sensor is OK
     }else{
       temp_status = 0;//the sensor must be bad as the temperature must never get down as low as 32 degrees F, the snake would die!!
     }
   }else{//if using degrees C, the temp reading is good as is.
     temp_whole = Tc_100/100;
     temp_fract = Tc_100%100;
     if (temp_whole > 0){
       temp_status = 1;//if the temperature is above 0 degrees C, then the sensor is OK
     }else{
       temp_status = 0;//the sensor must be bad as the temperature must never get down as low as 0 degrees C
     }
   }
}
