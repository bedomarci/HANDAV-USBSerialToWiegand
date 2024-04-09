
#include <usbhub.h>
#include <Arduino.h>
#include "FTDIAsync.h"
#include "pgmstrings.h"

#include "wiegandOutput.h"
#include <TaskScheduler.h>
#include <SPI.h>

#define PIN_WIEGAND_D0 16
#define PIN_WIEGAND_D1 4
#define PIN_LED 2

#define PWMRANGE 255
#define BLINK_SPEED 100
#define BREATHE_STEP 64
#define BREATHE_DURATION 3000
#define BREATHE_BRIGHTNESS PWMRANGE / 8
#define BLINK_BOOT 5
#define BLINK_CONNECT 3
#define BLINK_TRANSMIT 2
#define USB_READ_INTERVAL 300
#define WIEGAND_BIT_LENGTH 64
//#define WIEGAND_BIT_LENGTH 26
#define DEVICE_DEBUG 1
#define SEPARATOR_LENGTH 57

void blinkLed();

void blink(int cnt);

void breatheLed();

void readUSB();

void sendWiegand(uint64_t data);

void processData(char *stringValue);

USB Usb;
FTDIAsync FtdiAsync;
FTDI Ftdi(&Usb, &FtdiAsync);
WiegandOut wiegandOut(PIN_WIEGAND_D0, PIN_WIEGAND_D1, DEVICE_DEBUG);
Scheduler ts;


bool isFTDIDeviceConnected = false;

uint32_t valueModulo = 0;


Task tBlinkLed(BLINK_SPEED, BLINK_BOOT * 2, &blinkLed, &ts);
Task tBreatheLed(BREATHE_DURATION / BREATHE_STEP, BREATHE_STEP, &breatheLed, &ts);
Task tUSBRead(USB_READ_INTERVAL, TASK_FOREVER, &readUSB, &ts, true);


void setup() {
    //PIN configuration
    pinMode(PIN_LED, OUTPUT);

    valueModulo = pow(10, NUM_OF_CROPPED_DIGITS);

    Serial.begin(115200);
    Serial.println(F("Start"));

    if (Usb.Init() == -1)
        Serial.println(F("OSC did not start."));

    blink(BLINK_BOOT);
}


void loop() {
    ts.execute();
}

void blink(int cnt) {
    tBreatheLed.disable();
    digitalWrite(PIN_LED, HIGH);
    tBlinkLed.setIterations(cnt * 2);
    tBlinkLed.restartDelayed(200);
}

void blinkLed() {
    int count = tBlinkLed.getRunCounter();
    digitalWrite(PIN_LED, (count + 1) % 2);
    if (tBlinkLed.isLastIteration()) {
        tBreatheLed.restartDelayed(BREATHE_DURATION);
    }
}

void breatheLed() {
    if (tBreatheLed.isFirstIteration()) {
        digitalWrite(PIN_LED, 0);
        tBlinkLed.disable();
    }
    int count = tBreatheLed.getRunCounter() % BREATHE_STEP;

    float step = abs(count - (BREATHE_STEP / 2));
    step = (BREATHE_STEP / 2) - abs(count - (BREATHE_STEP / 2));
    step *= ((PWMRANGE + 1) / (BREATHE_STEP / 2));
    step = PWMRANGE - map(step, 0, PWMRANGE, 0, BREATHE_BRIGHTNESS);

    analogWrite(PIN_LED, constrain(step, 0, 255));
    if (tBreatheLed.isLastIteration()) {
        tBreatheLed.restartDelayed(BREATHE_DURATION / 2);
    }
}

void readUSB() {
    Usb.Task();
    if (Usb.getUsbTaskState() == USB_STATE_RUNNING) {
        uint8_t rcode;
        uint8_t buf[64];
        for (uint8_t i = 0; i < 64; i++)
            buf[i] = 0;
        uint16_t rcvd = 64;
        rcode = Ftdi.RcvData(&rcvd, buf);

        if (rcode == hrSUCCESS) {
            if ((int) rcvd > 2) {
                processData((char *) (buf + 2));
            }
        }
    }

    if (Usb.getUsbTaskState() == USB_STATE_RUNNING && !isFTDIDeviceConnected) {
        isFTDIDeviceConnected = true;
        blink(BLINK_CONNECT);
        Serial.println(F("FTDI Device connected."));
    }
    if (Usb.getUsbTaskState() != USB_STATE_RUNNING) {
        isFTDIDeviceConnected = false;
    }
}

void processData(char *stringValue) {
    char buffer[64];
    uint64_t numericIdentifier = strtoull(stringValue, nullptr, 10);
//    uint64_t alteredNumericIdentifier = numericIdentifier % valueModulo;
    if (numericIdentifier > 0) {
        if (DEVICE_DEBUG) {
            memset(buffer, '-', sizeof(buffer));
            strcpy(buffer + SEPARATOR_LENGTH, "\n\0");
            Serial.print(buffer);
            memset(buffer, '\0', sizeof(buffer));
            sprintf(buffer, "Received string:               %s", stringValue);
            Serial.print(buffer);
            sprintf(buffer, "Converted numeric value:       %llu\t(0x%08X)", numericIdentifier, numericIdentifier);
            Serial.println(buffer);
//            sprintf(buffer, "Altered value:                 %llu\t(0x%08X)", alteredNumericIdentifier, alteredNumericIdentifier);
//            Serial.println(buffer);
            Serial.println("Method = 64 bit - no conversion");
            sprintf(buffer, "Sending wiegand %d bit binary: ", WIEGAND_BIT_LENGTH);
            Serial.print(buffer);
        }
//        sendWiegand(alteredNumericIdentifier);
        sendWiegand(numericIdentifier);
    }
}

void sendWiegand(uint64_t data) {
    bool useFacilityCode = false;
    wiegandOut.send(data, WIEGAND_BIT_LENGTH, useFacilityCode);
    blink(BLINK_TRANSMIT);

//    shell_print_pm(PSTR("Wiegand send: "));
//    uint64_t value = data;
//    const int nummberOfDigits = log10(value) + 1;
//    char stringNumber[nummberOfDigits + 1];
//    stringNumber[nummberOfDigits] = 0;
//    for (size_t i = nummberOfDigits; i--; value /= 10)
//        stringNumber[i] = '0' + (value % 10);
//    shell_println(stringNumber);

}