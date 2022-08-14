/*
  Author: Tobias Grundtvig
*/

#include <Arduino.h>
#include "TrainCtrl.h"

TrainCtrl::TrainCtrl(uint8_t powerPin, uint8_t dirPin, uint8_t hallSensorPinA, uint8_t hallSensorPinB)
{
    HallSensors::begin(hallSensorPinA, hallSensorPinB);
    pinMode(powerPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    _powerPin = powerPin;
    _dirPin = dirPin;
    _distanceToGoal = 0;

    _lastMagnetTime = 0;
    _lastPowerIncreaseTime = 0;
  
    _curPower = 0;
    _curDir = 0;

    _overShoot = 0;
    _state = TRAINCTRL_STATE_INIT_FWD;
    _minPwmFwd = 0;
    _minPwmBck = 0;

    _cmdFirst = 0;
    _cmdSize = 0;
}

void TrainCtrl::setListener(TrainListener* pListener)
{
  _pTrainListener = pListener;
}

uint32_t TrainCtrl::getDistanceToGoal()
{
  return _distanceToGoal;
}

void TrainCtrl::update(unsigned long curTime)
{
  int delta = HallSensors::getAndResetDeltaCount();
  while(delta > 0)
  {
    --delta;
    _onMagnet(curTime);
  }
  while(delta < 0)
  {
    ++delta;
    _onMagnet(curTime);
  }

  switch(_state)
  {
    case TRAINCTRL_STATE_READY:
      if(_powerCtrl.getSize() > 0)
      {
        Serial.println("READY -> MOVING");
        uint8_t dir = _powerCtrl.getDir();
        uint8_t pwr = _powerCtrl.getNext();
        Serial.print("Dir: ");
        Serial.print(dir);
        Serial.print(" pwr: ");
        Serial.println(pwr);
        _setDirection(dir);
        _setPower(pwr);
        _state = TRAINCTRL_STATE_MOVING;
      }
      break;
    case TRAINCTRL_STATE_MOVING:
      //Check if train seems to be needing more power to run:
      if(_curPower < 100 && curTime - _lastMagnetTime > 30 && curTime - _lastPowerIncreaseTime > 30)
      {
        _lastPowerIncreaseTime = curTime;
        ++_curPower;
        _setPower(_curPower);
      }
      break;
    case TRAINCTRL_STATE_STOPPING:
      if(curTime - _lastMagnetTime > TRAINCTRL_STOPPING_TIME)
      {
        Serial.println("Train stopped allowing direction change!");
        _state = TRAINCTRL_STATE_READY;
        if(_powerCtrl.getSize() == 0)
        {
          _powerCtrl.allowDirectionChange();
          _transferCommands();
        }
        Serial.println("STOPPING -> READY");
      }
      break;
    case TRAINCTRL_STATE_OVERSHOT:
      if(curTime - _lastMagnetTime > TRAINCTRL_STOPPING_TIME)
      {
        _state = TRAINCTRL_STATE_OVERSHOT_CORRECTION;
        _setDirection(_curDir == 0 ? 1 : 0);
        _setPower(1);
        Serial.println("OVERSHOT -> CORRECTION");
      }
      break;
    case TRAINCTRL_STATE_OVERSHOT_CORRECTION:
      if(_curPower < 100 && curTime - _lastMagnetTime > 2000 && curTime - _lastPowerIncreaseTime > 100)
      {
        _lastPowerIncreaseTime = curTime;
        ++_curPower;
        _setPower(_curPower);
      }
      break;
    case TRAINCTRL_STATE_INIT_FWD:
      if(curTime - _lastPowerIncreaseTime > 10)
      {
        _lastPowerIncreaseTime = curTime;
        ++_minPwmFwd;
        if(_minPwmFwd > 1023) _minPwmFwd = 1023;
        analogWrite(_powerPin, _minPwmFwd);
      }
      break;
    case TRAINCTRL_STATE_INIT_STOP:
      if(curTime - _lastMagnetTime > 2000)
      {
        _state = TRAINCTRL_STATE_INIT_BCK;
        _setDirection(_curDir == 0 ? 1 : 0);
      }
      break;
    case TRAINCTRL_STATE_INIT_BCK:
      if(curTime - _lastPowerIncreaseTime > 10)
      {
        _lastPowerIncreaseTime = curTime;
        ++_minPwmBck;
        if(_minPwmBck > 1023) _minPwmBck = 1023;
        analogWrite(_powerPin, _minPwmBck);
      }
      break;
  }   
}

bool TrainCtrl::addCommand(uint8_t direction, uint8_t maxPower, uint16_t distance)
{
  if(_cmdSize >= CMD_BUFFER_SIZE)
  {
    //Command buffer is full...
    return false;
  }
  uint8_t lastIndex = (_cmdFirst + _cmdSize) % CMD_BUFFER_SIZE;
  _cmdDir[lastIndex] = direction;
  _cmdMaxPower[lastIndex] = maxPower;
  _cmdMagnets[lastIndex] = distance;
  ++_cmdSize;
  _distanceToGoal += distance;
  _transferCommands();
  if(_pTrainListener != 0)
  {
    _pTrainListener->onDistanceToGoalChange(_distanceToGoal);
  }
  return true;
}

void TrainCtrl::_transferCommands()
{
  while(_cmdSize > 0)
  {
    while(_cmdMagnets[_cmdFirst] > 0)
    {
      if(!_powerCtrl.add(_cmdDir[_cmdFirst], _cmdMaxPower[_cmdFirst]))
      {
        //Can't add any more...
        return;
      }
      Serial.print("Added: (dir: ");
      Serial.print(_cmdDir[_cmdFirst]);
      Serial.print(", power: ");
      Serial.print(_cmdMaxPower[_cmdFirst]);
      Serial.println(")");
      --_cmdMagnets[_cmdFirst];
    }
    _cmdFirst = (_cmdFirst + 1) % CMD_BUFFER_SIZE;
    --_cmdSize;
  }
}

void TrainCtrl::_onMagnet(unsigned long curTime)
{
  uint8_t pwr;
  _lastMagnetTime = curTime;
  Serial.println("OnMagnet!");
  switch(_state)
  {
    case TRAINCTRL_STATE_READY:
      //Who moved the train?????
      Serial.println("Who moved the train?????");
      break;
    case TRAINCTRL_STATE_MOVING:
      --_distanceToGoal;
      pwr = _powerCtrl.getNext();
      //Serial.print("OnMagnet, moving set power: ");
      //Serial.println(pwr);
      _setPower(pwr);
      if(_powerCtrl.getSize() == 0 && _curPower == 0)
      {
        _state = TRAINCTRL_STATE_STOPPING;
        Serial.println("MOVING -> STOPPING");
      }
      _transferCommands();
      if(_pTrainListener != 0)
      {
        _pTrainListener->onDistanceToGoalChange(_distanceToGoal);
      }
      break;
    case TRAINCTRL_STATE_STOPPING:
      _state = TRAINCTRL_STATE_OVERSHOT;
      _overShoot = 1;
      Serial.println("STOPPING -> OVERSHOT");
      break;
    case TRAINCTRL_STATE_OVERSHOT:
      ++_overShoot;
      break;
    case TRAINCTRL_STATE_OVERSHOT_CORRECTION:
      --_overShoot;
      if(_overShoot == 0)
      {
        _setPower(0);
        _state = TRAINCTRL_STATE_STOPPING;
        Serial.println("CORRECTION -> STOPPING");
      }
      break;
    case TRAINCTRL_STATE_INIT_FWD:
      _setPower(0);
      _state = TRAINCTRL_STATE_INIT_STOP;
      Serial.println("INIT_FWD -> INIT_STOP");
      break;
    case TRAINCTRL_STATE_INIT_BCK:
      _setPower(0);
      _state = TRAINCTRL_STATE_STOPPING;
      Serial.println("INIT_BCK -> STOPPING");
      break;
  }
}

void TrainCtrl::_setPower(uint8_t power)
{
  _curPower = power;
  uint16_t pwm = 0;
  if(power > 0)
  {
    pwm = map(power, 1, 100, (_curDir == 0 ? (_minPwmFwd / 4) : (_minPwmBck / 4)), 1023);
    //pwm = map(power, 1, 100, 1, 1023);
  }
  analogWrite(_powerPin, pwm);
}

void TrainCtrl::_setDirection(uint8_t dir)
{
  _curDir = dir;
  HallSensors::setDirection(dir);
  digitalWrite(_dirPin, dir);
}