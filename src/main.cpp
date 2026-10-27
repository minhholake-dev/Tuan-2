
#include <Arduino.h>
#include "esp_sleep.h"
#include "driver/rtc_io.h"

#define BUTTON_PIN1 25
#define BUTTON_PIN2 26
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

const uint32_t debounceTime = 50;
const uint32_t countInterval = 100;

// MAX7219
void matrixWrite(byte address, byte data) {
    digitalWrite(CS, LOW);
    shiftOut(DIN, CLK, MSBFIRST, address);
    shiftOut(DIN, CLK, MSBFIRST, data);
    digitalWrite(CS, HIGH);
}

void clearMatrix() {
    for (int i = 1; i <= 8; i++) {
        matrixWrite(i, 0);
    }
}

void sadFace() {
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

    for (int i = 0; i < 8; i++) {
        matrixWrite(i + 1, face[i]);
    }
}

// Enter Deep Sleep
void enterDeepSleep() {
    Serial.println("Entering Deep Sleep");

    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);

    // Put MAX7219 into shutdown mode
    matrixWrite(0x0C, 0);

    // Button 1 will wake the ESP32 when LOW
    rtc_gpio_pullup_en(GPIO_NUM_25);
    rtc_gpio_pulldown_dis(GPIO_NUM_25);

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, 0);

    Serial.flush();
    esp_deep_sleep_start();
}

void setup() {
    Serial.begin(115200);

    // Restore GPIO25 after Deep Sleep wakeup
    rtc_gpio_deinit(GPIO_NUM_25);

    pinMode(BUTTON_PIN1, INPUT_PULLUP);
    pinMode(BUTTON_PIN2, INPUT_PULLUP);
    pinMode(SLEEP_PIN, INPUT_PULLUP);

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);

    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);

    pinMode(DIN, OUTPUT);
    pinMode(CLK, OUTPUT);
    pinMode(CS, OUTPUT);

    digitalWrite(CS, HIGH);

    matrixWrite(0x0F, 0);
    matrixWrite(0x09, 0);
    matrixWrite(0x0B, 7);
    matrixWrite(0x0A, 4);
    matrixWrite(0x0C, 1);

    clearMatrix();

    if (esp_sleep_get_wakeup_cause()
        == ESP_SLEEP_WAKEUP_EXT0) {
        Serial.println("Woken by Button 1");
    } else {
        Serial.println("ESP32 awake");
    }

    delay(3000);
}

void loop() {
    uint32_t now = millis();

    // BUTTON 2: DEBOUNCE
    bool reading = digitalRead(BUTTON_PIN2);

    if (reading != lastReading) {
        lastDebounce = now;
    }

    if (now - lastDebounce >= debounceTime) {
        if (reading != stableState) {
            stableState = reading;

            if (stableState == LOW) {
                count = 0;
                finished = false;
                lastCount = now;

                clearMatrix();
                digitalWrite(LED2, LOW);

                Serial.println("Hold for gift");
            }
        }
    }

    lastReading = reading;

    // BUTTON 2: COUNT
    if (stableState == LOW && !finished) {
        if (now - lastCount >= countInterval) {
            lastCount = now;
            count++;

            Serial.println(count);

            if (count == 50) {
                Serial.println("There are no gifts :(");

                sadFace();
                digitalWrite(LED2, HIGH);

                finished = true;
            }
        }
    }

    if (digitalRead(SLEEP_PIN) == LOW &&
    digitalRead(BUTTON_PIN1) == HIGH) {

    enterDeepSleep();
}

    // BUTTON 1: INTENTIONAL INTERFERENCE
    if (digitalRead(BUTTON_PIN1) == LOW) {
        Serial.println("Message received from Button 1");
        Serial.println("Waiting server response for 3 seconds...");

        digitalWrite(LED1, HIGH);

        delay(3000);

        digitalWrite(LED1, LOW);
    }
}
