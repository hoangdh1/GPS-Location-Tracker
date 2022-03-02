#include <NTPClient.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <SoftwareSerial.h>
#include <SocketIoClient.h>
#include <WiFiUdp.h>
#include <TinyGPS.h>
TinyGPS gps;
#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
//String formattedDate;
//String dayStamp;
//String timeStamp;

SocketIoClient webSocket;

void event(const char * payload, size_t length) {
  USE_SERIAL.printf("****************************got message: %s\n", payload);
}


const byte RX = D1;
const byte TX = D2;
SoftwareSerial gpsSerial(RX, TX);

struct data_GPS {
  float LON;
  float LAT;
};

// hàm đọc gps
struct data_GPS readDataGPS() {
  float flon; float flat;
  data_GPS data_LON_LAT;
  bool newData = false;

  flon = 105.84311241877799;
  flat = 21.00701423659785;       // ĐHBK HN


  // Mỗi 1s sẽ đọc GPS
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (gpsSerial.available())
    {
      char c = gpsSerial.read();
      Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    //float flat, flon; //flat = LAT ; flon = LON;
    gps.f_get_position(&flat, &flon);

    data_LON_LAT.LON = flon;
    data_LON_LAT.LAT = flat;
    return data_LON_LAT;
  }
}

// SIMULATION
int simulation = 1;
data_GPS fake_data_GPS, p0, p1;

//    dt = (t - t0) / (t1 - t0) // fraction of time elapsed between t0 & t1
float dt = 0.8;

void onSimulateFromServer(const char * payload, size_t length) {
  USE_SERIAL.printf("**************************Mode simulation: ", payload);
  if (payload == "on") {
    simulation++;
  }
  void offSimulateFromServer(const char * payload, size_t length) {
    USE_SERIAL.printf("**************************Mode simulation: ", payload);
    if (payload == "off") {
      simulation = 0;
    }
  }

}


void setup() {


  USE_SERIAL.begin(9600);
  gpsSerial.begin(9600);
  USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFiMulti.addAP("sam", "19701975");

  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  // Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(+7 * 60 * 60);

  webSocket.on("hi", event);
  webSocket.begin("192.168.1.228", 4000);
  // use HTTP Basic Authorization this is optional remove if not needed
  // webSocket.setAuthorization("username", "password");
}



void loop() {
  timeClient.update();
  //  String formattedTime = timeClient.getFormattedTime();
  unsigned long epochTime =  timeClient.getEpochTime();
  String timestamp = String(epochTime);

  //  String day = timeClient.getDay();

  //  unsigned long epcohTime =  timeClient.getEpochTime();
  //  String timestamp = String(timestamp);

  //  // The formattedDate comes with the following format:
  //  // 2018-05-28T16:00:13Z
  //  // We need to extract date and time
  //  formattedDate = timeClient.getFormattedDate();
  //  // Extract time
  //  int splitT = formattedDate.indexOf("T");
  //  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);

  webSocket.loop();
  data_GPS dataGPS;
  dataGPS = readDataGPS();
  float LO = dataGPS.LON;
  float LA = dataGPS.LAT;
  // LO = 105.84311241877799;
  //LA = 21.00701423659785;

  Serial.print("LONG="); Serial.println(LO, 6);
  Serial.print("LAT="); Serial.println(LA, 6);

  //  {\"data\":{\"geometry\":{\"type\":\"Point\",\"coordinates\":[105.84311241,20.0070142]},\"type\":\"Feature\",\"properties\":{\"title\":\"Xe Bus 16\"}}}

  String str2, str3; // String longtitude, latitude is sent to server
  /////////////////////////////////////////////
  webSocket.on("onSimulateFromServer", onSimulateFromServer);
  webSocket.on("offSimulateFromServer", offSimulateFromServer);
  if (simulation) {

    if (simulation == 1) {
      //      21.007417, 105.843992  21.007209, 105.844220  21.001866, 105.844997 , dai la: 20.996806, 105.845835
      p0.LON = 105.843992;
      p0.LAT = 21.007417;
      p1.LON = 105.844220;
      p1.LAT = 21.007209;
    }

    //    y = y1 + ((x - x1) / (x2 - x1)) * (y2 - y1)
    //    fake_data_GPS.LON = p1.LON + (p1.LON - p0.LON)/5;
    //    fake_data_GPS.LAT = p0.LAT + ((fake_data_GPS.LON - p0.LON) / (p1.LON - p0.LON)) * (p1.LAT - p0.LAT);  // interpolate from p0, p1 and longtitude of new data

    fake_data_GPS.LON = p1.LON + ( dt * (p1.LON - p0.LON) );  // the point's x is that same fraction between x0 and x1
    fake_data_GPS.LAT = p1.LAT + ( dt * (p1.LAT - p0.LAT) );  // ditto, y.

    p0.LON = p1.LON;
    p0.LAT = p1.LAT;

    p1.LON = fake_data_GPS.LON;
    p1.LAT = fake_data_GPS.LAT;

    str2 = String(fake_data_GPS.LON, 6); // String Longtitude to send
    str3 = String(fake_data_GPS.LAT, 6);  // String Latitude to send

    simulation++;
  }
  else {
    str2 = String(LO, 6);
    str3 = String(LA, 6);
  }

  /////////////////////////////////////////////////




  // This will send a string to the server
  Serial.println("sending data to server");
  String str1 = "\"data\":{\"timestamp\":" + timestamp + ",\"geometry\":{\"type\":\"Point\",\"coordinates\":["         ;
  String str4 = "]},\"type\":\"Feature\",\"properties\":{\"title\":\"Xe Bus 16\"}}"     ;
  String str = "{" + str1 + str2 + "," + str3 + str4 + "}" ;
  //  char* pl= strToChar(str);


  // Length (with one extra character for the null terminator)
  int str_len = str.length() + 1;
  Serial.println("data send:");
  Serial.println(str);
  // Prepare the character array (the buffer)
  char char_array[str_len];
  str.toCharArray(char_array, str_len);
  webSocket.emit("sendData", char_array);
  delay(2000);
}
