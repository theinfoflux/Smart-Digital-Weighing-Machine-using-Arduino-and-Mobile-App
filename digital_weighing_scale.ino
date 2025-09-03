#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include "HX711.h"

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Bluetooth RX/TX
SoftwareSerial BT(10, 11); // TX pin to D10 amd RX pin D11

// HX711 pins
#define DT  3
#define SCK 2
HX711 scale(DT, SCK);

// Bluetooth STATE pin
int btStatePin = 12;
bool bluetoothConnected = false;

// Variables
char receivedChar;
String inputPrice = "";
float pricePerKg = 0;
float weight = 0;
float totalCost = 0;
float calibration_factor = 104300; // adjust as needed

void setup() {
  Serial.begin(9600);
  BT.begin(9600);   // HC-05 Bluetooth baud rate
  lcd.init();
  lcd.backlight();
 Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");
  // HX711 setup
  scale.set_scale();
  scale.tare(); // Reset the scale to 0
  long zero_factor = scale.read_average();

  pinMode(btStatePin, INPUT);

  lcd.setCursor(0,0);
  lcd.print("Connect");
   lcd.setCursor(0,1);
     lcd.print("Bluetooth");
}

void loop() {
  // --- Check Bluetooth connection state ---
  if (digitalRead(btStatePin) == HIGH && !bluetoothConnected) {
    bluetoothConnected = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Bluetooth");
    lcd.setCursor(0, 1);
    lcd.print("Connected!");
     delay(1000);
    lcd.clear();
   
  }
  else if (digitalRead(btStatePin) == LOW && bluetoothConnected) {
    bluetoothConnected = false;
    pricePerKg = 0; // reset price when disconnected
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connect Bluetooth");
  }

  if (!bluetoothConnected) return; // Do nothing if not connected

  // --- Always show weight ---
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  Serial.print("Reading: ");
  weight = scale.get_units(5); 
  //Serial.print(scale.get_units(), 2);
 // Serial.print(" lbs"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  Serial.print("Kilogram:");
  Serial.print( weight); 
  Serial.print(" Kg");
  Serial.print(" calibration_factor: ");
  Serial.print(calibration_factor);
  Serial.println();
  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 50;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 50;
  }
  if (weight < 0) weight = 0;

  // Convert to kg
 // float weightKg = weight / 1000.0;

  // Calculate cost
  totalCost = weight * pricePerKg;

  // --- Display all values on LCD (NO CLEAR) ---
   
  lcd.setCursor(0, 0);
  lcd.print("W:");
  lcd.print( weight, 2);
  lcd.print("Kg   "); // spaces clear old chars

  lcd.setCursor(10, 0);
  lcd.print("P:");
  lcd.print(pricePerKg, 0);
   lcd.setCursor(0, 1);
  lcd.print("Cost:");
 // lcd.print(totalCost, 2);
    lcd.print(totalCost);

  // --- Handle Bluetooth input for price ---
  if (BT.available()) {
    receivedChar = BT.read();

    if (receivedChar == '#') {  
      pricePerKg = inputPrice.toFloat();
      inputPrice = "";
    }
    else if (receivedChar == '*') {  
      inputPrice = "";
      pricePerKg = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Price Reset");
      delay(3000);
    }
    else {
      inputPrice += receivedChar; // build string
    }
  }

  delay(300);
}
