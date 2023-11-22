// #include <SD.h>
// #include <SPI.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266Ping.h>

// const char* ssid = "276533VCT1";
// const char* password = "03734109213";

// const int chipSelect = D8; 

// File dataFile; // Declare Ping object

// void setup() {
//   Serial.begin(115200);

//   // Connect to Wi-Fi
//   connectToWiFi();

//   if (!SD.begin(chipSelect)) {
//     Serial.println("SD card initialization failed!");
//     return;
//   }

//   Serial.println("SD card initialized.");

//   // Open the file. Change "data.csv" to your file name.
//   dataFile = SD.open("temp.csv", FILE_WRITE);

//   if (dataFile) {
//     Serial.println("File opened successfully:");

//     // Read and print the file contents
//     while (dataFile.available()) {
//       dataFile.println("Timestamp,Value1,Value2");
//       Serial.write(dataFile.read());
//     }

//     // Close the file
//     dataFile.close();
//   } else {
//     Serial.println("Error opening file.");
//   }
// }

// void loop() {
//   // Ping test every 10 seconds
//   delay(10000);
//   if (WiFi.status() == WL_CONNECTED) {
//     if(Ping.ping("www.gooogg.com")) {
//       Serial.println("pingou");
//     }
//     else {
//       Serial.println("nao pingou");
//     }
//   } else {
//     Serial.println("Not connected to Wi-Fi.");
//     // Reconnect to Wi-Fi
//     connectToWiFi();
//   }
// }

// void connectToWiFi() {
//   Serial.println("Connecting to Wi-Fi...");
//   WiFi.begin(ssid, password);

//   while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     Serial.println("Connecting...");
//   }

//   Serial.println("Connected to Wi-Fi. IP address: " + WiFi.localIP().toString());
// }
