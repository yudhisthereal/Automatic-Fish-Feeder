#include <Arduino.h>
#line 1 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
#include <Wire.h>              // Include I2C library for LCD
#include <LiquidCrystal_I2C.h> // Include I2C LCD library
#include "RTClib.h"            // Include RTC library
#include <EEPROM.h>            // Include EEPROM library
#include <Servo.h>             // Include Servo library
#include <LowPower.h>          // Include Low Power library

// Define debug symbols
#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

// Define feeding time editing constants
#define MAX_TIME 1440
#define N_EDIT_STEPS 6

// Define LCD address and button pins
#define BTN_EDIT 2  // Enter edit mode or confirm edit
#define BTN_INCR 4  // Increase by 30 mins
#define BTN_DECR 5  // Decrease by 30 mins
#define BTN_FEED 6  // Instantly start feeding mechanism
#define ALARM_PIN 3 // connected to RTC's SQW for alarm interrupt

// Define servo pin and feeding duration
#define SERVO_PIN 11
#define FEED_DURATION 1000 // 1 second in milliseconds

// Define edit mode IDs
#define EDIT_MODE_FEED 0
#define EDIT_MODE_RTC 1

// Define intervals/timers (ms)
#define BLINK_DELAY 500           // 0.5 secs
#define SLEEP_DELAY 5 * 60000     // Time it takes before sleep: 5 mins
#define BACKLIGHT_OFF_DELAY 15000 // Time it takes before turning off LCD backlight: 10 secs
#define BTN_HOLD_DELAY 700        // Time it takes before triggering "button hold" event: 1 sec

// Timing variables
unsigned long lastEditingBlinkTime = millis();
unsigned long lastWakeUp = millis();
unsigned long lastLcdBacklight = millis();

// NOTE: buttonIndex = BUTTON_PIN - 2
unsigned long btnHoldStart[5] = {
    0,
    -1, // no button assigned here
    0,
    0,
    0,
};

bool keepBtnHold[5] = {
    0,
    false, // no button assigned here
    0,
    0,
    0,
};

// Device component variables
const int lcd_address = 0x27;              // Replace with your I2C LCD address if different
LiquidCrystal_I2C lcd(lcd_address, 16, 2); // Initialize I2C LCD
RTC_DS3231 rtc;                            // Create RTC object
Servo servo;                               // Create Servo object

// Feeding time and RTC time
int feedingTime = 0; // Adjusted during init
int rtcTime = 0;     // rtc time in minutes (max = 1440)

// RTC editing steps
const int EDIT_STEPS[6] = {1, 5, 10, 15, 30, 60}; // edit steps (minutes) to choose from
byte rtcStepID = 0;                               // index of minutes chosen from EDIT_STEPS
byte feedStepID = 1;                              // index of minutes chosen from EDIT_STEPS

// Flags
// bool alreadyFed = false;       // Already fed at a certain time, don't feed again
char freshStart = 'y';          // Is the device starting for the first time?
bool isEditingTimeFeed = false; // Is in editing feeding time?
bool isEditingTimeRtc = false;  // Is editing RTC time?
bool backlightOn = false;       // LCD backlight status (on/off)
bool showEditingText = true;    // is showing "[Editing]" or not.
bool buttonsEnabled = true;

// Button states
// NOTE: buttonIndex = BUTTON_PIN - 2
bool prevBtnState[5] = {
    HIGH,  // edit button
    false, // no button assigned here
    HIGH,  // increase button
    HIGH,  // decrease button
    HIGH,  // feed button
};

// Button pins
const int BTN_PINS[4] = {
    BTN_EDIT,
    BTN_INCR,
    BTN_DECR,
    BTN_FEED,
};

