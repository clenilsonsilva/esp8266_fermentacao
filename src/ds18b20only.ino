#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <SD.h>
#include <SPI.h>
#include <ESP8266Ping.h>

#include <Wire.h>
#include <RTClib.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <dht.h>
// teste

#define FIREBASE_HOST_1 "tcc-ifpa-default-rtdb.firebaseio.com" // the project name address from firebase id
#define FIREBASE_HOST_2 "backup-data.firebaseio.com"           // the project name address from firebase id
#define FIREBASE_AUTH "z6f4IHnNRvN48XS9hJ1Zd7T8MOjASumayA8g2zar"
#define API_KEY "AIzaSyDfAljJmaaMx79MySzUNE3gYFoeDAYLkRs"
#define EMAIL "luiz@gmail.com"
#define PASSWORD "123456"

#define WIFI_SSID "Clenilson" // input your home or public wifi name

#define WIFI_PASSWORD "08642731"

#define DHT11_PIN 2

FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;
File dataFile;

DHT DHT(DHT11_PIN, DHT11);
// Data wire is connected to GPIO pin D1 (NodeMCU pin mapping)
const int oneWireBus = D3;
const int sensorsQnt = 9;

RTC_DS3231 rtc;

const long gmtOffset_sec = 0 * 3600; // Adjust this according to your time zone offset
const int daylightOffset_sec = 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", gmtOffset_sec, daylightOffset_sec);

const int chipSelect = D8;

const String user = "QgWSvwQQnqcaeBGQbpyOHTilNcz1";

// Create a OneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass the OneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

String teste(int number)
{
    int res = round(ceil((number / 3) + 1));
    return "/colcho_" + String(res);
}
int count = 0;
String timeStamp = "";
const int sleepTimeSeconds = 300; // Sleep for 15 seconds

DeviceAddress sensorAddresses[] = {
    {0x28, 0x0A, 0x23, 0x57, 0x04, 0x2D, 0x3C, 0xF2},
    {0x28, 0x5E, 0x4C, 0x57, 0x04, 0xE1, 0x3C, 0x87},
    {0x28, 0x99, 0x6D, 0x57, 0x04, 0xD3, 0x3C, 0x14},
    {0x28, 0xAD, 0xA4, 0x57, 0x04, 0x19, 0x3C, 0xA7},
    {0x28, 0xAD, 0x2E, 0x57, 0x04, 0xDB, 0x3C, 0xF4},
    {0x28, 0xFF, 0x64, 0x1F, 0x6E, 0x5B, 0x5C, 0x99},
    {0x28, 0xFF, 0x64, 0x1F, 0x7E, 0x69, 0xC1, 0x83},
    {0x28, 0xFF, 0x64, 0x1F, 0x69, 0xF4, 0x53, 0x54},
    {0x28, 0xFF, 0x64, 0x1F, 0x7F, 0x8E, 0x2A, 0xFA}};
float tempC;
float humidity;

void setup()
{
    Serial.begin(115200);

    // Initialize RTC
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        while (1)
            ;
    }

    // Update the time from NTP server regardless of RTC power status
    // updateRTCFromNTP();

    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    if (WiFi.status() != WL_CONNECTED)
    {
        connectToWiFi();
    }

    // Check if RTC lost power
    if (rtc.lostPower())
    {
        if (Ping.ping("www.google.com"))
        {
            timeClient.begin();
            timeClient.update();
            time_t currentTime = timeClient.getEpochTime();
            rtc.adjust(DateTime(currentTime));
        }
    }

    pinMode(D1, INPUT_PULLUP);
    pinMode(D2, INPUT_PULLUP);

    // Start communication with the DS18B20 sensors
    sensors.begin();

    // initializeSDCard();

    initalizeFirebase(FIREBASE_HOST_1);

    firebaseData.setBSSLBufferSize(1024, 1024);
}

