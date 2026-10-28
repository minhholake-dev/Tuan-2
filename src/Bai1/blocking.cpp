#define LED1 2
#define ADC 34
void setup(){
    Serial.begin(115200);
    pinMode(LED1,OUTPUT);
    pinMode(ADC,INPUT);
}
void loop() {
    digitalWrite(LED1,HIGH);
    delay(500);
    digitalWrite(LED1,LOW);
    delay(500);

    int raw = analogRead(ADC);
    Serial.print("ADC: ");
    Serial.println(raw);
    delay(2000);

    Serial.println("heartbeat");
    delay(5000);
}