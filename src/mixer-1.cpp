#include "mixer-1.hpp"


Mixer_1::Mixer_1() {
	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	for (int i = 0; i < CHANNELS_NUMBER; i++) {
		configParam(SLIDER_LEVEL_PARAM + i, 0.f, 1.3f, 1.f, "Level of signal " + std::to_string(i), "%", 0.f, 100.f);
		configParam(KNOB_PAN_PARAM + i, -1.f, 1.f, 0.f, "Panning of signal " + std::to_string(i), "%", 0.f, 100.f);
	}
	configParam(KNOB_LEVELMAIN_PARAM, 0.f, 1.3f, 1.f, "Level of mixed signal", "%", 0.f, 100.f);
}

void Mixer_1::process(const ProcessArgs& args) {
	int maxChannels = 1;
	for (int i = 0; i < CHANNELS_NUMBER; i++) {
		if (inputs[JACK_IN_L_INPUT + i].isConnected()) {
			channels_l[i] = inputs[JACK_IN_L_INPUT + i].getChannels();
			maxChannels = std::max(maxChannels, channels_l[i]);
			for (int channel = 0; channel < channels_l[i]; channel++) {
				level_l_ParamValue[i][channel] = inputs[JACK_IN_L_INPUT + i].getPolyVoltage(channel);
				level_l_ParamValue[i][channel] *= params[SLIDER_LEVEL_PARAM + i].getValue();
				if (inputs[JACK_CV_INPUT + i].isConnected())
					level_l_ParamValue[i][channel] *= clamp(inputs[JACK_CV_INPUT + i].getPolyVoltage(channel) / 10.f, 0.f, 1.f);
				mix_l_ParamValue[channel] += level_l_ParamValue[i][channel];
			}
		}
		if (inputs[JACK_IN_R_INPUT + i].isConnected()) {
			channels_r[i] = inputs[JACK_IN_R_INPUT + i].getChannels();
			maxChannels = std::max(maxChannels, channels_r[i]);
			for (int channel = 0; channel < channels_r[i]; channel++) {
				level_r_ParamValue[i][channel] = inputs[JACK_IN_R_INPUT + i].getPolyVoltage(channel);
				level_r_ParamValue[i][channel] *= params[SLIDER_LEVEL_PARAM + i].getValue();
				if (inputs[JACK_CV_INPUT + i].isConnected())
					level_r_ParamValue[i][channel] *= clamp(inputs[JACK_CV_INPUT + i].getPolyVoltage(channel) / 10.f, 0.f, 1.f);
				mix_r_ParamValue[channel] += level_r_ParamValue[i][channel];
			}
		}

	}

	if (inputs[JACK_OUTMAIN_L_OUTPUT].isConnected()) {
		for (int channel = 0; channel < maxChannels; channel++) {
			mix_l_ParamValue[channel] *= params[KNOB_LEVELMAIN_PARAM].getValue();
			if (inputs[JACK_CVMAIN_INPUT].isConnected()) {
				float mainCV = inputs[JACK_CVMAIN_INPUT].getPolyVoltage(channel) / 10.f;
				mainCV *= params[KNOB_CVMAIN_PARAM].getValue();
				mix_l_ParamValue[channel] += mainCV;
			}
		}
		outputs[JACK_OUTMAIN_L_OUTPUT].setChannels(maxChannels);
		outputs[JACK_OUTMAIN_L_OUTPUT].writeVoltages(mix_l_ParamValue);
	}

	if (inputs[JACK_OUTMAIN_R_OUTPUT].isConnected()) {
		for (int channel = 0; channel < maxChannels; channel++) {
			mix_r_ParamValue[channel] *= params[KNOB_LEVELMAIN_PARAM].getValue();
			if (inputs[JACK_CVMAIN_INPUT].isConnected()) {
				float mainCV = inputs[JACK_CVMAIN_INPUT].getPolyVoltage(channel) / 10.f;
				mainCV *= params[KNOB_CVMAIN_PARAM].getValue();
				mix_r_ParamValue[channel] += mainCV;
			}
		}
		outputs[JACK_OUTMAIN_L_OUTPUT].setChannels(maxChannels);
		outputs[JACK_OUTMAIN_L_OUTPUT].writeVoltages(mix_r_ParamValue);
	}



}

