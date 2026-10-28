#include <Arduino.h>
#define ADC 34
#define LED1 2

uint32_t loopTick =0;
uint32_t timerMS = 5000;
uint32_t timer = millis();

void setup(){
    Serial.begin(115200);
    pinMode(LED1,OUTPUT);
    pinMode(ADC,INPUT);
}
void loop() {
    
}

void updateLoopTimer(){
    if(timer - loopTick > timerMS){
        loopTick += timerMS;
        Serial.println("To slow, you missed a beat");
        } else if(timer - loopTick  timerMS){
            loopTick += timerMS;
        Serial.println("Too fast, better slow down");
        } else if (timer - loopTick == timerMS){ {
             loopTick += timerMS;
        Serial.println("Perfect, you got it!");
        }
    

}