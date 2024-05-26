# Automatic Fish Feeder

The Automatic Fish Feeder is a handy embedded system project designed to simplify the task of feeding your aquatic pets. Developed by a collaborative team of enthusiasts, this project combines electronics, programming, and design to create a convenient solution for fish owners.

## Features

1. **Automatic Feeding**: The feeder is equipped with a Real-Time Clock (RTC) module that triggers feeding at predefined intervals, ensuring your fish are fed regularly even when you're not around.

2. **Instant Feeding**: With the press of a button, you can instantly dispense food to your fish outside of the scheduled feeding times.

3. **Time Editing**: Easily adjust the feeding schedule by entering edit mode, where you can increment or decrement the feeding time using dedicated buttons. Confirm changes by pressing the edit button again.

4. **RTC Time Sync**: Synchronize the device's internal clock with real-world time by toggling RTC edit mode. Hold down the edit button to enter RTC editing mode, then adjust the time as needed.

## Dependencies (Libraries)

1. **LiquidCrystal I2C**: [github repo](https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library)
  
2. **RTCLib by Adafruit**: [github repo](https://github.com/adafruit/RTClib)

3. **Servo**: [github repo](https://github.com/arduino-libraries/Servo)

4. **LowPower by LowPowerLab**: [github repo](https://github.com/LowPowerLab/LowPower)

## Installation

1. **Get Arduino IDE (or your preferred IDE/code editor)**: You can download the Arduino IDE from the [Arduino website](https://www.arduino.cc/en/software).

2. **Install Libraries**:
   - Open the Arduino IDE.
   - Go to **Sketch > Include Library > Manage Libraries...**.
   - Search for each of the libraries mentioned above and install them.

3. **Setup Hardware**:
   - Connect your Arduino board to your computer.
   - Connect the necessary components (RTC module, LCD display, servo motor) according to the schematic provided.

4. **Upload Code**:
   - Open the `feeding_scheduler.ino` file in the Arduino IDE.
   - Verify and upload the code to your Arduino board.

## How to Use

1. **Power On**: Connect the device to a power source.
2. **Initial Setup**: The feeder will initialize and display the current time on the LCD screen.
3. **Feeding Schedule**: By default, feeding occurs at a preset time. Adjust the feeding schedule as desired by entering edit mode (press edit button) and using the increase/decrease buttons.
4. **Manual Feeding**: Press the BLUE button to instantly dispense food to your fish outside of the scheduled feeding times.
5. **RTC Sync**: Hold down the edit button to enter RTC edit mode and synchronize the device's clock with real-world time.
6. **Maintenance**: Periodically check the device for any issues and refill the food reservoir as needed.

## Contributors

- **Shadam J'Verron**: Contributed to conceptualizing the project and provided valuable insights.
- **Yudhistira**: Developed the program logic and ensured seamless integration of hardware components.
- **Naufal Arsya Dinata**: Designed the electrical circuitry and PCB layout for the feeder.
- **Azriel Darmawan**: Designed the sleek and functional casing for the feeder, enhancing its aesthetic appeal.
- **Rifki Ramadhani**: Provided valuable contributions to the project.

## License

This project is licensed under the [MIT License](LICENSE).

---

**Disclaimer:** This project is intended for personal use and experimentation. Ensure proper care and monitoring of your fish at all times.
