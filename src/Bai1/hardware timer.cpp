
#include <Arduino.h>
#include <esp_timer.h>
#include <math.h>

#define LED1 2
#define ADC_PIN 34
#define BUTTON 25

const uint32_t PERIOD_MS = 1000;
const int64_t PERIOD_US = 1000000;

// ESP hardware-backed timer
esp_timer_handle_t hwtimer = nullptr;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t espTicks = 0;
int64_t espTimestamp = 0;

// millis timer
uint32_t lastMillisTick = 0;
int64_t lastMillisTime = 0;
int64_t lastEspTime = 0;

uint32_t receivedEspTicks = 0;
bool millisReady = false;
bool espReady = false;

int64_t millisTimestamp = 0;
int64_t pendingEspTime = 0;

// Store statistics using Welford's method
struct Statistics {
    uint32_t n = 0;
    double mean = 0;
    double M2 = 0;

    void add(double x) {
        n++;
        double d = x - mean;
        mean += d / n;
        M2 += d * (x - mean);
    }

    double sd() {
        if (n < 2) return 0;
        return sqrt(M2 / (n - 1));
    }
};

Statistics millisStats;
Statistics espStats;
Statistics diffStats;

// ESP timer callback
void onTimer(void *arg) {
    int64_t now = esp_timer_get_time();

    portENTER_CRITICAL(&timerMux);
    espTimestamp = now;
    espTicks++;
    portEXIT_CRITICAL(&timerMux);
}

// Initialize ESP timer
void khoiTaoTimer() {
    esp_timer_create_args_t args = {};

    args.callback = &onTimer;
    args.dispatch_method = ESP_TIMER_TASK;
    args.name = "hwtimer";

    ESP_ERROR_CHECK(esp_timer_create(&args, &hwtimer));

    ESP_ERROR_CHECK(
        esp_timer_start_periodic(hwtimer, PERIOD_US)
    );
}

// Print statistics when button is pressed
void printStatistics() {
    Serial.println("\n===== RESULTS =====");

    Serial.printf("Samples: %lu\n",
                  (unsigned long)millisStats.n);

    if (millisStats.n == 0) {
        Serial.println("No samples yet");
        return;
    }

    Serial.printf(
        "millis:    mean = %.2f us, SD = %.2f us\n",
        millisStats.mean, millisStats.sd()
    );

    Serial.printf(
        "esp_timer: mean = %.2f us, SD = %.2f us\n",
        espStats.mean, espStats.sd()
    );

    Serial.printf(
        "Difference: mean = %.2f us, SD = %.2f us\n",
        diffStats.mean, diffStats.sd()
    );

    Serial.println("===================\n");
}

void setup() {
    Serial.begin(115200);

    pinMode(LED1, OUTPUT);
    pinMode(ADC_PIN, INPUT);
    pinMode(BUTTON, INPUT_PULLUP);

    lastMillisTick = millis();

    int64_t start = esp_timer_get_time();
    lastMillisTime = start;
    lastEspTime = start;

    khoiTaoTimer();

    Serial.println("Timer comparison started");
}

void loop() {

    // 1. millis() timer
    if (millis() - lastMillisTick >= PERIOD_MS) {
        lastMillisTick += PERIOD_MS;
        millisTimestamp = esp_timer_get_time();
        millisReady = true;
    }

    // 2. Check ESP timer callback
    uint32_t ticks;
    int64_t timestamp;

    portENTER_CRITICAL(&timerMux);
    ticks = espTicks;
    timestamp = espTimestamp;
    portEXIT_CRITICAL(&timerMux);

    if (ticks != receivedEspTicks) {
        receivedEspTicks = ticks;
        pendingEspTime = timestamp;
        espReady = true;
    }

    // 3. Compare when both timers have triggered
    if (millisReady && espReady) {
        millisReady = false;
        espReady = false;

        int64_t dtMillis =
            millisTimestamp - lastMillisTime;

        int64_t dtEsp =
            pendingEspTime - lastEspTime;

        int64_t difference = dtMillis - dtEsp;

        lastMillisTime = millisTimestamp;
        lastEspTime = pendingEspTime;

        millisStats.add((double)dtMillis);
        espStats.add((double)dtEsp);
        diffStats.add((double)difference);

        Serial.printf(
            "\nSample %lu\n",
            (unsigned long)millisStats.n
        );

        Serial.printf(
            "millis: %lld us | esp: %lld us\n",
            (long long)dtMillis,
            (long long)dtEsp
        );

        Serial.printf(
            "Difference: %lld us\n",
            (long long)difference
        );

        Serial.printf(
            "Average: millis = %.2f, esp = %.2f us\n",
            millisStats.mean,
            espStats.mean
        );

        // Your original LED and ADC
        digitalWrite(LED1, !digitalRead(LED1));

        int raw = analogRead(ADC_PIN);
        Serial.printf("ADC: %d\n", raw);
    }

    // 4. Button debounce
    static bool lastReading = HIGH;
    static bool stableState = HIGH;
    static uint32_t lastDebounce = 0;

    bool reading = digitalRead(BUTTON);
    uint32_t now = millis();

    if (reading != lastReading) {
        lastDebounce = now;
    }

    if (now - lastDebounce >= 50 &&
        reading != stableState) {

        stableState = reading;

        if (stableState == LOW) {
            printStatistics();
        }
    }

    lastReading = reading;
}