#line 107 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void setup();
#line 133 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void loop();
#line 152 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void managePower();
#line 170 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void preSleep();
#line 182 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void postSleep();
#line 193 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void trySleeping();
#line 208 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
bool shouldNoBacklight();
#line 230 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void startLcd();
#line 252 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void initLcd();
#line 261 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void initRtc();
#line 304 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void setAlarmInterrupt();
#line 324 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void btnISR();
#line 329 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void alarmISR();
#line 340 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void enableButtons();
#line 345 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void disableButtons();
#line 350 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
bool isButtonsEnabled();
#line 360 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int btnPinToArrId(int btnPin);
#line 371 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void printEvent(int i, int event);
#line 457 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int checkBtn(int btnId);
#line 488 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int checkBtnEdit(int event);
#line 528 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int checkBtnIncr(int event);
#line 549 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int checkBtnDecr(int event);
#line 570 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int checkBtnFeed(int event);
#line 593 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void handleButtonEvents();
#line 633 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void startFeeding();
#line 672 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void handleAutoFeeding();
#line 689 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int feedingTimeHour();
#line 694 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int feedingTimeMinute();
#line 699 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int rtcHour();
#line 704 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int rtcMinute();
#line 709 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void setRtcTimeVar(int hour, int minute);
#line 714 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
bool syncRtcTimeVar();
#line 736 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void applyNewTimeRTC();
#line 748 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void updateUI();
#line 764 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void displayRtcEdit();
#line 792 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void displayFeedTimeEdit();
#line 821 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void displayTime();
#line 867 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void blinkEditText(bool editMode);
#line 899 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void handleUi();
#line 921 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void loadFromEEPROM();
#line 937 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void saveFeedingTime();
#line 107 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif
  DEBUG_PRINTLN("System Starting");

  servo.attach(SERVO_PIN);
  servo.write(0);
  delay(250);
  servo.detach();

  pinMode(BTN_EDIT, INPUT_PULLUP);
  pinMode(BTN_INCR, INPUT_PULLUP);
  pinMode(BTN_DECR, INPUT_PULLUP);
  pinMode(BTN_FEED, INPUT_PULLUP);

  loadFromEEPROM();

  initLcd();
  initRtc(); // pinMode for ALARM_PIN is set here.
  setAlarmInterrupt();

  updateUI();
}

void loop()
{
  managePower();
  handleButtonEvents();
  handleUi();
  handleAutoFeeding();

  delay(33); // Debounce buttons and avoid busy loop
}

////////////////////
// POWER MANAGEMENT
////////////////////
//
//

/**
 * @brief handles Low Power, i.e. LCD backlights and sleep
 */
void managePower()
{
  if (backlightOn)
  {
    if (shouldNoBacklight())
    {
      lcd.noBacklight();
      backlightOn = false;
      disableButtons();
    }
  }

  trySleeping();
}

/**
 * @brief specific things to do right before sleep
 */
void preSleep()
{
  attachInterrupt(digitalPinToInterrupt(BTN_EDIT), btnISR, LOW);
  lcd.noDisplay();

  DEBUG_PRINTLN("Sleeping...");
  Serial.flush();
}

/**
 * @brief specific things to do right after sleep
 */
void postSleep()
{
  detachInterrupt(digitalPinToInterrupt(BTN_EDIT));
  DEBUG_PRINTLN("I've woken up!");

  startLcd();
}

/**
 * @brief attempts to go PowerDown mode, if already awake for long enough
 */
void trySleeping()
{
  unsigned long currentTime = millis();
  if (currentTime - lastWakeUp >= SLEEP_DELAY)
  {
    preSleep();
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    postSleep();
  }
}

/**
 * @brief check if backlight should be off
 * @return a boolean: whether we should turn LCD backlight off
 */
bool shouldNoBacklight()
{
  bool goNoBacklight = false;

  unsigned long currentTime = millis();
  if (currentTime - lastLcdBacklight >= BACKLIGHT_OFF_DELAY)
  {
    goNoBacklight = true;
  }

  return goNoBacklight;
}

//////////////////////////
// DEVICE CONTROL / INIT
//////////////////////////
//
//

/**
 * @brief turn lcd display and backlight on
 */
void startLcd()
{
  lastLcdBacklight = lastWakeUp = millis(); // keeping it awake

  if (backlightOn)
  {
    return;
  }
  lcd.display();   // Turn on LCD display
  lcd.backlight(); // Turn on LCD backlight

  backlightOn = true;

  // enable the buttons after 100 ms,
  // so that the button event used to start the LCD isn't registered immediately
  delay(1000);
  enableButtons();
}

/**
 * @brief initializes the LCD, then calls startLcd()
 */
void initLcd()
{
  lcd.init();
  startLcd();
}

/**
 * @brief initializes the RTC and its alarm interrupt
 */
void initRtc()
{
  if (!rtc.begin())
  {
    DEBUG_PRINTLN("Couldn't find RTC");
    Serial.flush();
    while (1)
      delay(10);
  }

  // attach interrupt
  pinMode(ALARM_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ALARM_PIN), alarmISR, FALLING);

  if (rtc.lostPower())
  {
    DEBUG_PRINTLN("RTC lost power, let's set the time! ");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // we don't need the 32K Pin, so disable it
  rtc.disable32K();

  // Disable and clear both alarms
  rtc.clearAlarm(2);
  rtc.disableAlarm(2);

  rtc.writeSqwPinMode(DS3231_OFF); // Place SQW pin into alarm interrupt mode

  syncRtcTimeVar();
}

