#include "envelope-1.hpp"

Envelope_1::Envelope_1() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam<EnvelopeKnobParamQuantity>(KNOB_START_PARAM,   0.001f, 1.f, 0.f,  "Start",   " s");
	configParam<EnvelopeKnobParamQuantity>(KNOB_ATTACK_PARAM,  0.001f, 1.f, 0.2f, "Attack",  " s");
	configParam<EnvelopeKnobParamQuantity>(KNOB_HOLD_PARAM,    0.001f, 1.f, 0.f,  "Hold",    " s");
	configParam<EnvelopeKnobParamQuantity>(KNOB_DECAY_PARAM,   0.001f, 1.f, 0.2f, "Decay",   " s");
	configParam<EnvelopeKnobParamQuantity>(KNOB_DELAY_PARAM,   0.001f, 1.f, 0.5f, "Delay",   " s");
	configParam<EnvelopeKnobParamQuantity>(KNOB_RELEASE_PARAM, 0.001f, 1.f, 0.2f, "Release", " s");

	configParam(KNOB_ATTACKSLOPE_PARAM,  .0f, 1.f, .5f, "Attack slope");
	configParam(KNOB_DECAYSLOPE_PARAM,   .0f, 1.f, .5f, "Decay slope");
	configParam(KNOB_RELEASESLOPE_PARAM, .0f, 1.f, .5f, "Release slope");

	configParam(KNOB_TARGET_PARAM,  0.f, 10.f, 10.f, "Target level",  "%", 0, 10);
	configParam(KNOB_SUSTAIN_PARAM, 0.f, 10.f, 5.f,  "Sustain level", "%", 0, 10);
}

