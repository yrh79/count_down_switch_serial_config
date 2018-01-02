#include "Timer.h"
#include <EEPROM.h>

Timer t;

int ledEvent;

typedef struct {
  int day;
  int hour;
  int minute;
  int second;
} time_t;

time_t countdown = {0, 0, 0, 0};

//GPIO 2 is powerPin
const int powerPin = 2;

//GPIO 13 is LED pin
const int ledPin = 13;

//GPIO pin for configuration
const int num_config_bits = 6;
const int refPin   = 3;
const int bit1_pin = 4;
const int bit2_pin = 5;
const int bit3_pin = 6;
const int bit4_pin = 7;
const int bit5_pin = 8;
const int bit6_pin = 9;

int configuration = 0;

//power switch works for 5 seconds
const int switch_seconds = 5;

//Serial config stuffs
int recording = 0;
int idx = 0;
const int bufSize = 128;
char serialBuf[bufSize] = {0};

//EEPROM
#define EEPROM_SIZE         1024 //EEPROM size of arduino nano is 1k
#define CONFIG_ADDR         0
#define CONFIG_LEN          sizeof(time_t)
#define INIT_ADDR           CONFIG_ADDR+CONFIG_LEN
#define INIT_LEN          sizeof(time_t)

typedef enum {
  INIT = 0,
  CYCLE = 1
} config_type_t;

void setup()
{
  //initialize countdown to 11 hours at the first boot
  countdown = getConfig(INIT);

  Serial.begin(115200);

  //switch on powerPin to indicate the device is working
  //then switch off to indicate the devides is in action
  //start up event would switch on again in 5 seconds
  pinMode(powerPin, OUTPUT);

  switchOn();
  delay(1000);
  switchOff();

  int tickEvent = t.every(1000, doCountDown);

  pinMode(ledPin, OUTPUT);
  pinMode(refPin, OUTPUT);
  ledEvent = t.oscillate(ledPin, 50, HIGH);

  int afterEvent = t.after(switch_seconds * 1000, slowDownLed_switchOn);

  //set reference Pin to LOW
  digitalWrite(refPin, LOW);

  //set pull up resisters for configuration pins
  pinMode(bit1_pin, OUTPUT);
  pinMode(bit2_pin, OUTPUT);
  pinMode(bit3_pin, OUTPUT);
  pinMode(bit4_pin, OUTPUT);
  pinMode(bit5_pin, OUTPUT);
  pinMode(bit6_pin, OUTPUT);

  digitalWrite(bit1_pin, HIGH);
  digitalWrite(bit2_pin, HIGH);
  digitalWrite(bit3_pin, HIGH);
  digitalWrite(bit4_pin, HIGH);
  digitalWrite(bit5_pin, HIGH);
  digitalWrite(bit6_pin, HIGH);

  pinMode(bit1_pin, INPUT);
  pinMode(bit2_pin, INPUT);
  pinMode(bit3_pin, INPUT);
  pinMode(bit4_pin, INPUT);
  pinMode(bit5_pin, INPUT);
  pinMode(bit6_pin, INPUT);

}

void loop()
{
  t.update();

  while (Serial.available()) {
    char c = Serial.read();

    switch (c) {
      case '\n':
        break;
      case '\r':
        parseSerialConfig();
        memset(serialBuf, 0, bufSize);
        idx = 0;
        break;
      case '$':
        recording = 1;
        break;
      default:
        if (recording && (idx < bufSize)) {
          serialBuf[idx] = c;
          idx ++;
        }
        break;
    }
  }
}

void parseSerialConfig()
{
  time_t *c, configData = {0};
  char msg[64] = {0};
  c = &configData;

  int day = 0;
  int hour = 0;
  int minute = 0;
  int second = 0;

  if (strncmp(serialBuf, "cycle ", 6) == 0) {
    int n = sscanf(serialBuf + 6, "{%d, %d, %d, %d}", &day, &hour, &minute, &second);
    if (n == 4) { //success
      configData = {day, hour, minute, second};
      saveConfig(CYCLE, c);
      sprintf(msg, "New cycle config: {%d, %d, %d, %d}", day, hour, minute, second);
      Serial.println(msg);
    }
  }

  if (strncmp(serialBuf, "init ", 5) == 0) {
    int n = sscanf(serialBuf + 5, "{%d, %d, %d, %d}", &day, &hour, &minute, &second);
    if (n == 4) { //success
      configData = {day, hour, minute, second};
      saveConfig(INIT, c);
      sprintf(msg, "New init config: {%d, %d, %d, %d}", day, hour, minute, second);
      Serial.println(msg);
    }
  }

  if (strncmp(serialBuf, "get", 3) == 0) {
    *c = getConfig(INIT);
    sprintf(msg, "Init config: {%d, %d, %d, %d}", c->day, c->hour, c->minute, c->second);
    Serial.println(msg);
    memset(msg, 0, 64);
    *c = getConfig(CYCLE);
    sprintf(msg, "Cycle config: {%d, %d, %d, %d}", c->day, c->hour, c->minute, c->second);
    Serial.println(msg);
  }
}

