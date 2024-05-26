#include <Arduino.h>
#line 1 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
#include <Wire.h>               // Include I2C library for LCD
#include <LiquidCrystal_I2C.h>  // Include I2C LCD library
#include "RTClib.h"             // Include RTC library
#include <EEPROM.h>             // Include EEPROM library
#include <Servo.h>              // Include Servo library
#include <LowPower.h>           // Include Low Power library

// Define feeding time editing constants
#define TIME_EDIT_PRECISION 15
#define HOUR_FACTOR 60 / TIME_EDIT_PRECISION
#define TIME_EDIT_MOD 24 * HOUR_FACTOR
#define MAX_RTC 1440
#define N_RTC_EDIT_STEPS 6

// Define LCD address and button pins
#define BTN_EDIT 2   // Enter edit mode or confirm edit
#define BTN_INCR 4   // Increase by 30 mins
#define BTN_DECR 5   // Decrease by 30 mins
#define BTN_FEED 6   // Instantly start feeding mechanism
#define ALARM_PIN 3  // connected to RTC's SQW for alarm interrupt

// Define servo pin and feeding duration
#define SERVO_PIN 11
#define FEED_DURATION 1000  // 1 second in milliseconds

// Define intervals/timers (ms)
#define BLINK_DELAY 500           // 0.5 secs
#define SLEEP_DELAY 5 * 60000         // Time it takes before sleep: 5 mins
#define BACKLIGHT_OFF_DELAY 15000  // Time it takes before turning off LCD backlight: 10 secs
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
const int lcd_address = 0x27;               // Replace with your I2C LCD address if different
LiquidCrystal_I2C lcd(lcd_address, 16, 2);  // Initialize I2C LCD
RTC_DS3231 rtc;                             // Create RTC object
Servo servo;                                // Create Servo object

// Feeding time and RTC time
int feedingTime = 0;  // Adjusted during init
int rtcTime = 0; // rtc time in minutes (max = 1440)

// RTC editing steps
const int RTC_EDIT_STEPS[6] = {1, 5, 10, 15, 30, 60};  // edit steps (minutes) to choose from
int rtcStepID = 0;                              // index of minutes chosen from RTC_EDIT_STEPS

// Flags
// bool alreadyFed = false;       // Already fed at a certain time, don't feed again
char freshStart = 'y';            // Is the device starting for the first time?
bool isEditingTimeFeed = false;   // Is in editing feeding time?
bool isEditingTimeRtc = false;    // Is editing RTC time?
bool backlightOn = false;         // LCD backlight status (on/off)
bool showEditingText = true;      // is showing "[Editing]" or not.
bool buttonsEnabled = true;

// Button states
// NOTE: buttonIndex = BUTTON_PIN - 2
bool prevBtnState[5] = {
  HIGH,   // edit button
  false,  // no button assigned here
  HIGH,   // increase button
  HIGH,   // decrease button
  HIGH,   // feed button
};

// Button pins
const int BTN_PINS[4] = {
  BTN_EDIT,
  BTN_INCR,
  BTN_DECR,
  BTN_FEED,
};

#line 95 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void setup();
#line 118 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void loop();
#line 170 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void managePower();
#line 182 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void preSleep();
#line 190 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void postSleep();
#line 197 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void trySleeping();
#line 206 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
bool shouldNoBacklight();
#line 224 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void startLcd();
#line 242 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void initLcd();
#line 247 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void initRtc();
#line 284 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void setAlarmInterrupt();
#line 302 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void btnISR();
#line 307 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void alarmISR();
#line 318 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void enableButtons();
#line 322 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void disableButtons();
#line 326 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
bool isButtonsEnabled();
#line 330 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int btnPinToArrId(int btnId);
#line 334 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void printEvent(int i, int event);
#line 401 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int checkBtn(int btnId);
#line 429 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int checkBtnEdit(int event);
#line 460 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int checkBtnIncr(int event);
#line 476 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int checkBtnDecr(int event);
#line 492 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int checkBtnFeed(int event);
#line 527 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void startFeeding();
#line 556 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int rtcHour();
#line 560 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int rtcMinute();
#line 564 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void setRtcTimeVar(int hour, int minute);
#line 568 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
bool syncRtcTimeVar();
#line 587 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void applyNewTimeRTC();
#line 591 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int feedingTimeHour();
#line 598 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
int feedingTimeMinute();
#line 623 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void updateUI();
#line 631 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void displayRtcEdit();
#line 657 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void displayTime();
#line 695 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void blinkEditingText(int row);
#line 724 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void loadFromEEPROM();
#line 736 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void saveFeedingTime();
#line 95 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
void setup() {
  Serial.begin(9600);
  Serial.println("HI");

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
  initRtc();  // pinMode for ALARM_PIN is set here.
  setAlarmInterrupt();

  updateUI();
}

