#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <SD.h>
#include <SPI.h>
#include <ESP8266Ping.h>

#include <NTPClient.h>
#include <WiFiUdp.h>
// teste

#define FIREBASE_HOST "tcc-ifpa-default-rtdb.firebaseio.com" // the project name address from firebase id
#define FIREBASE_AUTH "z6f4IHnNRvN48XS9hJ1Zd7T8MOjASumayA8g2zar"

#define WIFI_SSID "276533VCT1" // input your home or public wifi name

#define WIFI_PASSWORD "03734109213"

FirebaseData firebaseData;
File dataFile;
String stringone;

// Data wire is connected to GPIO pin D1 (NodeMCU pin mapping)
const int oneWireBus = D1;
const int sensorsQnt = 9;

const int chipSelect = D8;

const String user = "clenilson";

// Create a OneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass the OneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

// Define NTP parameters
const char *ntpServerName = "pool.ntp.org";
const int utcOffsetInSeconds = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServerName, utcOffsetInSeconds);

String teste(int number)
{
    int res = round(ceil((number / 3) + 1));
    return "/colcho_" + String(res);
}

void setup()
{
    Serial.begin(460800);

    connectToWiFi();

    if (!SD.begin(chipSelect))
    {
        Serial.println("SD card initialization failed!");
        return;
    }

    Serial.println("SD card initialized.");

    // Open the file. Change "data.csv" to your file name.
    dataFile = SD.open("temp.csv", FILE_WRITE);

    if (dataFile)
    {
        Serial.println("File opened successfully:");

        dataFile.println(",colcho_1,,,colcho_2,,,colcho_3,");
        dataFile.println("timeStamp,sensor_de_baixo,sensor_do_meio,sensor_de_cima,sensor_de_baixo,sensor_do_meio,sensor_de_cima,sensor_de_baixo,sensor_do_meio,sensor_de_cima,");

        // Close the file
        dataFile.close();
    }
    else
    {
        Serial.println("Error opening file.");
    }

    // Start communication with the DS18B20 sensors
    sensors.begin();

    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

    timeClient.begin();
}

void loop()
{
    // Request temperature data from all sensors
    sensors.requestTemperatures();

    // Update the time client
    timeClient.update();

    // Print the current time
    Serial.println(timeClient.getFormattedTime());

    // Loop through each sensor and print the temperature
    float list[sensorsQnt] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    for (int i = 0; i < sensorsQnt; i++)
    {
        float temperature = sensors.getTempCByIndex(i);
        list[i] = temperature;
        Serial.print(list[i]);
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        if (Ping.ping("www.google.com"))
        {
            Serial.println("pingou");
            // uploadTemperature(list);
            // uploadSD(list);
            fromSdtoArd();
        }
        else
        {
            Serial.println("nao pingou");
        }
    }
    else
    {
        Serial.println("Not connected to Wi-Fi.");
        // Reconnect to Wi-Fi
        connectToWiFi();
    }
    String data = dataFile.readString();
    Serial.println(data);

    delay(10000); // Delay for 5 seconds before reading temperatures again
}

String replaceBl(const String &inputString)
{
    // Create a copy of the input string
    String modifiedString = inputString;

    // Remove dots
    modifiedString.replace(".", "");
    modifiedString.replace("\r", "");
    modifiedString.replace("\n", ";");
    return modifiedString;
}

void uploadTemperature(float *temperature)
{

    for (int i = 0; i < sensorsQnt; i += 3)
    {
        String low = user + teste(i + 1) + "/sensor_de_baixo";
        Firebase.setFloat(firebaseData, low.c_str(), temperature[i]);
        Serial.println(low);
        Serial.println(temperature[i]);

        String mid = user + teste(i + 1) + "/sensor_do_meio";
        Firebase.setFloat(firebaseData, mid.c_str(), temperature[i + 1]);
        Serial.println(mid);
        Serial.println(temperature[i + 1]);

        String high = user + teste(i + 1) + "/sensor_de_cima";
        Firebase.setFloat(firebaseData, high.c_str(), temperature[i + 2]);
        Serial.println(high);
        Serial.println(temperature[i + 2]);

        String usuario = user + teste(i + 1) + "/user";
        Firebase.setString(firebaseData, usuario.c_str(), user);
    }
    Serial.println("");
    Serial.println("");
}
// upload ds18b20 values to sdcard
void uploadSD(float *temperature)
{
    // Open the file. Change "data.csv" to your file name.
    dataFile = SD.open("temp.csv", FILE_WRITE);

    if (dataFile)
    {
        Serial.println("File opened successfully:");

        dataFile.print(timeClient.getFormattedTime());
        dataFile.print(",");
        for (int i = 0; i < sensorsQnt; i++)
        {
            dataFile.print(temperature[i]);
            dataFile.print(",");
        }
        dataFile.println("");

        // Close the file
        dataFile.close();
    }
    else
    {
        Serial.println("Error opening file.");
    }
}
void connectToWiFi()
{
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting...");
    }

    Serial.println("Connected to Wi-Fi. IP address: " + WiFi.localIP().toString());
}
void fromSdtoArd()
{

    // Open the file. Change "data.csv" to your file name.
    dataFile = SD.open("temperaturaa.csv");

    if (dataFile)
    {
        Serial.println("File opened successfully:");
        String stringone = "";

        while (dataFile.available())
        { // Read and print the file contents
            stringone = dataFile.readString();
            // String stringtwo = stringone.replace("", "/n");
            int index = 184;
            int count = 0;
            Serial.println(stringone);
            String low = "clenilson/backUp";
            stringone = replaceBl(stringone);
            Serial.println(stringone.indexOf('\n'));
            // for (size_t i = 0; i < stringone.length(); i++)
            // {
            //     if(stringone[i] == '\r'){
            //         count++;
            //     }
            // }
            // Serial.println(count);
            if (Firebase.setString(firebaseData, low.c_str(), stringone))
            {
                Serial.println("Data sent to Firestore!");
            }
            else
            {
                Serial.println("Error sending data to Firestore");
                Serial.println(firebaseData.errorReason());
            }
            Serial.println(stringone);

            // Close the file
            dataFile.close();
            //
        }
    }
    else
    {
        Serial.println("Error opening file.");
    }
}
void parseAndStoreValues(String input)
{
    // Define variables to store the parsed values
    String time;
    float values[9]; // Assuming 9 numeric values in the input

    // Find the position of the first comma
    int commaIndex = input.indexOf(',');

    // Extract the time value
    time = input.substring(0, commaIndex);
    input = input.substring(commaIndex + 1);

    // Parse and store the numeric values
    for (int i = 0; i < 9; i++)
    {
        commaIndex = input.indexOf(',');
        if (commaIndex == -1)
        {
            // Last value in the string
            values[i] = input.toFloat();
        }
        else
        {
            values[i] = input.substring(0, commaIndex).toFloat();
            input = input.substring(commaIndex + 1);
        }
    }

    // Print the parsed values for verification
    Serial.println("TimesTamp: " + time);
    for (int i = 0; i < 9; i += 3)
    {
        Serial.println(teste(i) + ": sensor_de_baixo: " + values[i]);

        Serial.println(teste(i + 1) + ": sensor_do_meio: " + values[i + 1]);

        Serial.println(teste(i + 2) + ": sensor_de_cima: " + values[i + 2]);
    }
}
