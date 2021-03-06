#include <connection4g.h>
#include <telstraiot.h>
#include <iotshield.h>
#include <shieldinterface.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <dht11.h>

// Initialize Shield, sensors, and other interfaces
ShieldInterface shieldif;
IoTShield shield(&shieldif);
Connection4G conn(true,&shieldif);
LiquidCrystal_I2C lcd(0x20,20,4);
dht11 DHT;
#define DHT11_PIN 4
int buoyPin = 5; //Pin 5 is the water buoy
int pumpPin = 8; //Pin 7 is the pump
TelstraIoT iotPlatform(&conn,&shield);

const char host[] = "tic2017team024.iot.telstra.com";

char id[8];
char tenant[32];
char username[32];
char password[32];

void setup() {
  // Initialize pins for read or write
  pinMode(buoyPin, INPUT); 
  pinMode(pumpPin, OUTPUT); 

  // Initialize USB Serial Interface. Useful for debugging.
  Serial.begin(115200);
  delay(500);

  // Initialize LCD Panel. Whilst Text displays, other setup tasks will be performed.
  lcd.init();
  Serial.println(F("[START] Starting Send All Measurments Script"));

  lcd.backlight();
  lcd.print("IoT Aquaponics");
  lcd.setCursor(0,1);
  lcd.print("Team 024");
  lcd.setCursor(0,2);
  lcd.print("v0.5a");

  lcd.setCursor(0,3);
  lcd.print("Connecting...");
  if(!shield.isShieldReady())
  {
     Serial.println("waiting for shield ...");
    shield.waitUntilShieldIsReady();
  } else {
    Serial.println("Shield ready!");
  }

  Serial.println("Reading credentials...");
  shield.readCredentials(id,tenant,username,password);
  Serial.print("ID: ");
  Serial.println(id);
  Serial.print("Tenant: ");
  Serial.println(tenant); 
  Serial.print("Username: ");
  Serial.println(username);
  Serial.print("Password: ");
  Serial.println(password);
  
  Serial.println("Setting credentials...");
  iotPlatform.setCredentials(id,tenant,username,password,"");
  
  Serial.println("Setting host...");
  iotPlatform.setHost(host,443);
  
  conn.openTCP(host,443);  
  lcd.setCursor(0,3);
  lcd.print("Connected..! ");
}

void loop() {
  // define and initialize measurement variables
    delay(1000);
    char lightString[15];
    char tempString[15];
    int moistureInt = 0;
    int humidityInt = 0;
    int waterSwitch = 0;

    // refresh LCD screen
    lcd.clear();
    
    //Read measurement from water buoy and digitalWrite to pumpPin accordingly
    waterSwitch = digitalRead(buoyPin);
    shield.getLightLevel(lightString);
    shield.getTemperature(tempString);
    moistureInt = analogRead(A0);
    int chk = DHT.read(DHT11_PIN);    // READ DATA
    humidityInt = DHT.humidity;
    
    if (waterSwitch == 1){
    Serial.println("############################ Preparing to read MEASUREMENTS #############################");
    //Read Light measurement from device
    Serial.print(F("[    ] Light: "));
    Serial.println(lightString);
    lcd.setCursor(0,0);
    lcd.print("[    ] Light: ");
    lcd.print(lightString);
    
    //Read temperature measurement from device
    Serial.print(F("[    ] Temp: "));
    Serial.println(tempString);
    lcd.setCursor(0,1);
    lcd.print("[    ] Temp: ");
    lcd.print(tempString);

    //Read moisture measurement from sensor
    Serial.print(F("[    ] Moisture: "));
    Serial.println(moistureInt);
    lcd.setCursor(0,2);
    lcd.print("[    ] Moisture: ");
    lcd.print(moistureInt);

    //Read humidity measurement from sensor
    Serial.print(F("[    ] Humidity: "));
    Serial.println(humidityInt);
    lcd.setCursor(0,3);
    lcd.print("[    ] Humidity: ");
    lcd.print(humidityInt);
    } else {
      Serial.print("NO WATER REMAINING");
      lcd.setCursor(0,2);
      lcd.print("NO WATER REMAINING");
    }

    //Activate Pump when the moisture level reaches a critical level
    if (moistureInt < 300){
    // Uninterrupt the pump
      digitalWrite(pumpPin, HIGH);
    } else if (moistureInt > 500){
    // Interrupt the pump
      digitalWrite(pumpPin, LOW);
    }
    
    //Send data to cloud
    Serial.println("############################ Preparing to send MEASUREMENTS #############################");  
    iotPlatform.sendMeasurement("LightMeasurement", "LightMeasurement", "Light level (lux)", lightString, "lux");
    
    iotPlatform.sendMeasurement("TemperatureMeasurement", "TemperatureMeasurement", "Temperature (degrees Celsius)", tempString, "degrees Celsius");

    iotPlatform.sendMeasurement("MoistureMeasurement","MoistureMeasurement", "Moisture Raw Value", moistureInt, "value");

    iotPlatform.sendMeasurement("HumidityMeasurement","HumidityMeasurement","Humidity (grams of water/cubic meter of air)",humidityInt,"grams of water/cubic meter of air");

    iotPlatform.sendMeasurement("WaterLevelMeasurement","WaterLevelMeasurement","Water Level (Enough(1)/Refill(0))",waterSwitch,"Enough(1)/Refill(0)");
    
    
}
