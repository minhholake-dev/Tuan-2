#define LED1 2
#define LED2 4
#define ADC 34
uint32_t timer = millis();
volatile long lastMillis1 = 0;
volatile long lastMillis2 = 0;
volatile long lastWrite = 0;
volatile long lastHeartbeat = 0;


void setup(){
    Serial.begin(115200);
    pinMode(LED1,OUTPUT);
    pinMode(LED2,OUTPUT);
    pinMode(ADC,INPUT);
}


void loop() {
if (timer - lastMillis1 >= 500) {
        lastMillis = timer;
        int raw = analogRead(ADC);
        if (raw > 2000) {
           digitalWrite(LED1,HIGH);
        } else {
            digitalWrite(LED1,LOW);
        }
    }
if (timer - lastMillis2 >= 800) {
        lastMillis = timer;
        int raw = analogRead(ADC);
        if (raw < 2000) {
            digitalWrite(LED2,HIGH);
        } else {
            digitalWrite(LED2,LOW);
        }
    }    
    if (timer - lastWrite >= 2000) {
        lastWrite = timer;
        int raw = analogRead(ADC);
        Serial.print("ADC: ");
        Serial.println(raw);
    }    
   

    if (timer - lastHeartbeat >= 5000) {
        lastHeartbeat = timer;
        Serial.println("heartbeat");
    }
}