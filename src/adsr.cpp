#include "adsr.hpp"

Model* modelADSR = createModel<ADSR, ADSRWidget>("ADSR");

ADSR::ADSR() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam<EnvelopeKnobParamQuantity>(KNOB_ATTACK_PARAM,  0.001f, 1.f, 0.2f, "Attack",  " s");
	configParam<EnvelopeKnobParamQuantity>(KNOB_DECAY_PARAM,   0.001f, 1.f, 0.2f, "Decay",   " s");
	configParam<EnvelopeKnobParamQuantity>(KNOB_RELEASE_PARAM, 0.001f, 1.f, 0.2f, "Release", " s");

	configParam(KNOB_SUSTAIN_PARAM, 0.f, 10.f, 5.f,  "Sustain level", "%", 0, 10);
}

ADSRWidget::ADSRWidget(ADSR* module) {
	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/ADSR.svg")));

	addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(9.0, 12.0)), module, ADSR::KNOB_ATTACK_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(9.0, 34.0)), module, ADSR::KNOB_DECAY_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(9.0, 56.0)), module, ADSR::KNOB_SUSTAIN_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhite>(mm2px(Vec(9.0, 78.0)), module, ADSR::KNOB_RELEASE_PARAM));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.0, 22.0)), module, ADSR::JACK_ATTACK_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.0, 44.0)), module, ADSR::JACK_DECAY_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.0, 66.0)), module, ADSR::JACK_SUSTAIN_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(5.0, 88.0)), module, ADSR::JACK_RELEASE_INPUT));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 101)), module, ADSR::JACK_GATE_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 110)), module, ADSR::JACK_RETRIGGER_INPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 119)), module, ADSR::JACK_ENVELOPE_OUTPUT));

	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(9.0,  6.015)), module, ADSR::LED_ATTACK_LIGHT));
	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(9.0, 28.015)), module, ADSR::LED_DECAY_LIGHT));
	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(9.0, 50.015)), module, ADSR::LED_SUSTAIN_LIGHT));
	addChild(createLightCentered<TinyLight<BlueLight>>(mm2px(Vec(9.0, 72.015)), module, ADSR::LED_RELEASE_LIGHT));
}

