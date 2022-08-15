/*
  Author: Tobias Grundtvig
*/

#include <Arduino.h>
#include "Locomotive.h"

Locomotive::Locomotive(uint8_t powerPin, uint8_t dirPin, uint8_t hallSensorPinA, uint8_t hallSensorPinB) :
                                                DeviceUDPClient(ESP.getChipId(), "LEGO_Locomotive" ,1),
                                                _trainCtrl(powerPin, dirPin, hallSensorPinA, hallSensorPinB),
                                                _led(LED_BUILTIN, false)
{
    _trainCtrl.setListener(this);
    pinMode(LED_BUILTIN, OUTPUT);
}

void Locomotive::update(unsigned long curTime)
{
    _curTime = curTime;
    DeviceUDPClient::update(curTime);
    _trainCtrl.update(curTime);
    _led.update(curTime);
    if(curTime - _ledStarted > 10000)
    {
        _led.stop();
    }
}

void Locomotive::onDistanceToGoalChange(uint32_t distanceToGoal)
{
    sendPacketToServer(CMD_DISTANCE_TO_GOAL, distanceToGoal, 0);
}

uint16_t Locomotive::onPacketReceived(uint16_t command, uint16_t arg1, uint16_t arg2, uint8_t* pData, uint16_t size)
{
    switch(command)
    {
        case CMD_IDENTIFY:
            _led.setBlinksPattern(arg1, 100, 200, 400);
            _led.start();
            _ledStarted = _curTime;
            return 0;
        case CMD_MOVE_TRAIN_FWD:
            Serial.print("Move train forward: MaxSpeed: ");
            Serial.print(arg1);
            Serial.print(" Distance: ");
            Serial.println(arg2);
            //_led.setBlinksPattern(1, 100, 200, 400);
            //_led.start();
            //_ledStarted = _curTime;
            if(!_trainCtrl.addCommand(0, arg1, arg2))
            {
                Serial.println("Train buffer full!");
                return 101;
            }
            return 0;
        case CMD_MOVE_TRAIN_BCK:
            Serial.print("Move train backward: MaxSpeed: ");
            Serial.print(arg1);
            Serial.print(" Distance: ");
            Serial.println(arg2);
            //_led.setBlinksPattern(2, 100, 200, 400);
            //_led.start();
            //_ledStarted = _curTime;
            if(!_trainCtrl.addCommand(1, arg1, arg2))
            {
                Serial.println("Train buffer full!");
                return 101;
            }
            return 0;
        default:
            return 102;
    }
}