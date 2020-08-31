#include "plugin.hpp"

template <typename TLightBase = RedLight>
struct LEDLightSliderFixed : LEDLightSlider<TLightBase> {
	LEDLightSliderFixed();
};

struct MixerMainLevelKnob : RoundKnob{
	MixerMainLevelKnob();
};

struct Mixer_1 : Module {
	enum ParamIds {
		SLIDER_LEVEL1_PARAM,
		SLIDER_LEVEL2_PARAM,
		SLIDER_LEVEL3_PARAM,
		SLIDER_LEVEL4_PARAM,
		KNOB_PAN1_PARAM,
		KNOB_PAN2_PARAM,
		KNOB_PAN3_PARAM,
		KNOB_PAN4_PARAM,
		KNOB_CVMAIN_PARAM,
		KNOB_LEVELMAIN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		JACK_IN1_L_INPUT, 
		JACK_IN1_R_INPUT,
		JACK_IN2_L_INPUT, 
		JACK_IN2_R_INPUT,
		JACK_IN3_L_INPUT, 
		JACK_IN3_R_INPUT,
		JACK_IN4_L_INPUT, 
		JACK_IN4_R_INPUT,
		JACK_CV1_INPUT, 
		JACK_CV2_INPUT,
		JACK_CV3_INPUT, 
		JACK_CV4_INPUT,
		JACK_PAN1_INPUT, 
		JACK_PAN2_INPUT,
		JACK_PAN3_INPUT, 
		JACK_PAN4_INPUT,
		JACK_CVMAIN_INPUT, 
		NUM_INPUTS
	};
	enum OutputIds {
		JACK_OUTMAIN_L_OUTPUT, 
		JACK_OUTMAIN_R_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LED_LEVEL1_LIGHT,
		LED_LEVEL2_LIGHT,
		LED_LEVEL3_LIGHT,
		LED_LEVEL4_LIGHT,
		LED_LEVELMAIN_LIGHT,
		NUM_LIGHTS
	};
	
	Mixer_1();
};

struct Mixer_1Widget : ModuleWidget {
	Mixer_1Widget(Mixer_1* module);
};