void Envelope_1::process(const ProcessArgs& args) {
	if (inputs[JACK_GATE_INPUT].isConnected()) {
		int voices = inputs[JACK_GATE_INPUT].getChannels();
		outputs[JACK_ENVELOPE_OUTPUT].setChannels(voices);

		float startled_brightness = 0.f;
		float attackled_brightness = 0.f;
		float holdled_brightness = 0.f;
		float decayled_brightness = 0.f;
		float delayled_brightness = 0.f;
		float releaseled_brightness = 0.f;

		lights[LED_START_LIGHT].setBrightness(0);
		lights[LED_ATTACK_LIGHT].setBrightness(0);
		lights[LED_HOLD_LIGHT].setBrightness(0);
		lights[LED_DECAY_LIGHT].setBrightness(0);
		lights[LED_DELAY_LIGHT].setBrightness(0);
		lights[LED_RELEASE_LIGHT].setBrightness(0);

		for (int voice = 0; voice < voices; voice++) {

			// Start
			start_stage_time[voice] = params[KNOB_START_PARAM].getValue();
			if (inputs[JACK_START_INPUT].isConnected())
				start_stage_time[voice] *= inputs[JACK_START_INPUT].getPolyVoltage(voice) / 10.f;
			start_stage_time[voice] = rescaleBigKnobs(start_stage_time[voice]);
			start_stage_time[voice] = clamp(start_stage_time[voice], 0.f, 10.f);

			// Attack
			attack_stage_time[voice] = params[KNOB_ATTACK_PARAM].getValue();
			if (inputs[JACK_ATTACK_INPUT].isConnected())
				attack_stage_time[voice] *= inputs[JACK_ATTACK_INPUT].getPolyVoltage(voice) / 10.f;
			attack_stage_time[voice] = rescaleBigKnobs(attack_stage_time[voice]);
			attack_stage_time[voice] = clamp(attack_stage_time[voice], 0.001f, 10.f);
			attack_stage_slope[voice] = params[KNOB_ATTACKSLOPE_PARAM].getValue();
			attack_stage_slope[voice] = rescaleTinyKnobs(attack_stage_slope[voice]);

			// Hold
			hold_stage_time[voice] = params[KNOB_HOLD_PARAM].getValue();
			if (inputs[JACK_HOLD_INPUT].isConnected())
				hold_stage_time[voice] += inputs[JACK_HOLD_INPUT].getPolyVoltage(voice) / 10.f;
			hold_stage_time[voice] = rescaleBigKnobs(hold_stage_time[voice]);
			hold_stage_time[voice] = clamp(hold_stage_time[voice], 0.f, 10.f);

			// Target
			target_level[voice] = params[KNOB_TARGET_PARAM].getValue();
			if (inputs[JACK_TARGET_INPUT].isConnected())
				target_level[voice] += inputs[JACK_TARGET_INPUT].getPolyVoltage(voice) / 10.f;
			target_level[voice] = clamp(target_level[voice], 0.f, 10.f);

			// Decay
			decay_stage_time[voice] = params[KNOB_DECAY_PARAM].getValue();
			if (inputs[JACK_DECAY_INPUT].isConnected())
				decay_stage_time[voice] += inputs[JACK_DECAY_INPUT].getPolyVoltage(voice) / 10.f;
			decay_stage_time[voice] = rescaleBigKnobs(decay_stage_time[voice]);
			decay_stage_time[voice] = clamp(decay_stage_time[voice], 0.001f, 10.f);
			decay_stage_slope[voice] = params[KNOB_DECAYSLOPE_PARAM].getValue();
			decay_stage_slope[voice] = rescaleTinyKnobs(decay_stage_slope[voice]);

			// Sustain
			sustain_level[voice] = params[KNOB_SUSTAIN_PARAM].getValue();
			if (inputs[JACK_SUSTAIN_INPUT].isConnected())
				sustain_level[voice] += inputs[JACK_SUSTAIN_INPUT].getPolyVoltage(voice) / 10.f;
			sustain_level[voice] = clamp(sustain_level[voice], 0.f, 10.f);

			// Delay
			delay_stage_time[voice] = params[KNOB_DELAY_PARAM].getValue();
			if (inputs[JACK_DELAY_INPUT].isConnected())
				delay_stage_time[voice] += inputs[JACK_DELAY_INPUT].getPolyVoltage(voice) / 10.f;
			delay_stage_time[voice] = rescaleBigKnobs(delay_stage_time[voice]);
			delay_stage_time[voice] = clamp(delay_stage_time[voice], 0.f, 10.f);


			// Release
			release_stage_time[voice] = params[KNOB_RELEASE_PARAM].getValue();
			if (inputs[JACK_RELEASE_INPUT].isConnected())
				release_stage_time[voice] += inputs[JACK_RELEASE_INPUT].getPolyVoltage(voice) / 10.f;
			release_stage_time[voice] = rescaleBigKnobs(release_stage_time[voice]);
			release_stage_time[voice] = clamp(release_stage_time[voice], 0.001f, 10.f);
			release_stage_slope[voice] = params[KNOB_RELEASESLOPE_PARAM].getValue();
			release_stage_slope[voice] = rescaleTinyKnobs(release_stage_slope[voice]);

			// Gate
			gate[voice].process(inputs[JACK_GATE_INPUT].getPolyVoltage(voice));

			// Retrigger
			retrigger[voice].process(inputs[JACK_RETRIGGER_INPUT].getPolyVoltage(voice));

			if (retrigger[voice].isHigh()) {
				if (gate[voice].isHigh()) {
					completed[voice] = 0.f;
					stage[voice] = START_STAGE;
					stagetime[voice] = 0.f;
					if (started[voice] == 0.f) {
						started[voice] = 1.f;
					}
				}
			} else {
				if (gate[voice].isHigh()) {
					if (completed[voice] == 1.f) {
						if (started[voice] == 0.f) {
							started[voice] = 1.f;
							completed[voice] = 0.f;
							stage[voice] = START_STAGE;
							stagetime[voice] = 0.f;
						}
					}
				} else {
					if (started[voice] == 1.f) {
						started[voice] = 0.f;
						if (!(stage[voice] == RELEASE_STAGE)) {
							if (!(stage[voice] == STOP_STAGE)) {
								stage[voice] = RELEASE_STAGE;
								stagetime[voice] = 0.f;
								lastenv[voice] = env[voice];
							}
						}
					}
				}
			}


			switch(stage[voice]) {
				case STOP_STAGE:
					break;
				case START_STAGE: {
						stagetime[voice] += args.sampleTime;
						if (stagetime[voice] >= start_stage_time[voice]) {
							stagetime[voice] = 0.f;
							stage[voice] = ATTACK_STAGE;
						}
						startled_brightness += 1.f / voices;
						lights[LED_START_LIGHT].setBrightness(startled_brightness);
						break;
					}
				case ATTACK_STAGE: {
						stagetime[voice] += args.sampleTime;
						env[voice] = target_level[voice] * std::pow(stagetime[voice] / attack_stage_time[voice], attack_stage_slope[voice]);
						lastenv[voice] = env[voice];
						if (stagetime[voice] >= attack_stage_time[voice]) {
							stagetime[voice] = 0.f;
							stage[voice] = HOLD_STAGE;
						}
						attackled_brightness += 1.f / voices;
						lights[LED_ATTACK_LIGHT].setBrightness(attackled_brightness);
						break;
					}
				case HOLD_STAGE: {
						stagetime[voice] += args.sampleTime;
						if (stagetime[voice] >= hold_stage_time[voice]) {
							stagetime[voice] = 0.f;
							stage[voice] = DECAY_STAGE;
						}
						holdled_brightness += 1.f / voices;
						lights[LED_HOLD_LIGHT].setBrightness(holdled_brightness);
						break;
					}
				case DECAY_STAGE: {
						stagetime[voice] += args.sampleTime;
						env[voice] = target_level[voice] - (target_level[voice] - sustain_level[voice]) * std::pow(stagetime[voice] / decay_stage_time[voice], decay_stage_slope[voice]);
						lastenv[voice] = env[voice];
						if (stagetime[voice] >= decay_stage_time[voice]) {
							stagetime[voice] = 0.f;
							stage[voice] = DELAY_STAGE;
						}
						decayled_brightness += 1.f / voices;
						lights[LED_DECAY_LIGHT].setBrightness(decayled_brightness);
						break;
					}
				case DELAY_STAGE: {
						stagetime[voice] += args.sampleTime;
						if (stagetime[voice] >= delay_stage_time[voice]) {
							stagetime[voice] = 0.f;
							stage[voice] = RELEASE_STAGE;
						}
						delayled_brightness += 1.f / voices;
						lights[LED_DELAY_LIGHT].setBrightness(delayled_brightness);
						break;
					}
				case RELEASE_STAGE: {
						completed[voice] = 1.f;
						stagetime[voice] += args.sampleTime;
						env[voice] = lastenv[voice] - lastenv[voice] * std::pow(stagetime[voice] / release_stage_time[voice], release_stage_slope[voice]);
						if (stagetime[voice] >= release_stage_time[voice]) {
							stagetime[voice] = 0.f;
							stage[voice] = STOP_STAGE;
						}
						releaseled_brightness += 1.f / voices;
						lights[LED_RELEASE_LIGHT].setBrightness(releaseled_brightness);
						break;
					}
			}

			env[voice] = clamp(env[voice], 0.f, 10.f);

			// Set output
			outputs[JACK_ENVELOPE_OUTPUT].setVoltage(env[voice], voice);
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

	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(34.4,  12.0)), module, Envelope_1::KNOB_START_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(31.0,  26.0)), module, Envelope_1::KNOB_ATTACK_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(20.0,  40.0)), module, Envelope_1::KNOB_TARGET_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(34.4,  50.0)), module, Envelope_1::KNOB_HOLD_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(31.0,  64.0)), module, Envelope_1::KNOB_DECAY_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(20.0,  78.0)), module, Envelope_1::KNOB_SUSTAIN_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(34.4,  88.0)), module, Envelope_1::KNOB_DELAY_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(31.0, 102.0)), module, Envelope_1::KNOB_RELEASE_PARAM));

	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(17.0,  26.0)), module, Envelope_1::KNOB_ATTACKSLOPE_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(17.0,  64.0)), module, Envelope_1::KNOB_DECAYSLOPE_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(17.0, 102.0)), module, Envelope_1::KNOB_RELEASESLOPE_PARAM));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.0,  12.0)), module, Envelope_1::JACK_START_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.0,  26.0)), module, Envelope_1::JACK_ATTACK_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.0,  40.0)), module, Envelope_1::JACK_TARGET_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.0,  50.0)), module, Envelope_1::JACK_HOLD_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.0,  64.0)), module, Envelope_1::JACK_DECAY_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.0,  78.0)), module, Envelope_1::JACK_SUSTAIN_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.0,  88.0)), module, Envelope_1::JACK_DELAY_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.0, 102.0)), module, Envelope_1::JACK_RELEASE_INPUT));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec( 8.72, 114.5)), module, Envelope_1::JACK_GATE_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.32, 114.5)), module, Envelope_1::JACK_RETRIGGER_INPUT));

	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(31.923, 114.5)), module, Envelope_1::JACK_ENVELOPE_OUTPUT));

	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(34.4,   6.0)), module, Envelope_1::LED_START_LIGHT));
	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(31.0,  20.0)), module, Envelope_1::LED_ATTACK_LIGHT));
	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(34.4,  44.0)), module, Envelope_1::LED_HOLD_LIGHT));
	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(31.0,  58.0)), module, Envelope_1::LED_DECAY_LIGHT));
	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(34.4,  82.0)), module, Envelope_1::LED_DELAY_LIGHT));
	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(31.0,  96.0)), module, Envelope_1::LED_RELEASE_LIGHT));
}

