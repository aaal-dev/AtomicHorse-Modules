#include "plugin.hpp"

struct Mixer2m1s : Module {
	enum ParamIds {
		FADER_LEVEL_L_PARAM,
		FADER_LEVEL_R_PARAM,
		KNOB_PAN_L_PARAM,
		KNOB_PAN_R_PARAM,
		KNOB_MAINCV_PARAM,
		KNOB_MAINLEVEL_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		JACK_IN_L_INPUT,
		JACK_IN_R_INPUT,
		JACK_CV_L_INPUT,
		JACK_CV_R_INPUT,
		JACK_PAN_L_INPUT,
		JACK_PAN_R_INPUT,
		JACK_MAINCV_INPUT, 
		NUM_INPUTS
	};
	enum OutputIds {
		JACK_MAIN_L_OUTPUT, 
		JACK_MAIN_R_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LED_LEVEL_L_LIGHT,
		LED_LEVEL_R_LIGHT,
		LED_LEVELMAIN_LIGHT,
		NUM_LIGHTS
	};
	
	float main_l_value[16] = {};
	float main_r_value[16] = {};
	
	Mixer2m1s();
	
	void process(const ProcessArgs& args) override;
	
	struct MixerMainLevelKnob : RoundKnob{
		MixerMainLevelKnob();
	};

	struct MixerLevel : SvgSlider {
		MixerLevel();
	};
};

struct Mixer2m1s_Widget : ModuleWidget {
	Mixer2m1s_Widget(Mixer2m1s* module);
};



// Additional components 

