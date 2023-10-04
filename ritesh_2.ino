#include <SoftwareSerial.h>
#include <Adafruit_SSD1306.h>     // Include the Adafruit SSD1306 library to interact with the display
 
#define SCREEN_WIDTH 128          // Set the width and height of the display in pixels
#define SCREEN_HEIGHT 64
#define OLED_RESET -1             // Set the OLED reset pin to -1 if it shares the Arduino reset pin
#define SCREEN_ADDRESS 0x3C       // Set the address of the screen as specified in its datasheet
 
// Create an Adafruit_SSD1306 object called display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
// Define the pins for RS485 communication
#define RE 8
#define DE 7
 
// Request frame for the soil sensor
const byte soilSensorRequest[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B};
byte soilSensorResponse[9];
SoftwareSerial mod(2, 3); // Software serial for RS485 communication
 
 
void setup()
{
  Serial.begin(9600); // Initialize serial communication for debugging
  mod.begin(4800);    // Initialize software serial communication at 4800 baud rate
  pinMode(RE, OUTPUT); // Set RE pin as output
  pinMode(DE, OUTPUT); // Set DE pin as output
  // Start the display, looping forever if it fails
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
 
  display.clearDisplay();     // Clear the display and display the cleared screen
  display.display();
  delay(500);
}
 
 
void loop()
{
  // Start the transmission mode for RS485
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
 
  // Send the request frame to the soil sensor
  mod.write(soilSensorRequest, sizeof(soilSensorRequest));
 
  // End the transmission mode and set to receive mode for RS485
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  delay(10);
 
  // Wait for the response from the sensor or timeout after 1 second
  unsigned long startTime = millis();
  while (mod.available() < 9 && millis() - startTime < 1000)
  {
    delay(1);
  }
 
  if (mod.available() >= 9) // If valid response received
  {
    // Read the response from the sensor
    byte index = 0;
    while (mod.available() && index < 9)
    {
      soilSensorResponse[index] = mod.read();
      Serial.print(soilSensorResponse[index], HEX); // Print the received byte in HEX format
      Serial.print(" ");
      index++;
    }
    Serial.println();
 
    // Parse and calculate the Moisture value
    int Moisture_Int = int(soilSensorResponse[3] << 8 | soilSensorResponse[4]);
    float Moisture_Percent = Moisture_Int / 10.0;
 
    Serial.print("Moisture: ");
    Serial.print(Moisture_Percent);
    Serial.println(" %RH");
 
 
 
    // Parse and calculate the Temperature value
    int Temperature_Int = int(soilSensorResponse[5] << 8 | soilSensorResponse[6]);
    float Temperature_Celsius = Temperature_Int / 10.0;
 
    // Check if temperature is negative and convert accordingly
    if (Temperature_Int > 0x7FFF)
    {
      Temperature_Celsius = 0x10000 - Temperature_Int;
      Temperature_Celsius = -Temperature_Celsius / 10.0;
    }
 
    Serial.print("Temperature: ");
    Serial.print(Temperature_Celsius);
    Serial.println(" °C");
 
    // Clear the display and set the text color
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    display.setTextSize(1);
    display.setCursor(25, 0);
    display.print("Soil Moisture");
    
    display.setTextSize(2);
    display.setCursor(30, 12);
    display.print(Moisture_Percent);
    display.setTextSize(1);
    display.print("% ");
 
    display.setTextSize(1);
    display.setCursor(30, 35);
    display.print("Temperature");
 
    display.setTextSize(2);
    display.setCursor(30, 47);
    display.print(Temperature_Celsius);
    display.setTextSize(1);
    display.print("*C");
    display.display();
 
 
  }
  else
  {
    // Print error message if no valid response received
    Serial.println("Sensor timeout or incomplete frame");
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
 
    display.setCursor(0, 0);
    display.print("Sensor timeout");
    display.display();
  }
 
  delay(1000); // Wait for a second before the next loop iteration