////////////////////
// TIME AND ALARMS
////////////////////
//
//

/**
 * @brief clears the alarm1 interrupt flag of the RTC, then set the alarm again.
 */
void setAlarmInterrupt()
{
  // clear alarm
  rtc.clearAlarm(1);

  DateTime alarmTime = DateTime(2024, 5, 12, feedingTimeHour(), feedingTimeMinute(), 0);
  if (!rtc.setAlarm1(alarmTime, DS3231_A1_Hour))
  {
    DEBUG_PRINTLN("ERROR: Alarm wasn't set! ");
  }
  else
  {
    DEBUG_PRINT("Alarm: ");
    DEBUG_PRINT(alarmTime.hour());
    DEBUG_PRINT(":");
    DEBUG_PRINTLN(alarmTime.minute());
    Serial.flush();
  }
}

void btnISR()
{
  // do nothing
}

void alarmISR()
{
  // do nothing
}

//////////////////////////
// BUTTON INPUT HANDLERS
//////////////////////////
//
//

void enableButtons()
{
  buttonsEnabled = true;
}

void disableButtons()
{
  buttonsEnabled = false;
}

bool isButtonsEnabled()
{
  return buttonsEnabled;
}

/**
 * @brief convert button pin number to array index. Is used to access button related arrays.
 * @param btnPin the button pin on the board
 * @return button ID used to access arrays related to buttons.
 */
int btnPinToArrId(int btnPin)
{
  return btnPin - 2;
}

#ifdef DEBUG
/**
 * @brief debug function to print button events
 * @param i button index
 * @param event button event
 */
void printEvent(int i, int event)
{
  char eventStr[8];
  switch (event)
  {
  case 1:
    strcpy(eventStr, "Pressed");
    break;

  case 2:
    strcpy(eventStr, "Released");
    break;

  case 3:
    strcpy(eventStr, "Hold");
    break;

  default:
    return;
  }

  DEBUG_PRINT("EVENT: btn ");
  DEBUG_PRINT(i);
  DEBUG_PRINT(" ");
  DEBUG_PRINTLN(eventStr);
  Serial.flush();
}

#else

void printEvent(int i, int event)
{
  // do nothing
}

#endif

/*
return values:
  0: no events
  1: pressed
  2: released
  3: hold
*/
int getButtonEvent(int btnId = BTN_EDIT)
{
  int event = 0;
  int i = btnPinToArrId(btnId); // array ID of the corresponding button pin

  if (digitalRead(btnId) && prevBtnState[i] == LOW)
  {
    prevBtnState[i] = HIGH; // button release event
    event = 2;
  }
  else if (!digitalRead(btnId))
  {
    delay(25); // debounce

    if (!digitalRead(btnId))
    {
      event = 1; // button press event

      if (prevBtnState[i] == LOW)
      {
        unsigned long currentTime = millis();
        if (currentTime - btnHoldStart[i] >= BTN_HOLD_DELAY)
        {
          event = 3; // button hold event
        }
      }
      else
      {
        // started pressing button
        prevBtnState[i] = LOW;
        btnHoldStart[i] = millis();
      }
    }
  }

  printEvent(i, event);
  Serial.flush();
  delay(3);

  return event;
}

int checkBtn(int btnId)
{
  int event = getButtonEvent(btnId);

  if (!isButtonsEnabled())
  {
    return event;
  }

  switch (btnId)
  {
  case BTN_EDIT:
    checkBtnEdit(event);
    break;

  case BTN_INCR:
    checkBtnIncr(event);
    break;

  case BTN_DECR:
    checkBtnDecr(event);
    break;

  case BTN_FEED:
    checkBtnFeed(event);
    break;
  }

  return event;
}

int checkBtnEdit(int event)
{
  int i = btnPinToArrId(BTN_EDIT);

  if (event == 3 && !isEditingTimeFeed)
  { // button hold
    isEditingTimeRtc = true;
    keepBtnHold[i] = 1;
    syncRtcTimeVar();
  }
  else if (event == 2)
  { // button release
    DEBUG_PRINTLN("EDIT BUTTON RELEASED");
    if (!keepBtnHold[i])
    { // and not right after a "Hold" event
      if (isEditingTimeRtc)
      {
        isEditingTimeRtc = false;
        applyNewTimeRTC();
      }
      else if (isEditingTimeFeed)
      {
        isEditingTimeFeed = false;
        saveFeedingTime(); // also sets an alarm interrupt (RTC)
      }
      else
      {
        isEditingTimeFeed = true;
        DEBUG_PRINTLN("editing feeding time...");
      }
    }
    else
    {
      keepBtnHold[i] = 0;
    }
  }

  return event;
}

