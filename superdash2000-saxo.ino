#include <LiquidCrystal_I2C.h>

/* Configuration */
const int button_debounce_ms = 250;  /* Debounce when pushing button */
const bool config_serial = false; /* Enable/disable serial output */
const int lcd_height = 2; /* LCD height in characters */
const int lcd_width = 16; /* LCD width in characters */
const int loop_period = 300;  /* Main loop period */
const int pin_button = 0x2; /* Digital pin for push button */
const int welcome_screen_duration_ms = 2000;  /* Welcome scren duration in ms */
/* This enum is used for the order of the modes.
   Just reorganize the enum for modifying the order.
   Modes can also be removed from this enum to not use them */
enum mode {
  MODE_MAX
};

/* Constants */
const int adc_resolution = 1024;
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
float adc_reference;
bool change_mode = false;
LiquidCrystal_I2C lcd(lcd_i2c_addr, lcd_width, lcd_height);
int mode = 0;

void setup() {
  if (config_serial) {
    Serial.begin(9600);
  }
  configure_lcd();
  welcome_screen();
  configure_adc();
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

void configure_adc() {
  analogReference(INTERNAL4V3);
  adc_reference = 4.3;
}

void configure_pins() {
  pinMode(pin_button, INPUT);
}

void configure_interrupts() {
  attachInterrupt(digitalPinToInterrupt(pin_button), button_irq_handler, RISING);
}

void loop() {
  unsigned long t1;
  unsigned long t2;
  t1 = millis();
  if (change_mode) {
    lcd.clear();  /* Clear the LCD when changing mode to avoid undesired characters */
    change_mode = false;
  }

  /* Run the selected mode */

  t2 = millis();

  if ((t2 - t1) < loop_period) {
    delay(loop_period - (t2 - t1));
  }
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

    mode++;
    if (mode >= MODE_MAX) {
      mode = 0;
    }
    change_mode = true;
  }

  last_interrupt_time = interrupt_time;
}