void loop() {
  managePower();

  // Track how many buttons pressed
  int btnEvents = 0;

  // Check button inputs
  for (int i = 0; i < 4; i++) {
    btnEvents += checkBtn(BTN_PINS[i]);
  }

  // Check if there are button events
  if (btnEvents > 0) {
    startLcd();
    updateUI();
  }

  // Update display if time changed (minute precision)
  if (! syncRtcTimeVar()) {
    updateUI();
  }

  // Blink "[Edit]" text when editing
  if (isEditingTimeRtc) {
    blinkEditingText(1);
  } else if (isEditingTimeFeed) {
    blinkEditingText(0);
  }

  ///////////////////////////////////////////////////////
  // Feeding is handled by RTC alarm
  // uncomment the code below if RTC alarm doesn't work
  ///////////////////////////////////////////////////////
  // 
  // checkFeedingTime();
  // 
  
  if (rtc.alarmFired(1)) {
    startFeeding();
    setAlarmInterrupt();
  }

  delay(25);  // Debounce buttons and avoid busy loop
}


////////////////////
// POWER MANAGEMENT
////////////////////
//
//

void managePower() {
  if (backlightOn) {
    if (shouldNoBacklight()) {
      lcd.noBacklight();
      backlightOn = false;
      disableButtons();
    }
  }

  trySleeping();
}

void preSleep() {
  attachInterrupt(digitalPinToInterrupt(BTN_EDIT), btnISR, LOW);
  lcd.noDisplay();

  Serial.println("Sleeping...");
  Serial.flush();
}

void postSleep() {
  detachInterrupt(digitalPinToInterrupt(BTN_EDIT));
  Serial.println("I've woken up!");

  startLcd();
}

void trySleeping() {
  unsigned long currentTime = millis();
  if (currentTime - lastWakeUp >= SLEEP_DELAY) {
    preSleep();
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    postSleep();
  }
}

bool shouldNoBacklight() {
  bool goNoBacklight = false;

  unsigned long currentTime = millis();
  if (currentTime - lastLcdBacklight >= BACKLIGHT_OFF_DELAY) {
    goNoBacklight = true;
  }

  return goNoBacklight;
}


//////////////////////////
// DEVICE CONTROL / INIT
//////////////////////////
//
//

void startLcd() {
  lastLcdBacklight = lastWakeUp = millis(); // keeping it awake

  if (backlightOn) {
    return;
  }

  lcd.display();    // Turn on LCD display
  lcd.backlight();  // Turn on LCD backlight

  backlightOn = true;

  // enable the buttons after 100 ms, 
  // so that the button event used to start the LCD isn't registered immediately 
  delay(1000);
  enableButtons();
}

void initLcd() {
  lcd.init();
  startLcd();
}

void initRtc() {
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  // attach interrupt
  pinMode(ALARM_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ALARM_PIN), alarmISR, FALLING);

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time! ");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //we don't need the 32K Pin, so disable it
  rtc.disable32K();

  // Disable and clear both alarms
  rtc.clearAlarm(2);
  rtc.disableAlarm(2);

  rtc.writeSqwPinMode(DS3231_OFF);  // Place SQW pin into alarm interrupt mode

  syncRtcTimeVar();
}


////////////////////
// TIME AND ALARMS
////////////////////
//
//

void setAlarmInterrupt() {
  // clear alarm
  rtc.clearAlarm(1);

  DateTime alarmTime = DateTime(2024, 5, 12, feedingTimeHour(), feedingTimeMinute(), 0);
  if (! rtc.setAlarm1(alarmTime, DS3231_A1_Hour)) {
    Serial.println("ERROR: Alarm wasn't set! ");
  } else {
    Serial.print("Alarm: ");
    Serial.print(alarmTime.hour());
    Serial.print(":");
    Serial.println(alarmTime.minute());
    Serial.flush();
  }
}



