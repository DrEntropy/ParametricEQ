/*
  ==============================================================================

    MeterValues.h
    Created: 2 May 2022 1:44:50pm
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include "Decibel.h"

struct MeterValues
{
    Decibel<float> leftPeakDb, rightPeakDb, leftRmsDb, rightRmsDb;
};
