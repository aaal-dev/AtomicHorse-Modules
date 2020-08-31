#include "lfo-1.hpp"

LFO_1::LFO_1 () {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	configParam<LFOFrequencyParamQuantity>(KNOB_FREQUENCY_PARAM, 0.f, 10.f, 0.f, "Frequency", " Hz");
	configParam(KNOB_FM1_PARAM, 0.f, 1.f, 0.f, "Main frequency modulation", " %", 0.f, 100.f);
	configParam(KNOB_FM2_PARAM, 0.f, 1.f, 0.f, "Sub frequency modulation", " %", 0.f, 100.f);

	configParam(KNOB_PULSEWIDTH_PARAM, 0.99f, 0.01f, 0.5f, "Pulse width", "%", 0.f, 100.f);
	configParam(KNOB_PWM_PARAM, 0.f, 1.f, 0.f, "Modulation depth", "%", 0.f, 100.f);

	configParam<LFOWaveFormParamQuantity>(KNOB_WAVEFORM1_PARAM, 0.f, 3.f, 0.f, "Wave form 1", "");
	configParam<LFOWaveFormParamQuantity>(KNOB_WAVEFORM2_PARAM, 0.f, 3.f, 1.f, "Wave form 2", "");
	configParam<LFOWaveFormParamQuantity>(KNOB_WAVEFORM3_PARAM, 0.f, 3.f, 2.f, "Wave form 3", "");
	configParam<LFOWaveFormParamQuantity>(KNOB_WAVEFORM4_PARAM, 0.f, 3.f, 3.f, "Wave form 4", "");

	configParam(KNOB_WAVESUBFREQ1_PARAM, -2.f, 2.f, 0.f, "Sub frequency", " Hz");
	configParam(KNOB_WAVESUBFREQ2_PARAM, -2.f, 2.f, 0.f, "Sub frequency", " Hz");
	configParam(KNOB_WAVESUBFREQ3_PARAM, -2.f, 2.f, 0.f, "Sub frequency", " Hz");
	configParam(KNOB_WAVESUBFREQ4_PARAM, -2.f, 2.f, 0.f, "Sub frequency", " Hz");

	configParam(KNOB_WAVEOUTLEVEL1_PARAM, 0.f, 1.f, 1.f, "Level", " %", 0.f, 100.f);
	configParam(KNOB_WAVEOUTLEVEL2_PARAM, 0.f, 1.f, 1.f, "Level", " %", 0.f, 100.f);
	configParam(KNOB_WAVEOUTLEVEL3_PARAM, 0.f, 1.f, 1.f, "Level", " %", 0.f, 100.f);
	configParam(KNOB_WAVEOUTLEVEL4_PARAM, 0.f, 1.f, 1.f, "Level", " %", 0.f, 100.f);

	configParam<SwitchVoltageModeParamQuantity>(SWITCH_VOLTAGEMODE_PARAM, 0.f, 1.f, 0.f, "Voltage mode");
	configParam<SwitchPhaseParamQuantity>(SWITCH_PHASE_PARAM, 0.f, 1.f, 1.f, "Phase");
}

