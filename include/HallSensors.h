/*
  Author: Tobias Grundtvig
*/

#ifndef HallSensors_h
#define HallSensors_h


#include <Arduino.h>

class HallSensors
{
public:
    static void begin(const uint8_t hallSensorAPin, const uint8_t hallSensorBPin);
    static int getAndResetDeltaCount();
    static unsigned long getMagnetTime();
    static uint8_t getDirection();
    static void setDirection(uint8_t dir);
private:
    //Hallsensor interrupt callbacks
    static void IRAM_ATTR _onHallSensorA();
    static void IRAM_ATTR _onHallSensorB();

    static volatile uint8_t _hallSensorAPin;
    static volatile uint8_t _hallSensorBPin;

    static volatile uint8_t _curDirection;
    //Keeping track of position
    static volatile uint8_t _curPos;
    static volatile uint8_t _lastPos;
    static volatile uint8_t _skipMagnets;

    //Keeping track of magnets
    static volatile bool _onMagnet;
    static volatile unsigned long _enterMagnetTime;
    static volatile unsigned long _onMagnetTime;
};


#endif