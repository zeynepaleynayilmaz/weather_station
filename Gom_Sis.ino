#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <DHTesp.h>

/* WiFi Erişim Noktası */
#define WLAN_SSID "Zeynepyilmaz"
#define WLAN_PASS "aleynayilmaz"

/* Adafruit.io Setup */

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  8883
#define AIO_USERNAME    "aleynayilmaz37"
#define AIO_KEY         "aio_GOPh79CrBfiJUU8oWb47RmOjG9YA"

/* SSL Fingerprint */
// Bu fingerprint değeri Adafruit IO'nun SSL/TLS sertifikasına karşılık gelir.

static const char fingerprint[] PROGMEM = "4E C1 52 73 24 A8 36 D6 7A 4C 67 C7 91 0C 0A 22 B9 2D 5B CA";

/* Global State */
WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish pressure = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/pressure");
Adafruit_MQTT_Publish air_quality = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/air_quality");
Adafruit_MQTT_Publish rain_status = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/rain_status");

/* Sensor Setup */
#define DHTPIN D5
#define DHTTYPE DHTesp::DHT11
DHTesp dht;

// BMP180 sensor
Adafruit_BMP085 bmp;

const int MQ135_PIN = A0;
const int RAIN_SENSOR_PIN = D7;

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit IO MQTT Weather Station Example"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Initialize sensors
  dht.setup(DHTPIN, DHTTYPE);
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    while (1);
  }

  pinMode(MQ135_PIN, INPUT);
  pinMode(RAIN_SENSOR_PIN, INPUT);

  // Set SSL Fingerprint
  client.setFingerprint(fingerprint);

  delay(2000);
}

unsigned long previousMillis = 0;
const long interval = 30000; // Send data every 30 seconds

void loop() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.begin(WLAN_SSID, WLAN_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("WiFi reconnected");
  }

  // Check MQTT connection
  MQTT_connect();

  // Read sensor data
  float temperatureValue = dht.getTemperature();
  float humidityValue = dht.getHumidity();
  float pressureValue = bmp.readPressure();
  int airQualityValue = analogRead(MQ135_PIN);
  int rainStatusValue = digitalRead(RAIN_SENSOR_PIN);

  // Publish sensor data to Adafruit IO
  if (!temperature.publish(temperatureValue)) {
    Serial.println("Failed to publish temperature");
  }

  if (!humidity.publish(humidityValue)) {
    Serial.println("Failed to publish humidity");
  }

  if (!pressure.publish(pressureValue)) {
    Serial.println("Failed to publish pressure");
  }

  if (!air_quality.publish(airQualityValue)) {
    Serial.println("Failed to publish air quality");
  }

  if (!rain_status.publish(rainStatusValue)) {
    Serial.println("Failed to publish rain status");
  }

  // Delay for the specified interval
  delay(interval);
}

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;

    if (retries == 0) {
      while (1);
    }
  }

  Serial.println("MQTT Connected!");
}