SmallEnvelope_1Widget::SmallEnvelope_1Widget(Envelope_1* module) {
	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/ADSR.svg")));

	addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(9.0, 12.0)), module, Envelope_1::KNOB_ATTACK_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(9.0, 34.0)), module, Envelope_1::KNOB_DECAY_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(9.0, 56.0)), module, Envelope_1::KNOB_SUSTAIN_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(9.0, 78.0)), module, Envelope_1::KNOB_RELEASE_PARAM));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.0, 22.0)), module, Envelope_1::JACK_ATTACK_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.0, 44.0)), module, Envelope_1::JACK_DECAY_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.0, 66.0)), module, Envelope_1::JACK_SUSTAIN_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.0, 88.0)), module, Envelope_1::JACK_RELEASE_INPUT));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 101)), module, Envelope_1::JACK_GATE_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 110)), module, Envelope_1::JACK_RETRIGGER_INPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 119)), module, Envelope_1::JACK_ENVELOPE_OUTPUT));

	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(9.0,  6.015)), module, Envelope_1::LED_ATTACK_LIGHT));
	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(9.0, 28.015)), module, Envelope_1::LED_DECAY_LIGHT));
	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(9.0, 50.015)), module, Envelope_1::LED_DELAY_LIGHT));
	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(9.0, 72.015)), module, Envelope_1::LED_RELEASE_LIGHT));
}