void read_eeprom(char *dist, int addr, int len)
{
  memset (dist, 0, len);
  for (int i = 0; i < len; i++)
  {
    dist[i] = EEPROM.read(i + addr);
  }
}

void write_eeprom(const char *src, int addr, int len)
{
  for (int i = 0; i < len; i++)
  {
    char c = *(src + i);
    EEPROM.write(addr + i, c);
  }
}

time_t getConfig(config_type_t type)
{
  time_t configData = {0};
  switch (type) {
    case INIT:
      read_eeprom((char*)&configData, INIT_ADDR, INIT_LEN);
      break;
    case CYCLE:
      read_eeprom((char*)&configData, CONFIG_ADDR, CONFIG_LEN);
      break;
    default:
      break;
  }
  return configData;
}

void saveConfig(config_type_t type, time_t *configData)
{
  switch (type) {
    case INIT:
      write_eeprom((char *)configData, INIT_ADDR, INIT_LEN);
      break;
    case CYCLE:
      write_eeprom((char *)configData, CONFIG_ADDR, CONFIG_LEN);
      break;
    default:
      break;
  }
}

void doCountDown()
{
  time_t *c = &countdown;
  char buf[64] = {0};
  sprintf(buf, "TTR: %d D %d:%d:%d", c->day, c->hour, c->minute, c->second);
  Serial.println(buf);

  if (countdown.second > 0) {
    countdown.second --;
  } else if (countdown.minute > 0) {
    countdown.minute --;
    countdown.second = 59;
  } else if (countdown.hour > 0) {
    countdown.hour --;
    countdown.minute = 59;
    countdown.second = 59;
  } else if (countdown.day > 0) {
    countdown.day --;
    countdown.hour = 23;
    countdown.minute = 59;
    countdown.second = 59;
  } else {
    //count down is completed, do poweroff action for the consumer

    //flash led to indicate action
    ledEvent = t.oscillate(ledPin, 50, HIGH);

    switchOff();

    //read hardware setup to determine cycle here:
    readConfig();

    //slow down led to indicate normal operation after 5 seconds and switch On
    t.after(switch_seconds * 1000, slowDownLed_switchOn);

  }
}

void slowDownLed_switchOn()
{
  t.stop(ledEvent);
  t.oscillate(ledPin, 500, HIGH);

  switchOn();
}

void switchOff()
{
  Serial.println("switch Off!!");
  digitalWrite(powerPin, LOW);
}

void switchOn()
{
  Serial.println("switch On!");
  digitalWrite(powerPin, HIGH);
}

bool is_quick_config(int configuration)
{
  Serial.print("Config Pin Setup: (b)");
  Serial.println(configuration, BIN);
  // this routine check whether there is more
  // than 1 'true' bits there in the configuration
  // return false if more than 1 'true' bits there
  bool found = false;
  for (int i = 0; i < num_config_bits; i++) {
    if ((configuration >> i)  & 0x1) {
      if (found) return false;
      found = true;
    }
  }

  return true;
}

void readConfig()
{
  configuration = 0;

  if (digitalRead(bit1_pin) == 0) configuration |= 0x1;
  if (digitalRead(bit2_pin) == 0) configuration |= 0x1 << 1;
  if (digitalRead(bit3_pin) == 0) configuration |= 0x1 << 2;
  if (digitalRead(bit4_pin) == 0) configuration |= 0x1 << 3;
  if (digitalRead(bit5_pin) == 0) configuration |= 0x1 << 4;
  if (digitalRead(bit6_pin) == 0) configuration |= 0x1 << 5;

  if (is_quick_config(configuration)) {
    //quick config:
    if (configuration == 0) { //no hard-wired config is done, so use the config in eeprom
      countdown = getConfig(CYCLE);
    } else if (configuration & 0x1) {
      countdown = {0, 1, 0, 0};
    } else if (configuration & 0x1 << 1) {
      countdown = {0, 2, 0, 0};
    } else if (configuration & 0x1 << 2) {
      countdown = {0, 4, 0, 0};
    } else if (configuration & 0x1 << 3) {
      countdown = {0, 6, 0, 0};
    } else if (configuration & 0x1 << 4) {
      countdown = {0, 12, 0, 0};
    } else if (configuration & 0x1 << 5) {
      countdown = {2, 0, 0, 0};
    }
  }
  else {
    //if it is not quick config, one bit equals to 30 minutes
    countdown.day    = (configuration / 24) / 2;
    countdown.hour   = (configuration / 2) % 24;
    countdown.minute = (configuration % 2) * 30;
    countdown.second = 0;
  }
}