void ADSR::process(const ProcessArgs& args) {
	if (inputs[JACK_GATE_INPUT].isConnected()) {
		int voices = inputs[JACK_GATE_INPUT].getChannels();
		outputs[JACK_ENVELOPE_OUTPUT].setChannels(voices);

		float attackled_brightness = 0.f;
		float decayled_brightness = 0.f;
		float sustainled_brightness = 0.f;
		float releaseled_brightness = 0.f;

		lights[LED_ATTACK_LIGHT].setBrightness(0);
		lights[LED_DECAY_LIGHT].setBrightness(0);
		lights[LED_SUSTAIN_LIGHT].setBrightness(0);
		lights[LED_RELEASE_LIGHT].setBrightness(0);

		for (int voice = 0; voice < voices; voice += 4) {

			// Attack
			attack_stage_time[voice] = params[KNOB_ATTACK_PARAM].getValue();
			if (inputs[JACK_ATTACK_INPUT].isConnected())
				attack_stage_time[voice] *= inputs[JACK_ATTACK_INPUT].getPolyVoltage(voice) / 10.f;
			attack_stage_time[voice] = rescaleBigKnobs(attack_stage_time[voice]);
			attack_stage_time[voice] = clamp(attack_stage_time[voice], 0.001f, 10.f);

			// Decay
			decay_stage_time[voice] = params[KNOB_DECAY_PARAM].getValue();
			if (inputs[JACK_DECAY_INPUT].isConnected())
				decay_stage_time[voice] += inputs[JACK_DECAY_INPUT].getPolyVoltage(voice) / 10.f;
			decay_stage_time[voice] = rescaleBigKnobs(decay_stage_time[voice]);
			decay_stage_time[voice] = clamp(decay_stage_time[voice], 0.001f, 10.f);

			// Sustain
			sustain_level[voice] = params[KNOB_SUSTAIN_PARAM].getValue();
			if (inputs[JACK_SUSTAIN_INPUT].isConnected())
				sustain_level[voice] += inputs[JACK_SUSTAIN_INPUT].getPolyVoltage(voice) / 10.f;
			sustain_level[voice] = clamp(sustain_level[voice], 0.f, 10.f);

			// Release
			release_stage_time[voice] = params[KNOB_RELEASE_PARAM].getValue();
			if (inputs[JACK_RELEASE_INPUT].isConnected())
				release_stage_time[voice] += inputs[JACK_RELEASE_INPUT].getPolyVoltage(voice) / 10.f;
			release_stage_time[voice] = rescaleBigKnobs(release_stage_time[voice]);
			release_stage_time[voice] = clamp(release_stage_time[voice], 0.001f, 10.f);

			// Gate
			gate[voice].process(inputs[JACK_GATE_INPUT].getPolyVoltage(voice));

			// Retrigger
			retrigger[voice].process(inputs[JACK_RETRIGGER_INPUT].getPolyVoltage(voice));

			if (retrigger[voice].isHigh()) {
				if (gate[voice].isHigh()) {
					completed[voice] = 0.f;
					stage[voice] = ATTACK_STAGE;
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
							stage[voice] = ATTACK_STAGE;
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
				case ATTACK_STAGE: {
						stagetime[voice] += args.sampleTime;
						//env[voice] = target_level[voice] * std::pow(stagetime[voice] / attack_stage_time[voice], attack_stage_slope[voice]);
						env[voice] = 10.f * stagetime[voice] / attack_stage_time[voice];
						lastenv[voice] = env[voice];
						if (stagetime[voice] >= attack_stage_time[voice]) {
							stagetime[voice] = 0.f;
							stage[voice] = DECAY_STAGE;
						}
						attackled_brightness += 1.f / voices;
						lights[LED_ATTACK_LIGHT].setBrightness(attackled_brightness);
						break;
					}
				case DECAY_STAGE: {
						stagetime[voice] += args.sampleTime;
						//env[voice] = target_level[voice] - (target_level[voice] - sustain_level[voice]) * std::pow(stagetime[voice] / decay_stage_time[voice], decay_stage_slope[voice]);
						env[voice] = 10.f - (10.f - sustain_level[voice]) * curveSlope(stagetime[voice] / decay_stage_time[voice], 50.f);
						lastenv[voice] = env[voice];
						if (stagetime[voice] >= decay_stage_time[voice]) {
							stagetime[voice] = 0.f;
							stage[voice] = SUSTAIN_STAGE;
						}
						decayled_brightness += 1.f / voices;
						lights[LED_DECAY_LIGHT].setBrightness(decayled_brightness);
						break;
					}
				case SUSTAIN_STAGE: {
						sustainled_brightness += 1.f / voices;
						lights[LED_SUSTAIN_LIGHT].setBrightness(sustainled_brightness);
						break;
					}
				case RELEASE_STAGE: {
						completed[voice] = 1.f;
						stagetime[voice] += args.sampleTime;
						//env[voice] = lastenv[voice] - lastenv[voice] * std::pow(stagetime[voice] / release_stage_time[voice], release_stage_slope[voice]);
						env[voice] = lastenv[voice] - lastenv[voice] * curveSlope(stagetime[voice] / release_stage_time[voice], 50.f);
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


float ADSR::curveSlope(float x, float y) {
	return logf(x * (y - 1) + 1) / logf(y);
}

// Quadratic rescale seconds with milliseconds
float ADSR::rescaleBigKnobs(float x) {
	if (x >= 0.f && x < 0.5f) x = math::rescale(x, 0.001f, .5f, 0.001f, 1.f);
	else if (x >= 0.5f) x = math::rescale(x, .5f, 1.f, 1.f, 10.f);

	int xx = (int)(x * 1000);
	x = xx * .001f;
	return x;
}

float ADSR::rescaleTinyKnobs(float x) {
	if (x >= .0f && x < .5f) x = math::rescale(x, .0f, .5f, .1f, 1.f);
	else if (x >= .5f) x = math::rescale(x, .5f, 1.f, 1.f, 3.f);

	return x;
}

float ADSR::EnvelopeKnobParamQuantity::getDisplayValue() {
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



