#include "plugin.hpp"

struct VCO_1 : Module {
	enum ParamIds {
		TUNEKNOB_PARAM,
		FREQKNOB_PARAM,
		OCTAVEKNOB_PARAM,
		PULSEWIDTHKNOB_PARAM,
		FMKNOB_PARAM,
		PWMKNOB_PARAM,
		SIGNALTYPE_PARAM,
		SYNCMODE_PARAM,
		NUM_PARAMS
	};

	enum InputIds {
		OCTAVEJACK_INPUT,
		TUNEJACK_INPUT,
		VOCTJACK_INPUT,
		FMJACK_INPUT,
		PWMJACK_INPUT,
		SYNCJACK_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		SINEWAVEJACK_OUTPUT,
		TRIANGLEWAVEJACK_OUTPUT,
		SAWWAVEJACK_OUTPUT,
		SQUAREWAVEJACK_OUTPUT,
		NUM_OUTPUTS
	};

	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};

	simd::float_4 phase = 0.f;
	simd::float_4 lastSyncValue = 0.f;
	simd::float_4 syncDirection = 1.f;

	float sync = 0.f;

	bool analogsignal = false;
	bool softsync = false;

	dsp::TRCFilter<simd::float_4> sqrFilter;

	dsp::MinBlepGenerator<16, 32, simd::float_4> sinewaveMinBlep;
	dsp::MinBlepGenerator<16, 32, simd::float_4> trianglewaveMinBlep;
	dsp::MinBlepGenerator<16, 32, simd::float_4> sawwaveMinBlep;
	dsp::MinBlepGenerator<16, 32, simd::float_4> squarewaveMinBlep;

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

struct AHMRotaryTumblerWhiteHugeSnap : RoundKnob{
	AHMRotaryTumblerWhiteHugeSnap();
};
