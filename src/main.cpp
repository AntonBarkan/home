#include <Arduino.h>
#include <Common.h>

#define LOG_OUT 1
#define FFT_N 128
#include <FFT.h>

#include <Rf.h>

#include "IRremote.h"
#include <SoftwareSerial.h>

#define JVCPower 0xC5E8
#define TVPower 0x4FB4AB5

// fast furie configuration

#define FREQ_LOW_FFT 2
#define FREQ_MIDDLE_FFT 30
#define FREQ_HIGH_FFT 60

#define FREQ_LOW_LEVEL 38
#define FREQ_MIDDLE_LEVEL 18
#define FREQ_HIGH_LEVEL 15

#define MIC_PIN A0

#define RED_LED A1
#define BLUE_LED A2
#define GREEN_LED A3
#define YELLOW_LED A4

#define RADIO_RX_PIN 2
// PIN 3 used for IR leds
#define RECV_PIN 4
#define LIGHT_SHOW_BUTTON_PIN 5
#define BT_SERIAL_RX 8
#define BT_SERIAL_TX 9
#define RADIO_TX_PIN 10
#define LED_PIN 11
#define BUTTON_PIN 12

#define ReciverPowerComandCode "0000000001"
#define TVPowerComandCode "0000000002"
#define TVAndReciverPowerComandCode "0000000003"
#define VolumeUp "0000000004"
#define VolumeDown "0000000005"

SoftwareSerial BTSerial(BT_SERIAL_RX, BT_SERIAL_TX);

IRsend irsend;
IRrecv irrecv(RECV_PIN);

decode_results results;
Rf rf("01", RADIO_TX_PIN, RADIO_RX_PIN);

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  BTSerial.begin(9600);
  irrecv.enableIRIn();

  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
}

bool isEqualsCommands(const char *s0, const char *s1) {
  for (int i = 0; i < COMMAND_SIZE; i++) {
    if (s0[i] != s1[i]) {
      return false;
    }
  }
  return true;
}

const char *readButton() {
  return digitalRead(BUTTON_PIN) == HIGH ? 0 : ReciverPowerComandCode;
}

const char *readRf() { return rf.readCommand(); }

const char *readBTSerial() {
  char *command = 0;
  if (BTSerial.available()) {
    command = new char[COMMAND_SIZE];
    for (int i = 0; i < COMMAND_SIZE; i++) {
      command[i] = BTSerial.read();
    }
  }
  return command;
}

const char *readIR() {
  const char *value = 0;
  if (irrecv.decode(&results)) {
    Serial.println(results.value);
    if (results.value == 0xFF6897) {
      value = ReciverPowerComandCode;
    } else if (results.value == 0xFF9867) {
      value = TVPowerComandCode;
    } else if (results.value == 0xFF02FD) {
      value = TVAndReciverPowerComandCode;
    }
    irrecv.resume();
  }
  return value;
}

void sendNEC(long command) {
  for (int i = 0; i < 15; i++) {
    Serial.println("tv");
    irsend.sendNEC(command, 32);
    delay(20);
  }
  delay(480);
}

void sendJVC(long command) {
  for (int i = 0; i < 2; i++) {
    Serial.println("JVC");
    irsend.sendJVC(command, 16, 0);
    delay(10);
  }
  delay(490);
}

const char *readSources() {
  const char *value = readIR();
  if (!value) {
    value = readButton();
  }
  if (!value) {
    value = readBTSerial();
  }
  if (!value) {
    value = readRf();
  }
  return value;
}

void recive() {
  const char *value = readSources();

  if (value != 0) {
    digitalWrite(LED_PIN, HIGH);
    if (isEqualsCommands(value, ReciverPowerComandCode)) {
      sendJVC(JVCPower);
    } else if (isEqualsCommands(value, TVPowerComandCode)) {
      sendNEC(TVPower);
    } else if (isEqualsCommands(value, TVAndReciverPowerComandCode)) {
      sendJVC(JVCPower);
      sendNEC(TVPower);
    }
    digitalWrite(LED_PIN, LOW);
    irrecv.enableIRIn();
  }
}

void flipLeds() {
  for (int i = 0; i < FFT_N; i++) {
    int sample = analogRead(MIC_PIN) - 511;
    if (sample < 5 && sample > -5) {
      sample = 0;
    }
    fft_input[i++] = sample;
    fft_input[i] = 0;
  }
  if (fft_log_out[FREQ_LOW_FFT] > FREQ_LOW_LEVEL) {
    digitalWrite(RED_LED, HIGH);
  } else {
    digitalWrite(RED_LED, LOW);
  }
  if (fft_log_out[FREQ_MIDDLE_FFT] > FREQ_MIDDLE_LEVEL) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BLUE_LED, LOW);
  } else {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);
  }
  if (fft_log_out[FREQ_HIGH_FFT] > FREQ_HIGH_LEVEL) {
    digitalWrite(YELLOW_LED, HIGH);
  } else {
    digitalWrite(YELLOW_LED, LOW);
  }
}

void loop() {
  if (digitalRead(LIGHT_SHOW_BUTTON_PIN) == HIGH) {
    flipLeds();
  } else {
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BLUE_LED, LOW);
  }
  recive();
}
