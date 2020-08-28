#include "envelope-1.hpp"

Envelope_1::Envelope_1() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam<EnvelopeKnobParamQuantity>(STARTKNOB_PARAM, 0.f, 1.f, 0.f, "Start", " s");
	configParam<EnvelopeKnobParamQuantity>(ATTACKKNOB_PARAM, 0.f, 1.f, 0.2f, "Attack", " s");
	configParam<EnvelopeKnobParamQuantity>(HOLDKNOB_PARAM, 0.f, 1.f, 0.f, "Hold", " s");
	configParam<EnvelopeKnobParamQuantity>(DECAYKNOB_PARAM, 0.f, 1.f, 0.2f, "Decay", " s");
	configParam<EnvelopeKnobParamQuantity>(DELAYKNOB_PARAM, 0.f, 1.f, 0.5f, "Delay", " s");
	configParam<EnvelopeKnobParamQuantity>(RELEASEKNOB_PARAM, 0.f, 1.f, 0.5f, "Release", " s");
	//<EnvelopeKnobParamQuantity>

	configParam(ATTACKSLOPEKNOB_PARAM, .1f, 3.f, 1.f, "Attack slope", "", 0, 100);
	configParam(DECAYSLOPEKNOB_PARAM, .1f, 3.f, 1.f, "Decay slope", "", 0, 100);
	configParam(RELEASESLOPEKNOB_PARAM, .1f, 3.f, 1.f, "Release slope", "", 0, 100);

	configParam(TARGETKNOB_PARAM, 0.000f, 10.000f, 10.000f, "Target level", "%", 0, 10);
	configParam(SUSTAINKNOB_PARAM, 0.f, 10.000f, 5.f, "Sustain level", "%", 0, 10);

	cvDivider.setDivision(16);
	lightDivider.setDivision(128);
}

// Quadratic rescale seconds with milliseconds
float Envelope_1::rescale(float x) {
	if (x >= 0.f && x < 0.2f) x = math::rescale(x, 0.f, .2f, 0.f, .1f);
	else if (x >= 0.2f && x < 0.5f) x = math::rescale(x, .2f, .5f, 0.1f, 1.f);
	else if (x >= 0.5f && x < 0.8f) x = math::rescale(x, .5f, .8f, 1.f, 5.f);
	else if (x >= 0.8f) x = math::rescale(x, .8f, 1.f, 5.f, 10.f);

	int xx = (int)(x * 1000);
	x = xx * .001f;
	return x;
}