// Quadratic rescale seconds with milliseconds
float Envelope_1::rescaleBigKnobs(float x) {
	if (x >= 0.f && x < 0.5f) x = math::rescale(x, 0.001f, .5f, 0.001f, 1.f);
	else if (x >= 0.5f) x = math::rescale(x, .5f, 1.f, 1.f, 10.f);

	int xx = (int)(x * 1000);
	x = xx * .001f;
	return x;
}

float Envelope_1::rescaleTinyKnobs(float x) {
	if (x >= .0f && x < .5f) x = math::rescale(x, .0f, .5f, .1f, 1.f);
	else if (x >= .5f) x = math::rescale(x, .5f, 1.f, 1.f, 3.f);

	return x;
}

float EnvelopeKnobParamQuantity::getDisplayValue() {
	float x = getValue();
	if (!module) {
		return x;
	}
	//int vv = (int)(v * v * v * 1000);
	//v = vv * .001f;
	if (x >= 0.f && x < 0.5f) x = math::rescale(x, 0.001f, .5f, 0.001f, 1.f);
	else if (x >= 0.5f) x = math::rescale(x, .5f, 1.f, 1.f, 10.f);

	int xx = (int)(x * 1000);
	x = xx * .001f;
	x += displayOffset;
	if (x < 0.0f) {
		return -x;
	}
	return x;
}




Model* modelEnvelope_1 = createModel<Envelope_1, Envelope_1Widget>("Envelope-1");
Model* modelSmallEnvelope_1 = createModel<Envelope_1, SmallEnvelope_1Widget>("ADSR");



