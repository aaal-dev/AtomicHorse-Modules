#include "plugin.hpp"

struct ADSR: Module {
	enum ParamIds {
		KNOB_ATTACK_PARAM,
		KNOB_DECAY_PARAM,
		KNOB_RELEASE_PARAM,
		KNOB_SUSTAIN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		JACK_ATTACK_INPUT,
		JACK_DECAY_INPUT,
		JACK_SUSTAIN_INPUT,
		JACK_RELEASE_INPUT,
		JACK_GATE_INPUT,
		JACK_RETRIGGER_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		JACK_ENVELOPE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LED_ATTACK_LIGHT,
		LED_DECAY_LIGHT,
		LED_SUSTAIN_LIGHT,
		LED_RELEASE_LIGHT,
		NUM_LIGHTS
	};
	enum StagesIds {
		STOP_STAGE,
		ATTACK_STAGE,
		DECAY_STAGE,
		SUSTAIN_STAGE,
		RELEASE_STAGE
	};

	float started[16] = {0.f};
	float completed[16] = {1.f};

	float env[16] = {0.f};
	float lastenv[16] = {0.f};
	float stagetime[16] = {0.f};
	StagesIds stage[16] = {STOP_STAGE};
	dsp::TSchmittTrigger<float> retrigger[16];
	dsp::TSchmittTrigger<float> gate[16];

	float attack_stage_time[16] = {0.f};
	float decay_stage_time[16] = {0.f};
	float release_stage_time[16] = {0.f};

	float sustain_level[16] = {0.f};

	ADSR();

	void process(const ProcessArgs& args) override;

	float curveSlope(float x, float y);

	float rescaleBigKnobs(float x);
	float rescaleTinyKnobs(float x);

	struct EnvelopeKnobParamQuantity : ParamQuantity {
		float getDisplayValue() override;
	};
};

struct ADSRWidget : ModuleWidget {
	ADSRWidget(ADSR* module);
};


