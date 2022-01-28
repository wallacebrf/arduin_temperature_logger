#include <avr/io.h>
#include <avr/pgmspace.h>
//#include <Ethernet2.h>//needed for newer 5500 Ethernet shield chip sets
#include <Ethernet.h>//needed for older 5200 Ethernet shield chip sets 
#include <OneWire.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

//USER DEFINED VARIABLES
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01 }; //MAC address assigned to the Arduino Ethernet Shield
byte localip[] = {192, 168, 1, 45}; //STATIC IP address assigned to the Arduino Ethernet Shield
byte serverip[] = {192, 168, 1, 13}; //target server IP address running the PHP script
#define CSG1wirepin 38 //arduino PIN used by the 1-wire temp sensor #1
#define MSG1wirepin 44 //arduino PIN used by the 1-wire temp sensor #2
long interval = 60000; // READING INTERVAL --> how often is a data sample taken, this default value = every 60 seconds
byte debug=0;

EthernetClient client;




long previousMillis = 0;
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

void setup() { 
  Serial.begin(115200);

  Ethernet.begin(mac, localip);
  Serial.print(F("Version 2.0 1/28/2022. booting....."));
  wdt_enable(WDTO_4S);
}

void loop(){
     wdt_reset();
     byte temp_whole, temp_fract, temp_status;
     currentMillis = millis();
     CONVERT_TEMP(MSG1wirepin, temp_whole, temp_fract, temp_status);
     middle_temp_whole = temp_whole;
     middle_temp_fract = temp_fract;
     middle_temp_status = temp_status;
     Serial.print(temp_whole);
     Serial.print(".");
     Serial.println(temp_fract);
     CONVERT_TEMP(CSG1wirepin, temp_whole, temp_fract, temp_status);
     cold_side_temp_whole = temp_whole;
     cold_side_temp_fract = temp_fract;
     cold_side_temp_status = temp_status;
     Serial.print(temp_whole);
     Serial.print(".");
     Serial.println(temp_fract);
     
  if(currentMillis - previousMillis > interval) { // READ ONLY ONCE PER INTERVAL
      previousMillis = currentMillis; 
      if(debug==1){
        Serial.print(F("Times Up"));
      }

      if (client.connect(serverip,80)) { // REPLACE WITH YOUR SERVER ADDRESS
        Serial.println(F("Client Connected"));
        client.print(F("GET /admin/temperature_add.php?"));
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
          Serial.print(F("could not connect to server"));
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

// Just a utility function to nicely format an IP address.
const char* ip_to_str(const uint8_t* ipAddr)
{
  static char buf[16];
  sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}