Mixer_1Widget::Mixer_1Widget(Mixer_1* module) {
	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/panels/mixer-1.svg")));

	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	MixerVuMeter* vuMeter1L = createWidget<MixerVuMeter>(mm2px(Vec(3.0, 29.464)));
	vuMeter1L->module = module;
	addChild(vuMeter1L);

	addParam(createParamCentered<MixerLevel>(mm2px(Vec(6.820, 46.464)), module, Mixer_1::SLIDER_LEVEL_PARAM + 0));
	addParam(createParamCentered<MixerLevel>(mm2px(Vec(15.82, 46.464)), module, Mixer_1::SLIDER_LEVEL_PARAM + 1));
	addParam(createParamCentered<MixerLevel>(mm2px(Vec(24.82, 46.464)), module, Mixer_1::SLIDER_LEVEL_PARAM + 2));
	addParam(createParamCentered<MixerLevel>(mm2px(Vec(33.82, 46.464)), module, Mixer_1::SLIDER_LEVEL_PARAM + 3));

	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(6.820, 85.312)), module, Mixer_1::KNOB_PAN_PARAM + 0));
	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(15.82, 85.312)), module, Mixer_1::KNOB_PAN_PARAM + 1));
	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(24.82, 85.312)), module, Mixer_1::KNOB_PAN_PARAM + 2));
	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(33.82, 85.312)), module, Mixer_1::KNOB_PAN_PARAM + 3));

	addParam(createParamCentered<AHMRoundKnobWhiteTiny>(mm2px(Vec(6.820, 106.566)), module, Mixer_1::KNOB_CVMAIN_PARAM));
	addParam(createParamCentered<MixerMainLevelKnob>(mm2px(Vec(20.32, 111.093)), module, Mixer_1::KNOB_LEVELMAIN_PARAM));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.820, 13.0)), module, Mixer_1::JACK_IN_L_INPUT + 0));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.82, 13.0)), module, Mixer_1::JACK_IN_L_INPUT + 1));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.82, 13.0)), module, Mixer_1::JACK_IN_L_INPUT + 2));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 13.0)), module, Mixer_1::JACK_IN_L_INPUT + 3));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.820, 22.0)), module, Mixer_1::JACK_IN_R_INPUT + 0));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.82, 22.0)), module, Mixer_1::JACK_IN_R_INPUT + 1));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.82, 22.0)), module, Mixer_1::JACK_IN_R_INPUT + 2));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 22.0)), module, Mixer_1::JACK_IN_R_INPUT + 3));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.820, 70.31)), module, Mixer_1::JACK_CV_INPUT + 0));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.82, 70.31)), module, Mixer_1::JACK_CV_INPUT + 1));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.82, 70.31)), module, Mixer_1::JACK_CV_INPUT + 2));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 70.31)), module, Mixer_1::JACK_CV_INPUT + 3));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.820, 93.63)), module, Mixer_1::JACK_PAN_INPUT + 0));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.82, 93.63)), module, Mixer_1::JACK_PAN_INPUT + 1));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(24.82, 93.63)), module, Mixer_1::JACK_PAN_INPUT + 2));
	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(33.82, 93.63)), module, Mixer_1::JACK_PAN_INPUT + 3));

	addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.82, 115.566)), module, Mixer_1::JACK_CVMAIN_INPUT));

	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.82, 106.566)), module, Mixer_1::JACK_OUTMAIN_L_OUTPUT));
	addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(33.82, 115.566)), module, Mixer_1::JACK_OUTMAIN_R_OUTPUT));
}

Model* modelMixer_1 = createModel<Mixer_1, Mixer_1Widget>("Mixer-1");

MixerMainLevelKnob::MixerMainLevelKnob() {
	setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_MaimLevelKnob.svg")));
	shadow->box.pos = Vec(0.0, 2.5);
	minAngle = -0.5 * M_PI;
	maxAngle = 0.8 * M_PI;
}

MixerLevel::MixerLevel() {
	setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_SliderPot.svg")));
	setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Mixer_SliderHandle.svg")));
	maxHandlePos = mm2px(Vec(-.5, 0));
	minHandlePos = mm2px(Vec(-.5, 30));
	//background->box.pos = margin;
	//box.size = background->box.size.plus(margin.mult(2));
}

MixerVuMeter::MixerVuMeter() {
	box.size = mm2px(Vec(1.5, 34));
}

void MixerVuMeter::draw(const DrawArgs& args) {
	float y = module->params[Mixer_1::SLIDER_LEVEL_PARAM + 0].getValue();
	nvgStrokeColor(args.vg, nvgRGBA(0xff, 0xff, 0xff, 0x10));
	{
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, box.pos.x, box.pos.y);
		nvgLineTo(args.vg, box.pos.x, box.pos.y - box.size.y * y);
		nvgClosePath(args.vg);
	}
	nvgStrokeWidth(args.vg, 3.f);
	nvgStroke(args.vg);
}
