/*
  Author: Tobias Grundtvig
*/

#include <Arduino.h>
#include "PowerCtrl.h"

PowerCtrl::PowerCtrl()
{
    _curPower = 0;
    _index = 0;
    _size = 0;
    _dir = 0;
    _newAdded = false;
    _allowDirectionChange = true;

    _accelleratePowerValues[0] = 1;
    _accelleratePowerValues[1] = 10;
    _accelleratePowerValues[2] = 20;
    _accelleratePowerValues[3] = 30;
    _accelleratePowerValues[4] = 40;
    _accelleratePowerValues[5] = 50;
    _accelleratePowerValues[6] = 60;
    _accelleratePowerValues[7] = 70;
    _accelleratePowerValues[8] = 80;
    _accelleratePowerValues[9] = 90;
    _accelleratePowerValues[10] = 100;
    _accelleratePowerValues[11] = 100;
    _accelleratePowerValues[12] = 100;
    _accelleratePowerValues[13] = 100;
    _accelleratePowerValues[14] = 100;
    _accelleratePowerValues[15] = 100;

    _decelleratePowerValues[0] = 1;
    _decelleratePowerValues[1] = 2;
    _decelleratePowerValues[2] = 4;
    _decelleratePowerValues[3] = 6;
    _decelleratePowerValues[4] = 8;
    _decelleratePowerValues[5] = 10;
    _decelleratePowerValues[6] = 20;
    _decelleratePowerValues[7] = 30;
    _decelleratePowerValues[8] = 50;
    _decelleratePowerValues[9] = 70;
    _decelleratePowerValues[10] = 100;
    _decelleratePowerValues[11] = 100;
    _decelleratePowerValues[12] = 100;
    _decelleratePowerValues[13] = 100;
    _decelleratePowerValues[14] = 100;
    _decelleratePowerValues[15] = 100;
}

uint8_t PowerCtrl::getNext()
{
    if(_size == 0)
    {
        _curPower = 0;
        return _curPower;
    }
    if(_newAdded && _lookAhead <= POWER_CTRL_PRESET_SIZE)
    {
        _updateActualPowerValues();
    }
    _curPower = _actualPowerValues[_index];
    _index = (_index + 1) % POWER_CTRL_BUFFER_SIZE;
    --_size;
    --_lookAhead;
    return _curPower;
}

void PowerCtrl::allowDirectionChange()
{
    _allowDirectionChange = true;
}


bool PowerCtrl::add(uint8_t dir, uint8_t maxPower)
{
    if(_size >= POWER_CTRL_BUFFER_SIZE || (!_allowDirectionChange && dir != _dir)) return false;
    _dir = dir;
    _allowDirectionChange = false;
    uint8_t bufferPos = (_index + _size) % POWER_CTRL_BUFFER_SIZE;
    _maxPowerValues[bufferPos] = maxPower;
    ++_size;
    _newAdded = true;
    return true;
}



void PowerCtrl::_updateActualPowerValues()
{
    //Take care of decellerate values
    uint8_t presetPos = 0;
    for(int i = _size - 1; i >= 0; --i)
    {
        uint8_t bufferPos = (_index + i) % POWER_CTRL_BUFFER_SIZE;
        if(_decelleratePowerValues[presetPos] <= _maxPowerValues[bufferPos])
        {
            _actualPowerValues[bufferPos] = _decelleratePowerValues[presetPos];
            presetPos = (presetPos < POWER_CTRL_PRESET_SIZE - 1 ? presetPos + 1 : presetPos);
        }
        else
        {
            _actualPowerValues[bufferPos] = _maxPowerValues[bufferPos];
            presetPos = POWER_CTRL_PRESET_SIZE - 1;
            for(uint8_t j = 0; j < POWER_CTRL_PRESET_SIZE - 1; ++j)
            {
                if(_decelleratePowerValues[j] > _maxPowerValues[bufferPos])
                {
                    presetPos = j;
                    break;
                }
            }
        }       
    }

    //Take care of accellerate values...
    if(_size > 0)
    {
        presetPos = POWER_CTRL_PRESET_SIZE - 1;
        for(uint8_t j = 0; j < POWER_CTRL_PRESET_SIZE - 1; ++j)
        {
            if(_accelleratePowerValues[j] > _curPower)
            {
                presetPos = j;
                break;
            }
        } 
    }
    for(int i = 0; i < _size; ++i)
    {
        uint8_t bufferPos = (_index + i) % POWER_CTRL_BUFFER_SIZE;
        if(_accelleratePowerValues[presetPos] <= _actualPowerValues[bufferPos])
        {
            _actualPowerValues[bufferPos] = _accelleratePowerValues[presetPos];
            presetPos = (presetPos < POWER_CTRL_PRESET_SIZE - 1 ? presetPos + 1 : presetPos);
        }
        else
        {
            presetPos = POWER_CTRL_PRESET_SIZE - 1;
            for(uint8_t j = 0; j < POWER_CTRL_PRESET_SIZE - 1; ++j)
            {
                if(_accelleratePowerValues[j] > _actualPowerValues[bufferPos])
                {
                    presetPos = j;
                    break;
                }
            }
        }       
    }
    _newAdded = false;
    _lookAhead = _size;
}