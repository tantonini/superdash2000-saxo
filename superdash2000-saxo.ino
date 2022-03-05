#include <LiquidCrystal_I2C.h>

/* Configuration */
const int button_debounce_ms = 250;  /* Debounce when pushing button */
const bool config_serial = false; /* Enable/disable serial output */
const int lcd_height = 2; /* LCD height in characters */
const int lcd_width = 16; /* LCD width in characters */
const int pin_button = 0x2; /* Digital pin for push button */
const int welcome_screen_duration_ms = 2000;  /* Welcome scren duration in ms */

/* Constants */
const int lcd_i2c_addr = 0x27;
const int char_empty = 0;
const int char_full = 1;
const byte lcd_char_empty[] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
const byte lcd_char_full[] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

/* Globals */
LiquidCrystal_I2C lcd(lcd_i2c_addr, lcd_width, lcd_height);

void setup() {
  if (config_serial) {
    Serial.begin(9600);
  }
  configure_lcd();
  welcome_screen();
  configure_pins();
  configure_interrupts();
}

void configure_lcd() {
  lcd.init();
  lcd.clear();
  lcd.backlight();

  lcd.createChar(char_empty, (unsigned char *)lcd_char_empty);
  lcd.createChar(char_full, (unsigned char *)lcd_char_full);
}

void welcome_screen() {
  lcd.setCursor(3, 0);
  lcd.print("SUPER DASH");
  lcd.setCursor(6, 1);
  lcd.print("2000");

  delay(welcome_screen_duration_ms);

  lcd.clear();
}

void configure_pins() {
  pinMode(pin_button, INPUT);
}

void configure_interrupts() {
  attachInterrupt(digitalPinToInterrupt(pin_button), button_irq_handler, RISING);
}

void loop() {
  // put your main code here, to run repeatedly:

}

/* IRQ Handlers */
void button_irq_handler(void) {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();

  /* If interrupts come faster than button_debounce_ms, assume it's a bounce and ignore */
  if (interrupt_time - last_interrupt_time > button_debounce_ms) {
    if (config_serial) {
      Serial.println("Button pushed");
    }
  }

  last_interrupt_time = interrupt_time;
}
