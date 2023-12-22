#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <SD.h>
#include <SPI.h>
#include <ESP8266Ping.h>

#include <Wire.h>
#include <RtcDS3231.h>
// teste

#define FIREBASE_HOST_1 "tcc-ifpa-default-rtdb.firebaseio.com" // the project name address from firebase id
#define FIREBASE_HOST_2 "backup-data.firebaseio.com"           // the project name address from firebase id
#define FIREBASE_AUTH "z6f4IHnNRvN48XS9hJ1Zd7T8MOjASumayA8g2zar"
#define API_KEY "AIzaSyDfAljJmaaMx79MySzUNE3gYFoeDAYLkRs"
#define EMAIL "clenilson@gmail.com"
#define PASSWORD "123456"

#define WIFI_SSID "276533VCT1" // input your home or public wifi name

#define WIFI_PASSWORD "03734109213"

FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;
File dataFile;

// Data wire is connected to GPIO pin D1 (NodeMCU pin mapping)
const int oneWireBus = D3;
const int sensorsQnt = 9;

RtcDS3231<TwoWire> Rtc(Wire);

const int chipSelect = D8;

const String user = "clenilson";

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

void setup()
{
    Serial.begin(460800);

    Rtc.Begin();

    connectToWiFi();

    if (!SD.begin(chipSelect))
    {
        Serial.println("SD card initialization failed!");
    }
    else
    {
        Serial.println("SD card initialized.");
    }

    // Start communication with the DS18B20 sensors
    sensors.begin();

    initalizeFirebase(FIREBASE_HOST_1);

    firebaseData.setBSSLBufferSize(1024, 1024);

    RtcDateTime tempoatual = RtcDateTime(__DATE__, __TIME__);
    Rtc.SetDateTime(tempoatual);
}

void loop()
{
    // Request temperature data from all sensors
    sensors.requestTemperatures();

    // Loop through each sensor and print the temperature
    float list[sensorsQnt] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    for (int i = 0; i < sensorsQnt; i++)
    {
        float temperature = sensors.getTempCByIndex(i);
        list[i] = temperature;
        Serial.print(list[i]);
    }
    count++;

    RtcDateTime instante = Rtc.GetDateTime();
    timeStamp = printFormattedTime(instante);

    if (WiFi.status() == WL_CONNECTED)
    {
        if (Ping.ping("www.google.com"))
        {
            Serial.println("pingou");
            // if (SD.exists("temperatura.csv"))
            // {
            //     fromSdtoArd();
            // }
            uploadTemperature(list, timeStamp);
        }
        else
        {
            Serial.println("nao pingou");
            uploadSD(list, timeStamp);
            // if (SD.exists("temperatura.csv"))
            // {
            //     fromSdtoArd();
            // }
            // uploadTemperature(list);
        }
    }
    else
    {
        Serial.println("Not connected to Wi-Fi.");
        uploadSD(list, timeStamp);
        // Reconnect to Wi-Fi
        connectToWiFi();
    }
    Serial.println("contagem: " + String(count));

    delay(60000); // Delay for 5 min before reading temperatures again
}

void uploadTemperature(float *temperature, String timeStamp)
{
    FirebaseJson colcho;

    // TODO: upload only available sensors
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
            json.add("timeStamp", timeStamp);
            colcho.add(test.substring(1, test.length()), json);

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
// upload ds18b20 values to sdcard
void uploadSD(float *temperature, String timeStamp)
{

    // ckeck if file exists
    if (!SD.exists("temperatura.csv"))
    {
        // Open the file. Change "data.csv" to your file name.
        dataFile = SD.open("temperatura.csv", FILE_WRITE);

        if (dataFile)
        {
            Serial.println("File opened:");

            dataFile.println(",,colcho_1,,,colcho_2,,,colcho_3,");
            dataFile.println("timeStamp,sensor_de_baixo,sensor_do_meio,sensor_de_cima,sensor_de_baixo,sensor_do_meio,sensor_de_cima,sensor_de_baixo,sensor_do_meio,sensor_de_cima,");

            // Close the file
            dataFile.close();
        }
        else
        {
            Serial.println("Error opening file.");
        }
    }

    // Open the file. Change "data.csv" to your file name.
    dataFile = SD.open("temperatura.csv", FILE_WRITE);

    if (dataFile)
    {
        Serial.println("File opened successfully:");

        dataFile.print(timeStamp);
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
    dataFile = SD.open("temperatura.csv");

    if (dataFile)
    {
        initalizeFirebase(FIREBASE_HOST_2);
        Serial.println("File opened successfully:");

        while (dataFile.available())
        { // Read data from file and send it to Firebase
            if (Firebase.setFile(firebaseData, StorageType::SD, "clenilson/backup", "temperatura.csv", rtdbUploadCallback /* callback function*/))
            {
                Serial.println("PASSED");
                // // Delete the file after sending data to Firestore
                // if (SD.remove("temperatura.csv"))
                // {
                //     Serial.println("File deleted successfully!");
                // }
                // else
                // {
                //     Serial.println("Error deleting file!");
                // }
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

String printFormattedTime(RtcDateTime instante)
{
    String formattedTime = "";

    formattedTime += String(instante.Month()) + "/";
    formattedTime += String(instante.Day()) + "/";
    formattedTime += String(instante.Year()) + ", ";
    formattedTime += String(((instante.Hour() + 3) > 12) ? (instante.Hour() + 3) - 12 : (instante.Hour() + 3)) + ":";
    formattedTime += printTwoDigits(instante.Minute()) + ":";
    formattedTime += printTwoDigits(instante.Second()) + " ";
    formattedTime += instante.Hour() >= 12 ? "PM" : "AM";

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