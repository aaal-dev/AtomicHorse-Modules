#include "plugin.hpp"

struct Envelope_1 : Module {
	enum ParamIds {
		KNOB_START_PARAM,
		KNOB_ATTACK_PARAM,
		KNOB_HOLD_PARAM,
		KNOB_DECAY_PARAM,
		KNOB_DELAY_PARAM,
		KNOB_RELEASE_PARAM,
		KNOB_ATTACKSLOPE_PARAM,
		KNOB_DECAYSLOPE_PARAM,
		KNOB_RELEASESLOPE_PARAM,
		KNOB_TARGET_PARAM,
		KNOB_SUSTAIN_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		JACK_START_INPUT,
		JACK_ATTACK_INPUT,
		JACK_TARGET_INPUT,
		JACK_HOLD_INPUT,
		JACK_DECAY_INPUT,
		JACK_SUSTAIN_INPUT,
		JACK_DELAY_INPUT,
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
		LED_START_LIGHT,
		LED_ATTACK_LIGHT,
		LED_HOLD_LIGHT,
		LED_DECAY_LIGHT,
		LED_DELAY_LIGHT,
		LED_RELEASE_LIGHT,
		NUM_LIGHTS
	};
	enum StagesIds {
		STOP_STAGE,
		START_STAGE,
		ATTACK_STAGE,
		HOLD_STAGE,
		DECAY_STAGE,
		DELAY_STAGE,
		RELEASE_STAGE
	};

	bool started = false;
	bool completed = true;

	float env[16] = {0.f};
	float lastenv[16] = {0.f};
	float stagetime[16] = {0.f};
	StagesIds stage[16] = {STOP_STAGE};
	dsp::TSchmittTrigger<float> retrigger[16];
	dsp::TSchmittTrigger<float> gate[16];

	float start_stage_time[16] = {0.f};
	float attack_stage_time[16] = {0.f};
	float hold_stage_time[16] = {0.f};
	float decay_stage_time[16] = {0.f};
	float delay_stage_time[16] = {0.f};
	float release_stage_time[16] = {0.f};

	float attack_stage_slope[16] = {0.f};
	float decay_stage_slope[16] = {0.f};
	float release_stage_slope[16] = {0.f};

	float target_level[16] = {0.f};
	float sustain_level[16] = {0.f};

	float curve[16] = {0.f};

	Envelope_1();

	void process(const ProcessArgs& args) override;

	float rescaleBigKnobs(float x);
	float rescaleTinyKnobs(float x);
};

struct Envelope_1Widget : ModuleWidget {
	Envelope_1Widget(Envelope_1* module);
};

struct SmallEnvelope_1Widget : ModuleWidget {
	SmallEnvelope_1Widget(Envelope_1* module);
};

struct EnvelopeKnobParamQuantity : ParamQuantity {
	float getDisplayValue() override;
	void setDisplayValue(float v) override;
};
