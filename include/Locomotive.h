/*
  Author: Tobias Grundtvig
*/

#ifndef Locomotive_h
#define Locomotive_h

#include <Arduino.h>
#include <TrainCtrl.h>
#include <DeviceUDPClient.h>
#include <LEDPattern.h>

#define CMD_IDENTIFY 1000
#define CMD_MOVE_TRAIN_FWD 1001
#define CMD_MOVE_TRAIN_BCK 1002
#define CMD_EMERGENCY_BRAKE 1003

#define CMD_DISTANCE_TO_GOAL 2000

class Locomotive : public DeviceUDPClient, TrainListener
{
public:
    Locomotive(uint8_t powerPin, uint8_t dirPin, uint8_t hallSensorPinA, uint8_t hallSensorPinB);
    void update(unsigned long curTime);
    void onDistanceToGoalChange(uint32_t distanceToGoal);
    uint16_t onPacketReceived(uint16_t command, uint16_t arg1, uint16_t arg2, uint8_t* pData, uint16_t size);
private:
    TrainCtrl _trainCtrl;
    LEDPattern _led;
    unsigned long _curTime;
    unsigned long _ledStarted;
};

#endif