int checkBtnIncr(int event)
{
  if (event == 1 || event == 3)
  { // button pressed
    if (isEditingTimeFeed)
    {
      // Increase feeding time (wrap around)
      feedingTime += EDIT_STEPS[feedStepID];
      feedingTime %= MAX_TIME;
    }
    else if (isEditingTimeRtc)
    {
      // decrease rtc time (wrap around)
      rtcTime += EDIT_STEPS[rtcStepID];
      rtcTime %= MAX_TIME;
    }
  }

  return event;
}

int checkBtnDecr(int event)
{
  if (event == 1 || event == 3)
  { // button pressed
    if (isEditingTimeFeed)
    {
      // decrease feeding time (wrap around)
      feedingTime -= EDIT_STEPS[feedStepID] - MAX_TIME;
      feedingTime %= MAX_TIME;
    }
    else if (isEditingTimeRtc)
    {
      // decrease rtc time (wrap around)
      rtcTime -= EDIT_STEPS[rtcStepID] - MAX_TIME;
      rtcTime %= MAX_TIME;
    }
  }

  return event;
}

int checkBtnFeed(int event)
{
  if (event == 1 || event == 3)
  {
    if (isEditingTimeRtc)
    { // change rtc editing step (minutes)
      rtcStepID++;
      rtcStepID %= N_EDIT_STEPS;
    }
    else if (isEditingTimeFeed)
    { // change feed time editing step (minutes)
      feedStepID++;
      feedStepID %= N_EDIT_STEPS;
    }
    else if (event != 3)
    { // button pressed
      startFeeding();
    }
  }

  return event;
}

void handleButtonEvents()
{
  // Track how many buttons pressed
  int btnEvents = 0;

  // Check button inputs
  for (int i = 0; i < 4; i++)
  {
    btnEvents += checkBtn(BTN_PINS[i]);
  }

  // Check if there are button events
  if (btnEvents > 0)
  {
    startLcd();
    updateUI();
  }
}

// void checkFeedingTime() {
//   // Check if current minute matches the feeding time (every 30 minute)
//   DateTime now = rtc.now();
//   if (now.minute() % 30 == 0 && now.hour() == feedingTimeHour()) {
//     if (! alreadyFed) {
//       startFeeding();
//       alreadyFed = true;
//       EEPROM.write(1, true);
//     }
//   } else {
//     alreadyFed = false;  // Reset flag if feeding time doesn't match and alreadyFed is true
//     EEPROM.write(1, false);
//   }
// }

///////////////////////
// TIMING AND FEEDING
///////////////////////
//
//

void startFeeding()
{
  // attempts to turn lcd on
  // won't do anything if backlight already on
  startLcd();

  // case: feeding triggered by alarm, in the middle of editing feed time
  if (isEditingTimeFeed)
  {
    isEditingTimeFeed = false;
  }
  else if (isEditingTimeRtc)
  {
    isEditingTimeRtc = false;
  }

  lcd.clear();
  lcd.print("Feeding...");

  servo.attach(SERVO_PIN); // Attach servo to pin
  for (int i = 0; i < 180; i++)
  { // Rotate servo to dispense food
    servo.write(i);
    delay(5);
  }

  delay(FEED_DURATION);

  for (int i = 180; i > 0; i--)
  { // Rotate servo to initial rotation
    servo.write(i);
    delay(5);
  }
  delay(1000);    // Make sure the servo finished rotating
  servo.detach(); // Detach servo to save power

  updateUI();
}

void handleAutoFeeding()
{
  //////////////////////////////////////////////////////
  // Feeding is handled by RTC alarm
  // uncomment the code below if RTC alarm doesn't work
  ///////////////////////////////////////////////////////
  //
  // checkFeedingTime();
  //

  if (rtc.alarmFired(1))
  {
    startFeeding();
    setAlarmInterrupt();
  }
}

int feedingTimeHour()
{
  return feedingTime / 60;
}

int feedingTimeMinute()
{
  return feedingTime % 60;
}

int rtcHour()
{
  return rtcTime / 60;
}

int rtcMinute()
{
  return rtcTime % 60;
}

