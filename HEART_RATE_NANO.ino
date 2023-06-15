#define USE_ARDUINO_INTERRUPTS true    // Set-up low-level interrupts for most acurate BPM math.
#include <Wire.h>                 // Library for I2C communication
#include <LiquidCrystal_I2C.h>    // Library for LCD display
#include <PulseSensorPlayground.h> // Library for pulse sensor
#include <SoftwareSerial.h>       // Library for ESP-01 module
#include "ThingSpeak.h"

// Define the I2C address for the LCD display
const int LCD_ADDRESS = 0x27;

// Initialize the LCD display
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);

const int PulseWire = A0;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
int Threshold = 550;

int MyBPM;
int Temp;
int error;

// Define the pulse sensor object
PulseSensorPlayground pulseSensor;

// Define the pins for the temperature sensor
const int tempSensorPin = 1;

// Define the pins for ESP-01 module
SoftwareSerial espSerial(9, 10);  // RX, TX

// Set your network credentials for Wi-Fi
/* const char* ssid = "Infinix S5";
const char* password = "11111111";*/

#define SSID "Infinix S5" // "SSID-WiFiname"
#define PASS "11111111" // "password"
#define IP "184.106.153.149"// thingspeak.com ip
String msg = "GET /update?key=VC5BPTB5SWX7BMBV"; //change it with your api key




// Set your Thingspeak API key
const char* apiKey = "VC5BPTB5SWX7BMBV";
const char* server = "api.thingspeak.com";
const long CHANNEL = 2149244; //In this field, enter the Channel ID
const char *WRITE_API = "VC5BPTB5SWX7BMBV";

void setup() 
{
  lcd.begin();     // initialize the lcd 
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Final Year");
  lcd.setCursor(5,1);
  lcd.print("Project");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" IoT Heart Rate");
  lcd.setCursor(5,1);
  lcd.print("Monitor");
  delay(2000);
 
  Serial.begin(115200);
  espSerial.begin(38400);
  Serial.println("AT");
  espSerial.println("AT");
  delay(5000);
  if(espSerial.find("OK")){
   // Connect to Wi-Fi
  connectToWiFi();
  }
 


  // Configure the PulseSensor object, by assigning our variables to it. 
  pulseSensor.analogInput(PulseWire);   

  pulseSensor.setThreshold(Threshold);   

  // Double-check the "pulseSensor" object was created and "began" seeing a signal. 
   if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset.  
  }

  // Clear Thingspeak channel
  clearThingspeakChannel();
}

void loop() {
  
  // Update pulse sensor
 // pulseSensor.update();

  // Read heart rate from pulse sensor
  int heartRate = pulseSensor.getBeatsPerMinute();
  
  if (pulseSensor.sawStartOfBeat()) {            // Constantly test to see if "a beat happened". 

    if ( heartRate >= 60 && heartRate <= 100){
      Serial.println("♥  A HeartBeat Happened ! "); // If test is "true", print a message "a heartbeat happened".
      Serial.print("BPM: ");                        // Print phrase "BPM: " 
      Serial.println(heartRate);                        // Print the value inside of myBPM. 
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("BPM: ");
      lcd.print(heartRate);

      int MyBPM = heartRate;
    }
  
  }else{
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("No Heartbeat");
  }

  // Read temperature from LM35 sensor
  int sensorValue = analogRead(tempSensorPin);
  float temperature = (sensorValue * 3.3/ 1024.0) * 100.0; //Esp32 temp to Celsius -- (Temperature * 3.3 / 4095) * 100;
  Serial.print(" | Temp: ");
  Serial.print(temperature);
  Serial.println(" C");
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");

     // Send data to Thingspeak
  sendToThingspeak(MyBPM, temperature);
  // Display heart rate and temperature on LCD
  /*lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BPM: ");
  lcd.print(heartRate);
  
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");*/

   // Print values to Serial Monitor
 /* Serial.print("BPM: ");
  Serial.print(heartRate);
  Serial.print(" | Temp: ");
  Serial.print(temperature);
  Serial.println(" C"); */

 
  delay(500); // Wait for 1 second before taking the next reading
}



boolean connectToWiFi() {
  Serial.println("AT+CWMODE=1");
  espSerial.println("AT+CWMODE=1"); 
  delay(2000);
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  Serial.println(cmd);
  espSerial.println(cmd);
  delay(5000);
  if(espSerial.find("OK")){
  Serial.println("OK");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Wifi Connected");
  return true;
  }else{
  return false;
  }
}

void clearThingspeakChannel() {
  String cmd = "AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80";
  espSerial.println(cmd);
  delay(2000);
  cmd = "GET /update?api_key=";
  cmd += apiKey;
  cmd += "&field1=0&field2=0";
  espSerial.print("AT+CIPSEND=");
  espSerial.println(cmd.length());
  delay(1000);
  espSerial.println(cmd);
  delay(2000);
  espSerial.println("AT+CIPCLOSE");
}

  void sendToThingspeak(int bpm, float temp) {
  String cmd = "AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80";
  Serial.println(cmd);
  espSerial.println(cmd);
  delay(2000);
  if(espSerial.find("Error")){
  return;
  }
  cmd = "GET /update?api_key=";
  cmd += apiKey;
  cmd += "&field1=";
  cmd += bpm;
  cmd += "&field2=";
  cmd += temp;
  cmd += "\r\n";
  Serial.print("AT+CIPSEND=");
  espSerial.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  espSerial.println(cmd.length());
  if(espSerial.find(">")){
  Serial.print(cmd);
  espSerial.print(cmd);
  }
  else{
  Serial.println("AT+CIPCLOSE");
  espSerial.println("AT+CIPCLOSE");
  error=1;
  }
}
 
