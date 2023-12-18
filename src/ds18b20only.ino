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
#define FIREBASE_HOST_2 "backup-data.firebaseio.com"            // the project name address from firebase id
#define FIREBASE_AUTH "z6f4IHnNRvN48XS9hJ1Zd7T8MOjASumayA8g2zar"

#define WIFI_SSID "276533VCT1" // input your home or public wifi name

#define WIFI_PASSWORD "03734109213"

FirebaseData firebaseData1;
FirebaseData firebaseData2;
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

    Firebase.begin(FIREBASE_HOST_1, FIREBASE_AUTH);
    Firebase.begin(FIREBASE_HOST_2, FIREBASE_AUTH);

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
    String valores = String(instante.Day()) + "/" + String(instante.Month()) + "/" + String(instante.Year()) + " " + String(instante.Hour()) + ":" + String(instante.Minute()) + ":" + String(instante.Second());
    Serial.println(valores);

    if (WiFi.status() == WL_CONNECTED)
    {
        if (Ping.ping("www.google.com"))
        {
            Serial.println("pingou");
            // if (SD.exists("temperatura.csv"))
            // {
            //     fromSdtoArd();
            // }
            // uploadTemperature(list);
            uploadSD(list, valores);
        }
        else
        {
            Serial.println("nao pingou");
            uploadSD(list, String(valores));
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
        // Reconnect to Wi-Fi
        connectToWiFi();
    }
    Serial.println("contagem: " + String(count));

    delay(2000); // Delay for 5 seconds before reading temperatures again
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
        Firebase.setFloat(firebaseData1, low.c_str(), temperature[i]);
        Serial.println(low);
        Serial.println(temperature[i]);

        String mid = user + teste(i + 1) + "/sensor_do_meio";
        Firebase.setFloat(firebaseData1, mid.c_str(), temperature[i + 1]);
        Serial.println(mid);
        Serial.println(temperature[i + 1]);

        String high = user + teste(i + 1) + "/sensor_de_cima";
        Firebase.setFloat(firebaseData1, high.c_str(), temperature[i + 2]);
        Serial.println(high);
        Serial.println(temperature[i + 2]);

        // String usuario = user + teste(i + 1) + "/user";
        // Firebase.setString(firebaseData, usuario.c_str(), user);
    }
    Serial.println("");
    Serial.println("");
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
        Serial.println("File opened successfully:");

        while (dataFile.available())
        { // Read and print the file contents
            // String stringone = dataFile.readString();
            // String stringtwo = stringone.replace("", "/n");
            // Serial.println("string:" + stringone);
            // String data = "clenilson/backUp/data";
            // unsigned long timeout = millis();
            // stringone = replaceBl(stringone);
            // unsigned long endtime = millis() - timeout;
            // sent file to firebase
            if (!Firebase.setFile(firebaseData2, StorageType::SD, "clenilson/backup", "temperatura.csv", rtdbUploadCallback /* callback function*/))
                Serial.println(firebaseData2.errorReason());
            // Serial.println("Data sent to Firestore!");

            // // Close the file
            dataFile.close();

            // // Delete the file after sending data to Firestore
            // if (SD.remove("temperatura.csv"))
            // {
            //     Serial.println("File deleted successfully!");
            // }
            // else
            // {
            //     Serial.println("Error deleting file!");
            // }

            // Serial.println(stringone);

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
