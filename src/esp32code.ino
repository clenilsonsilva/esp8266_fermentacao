#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>
#include <WiFi.h>          // Change library for ESP32
#include <FirebaseESP32.h> // Change library for ESP32
#include <SD.h>
#include <SPI.h>
#include <ESP32Ping.h> // Change library for ESP32

#include <Wire.h>
#include <RtcDS3231.h>

#define API_KEY "AIzaSyDfAljJmaaMx79MySzUNE3gYFoeDAYLkRs"
#define DATABASE_URL "tcc-ifpa-default-rtdb.firebaseio.com" // the project name address from firebase id
// #define FIREBASE_AUTH "z6f4IHnNRvN48XS9hJ1Zd7T8MOjASumayA8g2zar"

#define WIFI_SSID "276533VCT1" // input your home or public wifi name

#define WIFI_PASSWORD "03734109213"

#define USER_EMAIL "clenilson@gmail.com"
#define USER_PASSWORD "123456"

File dataFile;
String stringone;

/* 6. Define the Firebase Data object */
FirebaseData firebaseData;

/* 7. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;

/* 8. Define the FirebaseConfig data for config data */
FirebaseConfig config;

const int oneWireBus = 4; // Change to the appropriate GPIO for ESP32 (D3 maps to GPIO4 on ESP32)
const int sensorsQnt = 9;

RtcDS3231<TwoWire> Rtc(Wire);

const int chipSelect = 5; // Change to the appropriate GPIO for ESP32 (D8 maps to GPIO5 on ESP32)

const String user = "clenilson";

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

String teste(int number)
{
    int res = round(ceil((number / 3) + 1));
    return "/colcho_" + String(res);
}
int count = 0;

void setup()
{
    Serial.begin(115200); // Change baud rate for ESP32

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

    sensors.begin();

    //
    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the RTDB URL */
    config.database_url = DATABASE_URL;

    // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
    // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
    firebaseData.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

    firebaseData.setResponseSize(4096);

    /* Initialize the library with the Firebase authen and config */
    Firebase.begin(&config, &auth);

    RtcDateTime tempoatual = RtcDateTime(__DATE__, __TIME__);
    Rtc.SetDateTime(tempoatual);
}

void loop()
{
    sensors.requestTemperatures();

    float list[sensorsQnt] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    for (int i = 0; i < sensorsQnt; i++)
    {
        float temperature = sensors.getTempCByIndex(i);
        list[i] = temperature;
        Serial.print(list[i]);
    }
    count++;

    RtcDateTime instante = Rtc.GetDateTime();
    char valores[20];

    sprintf(valores, "%d/%d/%d %d:%d:%d",
            instante.Year(),
            instante.Month(),
            instante.Day(),
            instante.Hour(),
            instante.Minute(),
            instante.Second());

    Serial.println(valores);

    if (WiFi.status() == WL_CONNECTED)
    {
        if (Ping.ping("www.google.com"))
        {
            Serial.println("pingou");
            Serial.println("sim");
            fromSdtoArd();
        }
        else
        {
            Serial.println("nao pingou");
            uploadSD(list, String(valores));
        }
    }
    else
    {
        Serial.println("Not connected to Wi-Fi.");
        connectToWiFi();
    }
    Serial.println("contagem: " + String(count));

    delay(10000);
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
            Serial.println("File opened successfully:");

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
    dataFile = SD.open("/temperatura.csv", "r");
    Serial.println("entrou");

    if (dataFile)
    {
        Serial.println("File opened successfully:");

        while (dataFile.available())
        { // Read and print the file contents
            // String stringone = dataFile.readString().substring(0, 10000);
            // // String stringtwo = stringone.replace("", "/n");
            // // Serial.println("string:" + stringone);
            // String data = "clenilson/backUp/data";
            // // unsigned long timeout = millis();
            // stringone = replaceBl(stringone);
            // Serial.println("string:" + stringone);
            // unsigned long endtime = millis() - timeout;
            String firebasePath = "clenilson/backUp/data";
            // sent file to firebase
            if (!Firebase.setFile(firebaseData, StorageType::SD, "clenilson/backUp/data", "/temperatura.csv", rtdbUploadCallback /* callback function*/))
             Serial.println(firebaseData.errorReason());
            // Firebase.setString(firebaseData, F("clenilson/backUp/data"), "Hello World!") ? "ok" : firebaseData.errorReason().c_str();
            // Firebase.setString(firebaseData, firebasePath.c_str(), stringone.c_str());
            // Firebase.setString(firebaseData, firebasePath.c_str(), stringone.c_str());
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

            Serial.println(stringone);

            // Close the file
            // dataFile.close();
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
  if (info.status == firebase_rtdb_upload_status_init)
  {
    Serial.printf("Uploading file %s (%d) to %s\n", info.localFileName.c_str(), info.size, info.remotePath.c_str());
  }
  else if (info.status == firebase_rtdb_upload_status_upload)
  {
    Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
  }
  else if (info.status == firebase_rtdb_upload_status_complete)
  {
    Serial.println("Upload completed\n");
  }
  else if (info.status == firebase_rtdb_upload_status_error)
  {
    Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
  }
}