void loop()
{
    // // Request temperature data from all sensors
    sensors.requestTemperatures();

    // Loop through each sensor and print the temperature
    float list[sensorsQnt] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    for (int i = 0; i < sensorsQnt; i++)
    {
        uint8_t address[8];
        sensors.getAddress(address, i);
        printAddress(address);

        float temperature = sensors.getTempC(sensorAddresses[i]);
        list[i] = temperature;
        Serial.println("Temp" + String(i) + ": " + String(list[i]) + "°C\t");
    }
    count++;
    delay(1000); // Ensure Serial data is sent before sleeping

    DHT.begin();
    delay(100); // Ensure Serial data is sent before sleeping

    tempC = DHT.readTemperature();
    float tempF = DHT.convertCtoF(tempC);
    humidity = DHT.readHumidity();

    Serial.print("Temperature (C) = ");
    Serial.println(tempC);
    Serial.print("Temperature (F) = ");
    Serial.println(tempF);
    Serial.print("Humidity = ");
    Serial.println(humidity);

    DateTime now = rtc.now();
    timeStamp = printFormattedTime(now);
    Serial.println(timeStamp);

    if (WiFi.status() == WL_CONNECTED)
    {
        if (Ping.ping("www.google.com"))
        {
            Serial.println("pingou");
            if (initializeSDCard())
            {
                Serial.println("sd card initialized");
                if (SD.exists("temperatura.csv"))
                {
                    fromSdtoArd();
                }
                else
                {
                    uploadTemperature(list, timeStamp, tempC, humidity);
                }
                SD.end();
            }
            else
            {
                uploadTemperature(list, timeStamp, tempC, humidity);
            }

            // uploadTemperature(list, timeStamp, tempC, humidity);
            // uploadTemperature(list, timeStamp);
            // if (initializeSDCard())
            // {
            //     // if (SD.exists("temperatura.csv"))
            //     // {
            //     //     fromSdtoArd();
            //     // }
            //     uploadTemperature(list, timeStamp, tempC, humidity, true);
            //     SD.end();
            // }
            // else
            // {
            //     uploadTemperature(list, timeStamp, tempC, humidity, true);
            //     Serial.println("Error initializing SD card.");
            // }
        }
        else
        {
            Serial.println("nao pingou");
            if (initializeSDCard())
            {
                Serial.println("sd card initialized");
                uploadSD(list, timeStamp, tempC, humidity);
                SD.end();
            }
        }
    }
    else
    {
        Serial.println("Not connected to Wi-Fi.");
        if (initializeSDCard())
        {
            Serial.println("sd card initialized");
            uploadSD(list, timeStamp, tempC, humidity);
            SD.end();
        }
    }
    Serial.println("contagem: " + String(count));

    Serial.println("Going to sleep...");

    // Configure ESP8266 to sleep for a certain time
    ESP.deepSleep(sleepTimeSeconds * 1000000, WAKE_RF_DEFAULT);
    delay(100); // Ensure Serial data is sent before sleeping
}

