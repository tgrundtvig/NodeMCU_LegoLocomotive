/*
  Author: Tobias Grundtvig
*/

#ifndef PowerCtrl_h
#define PowerCtrl_h

#define POWER_CTRL_BUFFER_SIZE 32
#define POWER_CTRL_PRESET_SIZE 16


#include <Arduino.h>

class PowerCtrl
{
public:
    PowerCtrl();
    uint8_t getSize() {return _size;}
    bool add(uint8_t dir, uint8_t maxPower);
    uint8_t getNext();
    uint8_t getDir() {return _dir;}
    void allowDirectionChange();
private:
    void _updateActualPowerValues();
    uint8_t _accelleratePowerValues[POWER_CTRL_PRESET_SIZE];
    uint8_t _decelleratePowerValues[POWER_CTRL_PRESET_SIZE];
    uint8_t _actualPowerValues[POWER_CTRL_BUFFER_SIZE];
    uint8_t _maxPowerValues[POWER_CTRL_BUFFER_SIZE]; 
    uint8_t _index;
    uint8_t _size;
    uint8_t _curPower;
    uint8_t _lookAhead;
    uint8_t _dir;
    bool _newAdded;
    bool _allowDirectionChange;
};


#endif