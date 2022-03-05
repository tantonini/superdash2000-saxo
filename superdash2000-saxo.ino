#include <LiquidCrystal_I2C.h>

/* Configuration */
const int button_debounce_ms = 250;  /* Debounce when pushing button */
const bool config_serial = false; /* Enable/disable serial output */
const int lcd_height = 2; /* LCD height in characters */
const int lcd_refresh_period_ms = 3000; /* Period for LCD refresh */
const int lcd_width = 16; /* LCD width in characters */
const int loop_period = 300;  /* Main loop period */
const int pin_battery = A0; /* Analog pin for measuring battery voltage */
const int pin_button = 0x2; /* Digital pin for push button */
const int pin_water = 0x3;  /* Digital pin for measuring water temperature */
const int welcome_screen_duration_ms = 2000;  /* Welcome scren duration in ms */
/* This enum is used for the order of the modes.
   Just reorganize the enum for modifying the order.
   Modes can also be removed from this enum to not use them */
enum mode {
  MODE_WATER = 0,
  MODE_BATTERY,
  MODE_MAX
};

/* Constants */
const int adc_resolution = 1024;
const int lcd_i2c_addr = 0x27;
const int char_empty = 0;
const int char_full = 1;
const int char_degree = 2;
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
const byte lcd_char_degree[] = {
  0b00100,
  0b01010,
  0b00100,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

/* Globals */
float adc_reference;
bool change_mode = false;
LiquidCrystal_I2C lcd(lcd_i2c_addr, lcd_width, lcd_height);
int mode = 0;
float water_duty_cycle;

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
  lcd.createChar(char_degree, (unsigned char *)lcd_char_degree);
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
  pinMode(pin_water, INPUT);
}

void configure_interrupts() {
  attachInterrupt(digitalPinToInterrupt(pin_button), button_irq_handler, RISING);
  attachInterrupt(digitalPinToInterrupt(pin_water), water_irq_handler, CHANGE);
}

void loop() {
  unsigned long t1;
  unsigned long t2;
  static unsigned long lcd_refresh_t = lcd_refresh_period_ms;

  t1 = millis();
  /* Only run mode after reaching LCD refresh or changing mode */
  lcd_refresh_t += loop_period;
  if ((lcd_refresh_t >= lcd_refresh_period_ms) || change_mode) {
    lcd_refresh_t = 0;
    if (change_mode) {
      lcd.clear();  /* Clear the LCD when changing mode to avoid undesired characters */
      change_mode = false;
    }

    /* Run the selected mode */
    if (MODE_BATTERY == mode) {
      mode_battery();
    }
    else if (MODE_WATER == mode) {
      mode_water();
    }
  }

  t2 = millis();

  if ((t2 - t1) < loop_period) {
    delay(loop_period - (t2 - t1));
  }
}

void mode_battery(void) {
  float battery_voltage;
  const int r1 = 5100;
  const int r2 = 2000;
  int battery_adc;

  /* Get battery voltage */
  battery_adc = analogRead(pin_battery);
  battery_voltage = battery_adc * adc_reference / adc_resolution;
  /* Voltage divider */
  battery_voltage = battery_voltage * (r1 + r2) / r2;

  /* Print battery voltage */
  lcd.setCursor(0, 0);
  lcd.print("BATTERY");
  lcd.setCursor(0, 1);

  if (battery_voltage <= 9.99) {
    lcd.print(" ");
  }

  lcd.print(battery_voltage);
  lcd.print(" V");
}

void mode_water(void) {
  int water_temp;

  /* Water temp is obtained from the signal duty cycle with the following function:
   * temp = -193 * duty_cycle + 145 */
  water_temp = -193 * water_duty_cycle + 145;

  /* Print battery voltage */
  lcd.setCursor(0, 0);
  lcd.print("WATER");
  lcd.setCursor(0, 1);

  if (water_temp <= -10) {
  }
  else if (water_temp <= -1) {
    lcd.print(" ");
  }
  else if (water_temp <= 9) {
    lcd.print("  ");
  }
  else if (water_temp <= 99) {
    lcd.print(" ");
  }
  else if (water_temp >= 100) {
  }

  lcd.print(water_temp);
  lcd.print(" ");
  lcd.write(char_degree);
  lcd.print("C");
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

void water_irq_handler(void) {
  /*        ______       ______
            |    |       |    |
            |    |       |    |
     _______|    |_______|    |___
            t1   t2      t1   t2
  */
  int state = digitalRead(pin_water);
  static unsigned long t1;
  static unsigned long t2;
  static unsigned long high_time;
  static unsigned long low_time;

  if (HIGH == state) {
    t1 = micros();
    low_time = t1 - t2;
    water_duty_cycle = float(high_time) / float(high_time + low_time);
  }
  else {
    t2 = micros();
    high_time = t2 - t1;
  }
}
