#include "plugin.hpp"

struct Envelope_1 : Module {
	enum ParamIds {
		STARTKNOB_PARAM,
		ATTACKKNOB_PARAM,
		ATTACKSLOPEKNOB_PARAM,
		TARGETKNOB_PARAM,
		HOLDKNOB_PARAM,
		DECAYKNOB_PARAM,
		DECAYSLOPEKNOB_PARAM,
		SUSTAINKNOB_PARAM,
		DELAYKNOB_PARAM,
		RELEASEKNOB_PARAM,
		RELEASESLOPEKNOB_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		STARTJACK_INPUT,
		ATTACKJACK_INPUT,
		TARGETJACK_INPUT,
		HOLDJACK_INPUT,
		DECAYJACK_INPUT,
		SUSTAINJACK_INPUT,
		DELAYJACK_INPUT,
		RELEASEJACK_INPUT,
		GATEJACK_INPUT,
		TRIGJACK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENVELOPEJACK_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		STARTLED_LIGHT,
		ATTACKLED_LIGHT,
		HOLDLED_LIGHT,
		DECAYLED_LIGHT,
		DELAYLED_LIGHT,
		RELEASELED_LIGHT,
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

	float env[16] = {0.f};
	float lastenv[16] = {0.f};
	float stagetime[16] = {0.f};
	StagesIds stage[16] = {STOP_STAGE};
	dsp::TSchmittTrigger<float> trigger[16];
	dsp::TSchmittTrigger<float> gate[16];
	dsp::ClockDivider cvDivider;

	float startParamValue[16] = {0.f};
	float attackParamValue[16] = {0.f};
	float holdParamValue[16] = {0.f};
	float decayParamValue[16] = {0.f};
	float delayParamValue[16] = {0.f};
	float releaseParamValue[16] = {0.f};

	float attackSlopeParamValue[16] = {0.f};
	float decaySlopeParamValue[16] = {0.f};
	float releaseSlopeParamValue[16] = {0.f};

	float targetParamValue[16] = {0.f};
	float sustainParamValue[16] = {0.f};

	float curve[16] = {0.f};

	dsp::ClockDivider lightDivider;

	Envelope_1();

	float rescale(float x);

	void process(const ProcessArgs& args) override;

};

struct Envelope_1Widget : ModuleWidget {
	Envelope_1Widget(Envelope_1* module);
};

struct EnvelopeKnobParamQuantity : ParamQuantity {
	float getDisplayValue() override;
	void setDisplayValue(float v) override;
};
