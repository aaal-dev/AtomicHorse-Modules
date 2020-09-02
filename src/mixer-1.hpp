#include "plugin.hpp"

#define CHANNELS_NUMBER 4

struct Mixer_1 : Module {
	enum ParamIds {
		ENUMS(SLIDER_LEVEL_PARAM, 4),
		ENUMS(KNOB_PAN_PARAM, 4),
		KNOB_CVMAIN_PARAM,
		KNOB_LEVELMAIN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(JACK_IN_L_INPUT, 4), 
		ENUMS(JACK_IN_R_INPUT, 4),
		ENUMS(JACK_CV_INPUT, 4), 
		ENUMS(JACK_PAN_INPUT, 4), 
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
	int channels_l[CHANNELS_NUMBER] = {0};
	int channels_r[CHANNELS_NUMBER] = {0};
	
	float level_l_ParamValue[CHANNELS_NUMBER][16] = {0.f};
	float level_r_ParamValue[CHANNELS_NUMBER][16] = {0.f};
	float mix_l_ParamValue[16] = {0.f};
	float mix_r_ParamValue[16] = {0.f};
	
	Mixer_1();
	
	void process(const ProcessArgs& args) override;
};

struct Mixer_1Widget : ModuleWidget {
	Mixer_1Widget(Mixer_1* module);
};



// Additional components 

struct MixerMainLevelKnob : RoundKnob{
	MixerMainLevelKnob();
};

struct MixerLevel : SvgSlider {
	MixerLevel();
};

struct MixerVuMeter : LightWidget {
	Mixer_1* module;
	MixerVuMeter();
	void draw(const DrawArgs& args) override;
};
