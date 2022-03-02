#include <LiquidCrystal_I2C.h>

/* Configuration */
const int lcd_height = 2; /* LCD height in characters */
const int lcd_width = 16; /* LCD width in characters */

/* Constants */
const int lcd_i2c_addr = 0x27;

/* Globals */
LiquidCrystal_I2C lcd(lcd_i2c_addr, lcd_width, lcd_height);

void setup() {
  configure_lcd();
}

void configure_lcd() {
  lcd.init();
  lcd.clear();
  lcd.backlight();

  lcd.setCursor(3, 0);
  lcd.print("SUPER DASH");
  lcd.setCursor(6, 1);
  lcd.print("2000");
}

void loop() {
  // put your main code here, to run repeatedly:

}
