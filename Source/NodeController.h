/*
  ==============================================================================

    NodeController.h
    Created: 30 May 2022 10:10:21am
    Author:  Ronald Legere

  ==============================================================================
*/

#pragma once
#include "AnalyzerBase.h"
#include "AnalyzerWidgets.h"
#include "AllParamsListener.h"

struct NodeController : AnalyzerBase
{
    NodeController();
    
    
private:
    
    std::array<std::unique_ptr<AnalyzerNode> , 16> nodes;
    std::unique_ptr<AllParamsListener> allParamsListener;
    
};

