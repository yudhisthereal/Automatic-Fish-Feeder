# 1 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
# 2 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino" 2
# 3 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino" 2
# 4 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino" 2
# 5 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino" 2
# 6 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino" 2
# 7 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino" 2

// Define feeding time editing constants






// Define LCD address and button pins






// Define servo pin and feeding duration



// Define intervals/timers (ms)





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
const int lcd_address = 0x27; // Replace with your I2C LCD address if different
LiquidCrystal_I2C lcd(lcd_address, 16, 2); // Initialize I2C LCD
RTC_DS3231 rtc; // Create RTC object
Servo servo; // Create Servo object

// Feeding time and RTC time
int feedingTime = 0; // Adjusted during init
int rtcTime = 0; // rtc time in minutes (max = 1440)

// RTC editing steps
const int RTC_EDIT_STEPS[6] = {1, 5, 10, 15, 30, 60}; // edit steps (minutes) to choose from
int rtcStepID = 0; // index of minutes chosen from RTC_EDIT_STEPS

// Flags
// bool alreadyFed = false;       // Already fed at a certain time, don't feed again
char freshStart = 'y'; // Is the device starting for the first time?
bool isEditingTimeFeed = false; // Is in editing feeding time?
bool isEditingTimeRtc = false; // Is editing RTC time?
bool backlightOn = false; // LCD backlight status (on/off)
bool showEditingText = true; // is showing "[Editing]" or not.
bool buttonsEnabled = true;

// Button states
// NOTE: buttonIndex = BUTTON_PIN - 2
bool prevBtnState[5] = {
  0x1, // edit button
  false, // no button assigned here
  0x1, // increase button
  0x1, // decrease button
  0x1, // feed button
};

// Button pins
const int BTN_PINS[4] = {
  2 /* Enter edit mode or confirm edit*/,
  4 /* Increase by 30 mins*/,
  5 /* Decrease by 30 mins*/,
  6 /* Instantly start feeding mechanism*/,
};

void setup() {
  Serial.begin(9600);
  Serial.println("HI");

  servo.attach(11);
  servo.write(0);
  delay(250);
  servo.detach();

  pinMode(2 /* Enter edit mode or confirm edit*/, 0x2);
  pinMode(4 /* Increase by 30 mins*/, 0x2);
  pinMode(5 /* Decrease by 30 mins*/, 0x2);
  pinMode(6 /* Instantly start feeding mechanism*/, 0x2);

  loadFromEEPROM();

  initLcd();
  initRtc(); // pinMode for ALARM_PIN is set here.
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

  delay(25); // Debounce buttons and avoid busy loop
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
  attachInterrupt(((2 /* Enter edit mode or confirm edit*/) == 2 ? 0 : ((2 /* Enter edit mode or confirm edit*/) == 3 ? 1 : -1)), btnISR, 0x0);
  lcd.noDisplay();

  Serial.println("Sleeping...");
  Serial.flush();
}

void postSleep() {
  detachInterrupt(((2 /* Enter edit mode or confirm edit*/) == 2 ? 0 : ((2 /* Enter edit mode or confirm edit*/) == 3 ? 1 : -1)));
  Serial.println("I've woken up!");

  startLcd();
}

void trySleeping() {
  unsigned long currentTime = millis();
  if (currentTime - lastWakeUp >= 5 * 60000 /* Time it takes before sleep: 5 mins*/) {
    preSleep();
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    postSleep();
  }
}

