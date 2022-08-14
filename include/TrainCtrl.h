/*
  Author: Tobias Grundtvig
*/

#ifndef TrainCtrl_h
#define TrainCtrl_h

#include <Arduino.h>
#include <PowerCtrl.h>
#include <HallSensors.h>
#include "TrainListener.h"

#define CMD_BUFFER_SIZE 256

#define TRAINCTRL_STOPPING_TIME 2000

#define TRAINCTRL_STATE_READY 0
#define TRAINCTRL_STATE_MOVING 1
#define TRAINCTRL_STATE_STOPPING 2
#define TRAINCTRL_STATE_OVERSHOT 3
#define TRAINCTRL_STATE_OVERSHOT_CORRECTION 4
#define TRAINCTRL_STATE_INIT_FWD 5
#define TRAINCTRL_STATE_INIT_STOP 6
#define TRAINCTRL_STATE_INIT_BCK 7


class TrainCtrl
{
public:
    TrainCtrl(uint8_t powerPin, uint8_t dirPin, uint8_t hallSensorPinA, uint8_t hallSensorPinB);
    void setListener(TrainListener* pListener);
    void update(unsigned long curTime);
    bool addCommand(uint8_t direction, uint8_t maxPower, uint16_t distance);
    uint32_t getDistanceToGoal();
private:
    void _transferCommands();
    void _onMagnet(unsigned long curTime);
    void _setPower(uint8_t power);
    void _setDirection(uint8_t dir);
    TrainListener* _pTrainListener;
    uint8_t _powerPin;
    uint8_t _dirPin;
    uint16_t _minPwmFwd;
    uint16_t _minPwmBck;
    PowerCtrl _powerCtrl;

    unsigned long _lastMagnetTime;
    unsigned long _lastPowerIncreaseTime;
    
    uint32_t _distanceToGoal;
    
    uint8_t _curPower;
    uint8_t _curDir;

    uint8_t _overShoot;
    uint8_t _state;

    uint16_t _cmdMagnets[CMD_BUFFER_SIZE];
    uint8_t _cmdDir[CMD_BUFFER_SIZE];
    uint8_t _cmdMaxPower[CMD_BUFFER_SIZE];
    uint8_t _cmdFirst;
    uint8_t _cmdSize;
};


#endif