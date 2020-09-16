#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* model_VCO_1;
extern Model* model_Envelope_1;
extern Model* model_ADSR;
extern Model* model_LFO_1;
extern Model* model_Mixer4m1m;
extern Model* model_Mixer10m1m;
extern Model* model_Mixer2m1s;

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

// Switches

struct AHMCKSS : SvgSwitch{
	AHMCKSS();
};


