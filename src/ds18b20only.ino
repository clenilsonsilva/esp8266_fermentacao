#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
//teste

#define FIREBASE_HOST "tcc-ifpa-default-rtdb.firebaseio.com"  // the project name address from firebase id
#define FIREBASE_AUTH "z6f4IHnNRvN48XS9hJ1Zd7T8MOjASumayA8g2zar"

#define WIFI_SSID "276533VCT1"  // input your home or public wifi name

#define WIFI_PASSWORD "03734109213"

FirebaseData firebaseData;

// Data wire is connected to GPIO pin D1 (NodeMCU pin mapping)
const int oneWireBus = D1;
const int sensorsQnt = 9;

const String user = "clenilson";

// Create a OneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass the OneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

String teste(int number) {
  int res = round(ceil((number / 3) + 1));
  return "/colcho_" + String(res);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  //try to connect with wifi

  Serial.print("Connecting to ");

  Serial.print(WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED) {

    Serial.print(".");

    delay(500);
  }

  Serial.println();

  Serial.print("Connected to ");

  Serial.println(WIFI_SSID);

  Serial.print("IP Address is : ");

  Serial.println(WiFi.localIP());

  // Start communication with the DS18B20 sensors
  sensors.begin();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  // Request temperature data from all sensors
  sensors.requestTemperatures();

  // Loop through each sensor and print the temperature
  float list[sensorsQnt] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  for (int i = 0; i < sensorsQnt; i++) {
    float temperature = sensors.getTempCByIndex(i);
    list[i] = temperature;
    // Serial.print("Sensor ");
    // Serial.print(i);
    // Serial.print(": ");

    // if (temperature != DEVICE_DISCONNECTED_C) {
    //   Serial.print(temperature);
    //   Serial.println("°C");
    // } else {
    //   Serial.println("Error reading temperature.");
    // }
    // if (i < 3) {
    //   Serial.println("colcho_1");

    //   Serial.print("sensor_de_baixo:");
    //   Serial.print(temperature);
    //   Serial.println("°C");

    //   Serial.print("sensor_do_meio:");
    //   Serial.print(temperature);
    //   Serial.println("°C");

    //   Serial.print("sensor_de_cima:");
    //   Serial.print(temperature);
    //   Serial.println("°C");

    //   Serial.println();


    // } else if (i < 6) {
    //   Serial.println("colcho_2");

    //   Serial.print("sensor_de_baixo:");
    //   Serial.print(temperature);
    //   Serial.println("°C");

    //   Serial.print("sensor_do_meio:");
    //   Serial.print(temperature);
    //   Serial.println("°C");

    //   Serial.print("sensor_de_cima:");
    //   Serial.print(temperature);
    //   Serial.println("°C");

    //   Serial.println();

    // } else {
    //   Serial.println("colcho_3");

    //   Serial.print("sensor_de_baixo:");
    //   Serial.print(temperature);
    //   Serial.println("°C");

    //   Serial.print("sensor_do_meio:");
    //   Serial.print(temperature);
    //   Serial.println("°C");

    //   Serial.print("sensor_de_cima:");
    //   Serial.print(temperature);
    //   Serial.println("°C");

    //   Serial.println();
    //}
  }
  uploadTemperature(list);
  // for (int i = 0; i < sensorsQnt; i++) {
  //   Serial.print(list[i]);
  //   Serial.print(", ");
  // }

  delay(10000);  // Delay for 5 seconds before reading temperatures again
}
void uploadTemperature(float* temperature) {

  for (int i = 0; i < sensorsQnt; i += 3) {
    // {
    //   Serial.println(teste(i + 1));

    //   Serial.print("sensor_de_baixo:");
    //   Serial.print(temperature[i]);
    //   Serial.println("°C");

    //   Serial.print("sensor_do_meio:");
    //   Serial.print(temperature[i + 1]);
    //   Serial.println("°C");

    //   Serial.print("sensor_de_cima:");
    //   Serial.print(temperature[i + 2]);
    //   Serial.println("°C");
    //   Serial.println("");
    // }

    String low = user + teste(i + 1) + "/sensor_de_baixo";
    Firebase.setFloat(firebaseData, low.c_str(), temperature[i]);
    Serial.println(low);

    String mid = user + teste(i + 1) + "/sensor_do_meio";
    Firebase.setFloat(firebaseData, mid.c_str(), temperature[i + 1]);
    Serial.println(mid);

    String high = user + teste(i + 1) + "/sensor_de_cima";
    Firebase.setFloat(firebaseData, high.c_str(), temperature[i + 2]);
    Serial.println(high);

    String usuario = user + teste(i + 1) + "/user";
    Firebase.setString(firebaseData, usuario.c_str(), user);
  }
  Serial.println("");
  Serial.println("");
}