LFO_1Widget::LFO_1Widget(LFO_1* module) {
	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/LFO-1.svg")));

	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	addParam(createParamCentered<CKSS>(mm2px(Vec(6.820, 68.427)), module, LFO_1::SWITCH_VOLTAGEMODE_PARAM));
	addParam(createParamCentered<CKSS>(mm2px(Vec(33.82, 68.427)), module, LFO_1::SWITCH_PHASE_PARAM));

	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(6.820, 13.0)), module, LFO_1::KNOB_FM1_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(33.82, 13.0)), module, LFO_1::KNOB_FM2_PARAM));
	addParam(createParamCentered<LFOFrequencyKnob>(mm2px(Vec(20.32, 23.0)), module, LFO_1::KNOB_FREQUENCY_PARAM));

	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(6.82, 48.1)), module, LFO_1::KNOB_PWM_PARAM));
	addParam(createParamCentered<AHMRoundKnobWhiteLarge>(mm2px(Vec(20.32, 48.1)), module, LFO_1::KNOB_PULSEWIDTH_PARAM));

	addParam(createParamCentered<LFOWaveformRotaryTumbler>(mm2px(Vec(6.820, 86.627)), module, LFO_1::KNOB_WAVEFORM1_PARAM));
	addParam(createParamCentered<LFOWaveformRotaryTumbler>(mm2px(Vec(15.82, 86.627)), module, LFO_1::KNOB_WAVEFORM2_PARAM));
	addParam(createParamCentered<LFOWaveformRotaryTumbler>(mm2px(Vec(24.82, 86.627)), module, LFO_1::KNOB_WAVEFORM3_PARAM));
	addParam(createParamCentered<LFOWaveformRotaryTumbler>(mm2px(Vec(33.82, 86.627)), module, LFO_1::KNOB_WAVEFORM4_PARAM));

	addParam(createParamCentered<AHMRoundKnob3WhiteTiny>(mm2px(Vec(6.820, 95.0)), module, LFO_1::KNOB_WAVESUBFREQ1_PARAM));
	addParam(createParamCentered<AHMRoundKnob3WhiteTiny>(mm2px(Vec(15.82, 95.0)), module, LFO_1::KNOB_WAVESUBFREQ2_PARAM));
	addParam(createParamCentered<AHMRoundKnob3WhiteTiny>(mm2px(Vec(24.82, 95.0)), module, LFO_1::KNOB_WAVESUBFREQ3_PARAM));
	addParam(createParamCentered<AHMRoundKnob3WhiteTiny>(mm2px(Vec(33.82, 95.0)), module, LFO_1::KNOB_WAVESUBFREQ4_PARAM));

	addParam(createParamCentered<AHMRoundKnob3WhiteTiny>(mm2px(Vec(6.820, 103.574)), module, LFO_1::KNOB_WAVEOUTLEVEL1_PARAM));
	addParam(createParamCentered<AHMRoundKnob3WhiteTiny>(mm2px(Vec(15.82, 103.574)), module, LFO_1::KNOB_WAVEOUTLEVEL2_PARAM));
	addParam(createParamCentered<AHMRoundKnob3WhiteTiny>(mm2px(Vec(24.82, 103.574)), module, LFO_1::KNOB_WAVEOUTLEVEL3_PARAM));
	addParam(createParamCentered<AHMRoundKnob3WhiteTiny>(mm2px(Vec(33.82, 103.574)), module, LFO_1::KNOB_WAVEOUTLEVEL4_PARAM));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.820, 33.000)), module, LFO_1::JACK_FM1_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 33.000)), module, LFO_1::JACK_FM2_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 48.100)), module, LFO_1::JACK_PWM_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.82, 71.954)), module, LFO_1::JACK_SYNC_INPUT));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.82, 71.954)), module, LFO_1::JACK_RESET_INPUT));

	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.82, 112.573)), module, LFO_1::JACK_OUTPUT1_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.82, 112.573)), module, LFO_1::JACK_OUTPUT2_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(24.82, 112.573)), module, LFO_1::JACK_OUTPUT3_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.82, 112.573)), module, LFO_1::JACK_OUTPUT4_OUTPUT));

	addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(6.820, 23.0)), module, LFO_1::LED_FREQUENCY_LIGHT));
	addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.320, 108.073)), module, LFO_1::LED_WAVE1_LIGHT));
	addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(11.32, 108.073)), module, LFO_1::LED_WAVE2_LIGHT));
	addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(29.32, 108.073)), module, LFO_1::LED_WAVE3_LIGHT));
	addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(38.32, 108.073)), module, LFO_1::LED_WAVE4_LIGHT));

}

Model* modelLFO_1 = createModel<LFO_1, LFO_1Widget>("LFO-1");


// Additional stuff

LFOFrequencyKnob::LFOFrequencyKnob() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/LFO_FreqKnob.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.8 * M_PI;
	maxAngle = 0.8 * M_PI;
}

LFOWaveformRotaryTumbler::LFOWaveformRotaryTumbler() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/LFO_WaveformRotaryTumbler.svg")));
	minAngle = -0.3 * M_PI;
	maxAngle = 0.3 * M_PI;
	snap = true;
	shadow->opacity = 0.f;
}

float LFOFrequencyParamQuantity::getDisplayValue() {
	float v = getValue();
	if (!module) {
		return v;
	}
	v = powf(2.0f, v);
	int vv = v * 1000;
	v = vv * .001f;
	return v;
}

std::string LFOWaveFormParamQuantity::getDisplayValueString() {
	int value = getValue();
	std::string text;
	if (!module) {
		return "";
	}
	switch (value) {
		case 0: {
				text = "sine";
				break;
			}
		case 1: {
				text = "triangle";
				break;
			}
		case 2: {
				text = "saw";
				break;
			}
		case 3: {
				text = "square";
				break;
			}
	}
	return text;

}

std::string SwitchVoltageModeParamQuantity::getDisplayValueString() {
	int value = getValue();
	std::string text;
	if (!module) {
		return "";
	}
	switch (value) {
		case 0: {
				text = "unipolar";
				break;
			}
		case 1: {
				text = "bipolar";
				break;
			}
	}
	return text;
}

std::string SwitchPhaseParamQuantity::getDisplayValueString() {
	int value = getValue();
	std::string text;
	if (!module) {
		return "";
	}
	switch (value) {
		case 0: {
				text = "180°";
				break;
			}
		case 1: {
				text = "0°";
				break;
			}
	}
	return text;
}
