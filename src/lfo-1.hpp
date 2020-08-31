#include "plugin.hpp"

struct LFO_1 : Module {
	enum ParamIds {
		KNOB_FREQUENCY_PARAM,
		KNOB_FM1_PARAM,
		KNOB_FM2_PARAM,
		KNOB_PULSEWIDTH_PARAM,
		KNOB_PWM_PARAM,
		KNOB_WAVESUBFREQ1_PARAM,
		KNOB_WAVESUBFREQ2_PARAM,
		KNOB_WAVESUBFREQ3_PARAM,
		KNOB_WAVESUBFREQ4_PARAM,
		KNOB_WAVEFORM1_PARAM,
		KNOB_WAVEFORM2_PARAM,
		KNOB_WAVEFORM3_PARAM,
		KNOB_WAVEFORM4_PARAM,
		KNOB_WAVEOUTLEVEL1_PARAM,
		KNOB_WAVEOUTLEVEL2_PARAM,
		KNOB_WAVEOUTLEVEL3_PARAM,
		KNOB_WAVEOUTLEVEL4_PARAM,
		SWITCH_VOLTAGEMODE_PARAM,
		SWITCH_PHASE_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		JACK_FM1_INPUT,
		JACK_FM2_INPUT,
		JACK_PWM_INPUT,
		JACK_SYNC_INPUT,
		JACK_RESET_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		JACK_OUTPUT1_OUTPUT,
		JACK_OUTPUT2_OUTPUT,
		JACK_OUTPUT3_OUTPUT,
		JACK_OUTPUT4_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
		LED_FREQUENCY_LIGHT,
		LED_WAVE1_LIGHT,
		LED_WAVE2_LIGHT,
		LED_WAVE3_LIGHT,
		LED_WAVE4_LIGHT,
		NUM_LIGHTS
	};
	
	LFO_1();
    
};

struct LFO_1Widget : ModuleWidget {
	LFO_1Widget(LFO_1* module);
};



// Additional stuff

struct LFOFrequencyKnob : RoundKnob{
	LFOFrequencyKnob();
};

struct LFOWaveformRotaryTumbler : RoundKnob{
	LFOWaveformRotaryTumbler();
};

struct LFOFrequencyParamQuantity : ParamQuantity {
	float getDisplayValue() override;
};

struct LFOWaveFormParamQuantity : ParamQuantity {
	std::string getDisplayValueString() override;
};

struct SwitchVoltageModeParamQuantity : ParamQuantity {
	std::string getDisplayValueString() override;
};

struct SwitchPhaseParamQuantity : ParamQuantity {
	std::string getDisplayValueString() override;
};
