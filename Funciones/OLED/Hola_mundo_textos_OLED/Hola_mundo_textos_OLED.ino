#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

/*
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
*/

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize with the I2C addr 0x3D if not working use 0x3C (for the 128x64)
  display.setTextColor(WHITE);
}

void loop() {
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  display.println("VICTIMA");
  display.display();

  delay(500);

  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0, 20);
  display.println("VICTIMA");
  display.display();
  delay(500);
  
  //setTextColor
  // draw scrolling text
  /*
    // text display tests
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Analog Read V1.0");
    display.setTextColor(BLACK, WHITE);
    display.println();
    display.println("A0");
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println();
    display.setTextSize(2);
    display.setTextSize(2);
    int sensorValue = analogRead(A0);
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
    float voltage = sensorValue * (5.0 / 1023.0);
    // print out the value you read:
    display.print(voltage); display.println("V");
    display.display();
  */
}
