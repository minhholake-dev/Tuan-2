
#include <Arduino.h>
#include "esp_sleep.h"

#define LED_PIN 2
#define ADC_PIN 34

RTC_DATA_ATTR int deepSleepCounter = 0;
int wakeupCounter = 0;

void sleepCounter() {
    deepSleepCounter++;

    digitalWrite(LED_PIN, LOW);

    Serial.println("Entering Deep Sleep");
    Serial.flush();

    esp_sleep_enable_timer_wakeup(
        10ULL * 1000000ULL
    );

    esp_deep_sleep_start();
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    pinMode(ADC_PIN, INPUT);

    esp_sleep_wakeup_cause_t reason =
        esp_sleep_get_wakeup_cause();

    if (reason == ESP_SLEEP_WAKEUP_TIMER) {
        wakeupCounter++;
        Serial.println("Wakeup caused by timer");
    } else {
        Serial.println("Not a timer wakeup");
    }

    Serial.print("Deep Sleep Counter: ");
    Serial.println(deepSleepCounter);

    Serial.print("Wakeup Counter: ");
    Serial.println(wakeupCounter);

    delay(2000);

    sleepCounter();
}

void loop() {
}
