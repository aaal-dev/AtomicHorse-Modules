#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelVCO_1;
extern Model* modelEnvelope_1;

// Knobs

struct AHMRoundKnobWhite : RoundKnob{
	AHMRoundKnobWhite();
};

struct AHMRoundKnobWhiteHuge : RoundKnob{
	AHMRoundKnobWhiteHuge();
};

struct AHMRoundKnob2WhiteHuge : RoundKnob{
	AHMRoundKnob2WhiteHuge();
};

struct AHMRoundKnobWhiteLarge : RoundKnob{
	AHMRoundKnobWhiteLarge();
};

struct AHMRoundKnobWhiteTiny : RoundKnob{
	AHMRoundKnobWhiteTiny();
};

struct AHMRoundKnob2WhiteTiny : RoundKnob{
	AHMRoundKnob2WhiteTiny();
};

struct AHMRoundKnob3WhiteTiny : RoundKnob{
	AHMRoundKnob3WhiteTiny();
};

struct AHMRoundKnob4WhiteTiny : RoundKnob{
	AHMRoundKnob4WhiteTiny();
};




