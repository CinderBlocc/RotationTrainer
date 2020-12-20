#pragma once
#include "bakkesmod/wrappers/wrapperstructs.h"

struct SequenceProperties
{
    bool bUsesBall = true;
    bool bSetStartBoost = false;
    bool bSetMaxBoost = false;
    bool bSetStartPosition = false;
    bool bSetStartRotation = false;

    int StartBoostAmount = 100;
    int MaxBoostAmount = 100;
    Vector StartPosition = {0, 0, 0};
    Rotator StartRotation = {0, 0, 0};
};
