// #include <SD.h>
// #include <SPI.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266Ping.h>

// const char *ssid = "276533VCT1";
// const char *password = "03734109213";

// const int chipSelect = D8;

// File dataFile; // Declare Ping object
// String stringone;

// String teste(int number)
// {
//   int res = round(ceil((number / 3) + 1));
//   return "colcho_" + String(res);
// }

// void setup()
// {
//   Serial.begin(460800);

//   Connect to Wi-Fi
//     connectToWiFi();

//   if (!SD.begin(chipSelect))
//   {
//     Serial.println("SD card initialization failed!");
//     return;
//   }

//   Serial.println("SD card initialized.");
// }

// void loop()
// {
//   fromSdtoArd();
//   Ping test every 10 seconds
//   delay(20000);
//     if (WiFi.status() == WL_CONNECTED) {
//       if(Ping.ping("www.gooogg.com")) {
//         Serial.println("pingou");
//       }
//       else {
//         Serial.println("nao pingou");
//       }
//     } else {
//       Serial.println("Not connected to Wi-Fi.");
//       // Reconnect to Wi-Fi
//       connectToWiFi();
//     }
// }

// void connectToWiFi()
// {
//   Serial.println("Connecting to Wi-Fi...");
//   WiFi.begin(ssid, password);

//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(1000);
//     Serial.println("Connecting...");
//   }

//   Serial.println("Connected to Wi-Fi. IP address: " + WiFi.localIP().toString());
// }

// void fromSdtoArd()
// {

//   Open the file. Change "data.csv" to your file name.
//   dataFile = SD.open("temperaturaa.csv");

//   if (dataFile)
//   {
//     Serial.println("File opened successfully:");

//     while (dataFile.available())
//     { // Read and print the file contents
//       stringone = dataFile.readString();
//       String stringtwo = stringone.replace("", "/n");
//       int index = 184;
//       int count = 0;
//       for (size_t i = 184; i < stringone.length(); i++)
//       {
//         if (stringone[i] == '\n')
//         {
//           count++;
//           parseAndStoreValues(stringone.substring(index, i));
//           Serial.println(stringone.substring(index, i));
//           Serial.println(" ");
//           index = i + 1;
//         }
//       }
//       Serial.println(count);

//       Serial.println(stringone);
//       parseAndStoreValues(stringone);

//       Close the file
//       dataFile.close();
//     }
//   }
//   else
//   {
//     Serial.println("Error opening file.");
//   }
// }
// void parseAndStoreValues(String input)
// {
//   Define variables to store the parsed values
//   String time;
//   float values[9]; // Assuming 9 numeric values in the input

//   Find the position of the first comma
//   int commaIndex = input.indexOf(',');

//   Extract the time value
//   time = input.substring(0, commaIndex);
//   input = input.substring(commaIndex + 1);

//   Parse and store the numeric values
//   for (int i = 0; i < 9; i++)
//   {
//     commaIndex = input.indexOf(',');
//     if (commaIndex == -1)
//     {
//       Last value in the string
//       values[i] = input.toFloat();
//     }
//     else
//     {
//       values[i] = input.substring(0, commaIndex).toFloat();
//       input = input.substring(commaIndex + 1);
//     }
//   }

//   Print the parsed values for verification
//   Serial.println("TimesTamp: " + time);
//   for (int i = 0; i < 9; i += 3)
//   {
//     Serial.println(teste(i) + ": sensor_de_baixo: " + values[i]);

//     Serial.println(teste(i + 1) + ": sensor_do_meio: " + values[i + 1]);

//     Serial.println(teste(i + 2) + ": sensor_de_cima: " + values[i + 2]);
//   }
// }