void uploadTemperature(float *temperature, String timeStamp, float tempC, float humidity)
{
    FirebaseJson colcho;
    int count = 0;
    for (int i = 0; i < sensorsQnt; i += 3)
    {
        if (temperature[i] == DEVICE_DISCONNECTED_C && temperature[i + 1] == DEVICE_DISCONNECTED_C && temperature[i + 2] == DEVICE_DISCONNECTED_C)
        {
            continue;
        }
        else
        {
            String test = teste(i + 1);
            FirebaseJson json;
            json.add("sensor_de_baixo", temperature[i]);
            json.add("sensor_do_meio", temperature[i + 1]);
            json.add("sensor_de_cima", temperature[i + 2]);
            json.add("dht11_temperature", tempC);
            json.add("dht11_humidity", humidity);
            json.add("timeStamp", timeStamp);
            colcho.add(test.substring(1, test.length()), json);
            count++;

            // String low = user + teste(i + 1) + "/sensor_de_baixo";
            // Firebase.setFloat(firebaseData, low.c_str(), temperature[i]);
            // Serial.println(low);
            // Serial.println(temperature[i]);

            // String mid = user + teste(i + 1) + "/sensor_do_meio";
            // Firebase.setFloat(firebaseData, mid.c_str(), temperature[i + 1]);
            // Serial.println(mid);
            // Serial.println(temperature[i + 1]);

            // String high = user + teste(i + 1) + "/sensor_de_cima";
            // Firebase.setFloat(firebaseData, high.c_str(), temperature[i + 2]);
            // Serial.println(high);
            // Serial.println(temperature[i + 1]);

            // String time = user + teste(i + 1) + "/timeStamp";
            // Firebase.setString(firebaseData, time.c_str(), timeStamp);
        }
    }
    if (count > 0)
    {
        if (Firebase.setJSON(firebaseData, user.c_str(), colcho))
        {
            Serial.println("PASSED");
        }
        else
        {
            Serial.println("FAILED");
            Serial.println(firebaseData.errorReason());
        }
    }
    // colcho.
}
// upload ds18b20 values to sdcard
void uploadSD(float *temperature, String timeStamp, float tempC, float humidity)
{

    // ckeck if file exists
    if (!SD.exists("temperatura.csv"))
    {
        // Open the file. Change "data.csv" to your file name.
        dataFile = SD.open("temperatura.csv", FILE_WRITE);

        if (dataFile)
        {
            Serial.println("File opened:");

            dataFile.println(",,,,colcho_1,,,colcho_2,,,colcho_3,");
            dataFile.println("timeStamp,dht11_temperature,dht11_humidity,sensor_de_baixo,sensor_do_meio,sensor_de_cima,sensor_de_baixo,sensor_do_meio,sensor_de_cima,sensor_de_baixo,sensor_do_meio,sensor_de_cima,");

            // Close the file
            dataFile.close();
        }
        else
        {
            Serial.println("1Error opening file.");
        }
    }

    // Open the file. Change "data.csv" to your file name.
    dataFile = SD.open("temperatura.csv", FILE_WRITE);

    if (dataFile)
    {
        Serial.println("File opened successfully:");

        dataFile.print(timeStamp);
        dataFile.print(",");
        dataFile.print(tempC);
        dataFile.print(",");
        dataFile.print(humidity);
        dataFile.print(",");
        for (int i = 0; i < sensorsQnt; i++)
        {
            // jump if sensor is disconnected
            if (temperature[i] == DEVICE_DISCONNECTED_C)
            {
                dataFile.print(",,");
                continue;
            }
            else
            {
                dataFile.print(temperature[i]);
                dataFile.print(",");
            }
        }
        dataFile.println("");

        // Close the file
        dataFile.close();
    }
    else
    {
        Serial.println("2Error opening file.");
    }

    // off sd
    SD.end();
}
void connectToWiFi()
{
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int attempts = 0;

    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(1000);
        Serial.println("Connecting...");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Connected to Wi-Fi. IP address: " + WiFi.localIP().toString());
    }
    else
    {
        Serial.println("Failed to connect to Wi-Fi within 20 seconds.");
    }
}

void fromSdtoArd()
{

    // Open the file. Change "data.csv" to your file name.
    dataFile = SD.open("temperatura.csv");

    if (dataFile)
    {
        initalizeFirebase(FIREBASE_HOST_2);
        Serial.println("File opened successfully:");

        while (dataFile.available())
        { // Read data from file and send it to Firebase
            if (Firebase.setFile(firebaseData, StorageType::SD, user, "temperatura.csv", rtdbUploadCallback /* callback function*/))
            {
                Serial.println("PASSED");
                // // Delete the file after sending data to Firestore
                if (SD.remove("temperatura.csv"))
                {
                    Serial.println("File deleted successfully!");
                }
                else
                {
                    Serial.println("Error deleting file!");
                }
            }
            else
            {
                Serial.println("FAILED");
                Serial.println(firebaseData.errorReason());
            }
            // Serial.println("Data sent to Firestore!");

            // // Close the file
            dataFile.close();
        }
        initalizeFirebase(FIREBASE_HOST_1);
    }
    else
    {
        Serial.println("Error opening file.");
    }
}