void btnISR() {
  // do nothing
}


void alarmISR() {
  // do nothing
}


//////////////////////////
// BUTTON INPUT HANDLERS
//////////////////////////
//
//

void enableButtons() {
  buttonsEnabled = true;
}

void disableButtons() {
  buttonsEnabled = false;
}

bool isButtonsEnabled() {
  return buttonsEnabled;
}

int btnPinToArrId(int btnId) {
  return btnId - 2;
}

void printEvent(int i, int event) {
  char eventStr[8];
  switch (event) {
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

  Serial.print("EVENT: btn ");
  Serial.print(i);
  Serial.print(" ");
  Serial.println(eventStr);
  Serial.flush();
}

/*
return values:
  0: no events
  1: pressed
  2: released
  3: hold
*/
int getButtonEvent(int btnId = BTN_EDIT) {
  int event = 0;
  int i = btnPinToArrId(btnId); // array ID of the corresponding button pin

  if (digitalRead(btnId) && prevBtnState[i] == LOW) {
    prevBtnState[i] = HIGH;    // button release event
    event = 2;

  } else if (! digitalRead(btnId)) {
    delay(25); // debounce

    if (! digitalRead(btnId)) {
      event = 1;                // button press event

      if (prevBtnState[i] == LOW) {
        unsigned long currentTime = millis();
        if (currentTime - btnHoldStart[i] >= BTN_HOLD_DELAY) {
          event = 3;            // button hold event
        }
      } else { 
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

int checkBtn(int btnId) {
  int event = getButtonEvent(btnId);

  if (! isButtonsEnabled()) {
    return event;
  }

  switch (btnId) {
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

int checkBtnEdit(int event) {
  int i = btnPinToArrId(BTN_EDIT);

  if (event == 3 && ! isEditingTimeFeed) { // button hold
    isEditingTimeRtc = true;
    keepBtnHold[i] = 1;
    syncRtcTimeVar();

  } else if (event == 2) { // button release
    Serial.println("EDIT BUTTON RELEASED");
    if (! keepBtnHold[i]) { // and not right after a "Hold" event
      if (isEditingTimeRtc) {
        isEditingTimeRtc = false;
        applyNewTimeRTC();

      } else if (isEditingTimeFeed) {
        isEditingTimeFeed = false;
        saveFeedingTime();  // also sets an alarm interrupt (RTC)

      } else {
        isEditingTimeFeed = true;
        Serial.println("editing feeding time...");
      }
    } else {
      keepBtnHold[i] = 0;
    }
  }
  
  return event;
}

int checkBtnIncr(int event) {
  if (event == 1 || event == 3) { // button pressed
    if (isEditingTimeFeed) {
      // Increase feeding time (wrap around)
      feedingTime++;
      feedingTime %= TIME_EDIT_MOD;
    } else if (isEditingTimeRtc) {
      // decrease rtc time (wrap around)
      rtcTime += RTC_EDIT_STEPS[rtcStepID];
      rtcTime %= MAX_RTC;
    }
  }

  return event;
}

int checkBtnDecr(int event) {
  if (event == 1 || event == 3) { // button pressed
    if (isEditingTimeFeed) {
      // decrease feeding time (wrap around)
      feedingTime -= 1 - TIME_EDIT_MOD;
      feedingTime %= TIME_EDIT_MOD;
    } else if (isEditingTimeRtc) {
      // decrease rtc time (wrap around)
      rtcTime -= RTC_EDIT_STEPS[rtcStepID] - MAX_RTC;
      rtcTime %= MAX_RTC;
    }
  }

  return event;
}

int checkBtnFeed(int event) {
  if (event == 1 || event == 3) {
    if (isEditingTimeRtc) { // change rtc editing step (minutes)
      rtcStepID++;
      rtcStepID %= N_RTC_EDIT_STEPS;
    } else if (! isEditingTimeFeed && event != 3) { // button pressed
      startFeeding();
    }
  }

  return event;
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

void startFeeding() {
  // case: feeding triggered by alarm, in the middle of editing feed time
  if (isEditingTimeFeed) {
    isEditingTimeFeed = false;
  } else if(isEditingTimeRtc) {
    isEditingTimeRtc = false;
  }

  lcd.clear();
  lcd.print("Feeding...");

  servo.attach(SERVO_PIN);          // Attach servo to pin
  for (int i = 0; i < 180; i++) {   // Rotate servo to dispense food
    servo.write(i);
    delay(5);
  }

  delay(FEED_DURATION);

  for (int i = 180; i > 0; i--) {   // Rotate servo to initial rotation
    servo.write(i);
    delay(5);
  }
  delay(1000);                // Make sure the servo finished rotating
  servo.detach();           // Detach servo to save power

  updateUI();
}

int rtcHour() {
  return rtcTime / 60;
}

int rtcMinute() {
  return rtcTime % 60;
}

void setRtcTimeVar(int hour, int minute) {
  rtcTime = hour * 60 + minute;
}

bool syncRtcTimeVar() {
  bool synced = true;

  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();

  // if currently editing rtc time var,
  // don't set the rtc time var from here
  if (! isEditingTimeRtc) {
    if (rtcMinute() != minute) { // not synced
      synced = false;
      setRtcTimeVar(hour, minute);
    }
  }

  return synced;
}

void applyNewTimeRTC() {
  rtc.adjust(DateTime(2024, 5, 12, rtcHour(), rtcMinute(), 0));
}

int feedingTimeHour() {
  // Convert from half-hour to hour
  int hour = feedingTime;
  hour /= HOUR_FACTOR;
  return hour;
}

int feedingTimeMinute() {
  // if it's odd, then the hour's not whole, aka an extra 30 minutes.
  int minute = feedingTime;
  minute %= HOUR_FACTOR;
  // Serial.print("Feeding minute (raw): ");
  // Serial.print(minute);
  // Serial.print(" (");
  // Serial.print(feedingTime);
  // Serial.println(")");
  minute *= TIME_EDIT_PRECISION;
  // Serial.print("Feeding minute: ");
  // Serial.println(minute);
  // Serial.flush();
  return minute;
}


/////////////
// DISPLAYS
/////////////
//
//


// display controller, choose which display to be shown by LCD
void updateUI() {
  if (isEditingTimeRtc) {
    displayRtcEdit();
  } else {
    displayTime();
  }
}

void displayRtcEdit() {
  lcd.clear();

  lcd.print("Step: ");
  lcd.print(RTC_EDIT_STEPS[rtcStepID]);
  lcd.print(" mins.");
  
  // set cursor to second row
  lcd.setCursor(0, 1);

  int hour = rtcHour();
  int minute = rtcMinute();
  
  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour);
  lcd.print(":");

  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute);
}

// display current time and feeding time in LCD
void displayTime() {
  // Display current time and feeding time on LCD
  lcd.clear();
  DateTime now = rtc.now();
  int hour = now.hour();
  int minute = now.minute();

  // Real Time
  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour);
  lcd.print(":");

  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute);

  // Feeding Time
  lcd.setCursor(0, 1);
  lcd.print("Feed Time: ");

  hour = feedingTimeHour();
  minute = feedingTimeMinute();
  if (hour < 10) {
    lcd.print("0");
  }
  lcd.print(hour);
  lcd.print(":");

  if (minute < 10) {
    lcd.print("0");
  }
  lcd.print(minute);
}

// Blink "[Editing]" text when editing feed time
void blinkEditingText(int row) {
  unsigned long currentTime = millis();
  if (currentTime - lastEditingBlinkTime >= BLINK_DELAY) {
    showEditingText = ! showEditingText;
    lastEditingBlinkTime = currentTime;

    // Move the cursor to the position of "[Editing]" text
    lcd.setCursor(10, row);

    if (showEditingText) {
      lcd.print("[Edit]");
    } else {
      lcd.print("      ");  // Clear it
    }

    // Set the cursor position back to origin
    lcd.setCursor(0, 0);
  }
}



//////////////////////////
// EEPROM READ AND WRITE
//////////////////////////
//
//

// Load variables from EEPROM (if saved previously)
void loadFromEEPROM() {
  freshStart = EEPROM.read(2);
  if (freshStart == 'n') {
    feedingTime = EEPROM.read(0);
    // alreadyFed = EEPROM.read(1);
  } else {
    freshStart = 'y';      // only once
    EEPROM.write(2, 'n');  // afterwards, load from this 'n' instead
  }
}

// save feeding time to EEPROM
void saveFeedingTime() {
  // Save feeding time to EEPROM
  EEPROM.write(0, feedingTime);
  setAlarmInterrupt();
}

