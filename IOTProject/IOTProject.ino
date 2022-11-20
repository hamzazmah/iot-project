// Libraries Required
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <MQ135.h>

// OLED Dispay Intialisation
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C // OLED display 
#define OLED_RESET -1 // Reset Pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// DHT Initialisation
#define DATAPIN 50
#define DHTTYPE DHT22
DHT dht(DATAPIN, DHTTYPE);

// MQ135 Initialization
#define MAXCO2 4000
#define RZERO 212.5
MQ135 gasSensor = MQ135(A0);

// Variables
int co2Value = 0;
int temp = 0;
int humidity = 0;
int sensorValRaw = 0;

int graphVal = 0;

// Setup
void setup()
{
  Serial.begin(9600); // sets the serial port to 9600
  dht.begin();  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.setTextColor(WHITE);
  display.display();
  delay(3000);
  display.clearDisplay();
}

// Loop
void loop()
{
  display.clearDisplay();

  getAvgCo2AndDisplay();

  getReadings();

  sendData();

  displayVal("CO2: " + String(co2Value) + " PPM", 0);

  displayVal("Temperature: " + String(temp) + " C", 12);

  displayVal("Humidity: " + String(humidity) + " %", 24);

  delay(10000); // wait for 10 secs

  displayCo2AQI(co2Value); // display AQI info

  delay(5000);
}

// Method to read values from sensors
void getReadings()
{
  sensorValRaw = analogRead(0);
  Serial.println("Raw MQ-135 Val: " + String(sensorValRaw));

  temp = dht.readTemperature();
  humidity = dht.readHumidity();
}

void displayCo2AQI(int val)
{
	display.clearDisplay();

  int aqi = 1;
  String rating;
  String aqiMessage;

  if (val < 400)
  {
    aqi = 1;
    rating = "Excellent";
    aqiMessage = "The air inside is as fresh as the air outside.";     
  }
  else if (val < 1000)
  {
    aqi = 2;
    rating = "Fine";
    aqiMessage = "The air quality inside remains at harmless levels.";
  }
  else if (val < 1500)
  {
    aqi = 3;
    rating = "Moderate";    
    aqiMessage = "The air quality inside has reached conspicuous levels.";
  }
  else if (val < 2000)
  {
    aqi = 4;
    rating = "Poor";
    aqiMessage = "The air quality inside has reached precarious levels.";
  }
  else if (val < 5000)
  {
    aqi = 5;
    rating = "Very Poor";
    aqiMessage = "The air quality inside has reached unacceptable levels.";
  }
  else 
  {
    aqi = 6;
    rating = "Severe";
    aqiMessage = "The air quality inside has exceeded maximum workplace concentration values.";
  }

  displayVal("CO2 AQI: " + String(aqi), 0);
  displayVal("Rating: " + rating, 12);

  delay(5000);
  
  display.clearDisplay();
  displayVal(aqiMessage, 0);
}

// Method to Write data to Serial as a Json object
void sendData()
{
  StaticJsonDocument<200> doc;

  doc["temperature"] = String(temp);
  doc["humidity"] = String(humidity);
  doc["co2Value"] = String(co2Value);

  serializeJson(doc, Serial);
  Serial.println();
}

//Method to get average co2 value
void getAvgCo2AndDisplay()
{
  int co2AvgTotal = 0;
  int tempCo2Val = 0;

  for (int i = 0; i < 10; i++) 
  {
    tempCo2Val = gasSensor.getPPM();
    co2AvgTotal = co2AvgTotal + tempCo2Val;

    displayVal("CO2: " + String(tempCo2Val) + " PPM", 0);
    displayGraph(tempCo2Val, 12);

    delay(500);
    display.clearDisplay();
  }

  co2Value = co2AvgTotal / 10;
}

// Method to write val at given row
void displayVal(String data, int row)
{
  display.setCursor(0, row);
  display.println(data);
  display.display();
}

//Method to display a rectangle Gauge
void displayGraph(int val, int row)
{
  display.setCursor(0, row);
  graphVal = map(val, 0, MAXCO2, 1, SCREEN_WIDTH);
  display.fillRect(0, row, graphVal, 10, WHITE);
  display.display();  
}