bool shouldNoBacklight() {
  bool goNoBacklight = false;

  unsigned long currentTime = millis();
  if (currentTime - lastLcdBacklight >= 15000 /* Time it takes before turning off LCD backlight: 10 secs*/) {
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

  lcd.display(); // Turn on LCD display
  lcd.backlight(); // Turn on LCD backlight

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
  pinMode(3 /* connected to RTC's SQW for alarm interrupt*/, 0x2);
  attachInterrupt(((3 /* connected to RTC's SQW for alarm interrupt*/) == 2 ? 0 : ((3 /* connected to RTC's SQW for alarm interrupt*/) == 3 ? 1 : -1)), alarmISR, 2);

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time! ");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime((reinterpret_cast<const __FlashStringHelper *>(
# 262 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino" 3
                       (__extension__({static const char __c[] __attribute__((__progmem__)) = ("May 26 2024"); &__c[0];}))
# 262 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
                       )), (reinterpret_cast<const __FlashStringHelper *>(
# 262 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino" 3
                                    (__extension__({static const char __c[] __attribute__((__progmem__)) = ("22:49:14"); &__c[0];}))
# 262 "/home/yudhis/Documents/Kuliah/Embed/proyek/Automatic Fish Feeder/main/main.ino"
                                    ))));
  }

  //we don't need the 32K Pin, so disable it
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
int getButtonEvent(int btnId = 2 /* Enter edit mode or confirm edit*/) {
  int event = 0;
  int i = btnPinToArrId(btnId); // array ID of the corresponding button pin

  if (digitalRead(btnId) && prevBtnState[i] == 0x0) {
    prevBtnState[i] = 0x1; // button release event
    event = 2;

  } else if (! digitalRead(btnId)) {
    delay(25); // debounce

    if (! digitalRead(btnId)) {
      event = 1; // button press event

      if (prevBtnState[i] == 0x0) {
        unsigned long currentTime = millis();
        if (currentTime - btnHoldStart[i] >= 700 /* Time it takes before triggering "button hold" event: 1 sec*/) {
          event = 3; // button hold event
        }
      } else {
        // started pressing button
        prevBtnState[i] = 0x0;
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
    case 2 /* Enter edit mode or confirm edit*/:
    checkBtnEdit(event);
    break;

    case 4 /* Increase by 30 mins*/:
    checkBtnIncr(event);
    break;

    case 5 /* Decrease by 30 mins*/:
    checkBtnDecr(event);
    break;

    case 6 /* Instantly start feeding mechanism*/:
    checkBtnFeed(event);
    break;
  }

  return event;
}

int checkBtnEdit(int event) {
  int i = btnPinToArrId(2 /* Enter edit mode or confirm edit*/);

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
        saveFeedingTime(); // also sets an alarm interrupt (RTC)

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
      feedingTime %= 24 * 60 / 15;
    } else if (isEditingTimeRtc) {
      // decrease rtc time (wrap around)
      rtcTime += RTC_EDIT_STEPS[rtcStepID];
      rtcTime %= 1440;
    }
  }

  return event;
}

int checkBtnDecr(int event) {
  if (event == 1 || event == 3) { // button pressed
    if (isEditingTimeFeed) {
      // decrease feeding time (wrap around)
      feedingTime -= 1 - 24 * 60 / 15;
      feedingTime %= 24 * 60 / 15;
    } else if (isEditingTimeRtc) {
      // decrease rtc time (wrap around)
      rtcTime -= RTC_EDIT_STEPS[rtcStepID] - 1440;
      rtcTime %= 1440;
    }
  }

  return event;
}

int checkBtnFeed(int event) {
  if (event == 1 || event == 3) {
    if (isEditingTimeRtc) { // change rtc editing step (minutes)
      rtcStepID++;
      rtcStepID %= 6;
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

  servo.attach(11); // Attach servo to pin
  for (int i = 0; i < 180; i++) { // Rotate servo to dispense food
    servo.write(i);
    delay(5);
  }

  delay(1000 /* 1 second in milliseconds*/);

  for (int i = 180; i > 0; i--) { // Rotate servo to initial rotation
    servo.write(i);
    delay(5);
  }
  delay(1000); // Make sure the servo finished rotating
  servo.detach(); // Detach servo to save power

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
  hour /= 60 / 15;
  return hour;
}

int feedingTimeMinute() {
  // if it's odd, then the hour's not whole, aka an extra 30 minutes.
  int minute = feedingTime;
  minute %= 60 / 15;
  // Serial.print("Feeding minute (raw): ");
  // Serial.print(minute);
  // Serial.print(" (");
  // Serial.print(feedingTime);
  // Serial.println(")");
  minute *= 15;
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
  if (currentTime - lastEditingBlinkTime >= 500 /* 0.5 secs*/) {
    showEditingText = ! showEditingText;
    lastEditingBlinkTime = currentTime;

    // Move the cursor to the position of "[Editing]" text
    lcd.setCursor(10, row);

    if (showEditingText) {
      lcd.print("[Edit]");
    } else {
      lcd.print("      "); // Clear it
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
    freshStart = 'y'; // only once
    EEPROM.write(2, 'n'); // afterwards, load from this 'n' instead
  }
}

// save feeding time to EEPROM
void saveFeedingTime() {
  // Save feeding time to EEPROM
  EEPROM.write(0, feedingTime);
  setAlarmInterrupt();
}