void Envelope_1::process(const ProcessArgs& args) {
	// 0.16-0.19 us serial
	// 0.23 us serial with all lambdas computed
	// 0.15-0.18 us serial with all lambdas computed with SSE

	int channels = inputs[GATEJACK_INPUT].getChannels();

	// Compute lambdas
	if (cvDivider.process()) {
		for (int channel = 0; channel < channels; channel++) {

			// Start
			startParamValue[channel] = params[STARTKNOB_PARAM].getValue();
			if (inputs[STARTJACK_INPUT].isConnected())
				startParamValue[channel] += inputs[STARTJACK_INPUT].getPolyVoltage(channel) / 10.f;
			startParamValue[channel] = rescale(startParamValue[channel]);

			// Attack
			attackParamValue[channel] = params[ATTACKKNOB_PARAM].getValue();
			if (inputs[ATTACKJACK_INPUT].isConnected())
				attackParamValue[channel] += inputs[ATTACKJACK_INPUT].getPolyVoltage(channel) / 10.f;
			attackParamValue[channel] = rescale(attackParamValue[channel]);
			attackSlopeParamValue[channel] = params[ATTACKSLOPEKNOB_PARAM].getValue();

			// Hold
			holdParamValue[channel] = params[HOLDKNOB_PARAM].getValue();
			if (inputs[HOLDJACK_INPUT].isConnected())
				holdParamValue[channel] += inputs[HOLDJACK_INPUT].getPolyVoltage(channel) / 10.f;
			holdParamValue[channel] = rescale(holdParamValue[channel]);

			// Target
			targetParamValue[channel] = params[TARGETKNOB_PARAM].getValue();
			if (inputs[TARGETJACK_INPUT].isConnected())
				targetParamValue[channel] += inputs[TARGETJACK_INPUT].getPolyVoltage(channel) / 10.f;

			// Decay
			decayParamValue[channel] = params[DECAYKNOB_PARAM].getValue();
			if (inputs[DECAYJACK_INPUT].isConnected())
				decayParamValue[channel] += inputs[DECAYJACK_INPUT].getPolyVoltage(channel) / 10.f;
			decayParamValue[channel] = rescale(decayParamValue[channel]);
			decaySlopeParamValue[channel] = params[DECAYSLOPEKNOB_PARAM].getValue();

			// Sustain
			sustainParamValue[channel] = params[SUSTAINKNOB_PARAM].getValue();
			if (inputs[SUSTAINJACK_INPUT].isConnected())
				sustainParamValue[channel] += inputs[SUSTAINJACK_INPUT].getPolyVoltage(channel) / 10.f;

			// Delay
			delayParamValue[channel] = params[DELAYKNOB_PARAM].getValue();
			if (inputs[DELAYJACK_INPUT].isConnected())
				delayParamValue[channel] += inputs[DELAYJACK_INPUT].getPolyVoltage(channel) / 10.f;
			delayParamValue[channel] = rescale(delayParamValue[channel]);

			// Release
			releaseParamValue[channel] = params[RELEASEKNOB_PARAM].getValue();
			if (inputs[RELEASEJACK_INPUT].isConnected())
				releaseParamValue[channel] += inputs[RELEASEJACK_INPUT].getPolyVoltage(channel) / 10.f;
			releaseParamValue[channel] = rescale(releaseParamValue[channel]);
			releaseSlopeParamValue[channel] = params[RELEASESLOPEKNOB_PARAM].getValue();
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
			case STOP_STAGE:
				break;
			case START_STAGE: {
					stagetime[channel] += args.sampleTime;
					if (stagetime[channel] >= startParamValue[channel]) {
						stagetime[channel] = 0.f;
						stage[channel] = ATTACK_STAGE;
					}
					break;
				}
			case ATTACK_STAGE: {
					stagetime[channel] += args.sampleTime;
					env[channel] = targetParamValue[channel] * std::pow(stagetime[channel] / attackParamValue[channel], attackSlopeParamValue[channel]);
					lastenv[channel] = env[channel];
					if (stagetime[channel] >= attackParamValue[channel]) {
						stagetime[channel] = 0.f;
						stage[channel] = HOLD_STAGE;
					}
					break;
				}
			case HOLD_STAGE: {
					stagetime[channel] += args.sampleTime;
					if (stagetime[channel] >= holdParamValue[channel]) {
						stagetime[channel] = 0.f;
						stage[channel] = DECAY_STAGE;
					}
					break;
				}
			case DECAY_STAGE: {
					stagetime[channel] += args.sampleTime;
					env[channel] = targetParamValue[channel] - (targetParamValue[channel] - sustainParamValue[channel]) * std::pow(stagetime[channel] / decayParamValue[channel], decaySlopeParamValue[channel]);
					lastenv[channel] = env[channel];
					if (stagetime[channel] >= decayParamValue[channel]) {
						stagetime[channel] = 0.f;
						stage[channel] = DELAY_STAGE;
					}
					break;
				}
			case DELAY_STAGE: {
					stagetime[channel] += args.sampleTime;
					if (stagetime[channel] >= delayParamValue[channel]) {
						stagetime[channel] = 0.f;
						stage[channel] = RELEASE_STAGE;
					}
					break;
				}
			case RELEASE_STAGE: {
					stagetime[channel] += args.sampleTime;
					env[channel] = lastenv[channel] - (lastenv[channel]) * std::pow(stagetime[channel] / releaseParamValue[channel], releaseSlopeParamValue[channel]);
					if (stagetime[channel] >= releaseParamValue[channel]) {
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
		outputs[ENVELOPEJACK_OUTPUT].setVoltage(env[channel], channel);
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

		float startledBrightness = 0.f;
		float attackledBrightness = 0.f;
		float holdledBrightness = 0.f;
		float decayledBrightness = 0.f;
		float delayledBrightness = 0.f;
		float releaseledBrightness = 0.f;
		for (int channel = 0; channel < channels; channel++) {
			switch(stage[channel]) {
				case START_STAGE: {
						startledBrightness += 0.0625f;
						lights[STARTLED_LIGHT].setBrightness(startledBrightness);
						break;
					}
				case ATTACK_STAGE: {
						attackledBrightness += 0.0625f;
						lights[ATTACKLED_LIGHT].setBrightness(attackledBrightness);
						break;
					}
				case HOLD_STAGE: {
						holdledBrightness += 0.0625f;
						lights[HOLDLED_LIGHT].setBrightness(holdledBrightness);
						break;
					}
				case DECAY_STAGE: {
						decayledBrightness += 0.0625f;
						lights[DECAYLED_LIGHT].setBrightness(decayledBrightness);
						break;
					}
				case DELAY_STAGE: {
						delayledBrightness += 0.0625f;
						lights[DELAYLED_LIGHT].setBrightness(delayledBrightness);
						break;
					}
				case RELEASE_STAGE: {
						releaseledBrightness += 0.0625f;
						lights[RELEASELED_LIGHT].setBrightness(releaseledBrightness);
						break;
					}
				default: 	break;
			}
		}
	}
}

Envelope_1Widget::Envelope_1Widget(Envelope_1* module) {
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

Model* modelEnvelope_1 = createModel<Envelope_1, Envelope_1Widget>("Envelope-1");

float EnvelopeKnobParamQuantity::getDisplayValue() {
	float x = getValue();
	if (!module) {
		return x;
	}
	//int vv = (int)(v * v * v * 1000);
	//v = vv * .001f;
	if (x >= 0.f && x < 0.2f) x = math::rescale(x, 0.f, .2f, 0.f, .1f);
	if (x >= 0.2f && x < 0.5f) x = math::rescale(x, .2f, .5f, 0.1f, 1.f);
	else if (x >= 0.5f && x < 0.8f) x = math::rescale(x, .5f, .8f, 1.f, 5.f);
	else if (x >= 0.8f) x = math::rescale(x, .8f, 1.f, 5.f, 10.f);

	int xx = (int)(x * 1000);
	x = xx * .001f;
	x += displayOffset;
	if (x < 0.0f) {
		return -x;
	}
	return x;
}

void EnvelopeKnobParamQuantity::setDisplayValue(float displayValue) {
	if (!module) {
		return;
	}
	displayValue -= displayOffset;
	setValue(displayValue);
}
