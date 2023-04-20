
// LIBRARY
#include <Wire.h> 
#include <LiquidCrystal.h>

//DEFINITIONS
#define PUMP_PRIMING_TIME 11
#define MAX_DUTY 85
#define MAX_TEST_LENGTH 90
#define MAX_FREQUENCY 130

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);           // select the pins used on the LCD panel

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

#define FREQUENCY_MENU   0
#define DUTY_CYCLE_MENU  1
#define TEST_LENGTH_MENU 2
#define TEST_RUN_MENU    3

unsigned int dPin_pump = 11;
unsigned int dPin_injector = 12;     
unsigned int dPin_injector_led = 13;

//VARS
 int selector_menu=0;
 boolean start_test = false;
 int selector_value, hz_value, duty_cycle, rpm_engine, value, test_secs;

// ------------------------------------------------------------------
void setup()
{
  pinMode(dPin_pump, OUTPUT);
  digitalWrite(dPin_pump, LOW);

  pinMode(dPin_injector, OUTPUT);    // pin 2 as an output
  digitalWrite(dPin_injector, LOW);

  pinMode(dPin_injector_led, OUTPUT);
  digitalWrite(dPin_injector_led, LOW);

  hz_value = 30;
  test_secs = 10;
  duty_cycle = 20;

  lcd.begin(16, 2);               // start the library
  lcd.setCursor(2, 0);
  lcd.print("Gumis91");
  lcd.setCursor(0, 1);
  lcd.print("Injector tester");
  delay(2500);  //Sleep 3 secs
  lcd.clear();
}


void loop() 
{
  rpm_engine = hz_to_rpm_conversion();
  show_menu();

  lcd_key = read_LCD_buttons();
  switch (lcd_key)
  {
    case btnRIGHT:
    {
      switchMenu(1);
      break;
    }
    case btnLEFT:
    {
      switchMenu(-1);
      break;
    }    
    case btnUP:
    {
      modifySetting(selector_menu, 1);
      break;
    }
    case btnDOWN:
    {
      modifySetting(selector_menu, -1);
      break;
    }
  }

  switch (selector_menu)
  {
    case FREQUENCY_MENU:
    {
      if(hz_value < 100)
      {
        lcd.setCursor(6, 0);
        lcd.print(" ");
      }

      if(hz_value < 10)
      {
        lcd.setCursor(5, 0);
        lcd.print(" ");
      }

      lcd.setCursor(0, 0);
      lcd.print(">");
      lcd.setCursor(9, 0);
      lcd.print(" ");
      lcd.setCursor(0, 1);
      lcd.print(" ");
      lcd.setCursor(9, 1);
      lcd.print(" ");
      break;
    }

    case DUTY_CYCLE_MENU:
    {
      if(duty_cycle < 10)
      {
        lcd.setCursor(14, 0);
        lcd.print(" ");
      }
      lcd.setCursor(0, 0);
      lcd.print(" ");
      lcd.setCursor(9, 0);
      lcd.print(">");
      lcd.setCursor(0, 1);
      lcd.print(" ");
      lcd.setCursor(9, 1);
      lcd.print(" ");
      break;
    }

    case TEST_LENGTH_MENU:
    {
      if(test_secs < 10)
      {
        lcd.setCursor(4, 1);
        lcd.print(" ");
      }
      lcd.setCursor(0, 0);
      lcd.print(" ");
      lcd.setCursor(9, 0);
      lcd.print(" ");
      lcd.setCursor(0, 1);
      lcd.print(">");
      lcd.setCursor(9, 1);
      lcd.print(" ");
      break;
    }

    case TEST_RUN_MENU:
    {
      lcd.setCursor(0, 0);
      lcd.print(" ");
      lcd.setCursor(9, 0);
      lcd.print(" ");
      lcd.setCursor(0, 1);
      lcd.print(" ");
      lcd.setCursor(9, 1);
      lcd.print(">");

      if(lcd_key == btnSELECT)
      {
        selector_menu++;
        delay(250);
         //RUN THE TEST
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("STARTING TEST");
        delay(1000);
        lcd.setCursor(0, 1);
        lcd.print("3");
        delay(1000);
        lcd.setCursor(0, 1);
        lcd.print("2");
        delay(1000);
        lcd.setCursor(0, 1);
        lcd.print("1");
        delay(1000);
        // ----------------------------------------
        start_pump();
        run_injector(test_secs, hz_value, rpm_engine, duty_cycle);
        shutdown_pump();
        // ----------------------------------------
        selector_menu = 0;
        lcd.clear();
      }
      break;
    }
  }
}

int read_LCD_buttons()
{               // read the buttons
    adc_key_in = analogRead(0);       // read the value from the sensor 

    // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    // We make this the 1st option for speed reasons since it will be the most likely result

    if (adc_key_in > 1000) return btnNONE; 

    // For V1.1 us this threshold
    if (adc_key_in < 50)   return btnRIGHT;  
    if (adc_key_in < 250)  return btnUP; 
    if (adc_key_in < 450)  return btnDOWN; 
    if (adc_key_in < 650)  return btnLEFT; 
    if (adc_key_in < 850)  return btnSELECT;  

    return btnNONE;                // when all others fail, return this.
}

