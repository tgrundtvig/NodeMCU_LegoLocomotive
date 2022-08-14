/*
  Author: Tobias Grundtvig
*/

#ifndef TrainListener_h
#define TrainListener_h

#include <Arduino.h>


class TrainListener
{
public:
    virtual void onDistanceToGoalChange(uint32_t distanceToGoal) = 0;
};

#endif