void setRtcTimeVar(int hour, int minute)
{
  rtcTime = hour * 60 + minute;
}

bool syncRtcTimeVar()
{
  bool synced = true;

  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();

  // if currently editing rtc time var,
  // don't set the rtc time var from here
  if (!isEditingTimeRtc)
  {
    if (rtcMinute() != minute)
    { // not synced
      synced = false;
      setRtcTimeVar(hour, minute);
    }
  }

  return synced;
}

void applyNewTimeRTC()
{
  rtc.adjust(DateTime(2024, 5, 12, rtcHour(), rtcMinute(), 0));
}

/////////////
// DISPLAYS
/////////////
//
//

// display controller, choose which display to be shown by LCD
void updateUI()
{
  if (isEditingTimeRtc)
  {
    displayRtcEdit();
  }
  else if (isEditingTimeFeed)
  {
    displayFeedTimeEdit();
  }
  else
  {
    displayTime();
  }
}

void displayRtcEdit()
{
  lcd.clear();

  lcd.print("Step: ");
  lcd.print(EDIT_STEPS[rtcStepID]);
  lcd.print(" mins.");

  // set cursor to second row
  lcd.setCursor(0, 1);

  int hour = rtcHour();
  int minute = rtcMinute();

  if (hour < 10)
  {
    lcd.print("0");
  }
  lcd.print(hour);
  lcd.print(":");

  if (minute < 10)
  {
    lcd.print("0");
  }
  lcd.print(minute);
}

void displayFeedTimeEdit()
{
  lcd.clear();

  lcd.print("Step: ");
  lcd.print(EDIT_STEPS[feedStepID]);
  lcd.print(" mins.");

  // set cursor to second row
  lcd.setCursor(0, 1);

  int hour = feedingTimeHour();
  int minute = feedingTimeMinute();

  if (hour < 10)
  {
    lcd.print("0");
  }
  lcd.print(hour);
  lcd.print(":");

  if (minute < 10)
  {
    lcd.print("0");
  }
  lcd.print(minute);
}

// display current time and feeding time in LCD
void displayTime()
{
  // Display current time and feeding time on LCD
  lcd.clear();
  lcd.setCursor(5, 0);

  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();

  // Real Time
  if (hour < 10)
  {
    lcd.print("0");
  }
  lcd.print(hour);
  lcd.print(":");

  if (minute < 10)
  {
    lcd.print("0");
  }
  lcd.print(minute);
  lcd.print("'");

  // Feeding Time
  lcd.setCursor(0, 1);
  lcd.print("Feed Time: ");

  hour = feedingTimeHour();
  minute = feedingTimeMinute();
  if (hour < 10)
  {
    lcd.print("0");
  }
  lcd.print(hour);
  lcd.print(":");

  if (minute < 10)
  {
    lcd.print("0");
  }
  lcd.print(minute);
}

// Blink "[Editing]" text when editing feed time
void blinkEditText(bool editMode)
{
  unsigned long currentTime = millis();
  if (currentTime - lastEditingBlinkTime >= BLINK_DELAY)
  {
    showEditingText = !showEditingText;
    lastEditingBlinkTime = currentTime;

    // Move the cursor to the position of "[EDIT]" text
    lcd.setCursor(8, 1);

    if (showEditingText)
    {
      if (isEditingTimeRtc)
      {
        lcd.print(" EditRTC");
      }
      else
      {
        lcd.print("EditFeed");
      }
    }
    else
    {
      lcd.print("           "); // Clear it
    }

    // Set the cursor position back to origin
    lcd.setCursor(0, 0);
  }
}

void handleUi()
{
  // Update display if time changed (minute precision)
  if (!syncRtcTimeVar())
  {
    updateUI();
  }

  // Blink "[Edit]" text when editing
  if (isEditingTimeRtc || isEditingTimeFeed)
  {
    blinkEditText(1);
  }
}

//////////////////////////
// EEPROM READ AND WRITE
//////////////////////////
//
//

// Load variables from EEPROM (if saved previously)
void loadFromEEPROM()
{
  freshStart = EEPROM.read(2);
  if (freshStart == 'n')
  {
    feedingTime = EEPROM.read(0);
    // alreadyFed = EEPROM.read(1);
  }
  else
  {
    freshStart = 'y';     // only once
    EEPROM.write(2, 'n'); // afterwards, load from this 'n' instead
  }
}

// save feeding time to EEPROM
void saveFeedingTime()
{
  // Save feeding time to EEPROM
  EEPROM.write(0, feedingTime);
  setAlarmInterrupt();
}

