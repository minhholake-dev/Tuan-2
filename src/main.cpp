
#include <Arduino.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"

#define BUTTON1 25
#define BUTTON2 26
#define SLEEP_PIN 33
#define LED1 27
#define LED2 32
#define DIN 23
#define CLK 18
#define CS 5

int count = 0;
bool lastReading = HIGH;
bool stableState = HIGH;
bool finished = false;

uint32_t lastDebounce = 0;
uint32_t lastCount = 0;

hw_timer_t *timer;
volatile bool sleepRequested = false;

// Hardware timer interrupt
void ARDUINO_ISR_ATTR onTimer() {
    sleepRequested = true;
}

// Reset 5-minute inactivity timer
void resetTimer() {
    timerWrite(timer, 0);
    sleepRequested = false;
}

// Send command to MAX7219
void matrixWrite(byte address, byte data) {
    digitalWrite(CS, LOW);
    shiftOut(DIN, CLK, MSBFIRST, address);
    shiftOut(DIN, CLK, MSBFIRST, data);
    digitalWrite(CS, HIGH);
}

// Display or clear sad face
void showFace(bool show) {
    byte face[8] = {
        B00000000,
        B00100100,
        B00100100,
        B00000000,
        B00011000,
        B00100100,
        B01000010,
        B00000000
    };

    for (int i = 0; i < 8; i++)
        matrixWrite(i + 1, show ? face[i] : 0);
}

// Enter Deep Sleep
void enterDeepSleep() {
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    matrixWrite(0x0C, 0);

    rtc_gpio_pullup_en(GPIO_NUM_25);
    rtc_gpio_pulldown_dis(GPIO_NUM_25);

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, 0);

    Serial.println("Deep Sleep");
    Serial.flush();
    esp_deep_sleep_start();
}

void setup() {
    Serial.begin(115200);

    rtc_gpio_deinit(GPIO_NUM_25);

    pinMode(BUTTON1, INPUT_PULLUP);
    pinMode(BUTTON2, INPUT_PULLUP);
    pinMode(SLEEP_PIN, INPUT_PULLUP);

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);

    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);

    pinMode(DIN, OUTPUT);
    pinMode(CLK, OUTPUT);
    pinMode(CS, OUTPUT);
    digitalWrite(CS, HIGH);

    // Initialize LED matrix
    matrixWrite(0x0F, 0);
    matrixWrite(0x09, 0);
    matrixWrite(0x0B, 7);
    matrixWrite(0x0A, 4);
    matrixWrite(0x0C, 1);
    showFace(false);

    // Hardware timer: 5 minutes
    timer = timerBegin(1000000);
    timerAttachInterrupt(timer, &onTimer);
    timerAlarm(timer, 300000000ULL, true, 0);

    Serial.println("ESP32 awake");
}

void loop() {
    uint32_t now = millis();
    bool reading = digitalRead(BUTTON2);

    // Debounce Button 2
    if (reading != lastReading)
        lastDebounce = now;

    if (now - lastDebounce >= 50 &&
        reading != stableState) {

        stableState = reading;
        resetTimer();

        if (stableState == LOW) {
            count = 0;
            finished = false;
            lastCount = now;
            showFace(false);
            digitalWrite(LED2, LOW);
            Serial.println("Hold for gift");
        }
    }

    lastReading = reading;

    // Count while Button 2 is held
    if (stableState == LOW && !finished &&
        now - lastCount >= 100) {

        lastCount = now;
        resetTimer();
        count++;

        Serial.println(count);

        if (count == 50) {
            Serial.println("There are no gifts :(");
            showFace(true);
            digitalWrite(LED2, HIGH);
            finished = true;
        }
    }

    // Button 1: intentional interference
    if (digitalRead(BUTTON1) == LOW) {
        resetTimer();

        Serial.println("Waiting server response...");
        digitalWrite(LED1, HIGH);
        delay(3000);
        digitalWrite(LED1, LOW);
    }

    // Deep Sleep after inactivity or GPIO33 LOW
    if ((sleepRequested ||
         digitalRead(SLEEP_PIN) == LOW) &&
        digitalRead(BUTTON1) == HIGH) {

        enterDeepSleep();
    }
}
