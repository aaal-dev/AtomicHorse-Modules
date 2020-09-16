#include "plugin.hpp"

#define TRACKS_NUMBER 4

struct Mixer4m1m : Module {
	enum ParamIds {
		ENUMS(FADER_LEVEL_PARAM, TRACKS_NUMBER),
		KNOB_MAINCV_PARAM,
		KNOB_MAINLEVEL_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(JACK_IN_INPUT, TRACKS_NUMBER), 
		ENUMS(JACK_CV_INPUT, TRACKS_NUMBER), 
		JACK_MAINCV_INPUT, 
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(JACK_OUT_OUTPUT, TRACKS_NUMBER), 
		JACK_MAIN_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(LED_TRACKLEVEL_LIGHT, TRACKS_NUMBER), 
		LED_MAINLEVEL_LIGHT,
		NUM_LIGHTS
	};
	
	Mixer4m1m();
	
	void process(const ProcessArgs& args) override;
	float compensateVolume(int current_track_number, float fader_value[]);
	
	// Additional components
	struct MixerMainLevelKnob : RoundKnob{
		MixerMainLevelKnob();
	};

	struct MixerLevel : SvgSlider {
		MixerLevel();
	};
};

struct Mixer4m1m_Widget : ModuleWidget {
	Mixer4m1m_Widget(Mixer4m1m* module);
};



 


