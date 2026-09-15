// For a connection via I2C using the Arduino Wire include:
#include <Wire.h>               
#include "HT_SSD1306Wire.h"
#include <UltrasonicA02YYUW.h>

UltrasonicA02YYUW sensor(Serial2, 6, 7); // RX, TX

static SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

int counter = 1;

void setup() {
  Serial.begin(115200);
  sensor.begin();
  Serial.println();

  VextON();
  delay(100);

  // Initialising the UI will init the display too.
  display.init();
  display.setFont(ArialMT_Plain_10);

}

void VextON(void)
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
  pinMode(Vext,OUTPUT);
  digitalWrite(Vext, HIGH);
}

void loop() {

  sensor.update();  // panggil rutin di loop

  float d = sensor.getDistance();
 // if (d > 0) {
  //  Serial.print("Jarak: ");
  //  Serial.print(d);
  //  display.drawString(10, 10, String(d));
  //  Serial.println(" cm");
  //}
  // clear the display
  display.clear();

  //char str[30];
  int x = 0;
  int y = 0;
  String valueString = String(d);
  // The coordinates define the left starting point of the text
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(x, y, "distance: " + valueString + " cm");
  display.display();


  counter++;
  
  delay(1000);
}
