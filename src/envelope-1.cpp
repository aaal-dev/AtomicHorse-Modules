#include "plugin.hpp"

const float MIN_TIME = 1e-3f;
const float MAX_TIME = 10.f;
const float LAMBDA_BASE = MAX_TIME / MIN_TIME;

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

	float env = 0.f;
	float stagetime = 0.f;
	dsp::TSchmittTrigger< float > trigger;
	dsp::ClockDivider cvDivider;

	float startValueLambda = 0.f;
	float attackValueLambda = 0.f;
	float targetValueLambda = 0.f;
	float holdValueLambda = 0.f;
	float decayValueLambda = 0.f;
	float sustainValueLambda = 0.f;
	float delayValueLambda = 0.f;
	float releaseValueLambda = 0.f;

	dsp::ClockDivider lightDivider;

	int stage = STOP_STAGE;


	Envelope_1() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(STARTKNOB_PARAM, 0.f, 1.f, 0.f, "Start", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(ATTACKKNOB_PARAM, 0.f, 1.f, 0.f, "Attack", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(ATTACKSLOPEKNOB_PARAM, 0.f, 1.f, 0.5f, "Attack slope", "", 0, 100);
		configParam(HOLDKNOB_PARAM, 0.f, 1.f, 0.f, "Hold", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(TARGETKNOB_PARAM, 0.f, 1.f, 1.f, "Target", "%", 0, 100);
		configParam(DECAYKNOB_PARAM, 0.f, 1.f, 0.5f, "Decay", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(DECAYSLOPEKNOB_PARAM, 0.f, 1.f, 0.5f, "Decay slope", "", 0, 100);
		configParam(SUSTAINKNOB_PARAM, 0.f, 1.f, 0.5f, "Sustain", "%", 0, 100);
		configParam(DELAYKNOB_PARAM, 0.f, 1.f, 0.f, "Delay", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(RELEASEKNOB_PARAM, 0.f, 1.f, 0.5f, "Release", " ms", LAMBDA_BASE, MIN_TIME * 1000);
		configParam(RELEASESLOPEKNOB_PARAM, 0.f, 1.f, 0.5f, "Release slope", "", 0, 100);

		cvDivider.setDivision(16);
		lightDivider.setDivision(128);
	}

	void process(const ProcessArgs& args) override {
		// 0.16-0.19 us serial
		// 0.23 us serial with all lambdas computed
		// 0.15-0.18 us serial with all lambdas computed with SSE

		int channels = inputs[GATEJACK_INPUT].getChannels();

		// Compute lambdas
		if (cvDivider.process()) {
			float startParamValue = params[STARTKNOB_PARAM].getValue();
			float attackParamValue = params[ATTACKKNOB_PARAM].getValue();
			float targetParamValue = params[TARGETKNOB_PARAM].getValue();
			float holdParamValue = params[HOLDKNOB_PARAM].getValue();
			float decayParamValue = params[DECAYKNOB_PARAM].getValue();
			float sustainParamValue = params[SUSTAINKNOB_PARAM].getValue();
			float delayParamValue = params[DELAYKNOB_PARAM].getValue();
			float releaseParamValue = params[RELEASEKNOB_PARAM].getValue();

			//float attackSlopeParamValue = params[ATTACKSLOPEKNOB_PARAM].getValue();
			//float decaySlopeParamValue = params[DECAYSLOPEKNOB_PARAM].getValue();
			//float releaseSlopeParamValue = params[RELEASESLOPEKNOB_PARAM].getValue();

			for (int channel = 0; channel < channels; channel++) {
				// Start
				float startValue = startParamValue;
				if (inputs[STARTJACK_INPUT].isConnected())
					startValue += inputs[STARTJACK_INPUT].getPolyVoltage(channel) / 10.f;
				startValue = math::clamp(startValue, 0.f, 1.f);
				startValueLambda = std::pow(LAMBDA_BASE, -startValue) / MIN_TIME;

				// Attack
				float attackValue = attackParamValue;
				if (inputs[ATTACKJACK_INPUT].isConnected())
					attackValue += inputs[ATTACKJACK_INPUT].getPolyVoltage(channel) / 10.f;
				attackValue = math::clamp(attackValue, 0.f, 1.f);
				attackValueLambda = std::pow(LAMBDA_BASE, -attackValue) / MIN_TIME;

				// Target
				float targetValue = targetParamValue;
				if (inputs[TARGETJACK_INPUT].isConnected())
					targetValue += inputs[TARGETJACK_INPUT].getPolyVoltage(channel) / 10.f;
				targetValue = math::clamp(targetValue, 0.f, 1.f);
				targetValueLambda = std::pow(LAMBDA_BASE, -targetValue) / MIN_TIME;

				// Hold
				float holdValue = holdParamValue;
				if (inputs[HOLDJACK_INPUT].isConnected())
					holdValue += inputs[HOLDJACK_INPUT].getPolyVoltage(channel) / 10.f;
				holdValue = math::clamp(holdValue, 0.f, 1.f);
				holdValueLambda = std::pow(LAMBDA_BASE, -holdValue) / MIN_TIME;

				// Decay
				float decayValue = decayParamValue;
				if (inputs[DECAYJACK_INPUT].isConnected())
					decayValue += inputs[DECAYJACK_INPUT].getPolyVoltage(channel) / 10.f;
				decayValue = math::clamp(decayValue, 0.f, 1.f);
				decayValueLambda = std::pow(LAMBDA_BASE, -decayValue) / MIN_TIME;

				// Sustain
				float sustainValue = sustainParamValue;
				if (inputs[SUSTAINJACK_INPUT].isConnected())
					sustainValue += inputs[SUSTAINJACK_INPUT].getPolyVoltage(channel) / 10.f;
				sustainValue = math::clamp(sustainValue, 0.f, 1.f);
				sustainValueLambda = std::pow(LAMBDA_BASE, -sustainValue) / MIN_TIME;

				// Delay
				float delayValue = delayParamValue;
				if (inputs[DELAYJACK_INPUT].isConnected())
					delayValue += inputs[DELAYJACK_INPUT].getPolyVoltage(channel) / 10.f;
				delayValue = math::clamp(delayValue, 0.f, 1.f);
				delayValueLambda = std::pow(LAMBDA_BASE, -delayValue) / MIN_TIME;

				// Release
				float releaseValue = releaseParamValue;
				if (inputs[RELEASEJACK_INPUT].isConnected())
					releaseValue += inputs[RELEASEJACK_INPUT].getPolyVoltage(channel) / 10.f;
				releaseValue = math::clamp(releaseValue, 0.f, 1.f);
				releaseValueLambda = std::pow(LAMBDA_BASE, -releaseValue) / MIN_TIME;
			}
		}

		float gate;

		for (int channel = 0; channel < channels; channel++) {
			// Gate
			gate = inputs[GATEJACK_INPUT].getVoltage(channel) >= 1.f;

			// Retrigger
			bool triggered = trigger.process(inputs[TRIGJACK_INPUT].getPolyVoltage(channel));
			if (triggered) {
				stagetime = 0.f;
				stage = START_STAGE;
			}

			switch(stage) {
				case STOP_STAGE: {
						break;
					}
				case START_STAGE: {
						stagetime += (startValueLambda - stagetime) * startValueLambda;
						if (stagetime >= startValueLambda) {
							stagetime = 0.f;
							stage = ATTACK_STAGE;
						}
						break;
					}
				case ATTACK_STAGE: {
						stagetime += (attackValueLambda - stagetime) * attackValueLambda;
						env += (targetValueLambda - env) * attackValueLambda * args.sampleTime;
						if (stagetime >= attackValueLambda) {
							stagetime = 0.f;
							stage = HOLD_STAGE;
							}
						break;
					}
				case HOLD_STAGE: {
						stagetime += (holdValueLambda - stagetime) * holdValueLambda;
						if (stagetime >= holdValueLambda) {
							stagetime = 0.f;
							stage = DECAY_STAGE;
						}
						break;
					}
				case DECAY_STAGE: {
						stagetime += (decayValueLambda - stagetime) * decayValueLambda;
						env += (sustainValueLambda - env) * decayValueLambda * args.sampleTime;
						if (stagetime >= decayValueLambda) {
							stagetime = 0.f;
							stage = DELAY_STAGE;
						}
						break;
					}
				case DELAY_STAGE: {
						stagetime += (delayValueLambda - stagetime) * delayValueLambda;
						if (stagetime >= delayValueLambda) {
							stagetime = 0.f;
							stage = RELEASE_STAGE;
						}
						break;
					}
				case RELEASE_STAGE: {
						stagetime += (releaseValueLambda - stagetime) * releaseValueLambda;
						env += (0.f - env) * releaseValueLambda * args.sampleTime;
						if (stagetime >= releaseValueLambda) {
							stagetime = 0.f;
							stage = STOP_STAGE;
						}
						break;
					}
				default: {

						break;
					}
			}

			// Set output
			outputs[ENVELOPEJACK_OUTPUT].setVoltage(10.f * env, channel);
		}

		outputs[ENVELOPEJACK_OUTPUT].setChannels(channels);

		// Lights
		if (lightDivider.process()) {
			lights[STARTLED_LIGHT].setBrightness(0);
			lights[ATTACKLED_LIGHT].setBrightness(0);
			lights[HOLDLED_LIGHT].setBrightness(0);
			lights[DECAYLED_LIGHT].setBrightness(0);
			lights[DELAYLED_LIGHT].setBrightness(0);
			lights[RELEASELED_LIGHT].setBrightness(0);

			for (int channel = 0; channel < channels; channel++) {
				switch(stage) {
					case START_STAGE: {
							lights[STARTLED_LIGHT].setBrightness(1);
						break;
					}
					case ATTACK_STAGE: {
							lights[ATTACKLED_LIGHT].setBrightness(1);
						break;
					}
					case HOLD_STAGE: {
							lights[HOLDLED_LIGHT].setBrightness(1);
						break;
					}
					case DECAY_STAGE: {
							lights[DECAYLED_LIGHT].setBrightness(1);
						break;
					}
					case DELAY_STAGE: {
							lights[DELAYLED_LIGHT].setBrightness(1);
						break;
					}
					case RELEASE_STAGE: {
							lights[RELEASELED_LIGHT].setBrightness(1);
						break;
					}
					default: 	break;
				}
			}
		}
	}
};

struct Envelope_1Widget : ModuleWidget {
	Envelope_1Widget(Envelope_1* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/envelope-1.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(31.000, 13.00)), module, Envelope_1::STARTKNOB_PARAM));
		addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(31.000, 26.50)), module, Envelope_1::ATTACKKNOB_PARAM));
		addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(20.319, 40.00)), module, Envelope_1::TARGETKNOB_PARAM));
		addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(31.000, 50.00)), module, Envelope_1::HOLDKNOB_PARAM));
		addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(31.000, 63.50)), module, Envelope_1::DECAYKNOB_PARAM));
		addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(20.319, 77.00)), module, Envelope_1::SUSTAINKNOB_PARAM));
		addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(31.000, 87.00)), module, Envelope_1::DELAYKNOB_PARAM));
		addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(31.000, 100.5)), module, Envelope_1::RELEASEKNOB_PARAM));

		addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(19.394, 26.50)), module, Envelope_1::ATTACKSLOPEKNOB_PARAM));
		addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(19.394, 63.50)), module, Envelope_1::DECAYSLOPEKNOB_PARAM));
		addParam(createParamCentered<AHMRoundKnob4WhiteTiny>(mm2px(Vec(19.394, 100.5)), module, Envelope_1::RELEASESLOPEKNOB_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.72, 13.00)), module, Envelope_1::STARTJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.72, 26.50)), module, Envelope_1::ATTACKJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.72, 40.00)), module, Envelope_1::TARGETJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.72, 50.00)), module, Envelope_1::HOLDJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.72, 63.50)), module, Envelope_1::DECAYJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.72, 77.00)), module, Envelope_1::SUSTAINJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.72, 87.00)), module, Envelope_1::DELAYJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.72, 100.5)), module, Envelope_1::RELEASEJACK_INPUT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8.720, 112.69)), module, Envelope_1::GATEJACK_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.32, 112.69)), module, Envelope_1::TRIGJACK_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.923, 112.69)), module, Envelope_1::ENVELOPEJACK_OUTPUT));

		addChild(createLightCentered<TinyLight<YellowLight>>(mm2px(Vec(38.1, 13.00)), module, Envelope_1::STARTLED_LIGHT));
		addChild(createLightCentered<TinyLight<YellowLight>>(mm2px(Vec(38.1, 26.50)), module, Envelope_1::ATTACKLED_LIGHT));
		addChild(createLightCentered<TinyLight<YellowLight>>(mm2px(Vec(38.1, 50.00)), module, Envelope_1::HOLDLED_LIGHT));
		addChild(createLightCentered<TinyLight<YellowLight>>(mm2px(Vec(38.1, 63.50)), module, Envelope_1::DECAYLED_LIGHT));
		addChild(createLightCentered<TinyLight<YellowLight>>(mm2px(Vec(38.1, 87.00)), module, Envelope_1::DELAYLED_LIGHT));
		addChild(createLightCentered<TinyLight<YellowLight>>(mm2px(Vec(38.1, 100.5)), module, Envelope_1::RELEASELED_LIGHT));
	}
};

Model* modelEnvelope_1 = createModel<Envelope_1, Envelope_1Widget>("Envelope-1");