void rtdbUploadCallback(RTDB_UploadStatusInfo info)
{
    if (info.status == fb_esp_rtdb_upload_status_init)
    {
        Serial.printf("Uploading file %s (%d) to %s\n", info.localFileName.c_str(), info.size, info.remotePath.c_str());
    }
    else if (info.status == fb_esp_rtdb_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
    }
    else if (info.status == fb_esp_rtdb_upload_status_complete)
    {
        Serial.println("Upload completed\n");
    }
    else if (info.status == fb_esp_rtdb_upload_status_error)
    {
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}

String printFormattedTime(DateTime now)
{
    String formattedTime = "";

    formattedTime += String(now.month()) + "/";
    formattedTime += String(now.day()) + "/";
    formattedTime += String(now.year()) + "; ";
    formattedTime += String((now.hour() % 12 == 0 ? 12 : now.hour() % 12)) + ":";
    formattedTime += printTwoDigits(now.minute()) + ":";
    formattedTime += printTwoDigits(now.second()) + " ";
    formattedTime += (now.hour()) >= 12 ? "PM" : "AM";

    return formattedTime;
}

String printTwoDigits(int number)
{
    if (number < 10)
    {
        return "0" + String(number);
    }
    else
    {
        return String(number);
    }
}

void initalizeFirebase(const char *host)
{
    config.host = host;
    config.api_key = API_KEY;

    auth.user.email = EMAIL;
    auth.user.password = PASSWORD;

    Firebase.begin(&config, &auth);
}

bool initializeSDCard()
{
    if (!SD.begin(chipSelect))
    {
        Serial.println("Card failed, or not present");
        return false;
    }
    Serial.println("card initialized.");
    return true;
}
void printAddress(uint8_t address[8])
{
    for (int i = 0; i < 8; i++)
    {
        if (address[i] < 16)
        {
            Serial.print("0");
        }
        Serial.print(address[i], HEX);
        Serial.print(" ");
    }
}

// void updateRTCFromNTP()
// {
//     if (WiFi.status() != WL_CONNECTED)
//     {
//         connectToWiFi();
//     }

//     // Connect to NTP server
//     timeClient.begin();
//     timeClient.update();
//     time_t currentTime = timeClient.getEpochTime();
//     rtc.adjust(DateTime(currentTime));
// }

// String printDateTime()
// {
//     // Read the time and date from the RTC
//     DateTime now = rtc.now();

//     // Format the date and time as a string
//     String dateTimeString = String(now.month()) + "/" + String(now.day()) + "/" + String(now.year()) + "; " + formatTime(now.hour(), now.minute(), now.second());

//     // Print the time and date to the Serial Monitor
//     return dateTimeString;
// }

// String formatTime(int hour, int minute, int second)
// {
//     String formattedTime = "";
//     // Convert hour to 12-hour format and add AM/PM
//     if (hour == 0)
//     {
//         formattedTime += "12";
//     }
//     else if (hour > 12)
//     {
//         formattedTime += String(hour - 12);
//     }
//     else
//     {
//         formattedTime += String(hour);
//     }

//     // Add minutes and seconds
//     formattedTime += ":" + String(minute) + ":" + String(second);

//     // Add AM or PM
//     formattedTime += (hour < 12) ? " AM" : " PM";

//     return formattedTime;
// }

// DateTime parseUTCTime(String utcTimeString)
// {
//     // Parse the UTC time string in the format "YYYY-MM-DD HH:MM:SS"
//     int year = utcTimeString.substring(0, 4).toInt();
//     int month = utcTimeString.substring(5, 7).toInt();
//     int day = utcTimeString.substring(8, 10).toInt();
//     int hour = utcTimeString.substring(11, 13).toInt();
//     int minute = utcTimeString.substring(14, 16).toInt();
//     int second = utcTimeString.substring(17, 19).toInt();

//     return DateTime(year, month, day, hour, minute, second);
// }