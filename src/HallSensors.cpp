/*
  Author: Tobias Grundtvig
*/

#include <Arduino.h>
#include "HallSensors.h"

volatile uint8_t HallSensors::_hallSensorAPin;
volatile uint8_t HallSensors::_hallSensorBPin;
volatile uint8_t HallSensors::_curDirection;
volatile uint8_t HallSensors::_curPos;
volatile uint8_t HallSensors::_lastPos;
volatile bool HallSensors::_onMagnet;
volatile unsigned long HallSensors::_enterMagnetTime;
volatile unsigned long HallSensors::_onMagnetTime;

void HallSensors::begin(const uint8_t hallSensorAPin, const uint8_t hallSensorBPin )                          
{
    _hallSensorAPin = hallSensorAPin;
    _hallSensorBPin = hallSensorBPin;

    _curDirection = 0;
    _curPos = 0;
    _lastPos = 0;

    _onMagnet = false;
    _enterMagnetTime = 0;
    _onMagnetTime = 0;

    pinMode(_hallSensorAPin, INPUT_PULLUP);
    pinMode(_hallSensorBPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_hallSensorAPin), HallSensors::_onHallSensorA, FALLING);
    attachInterrupt(digitalPinToInterrupt(_hallSensorBPin), HallSensors::_onHallSensorB, FALLING);
}

int HallSensors::getAndResetDeltaCount()
{
    //Since curPos can be updated at any time, we make a local copy to work on...
    uint8_t tmpCurPos = _curPos;
    if(tmpCurPos == _lastPos)
    {
        return 0;
    }
    uint8_t deltaForward = tmpCurPos - _lastPos;
    uint8_t deltaBackward = _lastPos - tmpCurPos;
    if(deltaForward < deltaBackward)
    {
        //If we are going backward this is a correction
        if(_curDirection == 1) return 0;
        _lastPos = tmpCurPos;
        return deltaForward;

    }
    //If we are going forward this is a correction
    if(_curDirection == 0) return 0;
    _lastPos = tmpCurPos; 
    return -deltaBackward;
}

unsigned long HallSensors::getMagnetTime()
{
    return _onMagnetTime;
}

uint8_t HallSensors::getDirection()
{
    return _curDirection;
}

void HallSensors::setDirection(uint8_t dir)
{
    if(dir == _curDirection) return;
    if (_curDirection == 0)
    {
        _curDirection = 1;
        if (_onMagnet)
        {
            if (!digitalRead(_hallSensorAPin))
            {
                _onMagnet = false;
            }
            else
            {
                ++_curPos;
            }
        }
        else // !onMagnet
        {
            if (!digitalRead(_hallSensorBPin))
            {
                if(!digitalRead(_hallSensorAPin))
                {
                    --_curPos;
                }
                else
                {
                    _onMagnet = true;
                }
            }
        }
    }
    else //curDirection == 1
    {
        _curDirection = 0;
        if (_onMagnet)
        {
            if (!digitalRead(_hallSensorBPin))
            {
                _onMagnet = false;
            }
            else
            {
                --_curPos;
            }
        }
        else // !onMagnet
        {
            if (!digitalRead(_hallSensorAPin))
            {
                if (!digitalRead(_hallSensorBPin))
                {
                    ++_curPos;
                }
                else
                {
                    _onMagnet = true;
                }
            }
        }
    }
}

//Hallsensor interrupt callbacks
void HallSensors::_onHallSensorA()
{
    if (_curDirection == 0 && !_onMagnet)
    {
        _onMagnet = true;
        _enterMagnetTime = millis();
    }
    else if (_curDirection == 1 && _onMagnet)
    {
        _onMagnet = false;
        _onMagnetTime = millis() - _enterMagnetTime;
        --_curPos;
    }
}

void HallSensors::_onHallSensorB()
{
    if (_curDirection == 1 && !_onMagnet)
    {
        _onMagnet = true;
        _enterMagnetTime = millis();
    }
    else if (_curDirection == 0 && _onMagnet)
    {
        _onMagnet = false;
        _onMagnetTime = millis() - _enterMagnetTime;
        ++_curPos;
    }
}
