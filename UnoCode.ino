
#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <LoRa.h>
#include <BH1750.h>
#include <SHT2x.h>
#include <avr/interrupt.h>

BH1750 lightMeter;
SHT2x sht; 
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int LoRA_FREQ = 433E6;
const int CHIP_SELECT = 4;
int counter = 0;
float lux;
float hum;
float temp;

volatile bool sendFlag = false;

void setup() {
  Wire.begin();
  
  // Enable Serial communication for debugging
  Serial.begin(9600);
  while (!Serial);


  lcd_init();
  sensor_init();
  LoRa_init();
  timer1_init();
  delay(1000);
 }

void loop() {

 if (sendFlag) {
    sendFlag = false;



    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pkt:");
    lcd.print(counter);

    print_sensor_info();
    
    char luxStr[10], tempStr[10], humStr[10];
    char LoRa_packet[50];

    
    dtostrf(lux, 6, 2, luxStr); 
    dtostrf(temp, 5, 2, tempStr);
    dtostrf(hum, 5, 2, humStr);

    snprintf(LoRa_packet, sizeof(LoRa_packet), "%s lx | %sC | %s%% ", luxStr, tempStr, humStr);

    Serial.println(LoRa_packet);

    LoRa.beginPacket();\
    LoRa.print(LoRa_packet);
    LoRa.print(counter);
    LoRa.endPacket();

    counter++;
    
  }
}

void lcd_init() {
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LCD init");
  lcd.setCursor(0, 1);
  lcd.print("successful");
}


void print_sensor_info() {
  sht.read();

  lux = lightMeter.readLightLevel();
  temp = sht.getTemperature();
  hum  = sht.getHumidity();

  lcd.print(" H: ");
  lcd.print(hum);
  lcd.print("%");
  
  lcd.setCursor(0, 1);
  lcd.print(lux);
  lcd.print(" lx|");

  lcd.print(temp);
  lcd.print("C");
}

void sensor_init(){

  delay(100);  
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("\nBH1750 initialized.");
  } else {
    Serial.println("\nBH1750 failed.");
  }

  float testTemp = sht.getTemperature();
  if (!isnan(testTemp)) {
    Serial.println("SHT21 initialized.");
  } else {
    Serial.println("SHT21 failed.");
  }
}

void LoRa_init() {
  
  Serial.println("LoRa Sender");

  if (!LoRa.begin(LoRA_FREQ)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  } else {
    Serial.println("Lora init successful.");
  }

  LoRa.setTxPower(17);

}

void timer1_init() {

  cli();

  TCCR1A = 0;               // Set entire TCCR1A register to 0
  TCCR1B = 0;               // Same for TCCR1B
  TCNT1  = 0;               // Initialize counter value to 0

  // Set compare match register for 2 Hz increments (2 seconds)
  OCR1A = 31249;            // (16*10^6) / (1024*0.5Hz) - 1 = 31249
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10); // 1024 prescaler
  TIMSK1 |= (1 << OCIE1A);  // Enable timer compare interrupt

  sei(); 
}

ISR(TIMER1_COMPA_vect) {
  sendFlag = true;
}
