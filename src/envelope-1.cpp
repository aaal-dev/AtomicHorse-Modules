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

	float env[16] = {0.f};
	float stagetime[16] = {0.f};
	StagesIds stage[16] = {STOP_STAGE};
	dsp::TSchmittTrigger<float> trigger[16];
	dsp::TSchmittTrigger<float> gate[16];
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

	Envelope_1() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(STARTKNOB_PARAM, 0.f, 10.f, 0.f, "Start", " ms", 0.f, 1000.f);
		configParam(ATTACKKNOB_PARAM, 0.f, 1.f, 0.f, "Attack", " ms", 0.f, 10000.f);
		configParam(ATTACKSLOPEKNOB_PARAM, 0.5f, 2.f, 1.f, "Attack slope", "", 0, 100);
		configParam(HOLDKNOB_PARAM, 0.f, 1.f, 0.f, "Hold", " ms", 0.f, 10000.f);
		configParam(TARGETKNOB_PARAM, 0.f, 1.f, 1.f, "Target", "%", 0, 100);
		configParam(DECAYKNOB_PARAM, 0.f, 1.f, 0.5f, "Decay", " ms", 0.f, 10000.f);
		configParam(DECAYSLOPEKNOB_PARAM, 0.f, 1.f, 0.5f, "Decay slope", "", 0, 100);
		configParam(SUSTAINKNOB_PARAM, 0.f, 1.f, 0.5f, "Sustain", "%", 0, 100);
		configParam(DELAYKNOB_PARAM, 0.f, 1.f, 0.f, "Delay", " ms", 0.f, 10000.f);
		configParam(RELEASEKNOB_PARAM, 0.f, 1.f, 0.5f, "Release", " ms", 0.f, 10000.f);
		configParam(RELEASESLOPEKNOB_PARAM, 0.f, 1.f, 0.5f, "Release slope", "", 0, 100);

		cvDivider.setDivision(16);
		lightDivider.setDivision(128);
	}

	void process(const ProcessArgs& args) override {
		// 0.16-0.19 us serial
		// 0.23 us serial with all lambdas computed
		// 0.15-0.18 us serial with all lambdas computed with SSE

		int channels = inputs[GATEJACK_INPUT].getChannels();

		float startParamValue = params[STARTKNOB_PARAM].getValue();
		float attackParamValue = params[ATTACKKNOB_PARAM].getValue();
		float targetParamValue = params[TARGETKNOB_PARAM].getValue();
		float holdParamValue = params[HOLDKNOB_PARAM].getValue();
		float decayParamValue = params[DECAYKNOB_PARAM].getValue();
		float sustainParamValue = params[SUSTAINKNOB_PARAM].getValue();
		float delayParamValue = params[DELAYKNOB_PARAM].getValue();
		float releaseParamValue = params[RELEASEKNOB_PARAM].getValue();
		float attackSlopeParamValue = params[ATTACKSLOPEKNOB_PARAM].getValue();
		//float decaySlopeParamValue = params[DECAYSLOPEKNOB_PARAM].getValue();
		//float releaseSlopeParamValue = params[RELEASESLOPEKNOB_PARAM].getValue();

		// Compute lambdas
		if (cvDivider.process()) {


			for (int channel = 0; channel < channels; channel++) {
				// Start
				if (inputs[STARTJACK_INPUT].isConnected())
					startParamValue += inputs[STARTJACK_INPUT].getPolyVoltage(channel) / 10.f;
				//startParamValue = math::clamp(startParamValue, 0.f, 1.f);
				startValueLambda = startParamValue;
				//startValueLambda = std::pow(LAMBDA_BASE, -startValue) / MIN_TIME;
				//startValueLambda = startParamValue * args.sampleTime;

				// Attack
				if (inputs[ATTACKJACK_INPUT].isConnected())
					attackParamValue += inputs[ATTACKJACK_INPUT].getPolyVoltage(channel) / 10.f;
				//attackParamValue = math::clamp(attackParamValue, 0.f, 1.f);
				attackValueLambda = std::pow(attackParamValue, 2.0) * 10;
				//attackValueLambda = std::pow(LAMBDA_BASE, -attackValue) / MIN_TIME;
				//attackValueLambda = attackParamValue * args.sampleTime;

				// Target
				if (inputs[TARGETJACK_INPUT].isConnected())
					targetParamValue += inputs[TARGETJACK_INPUT].getPolyVoltage(channel) / 10.f;
				//targetParamValue = math::clamp(targetParamValue, 0.f, 1.f);
				targetValueLambda = std::pow(targetParamValue, 2.0);
				//targetValueLambda = std::pow(LAMBDA_BASE, -targetValue) / MIN_TIME;
				//targetValueLambda = targetParamValue * args.sampleTime;

				// Hold
				if (inputs[HOLDJACK_INPUT].isConnected())
					holdParamValue += inputs[HOLDJACK_INPUT].getPolyVoltage(channel) / 10.f;
				//holdParamValue = math::clamp(holdParamValue, 0.f, 1.f);
				holdValueLambda = std::pow(holdParamValue, 2.0) * 10;
				//holdValueLambda = std::pow(LAMBDA_BASE, -holdValue) / MIN_TIME;
				//holdValueLambda = holdParamValue * args.sampleTime;

				// Decay
				if (inputs[DECAYJACK_INPUT].isConnected())
					decayParamValue += inputs[DECAYJACK_INPUT].getPolyVoltage(channel) / 10.f;
				decayParamValue = math::clamp(decayParamValue, 0.f, 1.f);
				decayValueLambda = std::pow(decayParamValue, 2.0) * 10;
				//decayValueLambda = std::pow(LAMBDA_BASE, -decayValue) / MIN_TIME;
				//decayValueLambda = decayParamValue * args.sampleTime;

				// Sustain
				if (inputs[SUSTAINJACK_INPUT].isConnected())
					sustainParamValue += inputs[SUSTAINJACK_INPUT].getPolyVoltage(channel) / 10.f;
				sustainParamValue = math::clamp(sustainParamValue, 0.f, 1.f);
				sustainValueLambda = std::pow(sustainParamValue, 2.0) * 10;
				//sustainValueLambda = std::pow(LAMBDA_BASE, -sustainValue) / MIN_TIME;
				//sustainValueLambda = sustainParamValue * args.sampleTime;

				// Delay
				if (inputs[DELAYJACK_INPUT].isConnected())
					delayParamValue += inputs[DELAYJACK_INPUT].getPolyVoltage(channel) / 10.f;
				delayParamValue = math::clamp(delayParamValue, 0.f, 1.f);
				delayValueLambda = std::pow(delayParamValue, 2.0) * 10;
				//delayValueLambda = std::pow(LAMBDA_BASE, -delayValue) / MIN_TIME;
				//delayValueLambda = delayParamValue * args.sampleTime;

				// Release
				if (inputs[RELEASEJACK_INPUT].isConnected())
					releaseParamValue += inputs[RELEASEJACK_INPUT].getPolyVoltage(channel) / 10.f;
				releaseParamValue = math::clamp(releaseParamValue, 0.f, 1.f);
				releaseValueLambda = std::pow(releaseParamValue, 2.0) * 10;
				//releaseValueLambda = std::pow(LAMBDA_BASE, -releaseValue) / MIN_TIME;
				//releaseValueLambda = releaseParamValue * args.sampleTime;
			}
		}

		for (int channel = 0; channel < channels; channel++) {
			// Gate
			if (inputs[GATEJACK_INPUT].isConnected())
				gate[channel].process(inputs[GATEJACK_INPUT].getPolyVoltage(channel));

			// Retrigger
			if (inputs[TRIGJACK_INPUT].isConnected())
				trigger[channel].process(inputs[TRIGJACK_INPUT].getPolyVoltage(channel));

			if (gate[channel].isHigh() && trigger[channel].isHigh()) {
				stagetime[channel] = 0.f;
				stage[channel] = START_STAGE;
			} else if (!gate[channel].isHigh() && !(stage[channel] == STOP_STAGE) && !(stage[channel] == RELEASE_STAGE)) {
				stagetime[channel] = 0.f;
				stage[channel] = RELEASE_STAGE;
			}

			switch(stage[channel]) {
				case START_STAGE: {
						stagetime[channel] += args.sampleTime;
						if (stagetime[channel] >= startValueLambda) {
							stagetime[channel] = 0.f;
							stage[channel] = ATTACK_STAGE;
						}
						break;
					}
				case ATTACK_STAGE: {
						stagetime[channel] += args.sampleTime / attackValueLambda;
						//env[channel] += (targetValueLambda - env[channel]) * args.sampleTime / attackValueLambda;
						//env = stagetime + 1 / attackValueLambda;
						env[channel] += (env[channel] - targetValueLambda) * args.sampleTime * attackValueLambda;
						if (stagetime[channel] >= 1.f) {
							stagetime[channel] = 0.f;
							stage[channel] = HOLD_STAGE;
							}
						break;
					}
				case HOLD_STAGE: {
						//stagetime += (holdValueLambda - stagetime) * holdValueLambda;
						stagetime[channel] += args.sampleTime / holdValueLambda;
						if (stagetime[channel] >= 1.f) {
							stagetime[channel] = 0.f;
							stage[channel] = DECAY_STAGE;
						}
						break;
					}
				case DECAY_STAGE: {
						//stagetime += (decayValueLambda - stagetime) * decayValueLambda;
						stagetime[channel] += args.sampleTime / decayValueLambda;
						//env += (sustainValueLambda - env) * decayValueLambda * args.sampleTime;
						//env += args.sampleTime / attackValueLambda;
						if (stagetime[channel] >= 1.f) {
							stagetime[channel] = 0.f;
							stage[channel] = DELAY_STAGE;
						}
						break;
					}
				case DELAY_STAGE: {
						//stagetime += (delayValueLambda - stagetime) * delayValueLambda;
						stagetime[channel] += args.sampleTime / delayValueLambda;
						if (stagetime[channel] >= 1.f) {
							stagetime[channel] = 0.f;
							stage[channel] = RELEASE_STAGE;
						}
						break;
					}
				case RELEASE_STAGE: {
						//stagetime += (releaseValueLambda - stagetime) * releaseValueLambda;
						stagetime[channel] += args.sampleTime / releaseValueLambda;
						//env += (0.f - env) * releaseValueLambda * args.sampleTime;
						//env += attackValueLambda;
						if (stagetime[channel] >= 1.f) {
							stagetime[channel] = 0.f;
							stage[channel] = STOP_STAGE;
						}
						break;
					}
				default: {
						env[channel] = 0.f;
						stagetime[channel] = 0.f;
						break;
					}
			}

			// Set output
			outputs[ENVELOPEJACK_OUTPUT].setVoltage(10.f * env[channel], channel);
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
				switch(stage[channel]) {
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
