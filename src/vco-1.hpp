#include "plugin.hpp"

struct VCO_1 : Module {
	enum ParamIds {
		KNOB_FREQ_PARAM,
		KNOB_FM_PARAM,
		KNOB_TUNE_PARAM,
		KNOB_OCTAVE_PARAM,
		KNOB_PULSEWIDTH_PARAM,
		KNOB_PWM_PARAM,
		SWITCH_SIGNALTYPE_PARAM,
		SWITCH_SYNCMODE_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		JACK_OCTAVE_INPUT,
		JACK_TUNE_INPUT,
		JACK_VOCT_INPUT,
		JACK_FM_INPUT,
		JACK_PWM_INPUT,
		JACK_SYNC_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		JACK_SINEWAVE_OUTPUT,
		JACK_TRIANGLEWAVE_OUTPUT,
		JACK_SAWWAVE_OUTPUT,
		JACK_SQUAREWAVE_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
		LED_BLINK_LIGHT,
		NUM_LIGHTS
	};

	simd::float_4 phase[16] = {0.f};
	simd::float_4 last_sync[16] = {0.f};
	simd::float_4 sync_direction[16] = {1.f};

	float sync = 0.f;

	bool analogsignal = false;
	bool softsync = false;

	dsp::TRCFilter<simd::float_4> sqr_filter[16];

	dsp::MinBlepGenerator<16, 32, simd::float_4> sinewave_blep[16];
	dsp::MinBlepGenerator<16, 32, simd::float_4> trianglewave_blep[16];
	dsp::MinBlepGenerator<16, 32, simd::float_4> sawwave_blep[16];
	dsp::MinBlepGenerator<16, 32, simd::float_4> squarewave_blep[16];

	VCO_1();

	simd::float_4 sin2pi_pade_05_5_4(simd::float_4 x);
	simd::float_4 expCurve(simd::float_4 x);

	simd::float_4 sinewave(simd::float_4 phase);
	simd::float_4 trianglewave(simd::float_4 phase);
	simd::float_4 sawwave(simd::float_4 phase);
	simd::float_4 squarewave(simd::float_4 phase, float pwParamValue);

	void process(const ProcessArgs& args) override;
};

struct VCO_1Widget : ModuleWidget {
	VCO_1Widget(VCO_1* module);
};

struct VCOFreqKnobParamQuantity : ParamQuantity {
	virtual float offset();
	float getDisplayValue() override;
	void setDisplayValue(float v) override;
};


struct VCOFrequencyKnob : RoundKnob{
	VCOFrequencyKnob();
};

struct VCOOctaveRotaryTumbler : RoundKnob{
	VCOOctaveRotaryTumbler();
};