void show_menu()
{
  lcd.setCursor(1, 0);
  lcd.print("Hz");
  lcd.setCursor(4, 0);
  lcd.print(hz_value);
  
  lcd.setCursor(10, 0);
  lcd.print("DC");
  lcd.setCursor(13, 0);
  lcd.print(duty_cycle);
  lcd.setCursor(15, 0);
  lcd.print("%");
  //-------------------
  lcd.setCursor(1, 1);
  lcd.print("T");
  lcd.setCursor(3, 1);
  lcd.print(test_secs);
  lcd.setCursor(5, 1);
  lcd.print("s");
  
  lcd.setCursor(10, 1);
  lcd.print("RUN");
}

void modifySetting(int menuSelected, int delta)
{
  switch (menuSelected)
  {
    case FREQUENCY_MENU:
      hz_value = (hz_value + delta) % (MAX_FREQUENCY+1);
      if(hz_value < 0)
      {
        hz_value = MAX_FREQUENCY;
      }
      delay(250);
      break;
          
    case DUTY_CYCLE_MENU:
      duty_cycle = (duty_cycle + delta) % (MAX_DUTY+1);
      if(duty_cycle < 0)
      {
        duty_cycle = MAX_DUTY;
      }
      delay(250);
      break;

    case TEST_LENGTH_MENU:
      test_secs = (test_secs + delta) % (MAX_TEST_LENGTH+1);
      if(test_secs < 0)
      {
        test_secs = MAX_TEST_LENGTH;
      }
      delay(250);
  }
}

void switchMenu(int direction)
{
  selector_menu = (selector_menu + direction) % 4;
  if(selector_menu < 0)
  {
    selector_menu = 3;
  }
  delay(250);
}

// ------------------------------------------------------------------
// This function converts the hz selected to RPM to can know the "real" RPMs of the engine.
int hz_to_rpm_conversion()
{
  return hz_value * 60;
}

void start_pump()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PRIMING PUMP");
  
  digitalWrite(dPin_pump, HIGH);

  for(int i = PUMP_PRIMING_TIME; i--; i==0)
  {
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print(i);
  }
}

void shutdown_pump()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TURNING PUMP OFF...");
  digitalWrite(dPin_pump, LOW);
  delay(1000);
}

// ------------------------------------------------------------------
// Function to test x secs the injectors
// calculate the high and low times per siganl and sends to the functions
void run_injector(int secs, int hz, int rpm, int dc)
{
  unsigned long time_start_test, time_end;
  double ms;
  int ms_high, ms_low;
  long int send_us_low, send_us_high;
  
  ms_high = (pow(hz, -1)*dc) * 10;  //miliseconds with the duty cycle correction (High signal)
  ms_low = ((pow(hz, -1)*1000) - ms_high);  //miliseconds of the low signal of the cycle
  send_us_high = long(ms_high) * 1000;
  send_us_low = long(ms_low) * 1000;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RUNNING TEST");
  lcd.setCursor(0, 1);
  lcd.print(secs);
  lcd.setCursor(2, 1);
  lcd.print("s");
  lcd.setCursor(5, 1);
  lcd.print(rpm);
  lcd.print("rpm ");
  lcd.print(dc);
  lcd.print("%");
  
  time_start_test = micros();    // read the microseconds from the MCU
  time_end = time_start_test + (secs * 1000000);    // calc all the time of the test

  do
  {
    //Do a pulse of ms milisends in the injector and led pins
    do_pulse(send_us_high);
    
    // and wait ms_low ms to the next pulse
    wait_ms(send_us_low);

  }
  while( (micros() < time_end ));
  lcd.clear();
}

// ------------------------------------------------------------------
// this function generates a pulse. Recive the miliseconds of the duration and the pin trow th pulse has to be send
void do_pulse(long int us_wait)
{  
  long int micros_init, micros_end;
  long int i;

  micros_init = micros();  // Read the microseconds from the MCU
  micros_end = micros_init + us_wait; //Calculate when the end is

  digitalWrite(dPin_injector, HIGH);
  digitalWrite(dPin_injector_led, HIGH);

  do 
  {
    i = micros();
  } while( i < micros_end);

  digitalWrite(dPin_injector, LOW);
  digitalWrite(dPin_injector_led, LOW);
}

// ------------------------------------------------------------------
// This function wait ms ms that you pass as an argumen
void wait_ms(long int us_wait)
{  
  long int micros_wait;
  long int micros_init, micros_end;
  long int i;

  micros_init = micros();  // Read the microseconds from the MCU
  micros_end = micros_init + us_wait; //Calculate when the end is

  do
  {
    i = micros();
  } while ( i < micros_end );
}
