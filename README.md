# count_down_switch
A count down switch implemented with Arduino Uno

### Prerequisites
* This project depends on the Timer library from https://github.com/JChristensen/Timer, please clone and place it into the libraries folder of your arduino IDE.

## Description
This is an implement of the count down switch with Arduino Uno.

This project is an extended version of the https://github.com/yrh79/count_down_switch project, which has the capability of being configured through serial port. Another project that configures the device through a serial line would be implemented later with wxPython and the url to that would be included here later on.

People may have the needs to restart a WIFI AP everyday just to make sure it works with no problem, but maybe that AP does not have the auto-reboot functionalities. Power cycle the AP may be a good way to keep it works perperly.

This count down switch could switch off then back on the power suply of the power consumer (WIFI AP) on a hardware pre-configured basic. A user could use a set of jumppers (GPIO 4 through 9) to quickly setup a cycle value:


### Quick setup:
* pin 4 - 1 hour,
* pin 5 - 2 hours,
* pin 6 - 4 hours,
* pin 7 - 6 hours,
* pin 8 - 12 hours,
* pin 9 - 48 hours,

### Default setup:
All-open - Read configuration from EEPROM.

### Bit-setup:
1 bit equals to 30 minutes, e.g.:
* 0x11 = 1.5 hours (jumpper set on pin 4 and 5)
* 0x101 - 2.5 hours (jumpper set on pin 4 and 6)

### Initial switching delay:
The switch would do a switch off-on action 11 hours after the initial power on - this is to allow people setup the power supply (e.g.) on 2pm in the afternoon, then the first (and every power cycle) action would happen at 1am in the mid-night.

### Serial Configuration
* To configure the initial delay value, use the below command:
```
$init {0, 11, 0, 0}
This would set the initial delay value to 11 hours.
```

* To configure the interval value, use the below command:
```
$cycle {0, 1, 0, 0}
This would set the interval value to 1 hour.
```

### Basic Usage

* Connect Pin 2 to a relay's input and power it properly.
* Clone https://github.com/JChristensen/Timer and place it into the libraries folder of your arduino IDE.
* Flash the Arduino Uno as usual then enjoy. :)

This project is tested under arduno 1.8.5. Other version of arduino IDE may also work.

